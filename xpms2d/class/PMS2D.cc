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
PMS2D::PMS2D(const char xml_entry[], int recSize) : Probe(Probe::PMS2D, xml_entry, recSize, 32)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

  pms2d_init();

printf("PMS2D::OAP id=%s, name=%s, resolution=%zu, armWidth=%f, eaw=%f\n", _code, _name.c_str(), _resolution, _armWidth, _eaw);
}

/* -------------------------------------------------------------------- */
PMS2D::PMS2D(const char name[]) : Probe(Probe::PMS2D, name, 32)
{
  pms2d_init();

printf("PMS2D::NoHdr id=%s, name=%s, resolution=%zu, armWidth=%f, eaw=%f\n", _code, _name.c_str(), _resolution, _armWidth, _eaw);
}

/* -------------------------------------------------------------------- */
PMS2D::PMS2D(Header * hdr, const Pms2 * p, int cnt) : Probe(Probe::PMS2D, hdr, p, cnt, 32)
{
  pms2d_init();

printf("PMS2D::ADS2 id=%s, name=%s, resolution=%zu, armWidth=%f, eaw=%f\n", _code, _name.c_str(), _resolution, _armWidth, _eaw);
}

void PMS2D::pms2d_init()
{
  if (_code[0] == 'C')	// 2DC
    _armWidth = 61.0;

  if (_code[0] == 'P')	// 2DP
    _armWidth = 261.0;

  SetSampleArea();
}

extern ControlWindow	*controlWindow;

/* -------------------------------------------------------------------- */
bool PMS2D::isSyncWord(const unsigned char *p)
{
  return *p == 0x55;
}

/* -------------------------------------------------------------------- */
struct recStats PMS2D::ProcessRecord(const P2d_rec *record, float version)
{
  int		startTime, overload;
  size_t	nBins;
  uint32_t	*p, pSlice, syncWord, startMilliSec;
  bool		overloadAdded = false;
  double	sampleVolume[(nDiodes()<<2)+1], totalLiveTime;

  ClearStats(record);
  stats.DASelapsedTime = stats.thisTime - _prevTime;

  // ADS2 recorded tas encoded, decode here.
  if (version < 5.09)
  {
    stats.tas = stats.tas * 125 / 255;
    stats.frequency = Resolution() / stats.tas;
  }

  stats.SampleVolume = SampleArea() * stats.tas *
                        (stats.DASelapsedTime - record->overld) * 0.001;

  if (version == -1)	// This means set time stamp only
  {
    _prevTime = stats.thisTime;
    memcpy((char *)&_prevHdr, (char *)record, sizeof(P2d_hdr));
    return(stats);
  }

  if (version < 3.35)
    syncWord = SyncWordMask;
  else
    syncWord = StandardSyncWord;

#ifdef DEBUG
  printf("%02d:%02d:%02d.%d - ", record->hour, record->minute, record->second, record->msec);
#endif

  overload = _prevHdr.overld;

  totalLiveTime = 0.0;

  switch (controlWindow->GetConcentration()) {
    case CENTER_IN:		nBins = 64; break;
    case RECONSTRUCTION:	nBins = 128; break;
    default:			nBins = nDiodes();
    }


  // Compute frequency, which is used to convert timing words from TAS clock
  // pulses to milliseconds.  Most of sample volume is here, time comes in
  // later.
  for (size_t i = 0; i < nBins; ++i)
    sampleVolume[i] = stats.tas * sampleArea[i] * 0.001;

  // Scan record, compute tBarElapsedtime and stats.
  p = (uint32_t *)record->data;
  pSlice = _prevSlice;

  startTime = _prevTime / 1000;
  startMilliSec = _prevHdr.msec * 1000;

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
      cp->time = startTime;
      cp->msec = startMilliSec;

      cp->w = 1;        // first slice of particle is in sync word
      cp->h = 1;
      cp->area = 1;	// assume at list 1 pixel hidden in sync-word.
      cp->timeWord = 0;

      if ((ntohl(p[i]) & 0x00ffffff) != 0x00ffffff)
        cp->timeWord = ntohl(p[i]) & 0x00ffffff;
      cp->deltaTime = (uint32_t)((float)cp->timeWord * stats.frequency);
      stats.minBar = std::min(stats.minBar, cp->deltaTime);
      stats.maxBar = std::max(stats.maxBar, cp->deltaTime);

      if (!cp->timeReject)
        stats.tBarElapsedtime += cp->deltaTime;

      ++stats.nTimeBars;

      /* Determine height of particle.
       */
      i += 2;	// skip time & sync word.
      for (; i < nSlices() && p[i] != 0xffffffff && p[i] != 0x55; ++i)
        {
        ++cp->w;

        checkEdgeDiodes(cp, (const unsigned char *)&p[i]);
        cp->area += area((const unsigned char *)&p[i]);
        cp->h = std::max(height((const unsigned char *)&p[i]), cp->h);
        }

      /* If the particle becomes rejected later, we need to now much time the
       * particle consumed, so we can add it to the deadTime, so sampleVolume
       * can be reduced accordingly.
       */
      cp->liveTime = (uint32_t)((float)(cp->w + 3) * stats.frequency);

      cp->msec /= 1000;


      // This will not get caught in checkRejectionCriteria(), so do it here.
      if (controlWindow->RejectZeroAreaImage() && cp->w == 1 && cp->h == 1)
        cp->reject = true;

      totalLiveTime += checkRejectionCriteria(cp, stats);
      startMilliSec += (cp->deltaTime + cp->liveTime);

      stats.particles.push_back(cp);
      cp = new Particle();


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

stats.tBarElapsedtime += (uint32_t)(nSlices() * stats.frequency);

  if (stats.nTimeBars > 0)
    stats.meanBar = stats.tBarElapsedtime / stats.nTimeBars;

  stats.tBarElapsedtime /= 1000;	// convert to milliseconds
  stats.frequency /= 1000;


  // Compute "science" data.
  totalLiveTime /= 1000000;	// convert to seconds

  computeDerived(sampleVolume, nBins, totalLiveTime);

  // Save time for next round.
  _prevTime = stats.thisTime;
  memcpy((char *)&_prevHdr, (char *)record, sizeof(P2d_hdr));

  p = (uint32_t *)record->data;
  _prevSlice = p[1023];

  return(stats);

}	// END PROCESSPMS2D

// END PMS2D.CC
