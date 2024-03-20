
#include <algorithm>
#include <ctime>
#include <cctype>
#include <arpa/inet.h>

#include "config.h"
#include "particle.h"


// These are output SyncWords as opposed to input SyncWord in extract2ds.cc
const unsigned char Particle::_syncString[] = { 0xAA, 0xAA, 0xAA };
const uint64_t Particle::_syncWord = 0xAAAAAA0000000000;
const size_t Particle::_nDiodes = 128;



Particle::Particle(const char code[], FILE *out, Config *cfg) : _config(cfg), _out_fp(out), _pos(0), _nBits(0), _prevID(0), _lastTimeWord(0)
{
  memset(&_output, 0, sizeof(OAP::P2d_hdr));
  memset(_output.data, 0xFF, OAP::OAP_BUFF_SIZE);
  memcpy(_code, code, 2);
  _uncompressed = _output.data;
printf("_uncom=%p\n", _uncompressed);
}

Particle::~Particle()
{

}

/* ------------------------------------------------------------------------ */
void Particle::setHeader(const OAP::P2d_hdr &hdr, uint64_t ltw)
{
//  printf("setHeader --- %d/%02d/%02d %02d:%02d:%02d.%03d\n", ntohs(_compressedTime.year),
//	ntohs(_compressedTime.month), ntohs(_compressedTime.day), ntohs(_compressedTime.hour),
//	ntohs(_compressedTime.minute), ntohs(_compressedTime.second), ntohs(_compressedTime.msec));

  _prevDAQtime = _thisDAQtime;
  _prevMsec = _thisMsec;

  memcpy(&_compressedTime, &hdr, sizeof(OAP::P2d_hdr));
  memcpy(&_output, &hdr, sizeof(OAP::P2d_hdr));
  _lastTimeWord = ltw;	// last timing word in the new record


  struct tm tor;
  tor.tm_year = ntohs(_compressedTime.year);
  tor.tm_mon = ntohs(_compressedTime.month)-1;
  tor.tm_mday = ntohs(_compressedTime.day);
  tor.tm_hour = ntohs(_compressedTime.hour);
  tor.tm_min = ntohs(_compressedTime.minute);
  tor.tm_sec = ntohs(_compressedTime.second);
  _thisDAQtime = mktime(&tor);
  _thisMsec = ntohs(_compressedTime.msec);
}

/* ------------------------------------------------------------------------ */
void Particle::fixupTimeStamp()
{
//  double freq = (double)_config->Resolution() / (1.0e+6 * ntohs(_compressedTime.tas));	// seconds
  double freq = (double)_config->Resolution() / (1.0e+3 * ntohs(_compressedTime.tas));	// msec
//  double freq = (double)_config->Resolution() / (ntohs(_compressedTime.tas));	// usec


  uint64_t deltaT = _lastTimeWord;

  if (_lastTimeWord < _thisTimeWord && _config->DataFormat() == Type32)
  {
    deltaT += 4294967295;	// Rolled over
    printf("tWord rollover = %10lu %10lu %lu\n", _thisTimeWord, _lastTimeWord, deltaT);
  }

  deltaT -= _thisTimeWord;
  deltaT += _posFTW + _posLTW;	// add leading and trailing slices before/after timing words


  time_t tm_out = _thisDAQtime;
  int msec = (int)(freq * deltaT);

  if (msec <= _thisMsec)
    msec = _thisMsec - msec;
  else
  {
    msec -= _thisMsec;
    tm_out = _thisDAQtime - 1 - (msec / 1000);
    msec = 1000 - (msec % 1000);
  }

//printf("fix in %04u/%02u/%02u %02u:%02u:%02u.%03u - %lu\n", tor.tm_year, tor.tm_mon, tor.tm_mday, tor.tm_hour, tor.tm_min, tor.tm_sec, msec, deltaT);

  struct tm *tor_out = gmtime(&tm_out);
//printf("fix time_t = %lu.%03d %lu.%03d %lu.%03d\n", _prevDAQtime, _prevMsec, tm_out, msec, _thisDAQtime, _thisMsec);
//printf("fix tWord = %10lu %10lu %d\n", _thisTimeWord, _lastTimeWord, msec );

//printf("fix io %04u/%02u/%02u %02u:%02u:%02u.%03u\n", tor_out->tm_year, tor_out->tm_mon, tor_out->tm_mday, tor_out->tm_hour, tor_out->tm_min, tor_out->tm_sec, msec);

  _output.year = htons(tor_out->tm_year);
  _output.month = htons(tor_out->tm_mon+1);
  _output.day = htons(tor_out->tm_mday);
  _output.hour = htons(tor_out->tm_hour);
  _output.minute = htons(tor_out->tm_min);
  _output.second = htons(tor_out->tm_sec);
  _output.msec = htons(msec);
}

/* ------------------------------------------------------------------------ */
bool Particle::diodeCountCheck()
{
  static const size_t nSlices = OAP::OAP_BUFF_SIZE / _nDiodes;
  static const size_t nBytes = _nDiodes / 8;

  bool rejectRecord = false;
  size_t cnts[_nDiodes], nParticles = 0;

  memset(cnts, 0, sizeof(cnts));

  for (size_t i = 0; i < nSlices; ++i)
  {
    if (memcmp(&_output.data[(i * nBytes) + 13], _syncString, 3) == 0)
    {
      ++nParticles;
      continue;
    }

    for (size_t j = 0; j < nBytes; ++j)
    {
      for (size_t k = 0; k < 8; ++k)
        if (((_output.data[i*nBytes+j] << k) & 0x80) == 0)
          ++cnts[j*8+k];
    }
  }

  /* Count how many diodes were active in the buffer.  The idea is if only 1
   * diode was active, then we have a stuck bit.
   */
  size_t cnt = 0;
  for (size_t i = 0; i < _nDiodes; ++i) {
    if (cnts[i] > 0)
      ++cnt;
  }


//  if (cnt < _nDiodes * 0.05)
  if (cnt < 11)
    rejectRecord = true;

  if (nParticles < 2)
    rejectRecord = true;

  return rejectRecord;
}

/* ------------------------------------------------------------------------ */
void Particle::writeBuffer()
{
  diodeCountCheck();

if (_output.data != _uncompressed) printf("writeBuff: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);

  memcpy((unsigned char*)&_output.id, _code, 2);

//  if (_pos > 0 && diodeCountCheck() == false)
  if (_pos > 0 )
  {
    fixupTimeStamp();
    fwrite(&_output, sizeof(OAP::P2d_rec), 1, _out_fp);
  }

  // reset buffer.
  _pos = 0;
  _posFTW = _posLTW = 0;
  memset(_output.data, 0xFF, OAP::OAP_BUFF_SIZE);
}

/* ------------------------------------------------------------------------ */
void Particle::finishSlice()
{
  if (_nBits > 0)
    _pos += 16;
  _nBits = 0;

  if (_pos % 16 != 0) fprintf(stderr, "finishSlice: _pos not %%16 !!!!");

  if (_pos >= OAP::OAP_BUFF_SIZE)
    writeBuffer();
}

/* ------------------------------------------------------------------------ */
void Particle::printParticleHeader(const uint16_t *hdr) const
{
  int idx = 1;

  if ((hdr[idx] & 0x0FFF) == 0)  // this is H vs V
    ++idx;

  printf("  %x - ID=%6d nSlices=%3d ", hdr[0], hdr[3], hdr[4]);

  printf("%c nWords=%4d %s",
        idx == 1 ? 'H' : 'V',
        hdr[idx] & 0x0fff,
        hdr[idx] & 0x1000 ? "- NT" : "");
}

/* ------------------------------------------------------------------------ */
void Particle::particleHeaderSanityCheck(const uint16_t *hdr)
{
//printf(" - H = %04x, V = %04x", hdr[1], hdr[2]);
  if (hdr[1] & 0x6000) printf("\nassert: Bad H !!!!!!!!\n");
  if (hdr[2] & 0x6000) printf("\nassert: Bad V !!!!!!!!\n");
  if ((hdr[1] & 0x0fff) != 0 && (hdr[2] & 0x0fff) != 0) printf("\nassert: both H&V count greater than zero !!!\n");
  if ((hdr[1] & 0x0fff) == 0 && (hdr[2] & 0x0fff) == 0) printf("\nassert: both H&V count equal zero !!!\n");
}

/* ------------------------------------------------------------------------ */
void Particle::processParticle(uint16_t *wp, bool verbose)
{
  int i, nSlices = wp[nSLICES], nWords, sliceCnt = 0;

  uint16_t value, id = wp[PID], clear, shaded;
  bool timingWord = true, oflow = false;
  if (verbose) printf(" --- processParticle ---\n");
  particleHeaderSanityCheck(wp);

  if (verbose)
    printParticleHeader(wp);

  if (id > 0)
  {
    if (id == _prevID)
      printf("  : multi packet particle\n");
    else
    if (id != _prevID+1)
      printf("Non sequential particle ID %c : prev=%d, this=%d !!!\n", _code[1], _prevID, id);
  }

  _prevID = id;

  if (wp[V_CHN] != 0)
  {
    nWords = wp[V_CHN] & 0x0FFF;
    timingWord = !(wp[V_CHN] & 0x1000);
    oflow = wp[V_CHN] & 0x8000;
if (verbose) printf("\nStart particle V, pos=%lu\n", _pos);
if (nWords == 0) printf("assert V nWords == 0, no good\n");
  }
  else
  {
    nWords = wp[H_CHN] & 0x0FFF;
    timingWord = !(wp[H_CHN] & 0x1000);
    oflow = wp[H_CHN] & 0x8000;
if (verbose) printf("\nStart particle H, pos=%lu\n", _pos);
if (nWords == 0) printf("assert H nWords == 0, no good\n");
  }

  wp += 5;
  if (timingWord)
  {
    if (_code[0] == 'H' && isdigit(_code[1]))
      nWords -= 2;	// Type32 (HVPS)
    else
      nWords -= 3;	// Type48 (F2DS)
  }

  if (_nBits != 0)
    printf("assert fail : nBits=%lu at start of particle !!!!\n", _nBits);

  for (i = 0; i < nWords; ++i)
  {
    if (_pos >= OAP::OAP_BUFF_SIZE)
      writeBuffer();

    if (_pos > 4080)
      printf("Warning, _pos = %lu\n", _pos);

    if (wp[i] == 0x4000)	// Fully shadowed slice.
    {
if (verbose) printf(" full shade, _pos = %lu, i = %d\n", _pos, i);
      memset(&_uncompressed[_pos], 0, 16);
      _pos += 16;

      if (false)
      {
        printf("\n  %2d 1 0x", sliceCnt);
        for (size_t j = 0; j < _nDiodes; ++j)
          printf("1");
      }
      _nBits = 0;
      ++sliceCnt;
      continue;
    }

    if (wp[i] == 0x7FFF)	// Uncompressed slice.
    {
if (verbose) printf(" uncompressed, _pos = %lu, i = %d\n", _pos, i);
      memcpy(&_uncompressed[_pos], (char *)&wp[i+1], 16);
      _pos += 16;

      if (false)
      {
        printf("\n  %2d u 0x", sliceCnt);
        for (int j = 0; j < 8; ++j)
          for (int k = 0; k < 16; ++k)
            printf("%d", !((wp[i+1+j] << k) & 0x8000));
      }
      i += 8;
      _nBits = 0;
      ++sliceCnt;
      continue;
    }

    if (wp[i] & 0x4000)				// First word of slice
    {
if (verbose) printf(" first word of slice, pos=%lu, i = %d", _pos, i);
      finishSlice();

      ++sliceCnt;
      _nBits = 0;
      if (verbose)
        printf(", sliceCnt = %d\n", sliceCnt);
    }

    if ((clear = (wp[i] & 0x007F)) > 0)		// Number of clear pixels
    {
if (verbose) printf("  clear=%u, pos=%lu, i = %d\n", clear, _pos, i);
      _nBits += clear;

      if (false)
      {
        for (int j = 0; j < clear; ++j)
          printf("0");
      }
    }

    if ((shaded = (wp[i] & 0x3F80) >> 7) > 0)	// Number of shadowed pixels
    {
      size_t BN = _nBits / 8;
      size_t bN = _nBits % 8;
      size_t res = 8 - bN;
if (verbose) printf("  shaded=%u, pos=%lu\n", shaded, _pos);
if (verbose) printf("  nBits=%lu, BN=%lu, bN=%lu, res=%lu\n", _nBits, BN, bN, res);
      if (_nBits+shaded > _nDiodes)
      {
        printf("assert: _nBits+shaded = %lu\n", _nBits+shaded);
        continue;
      }
if (_output.data != _uncompressed) printf(" pp3: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);

      _nBits += shaded;
      value = shaded;
      if (bN)
      {
//printf("%x %d %x\n", _uncompressed[_pos + BN], (0xff >> res), _uncompressed[_pos + BN] & (0xff >> res));
        unsigned char mask = 0xff << value;

        int n = std::min((size_t)value, res);
        for (size_t j = 0; j < bN; ++j)
          mask = (mask << 1) | 0x01;
        _uncompressed[_pos + BN] &= mask;
        value -= n;
        if (value > 0) ++BN;
      }

if (_output.data != _uncompressed) printf(" pp3.1: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);
      while (value >= 8)
      {
//printf("  loop, value=%d, _pos=%d, BN=%d\n", value, _pos, BN);
        _uncompressed[_pos + BN] = 0x00;
        value -= 8;
        ++BN;
      }
if (_output.data != _uncompressed) printf(" pp3.2: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);

      if (value > 0)
      {
        unsigned char mask = 0xfe;
        mask <<= (value-1);
        _uncompressed[_pos + BN] &= mask;
      }

      if (false)
      {
        for (int j = 0; j < value; ++j)
          printf("1");
      }
    }
  }


  finishSlice();

  if (_posFTW == 0)
    _posFTW = _pos / 16;

  if (timingWord)
  {
    uint64_t tWord;
    if (_config->DataFormat() == Type48)
      tWord = ((uint64_t *)&wp[nWords])[0] & Type48_TimingWordMask;
    else
      tWord = ((uint64_t)wp[nWords] << 16) + wp[nWords+1];

    if (verbose)
      printf("\n   Timing = %10lu, deltaT=%lu\n", tWord, (tWord - _lastTimeWord));
    _thisTimeWord = tWord;

    memcpy(&_uncompressed[_pos+8], (unsigned char*)&_syncWord, 8);
    memcpy(&_uncompressed[_pos], (unsigned char*)&tWord, 8);
    _pos += 16;
  }
  else
    if (verbose) printf("\n   No timing word\n");

  _posLTW = (4096 - _pos) / 16;

  if (verbose)
    printf("   end - %d/%d words, sliceCnt = %d/%d\n\n", i, nWords, sliceCnt, nSlices);

if (_output.data != _uncompressed) printf(" pp5: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);
}

