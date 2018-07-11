/*
-------------------------------------------------------------------------
OBJECT NAME:	Probe.cc

FULL NAME:	Probe Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "Probe.h"
#include "ControlWindow.h"

const float	Probe::diodeDiameter = 0.2;

const unsigned char Probe::BlankSlice[] =
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };


/* -------------------------------------------------------------------- */
Probe::Probe(ProbeType type, const char xml_entry[], int recSize, size_t ndiodes) : _type(type), _nDiodes(ndiodes)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

  init();
}

/* -------------------------------------------------------------------- */
Probe::Probe(ProbeType type, const char name[], size_t ndiodes) : _type(type), _nDiodes(ndiodes)
{
  _name.push_back(name[0]);
  _name.push_back(name[1]);
  _name.push_back('\0');
  strcpy(_code, _name.c_str());

  _lrLen = 4116;

  if (_name[0] == 'C')
  {
    _resolution = 25;
    _armWidth = 61.0;
  }

  if (_name[0] == 'P')
  {
    _resolution = 200;
    _armWidth = 261.0;
  }

  init();
}

/* -------------------------------------------------------------------- */
Probe::Probe(ProbeType type, Header * hdr, const Pms2 * p, int cnt, size_t ndiodes) : _type(type), _nDiodes(ndiodes)
{
  // Extract stuff from Header.
  _name = hdr->VariableName(p);
  _name += "_";
  _name += hdr->AircraftLocation(p);
  _serialNumber = hdr->SerialNumber(p);

  _code[0] = _name[3]; _code[1] = cnt + '0'; _code[2] = '\0';

  _resolution = hdr->Resolution(p);

  init();

  _lrLen = hdr->lrLength(p);
  _lrPpr = hdr->lrPpr(p);
}

void Probe::init()
{
  sampleArea = 0;
  _nSlices = P2D_DATA / _nDiodes * 8;
  _eaw = nDiodes() * Resolution() * 0.001;
  _dof_const = 2.37;	// Default for PMS probes.

  _lrPpr = 1;

  _displayed = false;
  _prevTime = 0;
  _prevTimeWord = 0;

  cp = new Particle();
  memset((void *)&stats, 0, sizeof(stats));
}


extern ControlWindow *controlWindow;

/* -------------------------------------------------------------------- */
void Probe::ClearStats(const P2d_rec *record)
{
  stats.tBarElapsedtime = 0;
  stats.nTimeBars = 0;
  stats.nonRejectParticles = 0;
  stats.minBar = 10000000;
  stats.maxBar = 0;
  stats.area = 0;
  stats.DOFsampleVolume = 0.0;
  stats.duplicate = false;
  stats.particles.clear();
  stats.tas = (float)record->tas;
  stats.frequency = Resolution() / stats.tas;
  stats.thisTime = (record->hour * 3600 + record->minute * 60 + record->second) * 1000 + record->msec; // in milliseconds
  memset(stats.accum, 0, sizeof(stats.accum));
}

/* -------------------------------------------------------------------- */
void Probe::checkEdgeDiodes(Particle * cp, const unsigned char *p)
{
  /* Potential problem/bug with computing of x1, x2.  Works good if all
   * edge touches are contigious (water), not so good for snow, where
   * it will all get bunched up.  Counts total number of contacts for
   * each edge.
   */
  if ((p[0] & 0x80) == 0)	// touched upper edge
  {
    cp->edge |= 0x0F;
    cp->x1++;
  }

  if ((p[nDiodes()/8] & 0x01) == 0) // touched lower edge
  {
    cp->edge |= 0xF0;
    cp->x2++;
  }
}

/* -------------------------------------------------------------------- */
size_t Probe::area(const unsigned char *p)
{
  size_t area = 0;
  size_t nBytes = nDiodes() / 8;

  for (size_t i = 0; i < nBytes; ++i)
    for (size_t j = 0; j < 8; ++j)
      area += 1 - ((p[i] >> j) & 0x0001);	// data is inverted.

  return area;
}

/* -------------------------------------------------------------------- */
size_t Probe::height(const unsigned char *p)
{
  size_t j, h = nDiodes();
  size_t nBytes = nDiodes() / 8;

  for (size_t i = 0; i < nBytes; ++i)
  {
    for (j = 0; j < 8 && ((p[i] << j) & 0x80); ++j)
      h--;
    if (j < 8) break;
  }

  for (size_t i = 0; i < nBytes; ++i)
  {
    for (j = 0; j < 8 && ((p[7-i] >> j) & 0x01); ++j)
      h--;
    if (j < 8) break;
  }

  return h;
}

/* -------------------------------------------------------------------- */
size_t Probe::checkRejectionCriteria(Particle * cp, recStats & stats)
{
  if (controlWindow->RejectZeroAreaImage() && cp->w == 0 && cp->h == 0)
    cp->reject = true;

  if (cp->h == 1 && cp->w > 3)	// Stuck bit detection.
    cp->reject = true;

  if ((float)cp->area / (std::pow(std::max(cp->w, cp->h), 2.0) * M_PI / 4.0) <= controlWindow->GetAreaRatioReject())
    cp->reject = true;

  size_t bin = 0;
  switch (controlWindow->GetConcentration())
  {
    case BASIC:
      bin = std::max(cp->w, cp->h);
      cp->reject = false;	// Reject nothing.
      break;

    case ENTIRE_IN:
      if (cp->edge)
        cp->reject = true;

      bin = cp->h;
      break;

    case CENTER_IN:
      if (cp->edge && cp->w >= cp->h * 2)
      {
#ifdef DEBUG
        printf("reject no-center rule #%d\n", stats.nTimeBars);
#endif
        cp->reject = true;
      }

      bin = std::max(cp->w, cp->h);
      break;

    case RECONSTRUCTION:
      if (cp->edge && (float)cp->h / cp->w < 0.2)
      {
#ifdef DEBUG
        printf("reject 20%% rule #%d\n", stats.nTimeBars);
#endif
        cp->reject = true;
      }

      if (cp->edge == 0 || (cp->edge && cp->w / cp->h < 0.2))
        bin = std::max(cp->w, cp->h);
      else
      if (cp->edge == 0xf0 || cp->edge == 0x0f) // One edge, but not both.
        bin = (int)((pow(cp->w >> 1, 2.0) + pow(cp->h, 2.0)) / cp->h);
      else
      if (cp->edge == 0xff)
        bin = (int)sqrt(pow(cp->h + ((pow(cp->x2, 2.0) + pow(cp->x1, 2.0))
                                / 4 * cp->h), 2.0) + pow(cp->x1, 2.0));
      break;
    }

  if (!cp->reject && bin < (nDiodes()<<2))
  {
    stats.accum[bin]++;
    stats.area += cp->area;
    stats.nonRejectParticles++;
    return cp->liveTime + cp->deltaTime;
  }
  return 0;
}

/* -------------------------------------------------------------------- */
void Probe::computeDerived(double sampleVolume[], size_t nBins, double totalLiveTime)
{
  double	diameter, z, conc;
  stats.concentration = stats.lwc = stats.dbz = z = 0.0;

  for (size_t i = 1; i < nBins; ++i)
    {
    if (sampleVolume[i] > 0.0)
      {
      conc = stats.accum[i] / (sampleVolume[i] * totalLiveTime);
      stats.DOFsampleVolume += (sampleVolume[i] * totalLiveTime);
      }
    else
      conc = 0.0;

    diameter = i * Resolution();
    stats.lwc += conc * pow(diameter / 10, 3.0);
    z += conc * pow(diameter / 1000, 6.0);

    stats.concentration += conc;
    }

  z /= 1000;
  stats.lwc *= M_PI / 6.0 * 1.0e-6 * controlWindow->GetDensity();

  if (z > 0.0)
    stats.dbz = 10.0 * log10(z * 1.0e6);
  else
    stats.dbz = -100.0;
}


/* -------------------------------------------------------------------- */
void Probe::SetSampleArea()
{
  float diam, DoF;

  float prht	= _armWidth * 1000;	// in micron
  float mag	= diodeDiameter / ((float)Resolution() / 1000);
  	_eaw	= nDiodes() * Resolution() * 0.001;	// in mm
  	_sampleArea = _armWidth * _eaw;

  if (sampleArea == 0)
    sampleArea = new float[(nDiodes() << 2)];

//printf("SetSampleArea: %s: mag=%f _eaw=%f %d\n", _name.c_str(), mag, _eaw, controlWindow->GetConcentration());
  switch (controlWindow->GetConcentration())
    {
    case BASIC:
      for (size_t i = 1; i < nDiodes(); ++i)
        sampleArea[i] = _armWidth * _eaw;

      break;

    case ENTIRE_IN:
      for (size_t i = 1; i < nDiodes(); ++i)
        {
        diam = i * Resolution();
        DoF = std::min((_dof_const * diam*diam), prht) * 0.001;
        sampleArea[i] = DoF * (diodeDiameter * (nDiodes() - i - 1) / mag);
//printf(" %zu : %f = %f * %f\n", i, sampleArea[i], DoF, diodeDiameter * (nDiodes() - i - 1) / mag);
        }

      break;

    case CENTER_IN:
      for (size_t i = 1; i < nDiodes()<<1; ++i)
        {
        diam = i * Resolution();
        DoF = std::min((_dof_const * diam*diam), prht) * 0.001;
        sampleArea[i] = DoF * _eaw;
//printf(" %zu : %f = %f * %f\n", i, sampleArea[i], DoF, _eaw);
        }

      break;

    case RECONSTRUCTION:
      for (size_t i = 1; i < nDiodes()<<2; ++i)
        {
        diam = (float)i * Resolution();
        _eaw = ((nDiodes() * Resolution()) + (2 * (diam / 2 - diam / 7.2414))) * 0.001;
        DoF = std::min((_dof_const * diam*diam), prht) * 0.001;
        sampleArea[i] = DoF * _eaw;
//printf(" %zu : %f = %f * %f\n", i, sampleArea[i], DoF, _eaw);
        }

      break;
    }

//printf("\n");

}	/* END SETSAMPLEAREA */

// END PROBE.CC
