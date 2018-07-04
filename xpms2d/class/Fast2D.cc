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


#define TimeWord_Microseconds(slice)      ((slice & 0x000000ffffffffffLL) / 12)


/* -------------------------------------------------------------------- */
Fast2D::Fast2D(const char xml_entry[], int recSize) : Probe(xml_entry, recSize)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

  init();

printf("Fast2D:: id=%s, name=%s, resolution=%zu\n", _code, _name.c_str(), _resolution);
}

void Fast2D::init()
{
  _type = Probe::FAST2D;
  _nDiodes = 64;
  _nSlices = P2D_DATA / _nDiodes * 8;
  _lrPpr = 1;

  SetSampleArea();
}


extern ControlWindow	*controlWindow;

/* -------------------------------------------------------------------- */
bool Fast2D::isSyncWord(const unsigned char *p)
{
  return *p & 0x55;
}

/* -------------------------------------------------------------------- */
struct recStats Fast2D::ProcessRecord(const P2d_rec *record, float version)
{
  char	*probeID = (char *)&record->id;

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
  if (version < 5.09)
    stats.tas = stats.tas * 125 / 255;

  stats.thisTime = (record->hour * 3600 + record->minute * 60 + record->second) * 1000 + record->msec; // in milliseconds

  stats.resolution = Resolution();

  if (probeID[0] == 'P')
    stats.SampleVolume = 261.0 * (stats.resolution * nDiodes() / 1000);
  else
  if (probeID[0] == 'C')
    stats.SampleVolume = 61.0 * (stats.resolution * nDiodes() / 1000);

  stats.frequency = stats.resolution / stats.tas;
  stats.SampleVolume *= stats.tas *
                        (stats.DASelapsedTime - record->overld) * 0.001;


  int		startTime, overload = 0;
  size_t	nBins, probeIdx = 0;
  const unsigned char	*p;
  unsigned long long	slice;
  unsigned long	startMilliSec;
  double	sampleVolume[maxDiodes], totalLiveTime;

  static Particle	*cp = new Particle();

  static P2d_hdr	prevHdr[MAX_PROBES];
  static unsigned long	prevTime[MAX_PROBES] = { 1,1,1,1 };
  unsigned long long	firstTimeWord = 0;
  static unsigned long long prevTimeWord = 0;

  if (version == -1)    // This means set time stamp only
  {
    prevTime[probeIdx] = stats.thisTime;
    memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));
    return(stats);
  }

#ifdef DEBUG
  printf("C4 %02d:%02d:%02d.%d - ", record->hour, record->minute, record->second, record->msec);
#endif

  stats.DASelapsedTime = stats.thisTime - prevTime[probeIdx];

  totalLiveTime = 0.0;
  memset(stats.accum, 0, sizeof(stats.accum));

  switch (controlWindow->GetConcentration()) {
    case CENTER_IN:             nBins = 128; break;
    case RECONSTRUCTION:        nBins = 256; break;
    default:                    nBins = 64;
    }

  for (size_t i = 0; i < nBins; ++i)
    sampleVolume[i] = stats.tas * (sampleAreaC[i] * 2) * 0.001;
  
  // Scan record, compute tBarElapsedtime and stats.
  p = record->data;

  startTime = prevTime[probeIdx] / 1000;
  startMilliSec = prevHdr[probeIdx].msec;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices(); ++i, p += sizeof(long long))
  {
    slice = ntohll((long long *)p);
  
    /* Have particle, will travel.
     */
    if (memcmp(p, SyncString, 2) == 0 || memcmp(p, OverldString, 2) == 0)
    {
      unsigned long long thisTimeWord = TimeWord_Microseconds(slice);

      if (firstTimeWord == 0)
        firstTimeWord = thisTimeWord;

      if (memcmp(p, OverldString, 2) == 0)
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
        cp->deltaTime = cp->timeWord - prevTimeWord;
        cp->timeWord /= 1000;	// Store as millseconds for this probe, since this is not a 48 bit word
        totalLiveTime += checkRejectionCriteria(cp, stats);
        stats.particles.push_back(cp);
      }

      prevTimeWord = thisTimeWord;

      // Start new particle.
      cp = new Particle();

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

  stats.SampleVolume *= stats.tas *
                        (stats.DASelapsedTime - overload) * 0.001;

  stats.tBarElapsedtime = (prevTimeWord - firstTimeWord) / 1000;

  if (stats.nTimeBars > 0)
    stats.meanBar = stats.tBarElapsedtime / stats.nTimeBars;

  stats.frequency /= 1000;


  // Compute "science" data.
  totalLiveTime /= 1000000;     // convert to seconds

  computeDerived(sampleVolume, nBins, totalLiveTime);

  // Save time for next round.
  prevTime[probeIdx] = stats.thisTime;
  memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));

  return(stats);

}	// END PROCESSFAST2D

// END FAST2D.CC
