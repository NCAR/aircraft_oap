
#include <algorithm>

#include "particle.h"


const unsigned char Particle::_syncString[] = { 0xAA, 0xAA, 0xAA };
const unsigned long Particle::_syncWord = 0xAAAAAA0000000000;
const size_t Particle::_nDiodes = 128;

Particle::Particle(const char code[], FILE *out) : _out_fp(out), _pos(0), _nBits(0), _prevID(0), _prevTimeWord(0)
{
  memset(&_output, 0, sizeof(OAP::P2d_hdr));
  memset(_output.data, 0xFF, 4096);
  memcpy(_code, code, 2);
  _uncompressed = _output.data;
printf("_uncom=%p\n", _uncompressed);
}

Particle::~Particle()
{

}


bool Particle::diodeCountCheck()
{
  static const size_t nSlices = 4096 / _nDiodes;
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


void Particle::writeBuffer()
{
  diodeCountCheck();

if (_output.data != _uncompressed) printf("writeBuff: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);

  memcpy((unsigned char*)&_output.id, _code, 2);

  if (_pos > 0 && diodeCountCheck() == false)
  {
//    memcpy(_output.data, _uncompressed, _pos);
    fwrite(&_output, sizeof(OAP::P2d_rec), 1, _out_fp);
  }

  // reset buffer.
  _pos = 0;
  memset(_output.data, 0xFF, 4096);
}


void Particle::reset()
{
  _nBits = 0;
//  _pos = 0;

}


void Particle::finishSlice()
{
  if (_nBits > 0)
    _pos += 16;
  _nBits = 0;
if (_pos % 16 != 0) fprintf(stderr, "finishSlice: _pos not %%16 !!!!");

//  if (_pos % 16)
//    _pos += 16 - (_pos % 16);

  if (_pos >= 4096)
    writeBuffer();
}


void Particle::printParticleHeader(uint16_t *hdr)
{
  int idx = 1;
  printf("  %x - ID=%6d nSlices=%3d ", hdr[0], hdr[3], hdr[4]);

  if ((hdr[idx] & 0x0FFF) == 0)
    ++idx;

  printf("%c nWords=%4d %s",
        idx == 1 ? 'H' : 'V',
        hdr[idx] & 0x0fff,
        hdr[idx] & 0x1000 ? "- NT" : "");

printf(" - H = %04x, V = %04x", hdr[1], hdr[2]);
if (hdr[1] & 0x6000) printf("\nassert: Bad H !!!!!!!!\n");
if (hdr[2] & 0x6000) printf("\nassert: Bad V !!!!!!!!\n");
if ((hdr[1] & 0x0fff) != 0 && (hdr[2] & 0x0fff) != 0) printf("\nassert: both H&V count greater than zero !!!\n");
if ((hdr[1] & 0x0fff) == 0 && (hdr[2] & 0x0fff) == 0) printf("\nassert: both H&V count equal zero !!!\n");
}


void Particle::processParticle(uint16_t *wp, bool verbose)
{
  int i, nSlices = wp[4], nWords, sliceCnt = 0;

  uint16_t value, id = wp[3], clear, shaded;
  bool timingWord = true;

  if (verbose)
    printParticleHeader(wp);

  if (id > 0 && id != _prevID+1)
    printf("!!! Non sequential praticle ID : prev=%d, this=%d !!!\n", _prevID, id);


  _prevID = id;

  if (wp[2] != 0)
  {
    nWords = wp[2] & 0x0FFF;
    timingWord = !(wp[2] & 0x1000);
if (verbose) printf("\nStart particle V, pos=%lu\n", _pos);
if (nWords == 0) printf("assert V nWords == 0, no good\n");
  }
  else
  {
    nWords = wp[1] & 0x0FFF;
    timingWord = !(wp[1] & 0x1000);
if (verbose) printf("\nStart particle H, pos=%lu\n", _pos);
if (nWords == 0) printf("assert H nWords == 0, no good\n");
  }

  wp += 5;
  if (timingWord) nWords -= 3;

  if (_nBits != 0)
    printf("assert fail : nBits=%lu at start of particle !!!!\n", _nBits);

  for (i = 0; i < nWords; ++i)
  {
    if (_pos >= 4096)
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


if (verbose) printf(" leaving=%lu\n", _pos);
  finishSlice();
if (verbose) printf(" leaving=%lu\n", _pos);

  if (timingWord)
  {
    unsigned long tWord = ((unsigned long *)&wp[nWords])[0] & 0x0000FFFFFFFFFFFF;

    if (verbose)
      printf("\n  Timing = %lu, deltaT=%lu\n", tWord, tWord - _prevTimeWord);
    _prevTimeWord = tWord;

    memcpy(&_uncompressed[_pos+8], (unsigned char*)&_syncWord, 8);
    memcpy(&_uncompressed[_pos], (unsigned char*)&tWord, 8);
    _pos += 16;
  }
  else
    printf("\n  No timing word\n");

  if (verbose)
    printf(" end - %d/%d words, sliceCnt = %d/%d\n\n", i, nWords, sliceCnt, nSlices);

if (_output.data != _uncompressed) printf(" pp5: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);
if (verbose) printf(" leaving=%lu\n", _pos);
}

