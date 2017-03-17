/*
-------------------------------------------------------------------------
OBJECT NAME:	process.cc

FULL NAME:	

ENTRY POINTS:	ProcessRecord()
		ProcessHVPSrecord()
		SetSampleArea()

STATIC FNS:	none

DESCRIPTION:	This is the 2d record processor, generates a queue of
		particles and computes all the derived parameters.
		There is a corresponding function in class/MainWindow.cc
		to actually draw the record and information.

NOTES:		

COPYRIGHT:	University Corporation for Atmospheric Research, 1999-2006
-------------------------------------------------------------------------
*/

#include "define.h"
#include <raf/header.h>

#include <ControlWindow.h>
#include <DataFile.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <algorithm>

#define Fast2DTimeWord_Microseconds(slice)	((slice & 0x000000ffffffffffLL) / 12)

static bool debug = false;

static const size_t maxDiodes = 256;
static size_t nDiodes = 32;

static const float	TAS_COMPENSATE = 1.0;
static const size_t	lower_mask = 215, upper_mask = 40;	// HVPS masks.

static float	sampleAreaC[maxDiodes], sampleAreaP[maxDiodes];

static struct recStats	output;

extern ControlWindow	*controlWindow;

struct recStats &ProcessCIP(P2d_rec *record, float version);	// No implemented yet, stub only.
struct recStats &ProcessFast2D(P2d_rec *record, float version);
struct recStats &ProcessPMS2D(P2d_rec *record, float version);
struct recStats &ProcessHVPSrecord(P2d_rec *record, float version);

static void computeDerived(double sv[], size_t nBins, double liveTime);
static size_t checkRejectionCriteria(Particle * cp, recStats & output);

FILE *fout;


/* -------------------------------------------------------------------- */
ProbeType ProbeType(P2d_rec *record)
{
  char *p = (char *)&record->id;

  if (p[0] == 'C' || p[0] == 'P')
  {
    if (p[1] >= '4' && p[1] <= '7')
      return FAST2D;

    if (p[1] == '8')
      return CIP;

    return PMS2D;
  }

  if (p[0] == '2' || p[0] == '3')
    return TWODS;

  if (p[0] == 'H')
    return HVPS;

  if (p[0] == 'G')
    return GREYSCALE;

  return UNKNOWN;
}

/* -------------------------------------------------------------------- */
long long CIPTimeWord_Microseconds(unsigned long long slice)
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
struct recStats &ProcessRecord(P2d_rec *record, float version)
{
  char	*probeID = (char *)&record->id;

  output.tBarElapsedtime = 0;
  output.nTimeBars = 0;
  output.nonRejectParticles = 0;
  output.minBar = 10000000;
  output.maxBar = 0;
  output.area = 0;
  output.DOFsampleVolume = 0.0;
  output.duplicate = false;
  output.tas = (float)record->tas;
  if (version < 5.09)
    output.tas = output.tas * 125 / 255;

  output.thisTime = (record->hour * 3600 + record->minute * 60 + record->second) * 1000 + record->msec; // in milliseconds

  nDiodes = 32;

  if (ProbeType(record) == FAST2D || ProbeType(record) == CIP)
    nDiodes = 64;

  if (probeID[0] == 'P')
    {
    if (probeID[1] == '8')	// PIP
      {
      output.resolution = 100;
      output.SampleVolume = 400.0 * (output.resolution * nDiodes / 1000);
      }
    else
      {
      output.resolution = 200;
      output.SampleVolume = 261.0 * (output.resolution * nDiodes / 1000);
      }
    }
  else
  if (probeID[0] == 'C')
    {
    output.resolution = 25;
    if (probeID[1] == '6')
      output.resolution = 10;

if (probeID[1] == '8')	// CIP
  output.tas = 125;
    if (probeID[1] == '8')	// CIP
      output.SampleVolume = 100.0 * (output.resolution * nDiodes / 1000);
    else
      output.SampleVolume = 61.0 * (output.resolution * nDiodes / 1000);
    }

  if (ProbeType(record) == TWODS)
  {
    nDiodes = 128;
    output.resolution = 10;
  }

  if (ProbeType(record) == HVPS)
  {
    nDiodes = 128;
    output.resolution = 200;
  }

  output.frequency = output.resolution / output.tas;
  output.SampleVolume *= output.tas *
                        (output.DASelapsedTime - record->overld) * 0.001;


  if (ProbeType(record) == FAST2D)
    return(ProcessFast2D(record, version));

  if (ProbeType(record) == CIP)
    return(ProcessCIP(record, version));

  if (ProbeType(record) == PMS2D)
    return(ProcessPMS2D(record, version));

  if (ProbeType(record) == TWODS)
  {
//  Add ProcessSPEC2DS();    
    return(output);
  }

  if (ProbeType(record) == HVPS)
    return(ProcessHVPSrecord(record, version));

  return(output);
}

/* -------------------------------------------------------------------- */
struct recStats &ProcessPMS2D(P2d_rec *record, float version)
{
  int		startTime, overload;
  size_t	nBins, probeIdx;
  uint32_t	*p, slice, ppSlice, pSlice, syncWord, startMilliSec;
  bool		overloadAdded = false;
  double	sampleVolume[maxDiodes], totalLiveTime;
  char		*probeID = (char *)&record->id;

  static P2d_hdr	prevHdr[MAX_PROBES];
  static uint32_t	prevTime[MAX_PROBES] = { 1,1,1,1 };
  static uint32_t	prevSlice[MAX_PROBES][2];

  if (version < 3.35)
    syncWord = SyncWordMask;
  else
    syncWord = StandardSyncWord;

  probeIdx = ((char *)&record->id)[1] - '1';
  if (probeID[0] == 'C')
    probeIdx += 2;

//if (record->msec == 397) debug = true; else debug = false;

  if (version == -1)	// This means set time stamp only
  {
    prevTime[probeIdx] = output.thisTime;
    memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));
    return(output);
  }

if (debug)
  printf("%02d:%02d:%02d.%d - ", record->hour, record->minute, record->second, record->msec);

  overload = prevHdr[probeIdx].overld;
  output.DASelapsedTime = output.thisTime - prevTime[probeIdx];

  totalLiveTime = 0.0;
  memset(output.accum, 0, sizeof(output.accum));

  switch (controlWindow->GetConcentration()) {
    case CENTER_IN:		nBins = 64; break;
    case RECONSTRUCTION:	nBins = 128; break;
    default:			nBins = 32;
    }


  // Compute frequency, which is used to convert timing words from TAS clock
  // pulses to milliseconds.  Most of sample volume is here, time comes in
  // later.
  if (probeID[0] == 'P')
    {
    for (size_t i = 0; i < nBins; ++i)
      sampleVolume[i] = output.tas * sampleAreaP[i] * 0.001;
    }
  else
    {
    for (size_t i = 0; i < nBins; ++i)
      sampleVolume[i] = output.tas * sampleAreaC[i] * 0.001;
    }

  // Scan record, compute tBarElapsedtime and stats.
  p = (uint32_t *)record->data;
  ppSlice = prevSlice[probeIdx][0];
  pSlice = prevSlice[probeIdx][1];

  startTime = prevTime[probeIdx] / 1000;
  startMilliSec = prevHdr[probeIdx].msec * 1000;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices_32bit; ++i, ++p)
    {
    slice = *p;

    if (i > 515 && overloadAdded == false)
      {
      startMilliSec += overload * 1000;
      overloadAdded = true;
      }

    /* Have particle, will travel.
     */
if (debug) printf("%08x %08x %08x\n", ppSlice, pSlice, slice);
    if (slice == syncWord && ppSlice == 0xffffffff && pSlice != 0xffffffff)
      {
      Particle * cp = new Particle();
      cp->time = startTime;
      cp->msec = startMilliSec;

      cp->w = 1;        // first slice of particle is in sync word
      cp->h = 1;
      cp->area = 1;	// assume at list 1 pixel hidden in sync-word.

      cp->timeWord = pSlice & 0x00ffffff;
      cp->deltaTime = (uint32_t)((float)cp->timeWord * output.frequency);
      output.minBar = std::min(output.minBar, cp->deltaTime);
      output.maxBar = std::max(output.maxBar, cp->deltaTime);

      if (!cp->timeReject)
        output.tBarElapsedtime += cp->deltaTime;

      ++output.nTimeBars;

      /* Determine height of particle.
       */
      ++p; ++i;
      for (; i < nSlices_32bit && *p != 0xffffffff; ++p, ++i)
        {
        ++cp->w;

        slice = ~(*p);

        /* Potential problem/bug with computing of x1, x2.  Works good if all
         * edge touches are contigious (water), not so good for snow, where
         * it will all get bunched up.  Counts total number of contacts for
         * each edge.
         */
        if (slice & 0x80000000)	// touched edge
          {
          cp->edge |= 0x0F;
          cp->x1++;
          }

        if (slice & 0x00000001) // touched edge
          {
          cp->edge |= 0xF0;
          cp->x2++;
          }

        for (size_t j = 0; j < nDiodes; ++j, slice >>= 1)
          cp->area += slice & 0x0001;

        int h = nDiodes;
        slice = *p;
        for (size_t j = 0;
                j < nDiodes && (slice & 0x80000000); slice <<= 1, ++j)
          --h;
        slice = *p;
        for (size_t j = 0;
                j < nDiodes && (slice & 0x00000001); slice >>= 1, ++j)
          --h;

        if (h > 0)
          cp->h = std::max((size_t)h, cp->h);
        }

      /* If the particle becomes rejected later, we need to now much time the
       * particle consumed, so we can add it to the deadTime, so sampleVolume
       * can be reduced accordingly.
       */
      cp->liveTime = (uint32_t)((float)(cp->w + 3) * output.frequency);

      cp->msec /= 1000;

if (debug)
  printf("%06x %zu %zu\n", cp->timeWord, cp->w, cp->h);


      // This will not get caught in checkRejectionCriteria(), so do it here.
      if (controlWindow->RejectZeroAreaImage() && cp->w == 1 && cp->h == 1)
        cp->reject = true;

      totalLiveTime += checkRejectionCriteria(cp, output);

      output.particles.EnQueue((void *)cp);

      startMilliSec += (cp->deltaTime + cp->liveTime);

      if (startMilliSec >= 1000000)
        {
        startMilliSec -= 1000000;
        ++startTime;
        }
      }

    ppSlice = pSlice;
    pSlice = slice;
    }

output.tBarElapsedtime += (uint32_t)(nSlices_32bit * output.frequency);

  if (output.nTimeBars > 0)
    output.meanBar = output.tBarElapsedtime / output.nTimeBars;

  output.tBarElapsedtime /= 1000;	// convert to milliseconds
  output.frequency /= 1000;


  // Compute "science" data.
  totalLiveTime /= 1000000;	// convert to seconds

  computeDerived(sampleVolume, nBins, totalLiveTime);

  // Save time for next round.
  prevTime[probeIdx] = output.thisTime;
  memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));

  p = (uint32_t *)record->data;
  prevSlice[probeIdx][0] = p[1022];
  prevSlice[probeIdx][1] = p[1023];

  return(output);

}	// END PROCESSPMS2D

/* -------------------------------------------------------------------- */
struct recStats &ProcessHVPSrecord(P2d_rec *record, float version)
{
  int		startTime;
  size_t	nBins, probeIdx = 0, shaded, unshaded;
  unsigned short	*p, slice, ppSlice, pSlice;
  uint32_t	startMilliSec;
  bool		overloadAdded = false;
  double	diameter, z, conc, totalLiveTime;

  Particle	*cp;

  static P2d_hdr	prevHdr[MAX_PROBES];
  static uint32_t	prevTime[MAX_PROBES] = { 1,1,1,1 };
  static unsigned short	prevSlice[MAX_PROBES][2];

  p = (unsigned short *)record->data;

  if (p[0] == 0xcaaa)
    {
    int offset;

    if (p[1] == 0xcaaa)
      offset = 0;
    else
      if (p[17] == 0x2043)
        offset = -1;
    else
      return(output);

/*
    for (i = 2 + offset; i < 18+offset; ++i)
      printf("%x ", p[i]);

    for (i = 28 + offset; i < 286+offset; ++i)
      printf("%x ");
    printf("\n");

printf("\n");

    for (i = 0, cntr = 0; i < 2048; ++i)
      {
      if (i < 286 && p[i] == 0x2043)
        toft = i;

      if (i > 286 && p[i] == 0x5555)
        ++cntr;
      }
  
    printf("fives cntr = %d 2043 @ %d\n", cntr, toft);
*/
    output.tas = -1.0;
    return(output);
    }


  if (version == -1)	// This means set time stamp only
  {
    prevTime[probeIdx] = output.thisTime;
    memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));
    return(output);
  }

if (debug)
  printf("H1 %02d:%02d:%02d.%d - ", record->hour, record->minute, record->second, record->msec);
  output.DASelapsedTime = output.thisTime - prevTime[probeIdx];

  totalLiveTime = 0.0;
  memset(output.accum, 0, sizeof(output.accum));

  switch (controlWindow->GetConcentration()) {
    case CENTER_IN:		nBins = 512; break;
    default:			nBins = 256;
    }


  output.SampleVolume = 203.0 * output.resolution * (256-80) * 1.0e-6;
  output.SampleVolume *= (output.tas * TAS_COMPENSATE) *
			(output.DASelapsedTime - record->overld);

  // Scan record, compute tBarElapsedtime and stats.
  p = (unsigned short *)record->data;
  ppSlice = prevSlice[probeIdx][0];
  pSlice = prevSlice[probeIdx][1];

  startTime = prevTime[probeIdx] / 1000;
  startMilliSec = prevHdr[probeIdx].msec * 1000;

  // Loop through all slices in record.
  for (size_t i = 0; i < 2048; ++i, ++p)
    {
    slice = *p;

    if (i > 515 && overloadAdded == false)
      {
      startMilliSec += record->overld * 1000;
      overloadAdded = true;
      }

    /* Have particle, will travel.  By locating a timing word.
     */
if (debug) printf("%08x %08x %08x\n", ppSlice, pSlice, p[0]);
    if (slice & 0x8000)
      {
      /* Reject particle if there are not two timing words.  Usually will happen
       * when start of particle crosses record boundaries.
       */
      if ((p[1] & 0x8000) != 0x8000)
        continue;

      cp = new Particle();
      cp->time = startTime;
      cp->msec = startMilliSec;

//printf("timing words %x %x %x\n", p[-1], p[0], p[1]);

      cp->timeWord = (((uint32_t)p[0] << 14) & 0x0fffc000);
      cp->timeWord += (uint32_t)(p[1] & 0x3fff);
//printf("  time out = %x\n", cp->timeWord);
      cp->deltaTime = (uint32_t)((float)cp->timeWord * output.frequency);
      output.minBar = std::min(output.minBar, cp->deltaTime);
      output.maxBar = std::max(output.maxBar, cp->deltaTime);

      if (!cp->timeReject)
        output.tBarElapsedtime += cp->deltaTime;

      ++output.nTimeBars;
//printf("p[2] = %x\n", p[2]);

      /* Determine height of particle.
       */
      p += 2; i += 2;
      for (; i < 2048 && (*p & 0x8000) != 0x8000; ++p, ++i)
        {
        ++cp->w;
        slice = *p;

        unshaded = shaded = 0;

        if (slice == 0x4000)    // Starts in lower half of diode array.
          {
          ++p; ++i;
          slice = *p;

          unshaded = 128;
          }

        shaded = (slice & 0x3f80) >> 7;
        unshaded += slice & 0x007f;

//printf("  slice = %04x - %d %d\n", slice, unshaded, shaded);
        /* Loop through any additional words that make up this slice - e.g.
         * multiple particles in one array.
         */
        while ((p[1] & 0xc000) == 0)
          {
          ++p; ++i;
          slice = *p;

          shaded += (slice & 0x3f80) >> 7;
          shaded += slice & 0x007f;
//printf("        = %04x - %d %d\n", slice, unshaded, shaded);
          }



        /* Potential problem/bug with computing of x1, x2.  Works good if all
         * edge touches are contigious (water), not so good for snow, where
         * it will all get bunched up.  Counts total number of contacts for
         * each edge.
         */
        if (unshaded <= upper_mask) /* touched edge */
          {
          cp->edge |= 0xf0;
          cp->x1++;
          }

        if (shaded + unshaded >= lower_mask) /* touched lower edge (assuming 40 masked out) */
          {
          cp->edge |= 0x0f;
          cp->x2++;
          }

        cp->area += (size_t)((float)shaded * TAS_COMPENSATE);
        cp->h = std::max(cp->h, shaded);
        }

      --p; --i;


      /* If the particle becomes rejected later, we need to now much time the
       * particle consumed, so we can add it to the deadTime, so sampleVolume
       * can be reduced accordingly.
       */
      cp->liveTime = (uint32_t)((float)(cp->w) * output.frequency);
      cp->w = (size_t)((float)cp->w * TAS_COMPENSATE);

      cp->msec /= 1000;

if (debug)
  printf("%06x %zu %zu\n", cp->timeWord, cp->w, cp->h);

      totalLiveTime += checkRejectionCriteria(cp, output);

      output.particles.EnQueue((void *)cp);

      startMilliSec += (cp->deltaTime + cp->liveTime);

      if (startMilliSec >= 1000000)
        {
        startMilliSec -= 1000000;
        ++startTime;
        }
      }

    ppSlice = pSlice;
    pSlice = slice;
    }

  output.tBarElapsedtime /= 1000;	// convert to milliseconds

  if (output.nTimeBars > 0)
    output.meanBar = output.tBarElapsedtime / output.nTimeBars;

//output.tBarElapsedtime += (uint32_t)(2048 * output.frequency);

  output.frequency /= 1000;


  // Compute "science" data.
  totalLiveTime /= 1000000;	// convert to seconds
  output.concentration = output.lwc = output.dbz = z = 0.0;

  for (size_t i = 1; i < nBins; ++i)
    {
    if (output.SampleVolume > 0.0)
      {
      conc = output.accum[i] / (output.SampleVolume / 1000.0);
//      output.DOFsampleVolume += (output.SampleVolume * totalLiveTime);
      }
    else
      conc = 0.0;

//if (i < 30)printf("%d %d %f %f\n", i, output.accum[i], output.SampleVolume, totalLiveTime);

    diameter = i * output.resolution;
    output.lwc += conc * pow(diameter / 10, 3.0);
    z += conc * pow(diameter / 1000, 6.0);

    output.concentration += conc;
    }

//  output.concentration = output.nTimeBars / (output.SampleVolume / 1000.0);

  z /= 1000;
  output.lwc *= M_PI / 6.0 * 1.0e-6 * controlWindow->GetDensity();

  if (z > 0.0)
    output.dbz = 10.0 * log10(z * 1.0e6);
  else
    output.dbz = -100.0;

  // Save time for next round.
  prevTime[probeIdx] = output.thisTime;
  memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));

  p = (unsigned short *)record->data;
  prevSlice[probeIdx][0] = p[2046];
  prevSlice[probeIdx][1] = p[2047];

  return(output);

}	// END PROCESSHVPS

/* -------------------------------------------------------------------- */
// DMT CIP/PIP probes are run length encoded.  Decode here.
// Duplicated in class/MainCanvas.cc, not consolidated at this time due to static variables.
size_t uncompressCIP(unsigned char *dest, const unsigned char src[], int nbytes)
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
     if (memcmp(&dest[i], &CIP_Sync, 8) == 0)
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

/* -------------------------------------------------------------------- */
struct recStats &ProcessCIP(P2d_rec *record, float version)
{
  int		startTime, overload = 0;
  size_t	nBins, probeIdx = 0;
  unsigned long long	*p, slice;
  unsigned long		startMilliSec;
  double	sampleVolume[maxDiodes], totalLiveTime;

  static Particle	*cp = new Particle();

  static P2d_hdr	prevHdr[MAX_PROBES];
  static unsigned long	prevTime[MAX_PROBES] = { 1,1,1,1 };
  unsigned long long	firstTimeWord = 0;
  static unsigned long long prevTimeWord = 0;

  if (version == -1)    // This means set time stamp only
  {
    prevTime[probeIdx] = output.thisTime;
    memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));
    return(output);
  }

//if (debug)
  printf("C8 %02d:%02d:%02d.%d - \n", record->hour, record->minute, record->second, record->msec);

  output.DASelapsedTime = output.thisTime - prevTime[probeIdx];

  totalLiveTime = 0.0;
  memset(output.accum, 0, sizeof(output.accum));

  switch (controlWindow->GetConcentration()) {
    case CENTER_IN:             nBins = 128; break;
    case RECONSTRUCTION:        nBins = 256; break;
    default:                    nBins = 64;
    }

  for (size_t i = 0; i < nBins; ++i)
    sampleVolume[i] = output.tas * (sampleAreaC[i] * 2) * 0.001;
  
unsigned long long *o = (unsigned long long *)record->data;
for (int j = 0; j < 512; ++j, ++o)
  printf("%llx\n",  *o);

  unsigned char image[16000];
  size_t nSlices = uncompressCIP(image, record->data, 4096);

  // Scan record, compute tBarElapsedtime and stats.
  p = (unsigned long long *)image;

  startTime = prevTime[probeIdx] / 1000;
  startMilliSec = prevHdr[probeIdx].msec;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices; ++i, ++p)
  {
    slice = *p;
printf("%llx\n", slice);
    /* Have particle, will travel.
     */
    if (slice == CIP_Sync)
    {
printf("CIP sync in process\n");
      ++p;
      slice = *p;
      unsigned long long thisTimeWord = CIPTimeWord_Microseconds(slice);

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
        totalLiveTime += checkRejectionCriteria(cp, output);
        output.particles.EnQueue((void *)cp);
      }

      prevTimeWord = thisTimeWord;

      // Start new particle.
      cp = new Particle();

      ++output.nTimeBars;
      output.minBar = std::min(output.minBar, cp->deltaTime);
      output.maxBar = std::max(output.maxBar, cp->deltaTime);
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

    for (size_t j = 0; j < nDiodes; ++j, slice >>= 1)
      cp->area += slice & 0x0001;

    slice = *p;
    int h = nDiodes;
    for (size_t j = 0; j < nDiodes && (slice & 0x8000000000000000LL); slice <<= 1, ++j)
      --h;
    slice = *p;
    for (size_t j = 0; j < nDiodes && (slice & 0x00000001LL); slice >>= 1, ++j)
      --h;

    if (h > 0)
      cp->h = std::max((size_t)h, cp->h);


    /* If the particle becomes rejected later, we need to now much time the
     * particle consumed, so we can add it to the deadTime, so sampleVolume
     * can be reduced accordingly.
     */
    cp->liveTime = (unsigned long)((float)(cp->w) * output.frequency);

if (debug)
  printf("%06x %zu %zu\n", cp->timeWord, cp->w, cp->h);
  }

  output.SampleVolume *= output.tas *
                        (output.DASelapsedTime - overload) * 0.001;

  output.tBarElapsedtime = (prevTimeWord - firstTimeWord) / 1000;

  if (output.nTimeBars > 0)
    output.meanBar = output.tBarElapsedtime / output.nTimeBars;

  output.frequency /= 1000;


  // Compute "science" data.
  totalLiveTime /= 1000000;     // convert to seconds

  computeDerived(sampleVolume, nBins, totalLiveTime);

  // Save time for next round.
  prevTime[probeIdx] = output.thisTime;
  memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));

  return(output);

}	// END PROCESSCIP

/* -------------------------------------------------------------------- */
struct recStats &ProcessFast2D(P2d_rec *record, float version)
{
  int		startTime, overload = 0;
  size_t	nBins, probeIdx = 0;
  unsigned long long	*p, slice;
  unsigned long		startMilliSec;
  double	sampleVolume[maxDiodes], totalLiveTime;

  static Particle	*cp = new Particle();

  static P2d_hdr	prevHdr[MAX_PROBES];
  static unsigned long	prevTime[MAX_PROBES] = { 1,1,1,1 };
  unsigned long long	firstTimeWord = 0;
  static unsigned long long prevTimeWord = 0;

  if (version == -1)    // This means set time stamp only
  {
    prevTime[probeIdx] = output.thisTime;
    memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));
    return(output);
  }

if (debug)
  printf("C4 %02d:%02d:%02d.%d - ", record->hour, record->minute, record->second, record->msec);

  output.DASelapsedTime = output.thisTime - prevTime[probeIdx];

  totalLiveTime = 0.0;
  memset(output.accum, 0, sizeof(output.accum));

  switch (controlWindow->GetConcentration()) {
    case CENTER_IN:             nBins = 128; break;
    case RECONSTRUCTION:        nBins = 256; break;
    default:                    nBins = 64;
    }

  for (size_t i = 0; i < nBins; ++i)
    sampleVolume[i] = output.tas * (sampleAreaC[i] * 2) * 0.001;
  
  // Scan record, compute tBarElapsedtime and stats.
  p = (unsigned long long *)record->data;

  startTime = prevTime[probeIdx] / 1000;
  startMilliSec = prevHdr[probeIdx].msec;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices_64bit; ++i, ++p)
  {
    slice = *p;
  
    /* Have particle, will travel.
     */
    if ((slice & Fast2D_Mask) == Fast2D_Sync || (slice & Fast2D_Mask) == Fast2D_Overld)
    {
      unsigned long long thisTimeWord = Fast2DTimeWord_Microseconds(slice);

      if (firstTimeWord == 0)
        firstTimeWord = thisTimeWord;

      if ((slice & Fast2D_Mask) == Fast2D_Overld)
      {
        // Set 'overload' variable here.  There is no way to determine overload.
        // Leave zero, they are less than a milli-second anyways.
        record->overld = overload = 0;

        printf(">>> Fast2D overload @ %02d:%02d:%02d.%-3d. <<<\n",
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
        cp->deltaTime = cp->timeWord - prevTimeWord;
        cp->timeWord /= 1000;	// Store as millseconds for this probe, since this is not a 48 bit word
        totalLiveTime += checkRejectionCriteria(cp, output);
        output.particles.EnQueue((void *)cp);
      }

      prevTimeWord = thisTimeWord;

      // Start new particle.
      cp = new Particle();

      ++output.nTimeBars;
      output.minBar = std::min(output.minBar, cp->deltaTime);
      output.maxBar = std::max(output.maxBar, cp->deltaTime);
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

    for (size_t j = 0; j < nDiodes; ++j, slice >>= 1)
      cp->area += slice & 0x0001;

    slice = *p;
    int h = nDiodes;
    for (size_t j = 0; j < nDiodes && (slice & 0x8000000000000000LL); slice <<= 1, ++j)
      --h;
    slice = *p;
    for (size_t j = 0; j < nDiodes && (slice & 0x00000001LL); slice >>= 1, ++j)
      --h;

    if (h > 0)
      cp->h = std::max((size_t)h, cp->h);


    /* If the particle becomes rejected later, we need to now much time the
     * particle consumed, so we can add it to the deadTime, so sampleVolume
     * can be reduced accordingly.
     */
    cp->liveTime = (unsigned long)((float)(cp->w) * output.frequency);

if (debug)
  printf("%06x %zu %zu\n", cp->timeWord, cp->w, cp->h);
  }

  output.SampleVolume *= output.tas *
                        (output.DASelapsedTime - overload) * 0.001;

  output.tBarElapsedtime = (prevTimeWord - firstTimeWord) / 1000;

  if (output.nTimeBars > 0)
    output.meanBar = output.tBarElapsedtime / output.nTimeBars;

  output.frequency /= 1000;


  // Compute "science" data.
  totalLiveTime /= 1000000;     // convert to seconds

  computeDerived(sampleVolume, nBins, totalLiveTime);

  // Save time for next round.
  prevTime[probeIdx] = output.thisTime;
  memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));

  return(output);

}	// END PROCESSFAST2D

/* -------------------------------------------------------------------- */
static size_t checkRejectionCriteria(Particle * cp, recStats & output)
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
        if (debug) printf("reject no-center rule #%d\n", output.nTimeBars);
        cp->reject = true;
      }

      bin = std::max(cp->w, cp->h);
      break;

    case RECONSTRUCTION:
      if (cp->edge && (float)cp->h / cp->w < 0.2)
      {
        if (debug) printf("reject 20%% rule #%d\n", output.nTimeBars);
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
    output.accum[bin]++;
    output.area += cp->area;
    output.nonRejectParticles++;
    return cp->liveTime + cp->deltaTime;
  }
  return 0;
}

/* -------------------------------------------------------------------- */
static void computeDerived(double sampleVolume[], size_t nBins, double totalLiveTime)
{
  double	diameter, z, conc;
  output.concentration = output.lwc = output.dbz = z = 0.0;

  for (size_t i = 1; i < nBins; ++i)
    {
    if (sampleVolume[i] > 0.0)
      {
      conc = output.accum[i] / (sampleVolume[i] * totalLiveTime);
      output.DOFsampleVolume += (sampleVolume[i] * totalLiveTime);
      }
    else
      conc = 0.0;

    diameter = i * output.resolution;
    output.lwc += conc * pow(diameter / 10, 3.0);
    z += conc * pow(diameter / 1000, 6.0);

    output.concentration += conc;
    }

  z /= 1000;
  output.lwc *= M_PI / 6.0 * 1.0e-6 * controlWindow->GetDensity();

  if (z > 0.0)
    output.dbz = 10.0 * log10(z * 1.0e6);
  else
    output.dbz = -100.0;
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

static float	diodeDiameter = 0.2;

void SetSampleArea()
{
  float	dia, eaw, dof;

  switch (controlWindow->GetConcentration())
    {
    case BASIC:
      for (size_t i = 1; i < maxDiodes; ++i)
        {
        sampleAreaC[i] = 61.0 * 0.8;
        sampleAreaP[i] = 261.0 * 6.4;
        }

      break;

    case ENTIRE_IN:
      for (size_t i = 1; i <= nDiodes; ++i)
        {
        sampleAreaC[i] = DOF2dC[i] * diodeDiameter * (nDiodes - i - 1) / 8.0;
        sampleAreaP[i] = DOF2dP[i] * diodeDiameter * (nDiodes - i - 1) / 1.0;
//printf("%d %f %f %e %e\n", i, (float)i*25, (float)i*200, sampleAreaC[i], sampleAreaP[i]);
        }

      break;

    case CENTER_IN:
      for (size_t i = 1; i <= nDiodes<<1; ++i)
        {
        sampleAreaC[i] = DOF2dC[i] * 0.8;
        sampleAreaP[i] = DOF2dP[i] * 6.4;
//printf("%e %e\n", sampleAreaC[i], sampleAreaP[i]);
        }

      break;

    case RECONSTRUCTION:
      for (size_t i = 1; i <= maxDiodes; ++i)
        {
        dia = (float)i * 25;
        eaw = ((nDiodes * 25) + (2 * (dia / 2 - dia / 7.2414))) * 0.001;

        if (i < 60)
          dof = DOF2dC[i];
        else
          dof = DOF2dC[60];

        sampleAreaC[i] = dof * eaw;


        dia = (float)i * 200;
        eaw = ((nDiodes * 200) + (2 * (dia / 2 - dia / 7.2414))) * 0.001;

        if (i < 60)
          dof = DOF2dP[i];
        else
          dof = DOF2dP[60];

        sampleAreaP[i] = dof * eaw;
//printf("%d %f %f %e %e %e %e\n", i, (float)i * 25, dia, DOF2dC[i] * 0.8, sampleAreaC[i], DOF2dP[i] * 6.4, sampleAreaP[i]);
        }

      break;
    }

}	/* END SETSAMPLEAREA */

Particle::Particle() :	time(0), msec(0), w(0), h(0), area(0), vert_gap(false),
			horiz_gap(false), reject(false), edge(0), timeReject(false)
{
}

/* END PROCESS */
