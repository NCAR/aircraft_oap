/*
-------------------------------------------------------------------------
OBJECT NAME:	CIP.cc

FULL NAME:	DMT CIP/PIP Probe Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "CIP.h"
#include "ControlWindow.h"

const unsigned long long CIP::SyncWord = 0xAAAAAAAAAAAAAAAALL;

/* -------------------------------------------------------------------- */
CIP::CIP(const char xml_entry[], int recSize) : Probe(xml_entry, recSize)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

  init();

printf("CIP:: id=%s, name=%s, resolution=%zu\n", _code, _name.c_str(), _resolution);
}

/* -------------------------------------------------------------------- */
CIP::CIP(const char name[]) : Probe(name)
{
  _name.push_back(name[0]);
  _name.push_back(name[1]);
  _name.push_back('\0');
  strcpy(_code, _name.c_str());

  _resolution = 25;

  init();
  _lrLen = 4116;

printf("CIP:: %s, resolution = %zu\n", _name.c_str(), _resolution);
}

void CIP::init()
{
  _type = Probe::CIP;
  _nDiodes = 64;
  _nSlices = P2D_DATA / _nDiodes * 8;
  _lrPpr = 1;

  if (_code[0] == 'C')  // CIP
    _armWidth = 100.0;

  if (_code[0] == 'P')  // PIP
    _armWidth = 260.0;

  _sampleArea = _armWidth * Resolution() * nDiodes() * 0.001;

  SetSampleArea();
}


extern ControlWindow	*controlWindow;

/* -------------------------------------------------------------------- */
bool CIP::isSyncWord(const unsigned char *p)
{
  return memcmp(p, (void *)&SyncWord, sizeof(SyncWord)) == 0;
}

/* -------------------------------------------------------------------- */
struct recStats CIP::ProcessRecord(const P2d_rec *record, float version)
{
//  char		*probeID = (char *)&record->id;
  int		startTime, overload = 0;
  size_t	nBins;
  unsigned long long	*p, slice;
  unsigned long		startMilliSec;
  double	sampleVolume[maxDiodes], totalLiveTime;

  static Particle	*cp = new Particle();

  unsigned long long	firstTimeWord = 0;
  static unsigned long long prevTimeWord = 0;

  ClearStats(record);
  stats.DASelapsedTime = stats.thisTime - _prevTime;
  stats.frequency = Resolution() / stats.tas;
  stats.SampleVolume = SampleArea() * stats.tas;

  if (version == -1)    // This means set time stamp only
  {
    _prevTime = stats.thisTime;
    memcpy((char *)&_prevHdr, (char *)record, sizeof(P2d_hdr));
    return(stats);
  }

//#ifdef DEBUG
  printf("C8 %02d:%02d:%02d.%d - \n", record->hour, record->minute, record->second, record->msec);
//#endif

  totalLiveTime = 0.0;
  memset(stats.accum, 0, sizeof(stats.accum));

  switch (controlWindow->GetConcentration()) {
    case CENTER_IN:             nBins = 128; break;
    case RECONSTRUCTION:        nBins = 256; break;
    default:                    nBins = nDiodes();
    }

  for (size_t i = 0; i < nBins; ++i)
    sampleVolume[i] = stats.tas * (sampleAreaC[i] * 2) * 0.001;
  
unsigned long long *o = (unsigned long long *)record->data;
for (int j = 0; j < 512; ++j, ++o)
  printf("%llx\n",  *o);

  unsigned char image[16000];
  size_t nSlices = uncompress(image, record->data, 4096);

  // Scan record, compute tBarElapsedtime and stats.
  p = (unsigned long long *)image;

  startTime = _prevTime / 1000;
  startMilliSec = _prevHdr.msec;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices; ++i, ++p)
  {
    slice = *p;
printf("%llx\n", slice);
    /* Have particle, will travel.
     */
    if (slice == SyncWord)
    {
printf("CIP sync in process\n");
      ++p;
      slice = *p;
      unsigned long long thisTimeWord = TimeWord_Microseconds(slice);

      if (firstTimeWord == 0)
        firstTimeWord = thisTimeWord;

      // Close out particle.  Timeword belongs to previous particle.
      if (cp)
      {
        cp->timeWord = thisTimeWord;
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


    if (*p == 0xffffffffffffffffLL)	// Skip blank slice.
      continue;

    ++cp->w;

    slice = ~(*p);

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

    slice = *p;
    int h = nDiodes();
    for (size_t j = 0; j < nDiodes() && (slice & 0x8000000000000000LL); slice <<= 1, ++j)
      --h;
    slice = *p;
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

  stats.tBarElapsedtime = (prevTimeWord - firstTimeWord) / 1000;

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

}	// END PROCESSCIP

/* -------------------------------------------------------------------- */
long long CIP::TimeWord_Microseconds(unsigned long long slice)
{
  long long output;

  int hour = (slice >> 35) & 0x001F;
  int minute = (slice >> 29) & 0x003F;
  int second = (slice >> 23) & 0x003F;
  int msec = (slice >> 13) & 0x03FF;
  int usec = slice & 0x1FFF;
  output = (hour * 3600 + minute * 60 + second);
  output *= 1000000;
  output += msec * 1000;
  output += usec / 8;   // 8 MHz clock or 125nS

  return output;
}

/* -------------------------------------------------------------------- */
// DMT CIP/PIP probes are run length encoded.  Decode here.
// Duplicated in class/MainCanvas.cc, not consolidated at this time due to static variables.
size_t CIP::uncompress(unsigned char *dest, const unsigned char src[], int nbytes)
{
  int d_idx = 0, i = 0;

  static size_t nResidualBytes = 0;
  static unsigned char residualBytes[16];

  if (nResidualBytes)
  {
    memcpy(dest, residualBytes, nResidualBytes);
    d_idx = nResidualBytes;
    nResidualBytes = 0;
  }

  for (; i < nbytes; ++i)
  {
    unsigned char b = src[i];

    int nBytes = (b & 0x1F) + 1;

    if ((b & 0x20))     // This is a dummy byte; for alignment purposes.
    {
      continue;
    }

    if ((b & 0xE0) == 0)
    {
      memcpy(&dest[d_idx], &src[i+1], nBytes);
      d_idx += nBytes;
      i += nBytes;
    }

    if ((b & 0x80))
    {
      memset(&dest[d_idx], 0, nBytes);
      d_idx += nBytes;
    }
    else
    if ((b & 0x40))
    {
      memset(&dest[d_idx], 0xFF, nBytes);
      d_idx += nBytes;
    }
  }

  // Align data.  Find a sync word and put record on mod 8.
  for (i = 0; i < d_idx; ++i)
  {
     if (memcmp(&dest[i], &SyncWord, 8) == 0)
     {
       int n = (&dest[i] - dest) % 8;
       if (n > 0)
       {
         memmove(dest, &dest[n], d_idx);
         d_idx -= n;
       }
       break;
     }
  }

  if (d_idx % 8)
  {
    size_t idx = d_idx / 8 * 8;
    nResidualBytes = d_idx % 8;
    memcpy(residualBytes, &dest[idx], nResidualBytes);
  }

  return d_idx / 8;     // return number of slices.
}

// END CIP.CC
