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
Probe::Probe(const char xml_entry[], int recSize) : _type(UNKNOWN), _displayed(false)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;
  _lrPpr = 1;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());
}

/* -------------------------------------------------------------------- */
Probe::Probe(const char name[]) : _type(UNKNOWN), _displayed(false)
{
  _name.push_back(name[0]);
  _name.push_back(name[1]);
  _name.push_back('\0');
  strcpy(_code, _name.c_str());

  _lrLen = 4116;
  _lrPpr = 1;

  if (_name[0] == 'C')
    _resolution = 25;

  if (_name[0] == 'P')
    _resolution = 200;
}

/* -------------------------------------------------------------------- */
Probe::Probe(Header * hdr, const Pms2 * p, int cnt) :_type(UNKNOWN),  _displayed(false)
{
  // Extract stuff from Header.
  _name = hdr->VariableName(p);
  _name += "_";
  _name += hdr->AircraftLocation(p);
  _serialNumber = hdr->SerialNumber(p);

  _code[0] = _name[3]; _code[1] = cnt + '0'; _code[2] = '\0';

  _lrLen = hdr->lrLength(p);
  _lrPpr = hdr->lrPpr(p);
  _resolution = hdr->Resolution(p);
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
  stats.thisTime = (record->hour * 3600 + record->minute * 60 + record->second) * 1000 + record->msec; // in milliseconds
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

  if (!cp->reject && bin < maxDiodes)
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
static float DOF2dP[] = { 0.0, 145.203, 261.0, 261.0, 261.0,
  261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0,
  261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0,
  261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0,
  261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0,
  261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0,
  261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0, 261.0 };

static float DOF2dC[] = { 0.0, 1.56, 6.25, 14.06, 25.0, 39.06, 56.25,
  61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0,
  61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0,
  61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0,
  61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0,
  61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0, 61.0,
  61.0, 61.0, 61.0 };

void Probe::SetSampleArea()
{
  float	dia, dof;
  float	eawC = nDiodes() * Resolution() / 1000.0;
  float	eawP = nDiodes() * Resolution() / 1000.0;

  switch (controlWindow->GetConcentration())
    {
    case BASIC:
      for (size_t i = 1; i < maxDiodes; ++i)
        {
        sampleAreaC[i] = 61.0 * eawC;
        sampleAreaP[i] = 261.0 * eawP;
        }

      break;

    case ENTIRE_IN:
      for (size_t i = 1; i <= nDiodes(); ++i)
        {
        sampleAreaC[i] = DOF2dC[i] * diodeDiameter * (nDiodes() - i - 1) / 8.0;
        sampleAreaP[i] = DOF2dP[i] * diodeDiameter * (nDiodes() - i - 1) / 1.0;
//printf("%d %f %f %e %e\n", i, (float)i*25, (float)i*200, sampleAreaC[i], sampleAreaP[i]);
        }

      break;

    case CENTER_IN:
      for (size_t i = 1; i <= nDiodes()<<1; ++i)
        {
        sampleAreaC[i] = DOF2dC[i] * eawC;
        sampleAreaP[i] = DOF2dP[i] * eawP;
//printf("%e %e\n", sampleAreaC[i], sampleAreaP[i]);
        }

      break;

    case RECONSTRUCTION:
      for (size_t i = 1; i <= maxDiodes; ++i)
        {
        dia = (float)i * Resolution();
        eawC = ((nDiodes() * Resolution()) + (2 * (dia / 2 - dia / 7.2414))) * 0.001;

        if (i < 60)
          dof = DOF2dC[i];
        else
          dof = DOF2dC[60];

        sampleAreaC[i] = dof * eawC;


        dia = (float)i * 200;	// 200um
        eawP = ((nDiodes() * 200) + (2 * (dia / 2 - dia / 7.2414))) * 0.001;

        if (i < 60)
          dof = DOF2dP[i];
        else
          dof = DOF2dP[60];

        sampleAreaP[i] = dof * eawP;
//printf("%d %f %f %e %e %e %e\n", i, (float)i * 25, dia, DOF2dC[i] * 0.8, sampleAreaC[i], DOF2dP[i] * 6.4, sampleAreaP[i]);
        }

      break;
    }

}	/* END SETSAMPLEAREA */

// END PROBE.CC
