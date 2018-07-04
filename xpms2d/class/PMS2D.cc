/*
-------------------------------------------------------------------------
OBJECT NAME:	PMS2D.cc

FULL NAME:	PMS2D Probe Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "PMS2D.h"
#include "ControlWindow.h"

const uint32_t PMS2D::StandardSyncWord = 0x55000000;
const uint32_t PMS2D::SyncWordMask = 0xff000000;


/* -------------------------------------------------------------------- */
PMS2D::PMS2D(const char xml_entry[], int recSize) : Probe(xml_entry, recSize)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

  init();

printf("PMS2D:: id=%s, name=%s, resolution=%zu\n", _code, _name.c_str(), _resolution);
}

/* -------------------------------------------------------------------- */
PMS2D::PMS2D(const char name[]) : Probe(name)
{
  _name.push_back(name[0]);
  _name.push_back(name[1]);
  _name.push_back('\0');
  strcpy(_code, _name.c_str());

  _lrLen = 4116;

  if (_name[0] == 'C')	// 2DC
    _resolution = 25;

  if (_name[0] == 'P')	// 2DP
    _resolution = 200;

  init();

printf("PMS2D:: %s, resolution = %zu\n", _name.c_str(), _resolution);
}

/* -------------------------------------------------------------------- */
PMS2D::PMS2D(Header * hdr, const Pms2 * p, int cnt) : Probe(hdr, p, cnt)
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

  init();

printf("PMS2D:: %s - %s\n", _name.c_str(), _code);
}

void PMS2D::init()
{
  _type = Probe::PMS2D;
  _nDiodes = 32;
  _nSlices = P2D_DATA / _nDiodes * 8;
  _lrPpr = 1;

  SetSampleArea();
}

extern ControlWindow	*controlWindow;

/* -------------------------------------------------------------------- */
bool PMS2D::isSyncWord(const unsigned char *p)
{
  return *p & 0x55;
}

/* -------------------------------------------------------------------- */
struct recStats PMS2D::ProcessRecord(const P2d_rec *record, float version)
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
  output.particles.clear();
  output.tas = (float)record->tas;
  if (version < 5.09)
    output.tas = output.tas * 125 / 255;

  output.thisTime = (record->hour * 3600 + record->minute * 60 + record->second) * 1000 + record->msec; // in milliseconds

  output.resolution = Resolution();

  if (probeID[0] == 'P')
    output.SampleVolume = 261.0 * (output.resolution * nDiodes() / 1000);
  else
  if (probeID[0] == 'C')
    output.SampleVolume = 61.0 * (output.resolution * nDiodes() / 1000);

  output.frequency = output.resolution / output.tas;
  output.SampleVolume *= output.tas *
                        (output.DASelapsedTime - record->overld) * 0.001;


  int		startTime, overload;
  size_t	nBins, probeIdx;
  uint32_t	*p, slice, pSlice, syncWord, startMilliSec;
  bool		overloadAdded = false;
  double	sampleVolume[maxDiodes], totalLiveTime;

  static P2d_hdr	prevHdr[MAX_PROBES];
  static uint32_t	prevTime[MAX_PROBES] = { 1,1,1,1 };
  static uint32_t	prevSlice[MAX_PROBES];

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

#ifdef DEBUG
  printf("%02d:%02d:%02d.%d - ", record->hour, record->minute, record->second, record->msec);
#endif

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
  pSlice = prevSlice[probeIdx];

  startTime = prevTime[probeIdx] / 1000;
  startMilliSec = prevHdr[probeIdx].msec * 1000;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices(); )
    {
    if (i > 515 && overloadAdded == false)
      {
      startMilliSec += overload * 1000;
      overloadAdded = true;
      }

    /* Have particle, will travel.
     */
#ifdef DEBUG
  printf("%08x %08x %08x\n", pSlice, p[i], p[i+1]);
#endif
    if (pSlice == 0xffffffff && (p[i] & 0x55) == 0x55 && ntohl(p[i+1]) == syncWord)
      {
      Particle * cp = new Particle();
      cp->time = startTime;
      cp->msec = startMilliSec;

      cp->w = 1;        // first slice of particle is in sync word
      cp->h = 1;
      cp->area = 1;	// assume at list 1 pixel hidden in sync-word.
      cp->timeWord = 0;

      if ((ntohl(p[i]) & 0x00ffffff) != 0x00ffffff)
        cp->timeWord = ntohl(p[i]) & 0x00ffffff;
      cp->deltaTime = (uint32_t)((float)cp->timeWord * output.frequency);
      output.minBar = std::min(output.minBar, cp->deltaTime);
      output.maxBar = std::max(output.maxBar, cp->deltaTime);

      if (!cp->timeReject)
        output.tBarElapsedtime += cp->deltaTime;

      ++output.nTimeBars;

      /* Determine height of particle.
       */
      i += 2;	// skip time & sync word.
      for (; i < nSlices() && p[i] != 0xffffffff && p[i] != 0x55; ++i)
        {
        ++cp->w;

        slice = ~ntohl(p[i]);

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

        for (size_t j = 0; j < nDiodes(); ++j, slice >>= 1)
          cp->area += slice & 0x0001;

        int h = nDiodes();
        slice = ntohl(p[i]);
        for (size_t j = 0;
                j < nDiodes() && (slice & 0x80000000); slice <<= 1, ++j)
          --h;
        slice = ntohl(p[i]);
        for (size_t j = 0;
                j < nDiodes() && (slice & 0x00000001); slice >>= 1, ++j)
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

#ifdef DEBUG
  printf("%06x %zu %zu\n", cp->timeWord, cp->w, cp->h);
#endif


      // This will not get caught in checkRejectionCriteria(), so do it here.
      if (controlWindow->RejectZeroAreaImage() && cp->w == 1 && cp->h == 1)
        cp->reject = true;

      totalLiveTime += checkRejectionCriteria(cp, output);

      output.particles.push_back(cp);

      startMilliSec += (cp->deltaTime + cp->liveTime);

      if (startMilliSec >= 1000000)
        {
        startMilliSec -= 1000000;
        ++startTime;
        }
      }
    else
      ++i;

    pSlice = p[i-1];
    }

output.tBarElapsedtime += (uint32_t)(nSlices() * output.frequency);

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
  prevSlice[probeIdx] = p[1023];

  return(output);

}	// END PROCESSPMS2D

// END PMS2D.CC
