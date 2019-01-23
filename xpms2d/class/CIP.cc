/*
-------------------------------------------------------------------------
OBJECT NAME:	CIP.cc

FULL NAME:	DMT CIP/PIP Probe Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "CIP.h"
#include "UserConfig.h"

const unsigned long long CIP::SyncWord = 0xAAAAAAAAAAAAAAAALL;

/* -------------------------------------------------------------------- */
CIP::CIP(UserConfig *cfg, const char xml_entry[], int recSize) : Probe(Probe::CIP, cfg, xml_entry, recSize, 64)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

  dmt_init();

printf("CIP::OAP id=%s, name=%s, resolution=%zu, armWidth=%f, eaw=%f\n", _code, _name.c_str(), _resolution, _armWidth, _eaw);
}

/* -------------------------------------------------------------------- */
CIP::CIP(UserConfig *cfg, const char name[]) : Probe(Probe::CIP, cfg, name, 64)
{
  _resolution = 25;

  dmt_init();

printf("CIP::PADS id=%s, name=%s, resolution=%zu, armWidth=%f, eaw=%f\n", _code, _name.c_str(), _resolution, _armWidth, _eaw);
}

void CIP::dmt_init()
{
  _clockMhz = 8;
  _nResidualBytes = 0;
  _carryOver = false;

  if (_code[0] == 'C')  // CIP
    _armWidth = 100.0;

  if (_code[0] == 'P')  // PIP
    _armWidth = 260.0;

  SetSampleArea();
}


/* -------------------------------------------------------------------- */
bool CIP::isSyncWord(const unsigned char *p)
{
  return memcmp(p, (void *)&SyncWord, sizeof(SyncWord)) == 0;
}

/* -------------------------------------------------------------------- */
struct recStats CIP::ProcessRecord(const P2d_rec *record, float version)
{
  int		startTime, overload = 0;
  const unsigned char	*p;
  unsigned long		startMilliSec;
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
  printf("%c%c %02d:%02d:%02d.%d - \n", ((char *)record)[0], ((char *)record)[1], record->hour, record->minute, record->second, record->msec);
#endif

  totalLiveTime = 0.0;

  switch (_userConfig->GetConcentration()) {
    case CENTER_IN:
    case RECONSTRUCTION:	_numBins = 128; break;
    default:			_numBins = nDiodes();
    }

  for (size_t i = 0; i < NumberBins(); ++i)
    sampleVolume[i] = stats.tas * sampleArea[i] * 0.001;

  unsigned char image[16000];
  memset(image, 0, 16000);
  _nSlices = uncompress(image, record->data, 4096);

  // Scan record, compute tBarElapsedtime and stats.
  p = (unsigned char *)image;

  startTime = _prevTime / 1000;
  startMilliSec = _prevHdr.msec;

  // Loop through all slices in record.
  for (size_t i = 0; i < nSlices(); ++i, p += sizeof(long long))
  {
    /* Have particle, will travel.
     */
    if (isSyncWord(p) || _carryOver)
    {
      if (++i >= nSlices())	// Last slice is a timeWord
      {
        _carryOver = true;
        continue;
      }
      if (!_carryOver)
        p += 8;

      _carryOver = false;


      unsigned long long thisTimeWord = TimeWord_Microseconds(&p[2]);

      if (firstTimeWord == 0)
        firstTimeWord = thisTimeWord;

      // Start a particle.
      cp = new Particle();
      cp->timeWord = thisTimeWord;
      unsigned long msec = startMilliSec + ((thisTimeWord - firstTimeWord) / 1000);
      cp->time = startTime + (msec / 1000);
      cp->msec = msec % 1000;
      cp->deltaTime = cp->timeWord - _prevTimeWord;
      cp->timeWord /= 1000;	// Store as millseconds... can move to microseconds...
      totalLiveTime += checkRejectionCriteria(cp, stats);
      if ((p[7] & 0x01) == 0)
        cp->dofReject = true;
      stats.particles.push_back(cp);

      _prevTimeWord = thisTimeWord;

      ++stats.nTimeBars;
      stats.minBar = std::min(stats.minBar, cp->deltaTime);
      stats.maxBar = std::max(stats.maxBar, cp->deltaTime);
      continue;
    }


    if (!cp || isBlankSlice(p))
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

}	// END PROCESSCIP

/* -------------------------------------------------------------------- */
long long CIP::TimeWord_Microseconds(const unsigned char *p) const
{
  long long t = *(long long *)p & 0x000000FFFFFFFFFFLL;
  long long output;

  int hour = (t >> 35) & 0x001F;
  int minute = (t >> 29) & 0x003F;
  int second = (t >> 23) & 0x003F;
  int msec = (t >> 13) & 0x03FF;
  int usec = t & 0x1FFF;
  output = (hour * 3600 + minute * 60 + second);
  output *= 1000000;
  output += msec * 1000;
  output += usec / _clockMhz;

#ifdef DEBUG
  printf("%02d:%02d:%02d.%03d - (%lld)\n", hour, minute, second, msec, output);
#endif

  return output;
}

/* -------------------------------------------------------------------- */
// DMT CIP/PIP probes are run length encoded.  Decode here.
// Duplicated in class/MainCanvas.cc, not consolidated at this time due to static variables.
size_t CIP::uncompress(unsigned char *dest, const unsigned char src[], int nbytes)
{
  int d_idx = 0, i = 0;

  if (_nResidualBytes)
  {
    memcpy(dest, residualBytes, _nResidualBytes);
    d_idx = _nResidualBytes;
    _nResidualBytes = 0;
  }

  for (; i < nbytes; ++i)
  {
    unsigned char b = src[i];

    int nBytes = (b & 0x1F) + 1;

    if ((b & 0x20))     // This is a dummy byte; for alignment purposes.
    {
      continue;
    }

    if ((b & 0xE0) == 0)	// Unencoded data, copy it verbatim
    {
      memcpy(&dest[d_idx], &src[i+1], nBytes);
      d_idx += nBytes;
      i += nBytes;
    }
    else
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
    if ( isSyncWord(&dest[i]) )
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
    _nResidualBytes = d_idx % 8;
    memcpy(residualBytes, &dest[idx], _nResidualBytes);
  }

  SwapBytes(dest, d_idx / 8);	// Return number of slices.
  return d_idx / 8;     // return number of slices.
}

void CIP::SwapBytes(unsigned char *cp, size_t nSlices)
{
  unsigned char tmp[16];
  for (size_t i = 0; i < nSlices; ++i, cp += sizeof(long long))
  {
    // Don't swap sync or timing words.
    if (memcmp(cp, (unsigned char *)&SyncWord, 8) == 0)
    {
      i++; cp += sizeof(long long);
      continue;
    }
    for (size_t j = 0; j < 8; ++j)
      tmp[j] = cp[7-j];
    memcpy(cp, tmp, 8);
  }
}

// END CIP.CC
