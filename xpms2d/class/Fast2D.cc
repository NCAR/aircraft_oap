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


#define TimeWord_Microseconds(slice)      ((slice & 0x000000ffffffffffLL) / _clockMhz)


/* -------------------------------------------------------------------- */
Fast2D::Fast2D(const char xml_entry[], int recSize) : Probe(Probe::FAST2D, xml_entry, recSize, 64), _clockMhz(12)
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
  if (_code[0] == 'C')  // 2DC
    _armWidth = 61.0;

  if (_code[0] == 'P')  // 2DP
    _armWidth = 261.0;

  // Version two of the Fast2D uses a 33Mhz clock.
  if (_name.find("v2") != std::string::npos)
    _clockMhz = 33;

  SetSampleArea();
}


extern ControlWindow	*controlWindow;

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
  size_t	nBins;
  unsigned long long	slice;
  unsigned long	startMilliSec;
  double	sampleVolume[(nDiodes()<<2)+1], totalLiveTime;

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

  switch (controlWindow->GetConcentration()) {
    case CENTER_IN:             nBins = 128; break;
    case RECONSTRUCTION:        nBins = 256; break;
    default:                    nBins = nDiodes();
    }

  for (size_t i = 0; i < nBins; ++i)
    sampleVolume[i] = stats.tas * sampleArea[i] * 0.001;
  
  // Scan record, compute tBarElapsedtime and stats.
  const unsigned char *p = record->data;

  startTime = _prevTime / 1000;
  startMilliSec = _prevHdr.msec;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices(); ++i, p += sizeof(long long))
  {
    slice = ntohll((long long *)p);
  
    /* Have particle, will travel.
     */
    if (isSyncWord(p) || isOverloadWord(p))
    {
      unsigned long long thisTimeWord = TimeWord_Microseconds(slice);

      if (firstTimeWord == 0)
        firstTimeWord = thisTimeWord;

      if (isOverloadWord(p))
      {
        // Set 'overload' variable here.  There is no way to determine overload.
        // Leave zero, they are less than a milli-second anyways.
        overload = 0;

        printf(">>> Fast2D overload @ %02d:%02d:%02d.%-3d. <<<\n",
		record->hour, record->minute, record->second, record->msec);

        if (cp)
          cp->reject = true;
      }

      // Close out particle.  Timeword belongs to previous particle.
      if (cp)
      {
        cp->timeWord = thisTimeWord;	// !!!! Is timeWord big enough ????
        unsigned long msec = startMilliSec + ((thisTimeWord - firstTimeWord) / 1000);
        cp->time = startTime + (msec / 1000);
        cp->msec = msec % 1000;
        cp->deltaTime = cp->timeWord - _prevTimeWord;
        cp->timeWord /= 1000;	// Store as millseconds for this probe, since this is not a 48 bit word
        totalLiveTime += checkRejectionCriteria(cp, stats);
        stats.particles.push_back(cp);
        cp = new Particle();
      }

      _prevTimeWord = thisTimeWord;

      ++stats.nTimeBars;
      stats.minBar = std::min(stats.minBar, cp->deltaTime);
      stats.maxBar = std::max(stats.maxBar, cp->deltaTime);
      continue;
    }


    if (memcmp(p, BlankSlice, 8) == 0)	// Skip blank slice.
      continue;

    ++cp->w;

    slice = ~(ntohll((long long *)p));

    /* Potential problem/bug with computing of x1, x2.  Works good if all
     * edge touches are contigious (water), not so good for snow, where
     * it will all get bunched up.  Counts total number of contacts for
     * each edge.
     */
    if (slice & 0x8000000000000000LL) // touched edge
    {
      cp->edge |= 0x0F;
      cp->x1++;
    }

    if (slice & 0x00000001LL) // touched edge
    {
      cp->edge |= 0xF0;
      cp->x2++;
    }

    for (size_t j = 0; j < nDiodes(); ++j, slice >>= 1)
      cp->area += slice & 0x0001;

    slice = ntohll((long long *)p);
    int h = nDiodes();
    for (size_t j = 0; j < nDiodes() && (slice & 0x8000000000000000LL); slice <<= 1, ++j)
      --h;
    slice = ntohll((long long *)p);
    for (size_t j = 0; j < nDiodes() && (slice & 0x00000001LL); slice >>= 1, ++j)
      --h;

    if (h > 0)
      cp->h = std::max((size_t)h, cp->h);


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

  computeDerived(sampleVolume, nBins, totalLiveTime);

  // Save time for next round.
  _prevTime = stats.thisTime;
  memcpy((char *)&_prevHdr, (char *)record, sizeof(P2d_hdr));

  return(stats);

}	// END PROCESSFAST2D

// END FAST2D.CC
