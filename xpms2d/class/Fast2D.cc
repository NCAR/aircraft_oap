/*
-------------------------------------------------------------------------
OBJECT NAME:	Fast2D.cc

FULL NAME:	Fast2D Probe Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "Fast2D.h"
#include "ControlWindow.h"

const unsigned char Fast2D::SyncString[] = { 0xaa, 0xaa, 0xaa };
const unsigned char Fast2D::OverldString[] = { 0x55, 0x55, 0xaa };


/* -------------------------------------------------------------------- */
Fast2D::Fast2D(UserConfig *cfg, const char xml_entry[], int recSize) : Probe(Probe::FAST2D, cfg, xml_entry, recSize, 64)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

  f2d_init();

printf("Fast2D::OAP id=%s, name=%s, resolution=%zu, armWidth=%f, eaw=%f\n", _code, _name.c_str(), _resolution, _armWidth, _eaw);
}

void Fast2D::f2d_init()
{
  // Default for original version of probe.
  _dofMask = 0x01;

  if (_code[0] == 'C')  // 2DC
    _armWidth = 61.0;

  if (_code[0] == 'P')  // 2DP
    _armWidth = 261.0;

  // Version two of the Fast2D uses a 33Mhz clock.
  _clockMhz = 12;
  if (_name.find("v2") != std::string::npos)
  {
    _clockMhz = 33;
    _dofMask = 0x10;
  }

  SetSampleArea();
}


/* -------------------------------------------------------------------- */
unsigned long long Fast2D::TimeWord_Microseconds(const unsigned char *p)
{
  // Fast2D uses 40 bit timing word; v2 uses 42 bits.
  return (ntohll((long long *)p) & 0x000000ffffffffffLL) / _clockMhz;
}

/* -------------------------------------------------------------------- */
bool Fast2D::isSyncWord(const unsigned char *p)
{
  return memcmp(p, (void *)&SyncString, 2) == 0;
}

/* -------------------------------------------------------------------- */
bool Fast2D::isOverloadWord(const unsigned char *p)
{
  return memcmp(p, (void *)&OverldString, 2) == 0;
}

/* -------------------------------------------------------------------- */
struct recStats Fast2D::ProcessRecord(const P2d_rec *record, float version)
{
  int		startTime, overload = 0;
  unsigned long	startMilliSec;
  double	sampleVolume[(nDiodes()<<1)+1], totalLiveTime;

  unsigned long long	firstTimeWord = 0;

  ClearStats(record);
  stats.DASelapsedTime = stats.thisTime - _prevTime;
  stats.SampleVolume = SampleArea() * stats.tas;

  if (version == -1)    // This means set time stamp only
  {
    _prevTime = stats.thisTime;
    memcpy((char *)&_prevHdr, (char *)record, sizeof(P2d_hdr));
    return(stats);
  }

#ifdef DEBUG
  printf("C4 %02d:%02d:%02d.%d - ", record->hour, record->minute, record->second, record->msec);
#endif

  totalLiveTime = 0.0;

  switch (_userConfig->GetConcentration()) {
    case CENTER_IN:
    case RECONSTRUCTION:	_numBins = 128; break;
    default:			_numBins = nDiodes();
    }

  for (size_t i = 0; i < NumberBins(); ++i)
    sampleVolume[i] = stats.tas * sampleArea[i] * 0.001;
  
  // Scan record, compute tBarElapsedtime and stats.
  const unsigned char *p = record->data;

  startTime = _prevTime / 1000;
  startMilliSec = _prevHdr.msec;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices(); ++i, p += sizeof(long long))
  {
    /* Have particle, will travel.
     */
    if (isSyncWord(p) || isOverloadWord(p))
    {
      unsigned long long thisTimeWord = TimeWord_Microseconds(p);

      if (firstTimeWord == 0)
        firstTimeWord = thisTimeWord;

      if (isOverloadWord(p))
      {
        // Set 'overload' variable here.  There is no way to determine overload.
        // Leave zero, they are less than a milli-second anyways.
        overload = 0;

        printf(">>> %s overload @ %02d:%02d:%02d.%-3d. <<<\n", Name().c_str(),
		record->hour, record->minute, record->second, record->msec);

        if (cp)
          cp->reject = true;
      }

      // Close out particle.  Timeword belongs to previous particle.
      if (cp)
      {
        cp->timeWord = thisTimeWord;
        unsigned long msec = startMilliSec + ((thisTimeWord - firstTimeWord) / 1000);
        cp->time = startTime + (msec / 1000);
        cp->msec = msec % 1000;
        cp->deltaTime = cp->timeWord - _prevTimeWord;
        cp->timeWord /= 1000;	// Store as millseconds for this probe, since this is not a 48 bit word
        totalLiveTime += checkRejectionCriteria(cp, stats);
        if ((p[2] & _dofMask))
          cp->dofReject = cp->reject = true;

        stats.particles.push_back(cp);
        cp = new Particle();
      }

      _prevTimeWord = thisTimeWord;

      ++stats.nTimeBars;
      stats.minBar = std::min(stats.minBar, cp->deltaTime);
      stats.maxBar = std::max(stats.maxBar, cp->deltaTime);
      continue;
    }


    if (isBlankSlice(p))
      continue;

    ++cp->w;

    checkEdgeDiodes(cp, p);
    cp->area += area(p);
    cp->h = std::max(height(p), cp->h);

    /* If the particle becomes rejected later, we need to now much time the
     * particle consumed, so we can add it to the deadTime, so sampleVolume
     * can be reduced accordingly.
     */
    cp->liveTime = (unsigned long)((float)(cp->w) * stats.frequency);

#ifdef DEBUG
  printf("%06x %zu %zu\n", cp->timeWord, cp->w, cp->h);
#endif
  }

  stats.SampleVolume *= (stats.DASelapsedTime - overload) * 0.001;
  stats.tBarElapsedtime = (_prevTimeWord - firstTimeWord) / 1000;

  if (stats.nTimeBars > 0)
    stats.meanBar = stats.tBarElapsedtime / stats.nTimeBars;

  stats.frequency /= 1000;


  // Compute "science" data.
  totalLiveTime /= 1000000;     // convert to seconds

  computeDerived(sampleVolume, totalLiveTime);

  // Save time for next round.
  _prevTime = stats.thisTime;
  memcpy((char *)&_prevHdr, (char *)record, sizeof(P2d_hdr));

  return(stats);

}	// END PROCESSFAST2D

// END FAST2D.CC
