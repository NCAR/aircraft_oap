/*
-------------------------------------------------------------------------
OBJECT NAME:	HVPS.cc

FULL NAME:	HVPS Probe Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "HVPS.h"
#include "ControlWindow.h"

static const float	TAS_COMPENSATE = 1.0;

const size_t HVPS::lowerMask = 215;	// HVPS masks.
const size_t HVPS::upperMask = 40;	// HVPS masks.


/* -------------------------------------------------------------------- */
HVPS::HVPS(const char xml_entry[], int recSize) : Probe(xml_entry, recSize)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  init();
  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

printf("HVPS:: id=%s, name=%s, resolution=%zu\n", _code, _name.c_str(), _resolution);
}

/* -------------------------------------------------------------------- */
HVPS::HVPS(const char name[]) : Probe(name)
{
  _name.push_back(name[0]);
  _name.push_back(name[1]);
  _name.push_back('\0');
  strcpy(_code, _name.c_str());

  init();
  _lrLen = 4116;

  _resolution = 200;

printf("HVPS:: %s, resolution = %zu\n", _name.c_str(), _resolution);
}

/* -------------------------------------------------------------------- */
HVPS::HVPS(Header * hdr, const Pms2 * p, int cnt) : Probe(hdr, p, cnt)
{
  init();
  // Extract stuff from Header.
  _name = hdr->VariableName(p);
  _name += "_";
  _name += hdr->AircraftLocation(p);
  _serialNumber = hdr->SerialNumber(p);

  _code[0] = _name[3]; _code[1] = cnt + '0'; _code[2] = '\0';

  _lrLen = hdr->lrLength(p);
  _lrPpr = hdr->lrPpr(p);
  _resolution = hdr->Resolution(p);

printf("HVPS:: %s - %s\n", _name.c_str(), _code);
}

void HVPS::init()
{
  _type = Probe::HVPS;
  _nDiodes = 256;
  _lrPpr = 1;
}

extern ControlWindow	*controlWindow;

/* -------------------------------------------------------------------- */
bool HVPS::isSyncWord(const unsigned char *p)
{
  return *p & 0x55;
}

/* -------------------------------------------------------------------- */
struct recStats HVPS::ProcessRecord(const P2d_rec *record, float version)
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
  if (version < 5.09)
    stats.tas = stats.tas * 125 / 255;

  stats.thisTime = (record->hour * 3600 + record->minute * 60 + record->second) * 1000 + record->msec; // in milliseconds

  stats.resolution = Resolution();

  stats.frequency = stats.resolution / stats.tas;
  stats.SampleVolume = 203.0 * stats.resolution * (256-80) * 1.0e-6;
  stats.SampleVolume *= (stats.tas * TAS_COMPENSATE) *
                        (stats.DASelapsedTime - record->overld);

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
      return(stats);

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
    stats.tas = -1.0;
    return(stats);
    }


  if (version == -1)	// This means set time stamp only
  {
    prevTime[probeIdx] = stats.thisTime;
    memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));
    return(stats);
  }

#ifdef DEBUG
  printf("H1 %02d:%02d:%02d.%d - ", record->hour, record->minute, record->second, record->msec);
#endif
  stats.DASelapsedTime = stats.thisTime - prevTime[probeIdx];

  totalLiveTime = 0.0;
  memset(stats.accum, 0, sizeof(stats.accum));

  switch (controlWindow->GetConcentration()) {
    case CENTER_IN:		nBins = 512; break;
    default:			nBins = 256;
    }


  stats.SampleVolume = 203.0 * stats.resolution * (256-80) * 1.0e-6;
  stats.SampleVolume *= (stats.tas * TAS_COMPENSATE) *
			(stats.DASelapsedTime - record->overld);

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
#ifdef DEBUG
  printf("%08x %08x %08x\n", ppSlice, pSlice, p[0]);
#endif
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
      cp->deltaTime = (uint32_t)((float)cp->timeWord * stats.frequency);
      stats.minBar = std::min(stats.minBar, cp->deltaTime);
      stats.maxBar = std::max(stats.maxBar, cp->deltaTime);

      if (!cp->timeReject)
        stats.tBarElapsedtime += cp->deltaTime;

      ++stats.nTimeBars;
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
        if (unshaded <= upperMask) /* touched edge */
          {
          cp->edge |= 0xf0;
          cp->x1++;
          }

        if (shaded + unshaded >= lowerMask) /* touched lower edge (assuming 40 masked out) */
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
      cp->liveTime = (uint32_t)((float)(cp->w) * stats.frequency);
      cp->w = (size_t)((float)cp->w * TAS_COMPENSATE);

      cp->msec /= 1000;

#ifdef DEBUG
  printf("%06x %zu %zu\n", cp->timeWord, cp->w, cp->h);
#endif

      totalLiveTime += checkRejectionCriteria(cp, stats);

      stats.particles.push_back(cp);

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

  stats.tBarElapsedtime /= 1000;	// convert to milliseconds

  if (stats.nTimeBars > 0)
    stats.meanBar = stats.tBarElapsedtime / stats.nTimeBars;

//stats.tBarElapsedtime += (uint32_t)(2048 * stats.frequency);

  stats.frequency /= 1000;


  // Compute "science" data.
  totalLiveTime /= 1000000;	// convert to seconds
  stats.concentration = stats.lwc = stats.dbz = z = 0.0;

  for (size_t i = 1; i < nBins; ++i)
    {
    if (stats.SampleVolume > 0.0)
      {
      conc = stats.accum[i] / (stats.SampleVolume / 1000.0);
//      stats.DOFsampleVolume += (stats.SampleVolume * totalLiveTime);
      }
    else
      conc = 0.0;

//if (i < 30)printf("%d %d %f %f\n", i, stats.accum[i], stats.SampleVolume, totalLiveTime);

    diameter = i * stats.resolution;
    stats.lwc += conc * pow(diameter / 10, 3.0);
    z += conc * pow(diameter / 1000, 6.0);

    stats.concentration += conc;
    }

//  stats.concentration = stats.nTimeBars / (stats.SampleVolume / 1000.0);

  z /= 1000;
  stats.lwc *= M_PI / 6.0 * 1.0e-6 * controlWindow->GetDensity();

  if (z > 0.0)
    stats.dbz = 10.0 * log10(z * 1.0e6);
  else
    stats.dbz = -100.0;

  // Save time for next round.
  prevTime[probeIdx] = stats.thisTime;
  memcpy((char *)&prevHdr[probeIdx], (char *)record, sizeof(P2d_hdr));

  p = (unsigned short *)record->data;
  prevSlice[probeIdx][0] = p[2046];
  prevSlice[probeIdx][1] = p[2047];

  return(stats);

}	// END PROCESSHVPS

// END HVPS.CC
