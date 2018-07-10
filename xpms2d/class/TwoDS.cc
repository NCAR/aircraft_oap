/*
-------------------------------------------------------------------------
OBJECT NAME:	TwoDS.cc

FULL NAME:	SPEC TwoDS Probe Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "TwoDS.h"
#include "ControlWindow.h"

const unsigned char TwoDS::SyncString[] = { 0xaa, 0xaa, 0xaa };
const unsigned char TwoDS::OverldString[] = { 0x55, 0x55, 0xaa };

#define TimeWord_Microseconds(slice)      ((slice & 0x000000ffffffffffLL) / _clockMhz)


/* -------------------------------------------------------------------- */
TwoDS::TwoDS(const char xml_entry[], int recSize) : Probe(Probe::TWODS, xml_entry, recSize, 128)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());
  _armWidth = 50.8;
  _dof_const = 5.13;
  _clockMhz = 200;

  SetSampleArea();
printf("TwoDS::OAP id=%s, name=%s, resolution=%zu, armWidth=%f, eaw=%f\n", _code, _name.c_str(), _resolution, _armWidth, _eaw);
}


extern ControlWindow	*controlWindow;

/* -------------------------------------------------------------------- */
bool TwoDS::isSyncWord(const unsigned char *p)
{
  return memcmp(p, (void *)&SyncString, 3) == 0;
}

/* -------------------------------------------------------------------- */
bool TwoDS::isOverloadWord(const unsigned char *p)
{
  return memcmp(p, (void *)&OverldString, 2) == 0;
}

/* -------------------------------------------------------------------- */
struct recStats TwoDS::ProcessRecord(const P2d_rec *record, float version)
{
  int		startTime, overload = 0;
  size_t	nBins;
  unsigned long long slice;
  unsigned long startMilliSec;
  double	sampleVolume[(nDiodes()<<2)+1], totalLiveTime;

  unsigned long long    firstTimeWord = 0;

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
    case CENTER_IN:		nBins = 256; break;
    case RECONSTRUCTION:	nBins = 512; break;
    default:			nBins = nDiodes();
    }

  for (size_t i = 0; i < nBins; ++i)
    sampleVolume[i] = stats.tas * sampleArea[i] * 0.001;


  // Scan record, compute tBarElapsedtime and stats.
  const unsigned char *p = record->data;

  startTime = _prevTime / 1000;
  startMilliSec = _prevHdr.msec;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices(); ++i, p += 16)
  {
    /* Have particle, will travel.
     */
    if (isSyncWord(p) || isOverloadWord(p))
    {
      slice = ntohll((long long *)&p[8]);
      unsigned long long thisTimeWord = TimeWord_Microseconds(slice);

      if (firstTimeWord == 0)
        firstTimeWord = thisTimeWord;

      if (isOverloadWord(p))
      {
        // Set 'overload' variable here.  There is no way to determine overload.
        // Leave zero, they are less than a milli-second anyways.
        overload = 0;

        printf(">>> TwoDS overload @ %02d:%02d:%02d.%-3d. <<<\n",
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


    if (memcmp(p, BlankSlice, 16) == 0)	// Skip blank slice.
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

}	// END PROCESSTWODS

// END TWODS.CC
