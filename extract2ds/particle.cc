
#include <algorithm>

#include "particle.h"


Particle::Particle(const char code[], FILE *out) : _out_fp(out), _pos(0), _nBits(0)
{
  memset(_code, 0, sizeof(_code));
  memcpy(_code, code, 2);
  _uncompressed = _output.data;
printf("_uncom=%p\n", _uncompressed);
}

Particle::~Particle()
{

}

void Particle::writeBuffer()
{

if (_output.data != _uncompressed) printf("writeBuff: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);
  memcpy((unsigned char*)&_output.id, _code, 2);
printf("%x\n", _output.id);
printf("uncomp=%p _pos=%d\n", _uncompressed, _pos);
  if (_pos > 0)
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
}


void Particle::processParticle(uint16_t *wp, bool verbose)
{
  int i, nSlices = wp[4], nWords, sliceCnt = 0;

  uint16_t value, id = wp[3];
  bool timingWord = true;
  static uint16_t prevID = 0;

  if (verbose)
    printParticleHeader(wp);

  if (id > 0 && id != prevID+1)
    printf("!!! Non sequential praticle ID : prev=%d, this=%d !!!\n", prevID, id);


  prevID = id;
  reset();

  if (wp[2] != 0)
  {
    nWords = wp[2] & 0x0FFF;
    timingWord = !(wp[2] & 0x1000);
printf("\nStart particle V, pos=%d\n", _pos);
  }
  else
  {
    nWords = wp[1] & 0x0FFF;
    timingWord = !(wp[1] & 0x1000);
printf("\nStart particle H, pos=%d\n", _pos);
  }

  wp += 5;
  if (timingWord) nWords -= 3;


  for (i = 0; i < nWords; ++i)
  {
    if (_pos >= 4096)
      writeBuffer();

    if (_pos >= 4050)
      printf("Warning, _pos = %d\n", _pos);

    if (wp[i] == 0x4000)	// Fully shadowed slice.
    {
printf(" full shade\n");
      memset(&_uncompressed[_pos], 0, 16);
      _pos += 16;

      if (false)
      {
        printf("\n  %2d 1 0x", sliceCnt);
        for (int j = 0; j < 128; ++j)
          printf("1");
      }
      _nBits = 0;
      ++sliceCnt;
      continue;
    }

    if (wp[i] == 0x7FFF)	// Uncompressed slice.
    {
printf(" uncompressed\n");
      memcpy(&_uncompressed[_pos], (char *)&wp[i+1], 16);
      _pos += 16;

      if (false)
      {
        printf("\n  %2d u 0x", sliceCnt);
        for (int j = 0; j < 8; ++j)
          for (int k = 0; k < 16; ++k)
            printf("%d", !((wp[i+1+j] << k) & 0x8000));
      }
      i += 15;
      _nBits = 0;
      ++sliceCnt;
      continue;
    }

    if (wp[i] & 0x4000)				// First word of slice
    {
printf(" first word of slice, pos=%d\n", _pos);
      finishSlice();
printf(" first word of slice, pos=%d\n", _pos);

      ++sliceCnt;
      _nBits = 0;
      if (verbose)
        printf("\n  %2d 0x", sliceCnt);
    }

    if ((value = (wp[i] & 0x007F)) > 0)		// Number of clear pixels
    {
printf(" clear=%d, pos=%d\n", value, _pos);
      _nBits += value;

      if (false)
      {
        for (int j = 0; j < value; ++j)
          printf("0");
      }
    }

    if ((value = (wp[i] & 0x3F80) >> 7) > 0)	// Number of shadowed pixels
    {
      int BN = _nBits / 8;
      int bN = _nBits % 8;
      int res = 8 - bN;
printf(" shaded=%d, pos=%d\n", value, _pos);
printf(" nBits=%d, BN=%d, bN=%d, res=%d\n", _nBits, BN, bN, res);
if (_output.data != _uncompressed) printf(" pp3: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);

      _nBits += value;
      if (bN)
      {
printf("%x %d %x\n", _uncompressed[_pos + BN], (0xff >> res), _uncompressed[_pos + BN] & (0xff >> res));
        unsigned char mask = 0xff << value;

        int n = std::min((int)value, res);
        for (int j = 0; j < bN; ++j)
          mask = (mask << 1) | 0x01;
        _uncompressed[_pos + BN] &= mask;
printf("mask = %x\n", mask);
        value -= n;
        if (value > 0) ++BN;
      }

if (_output.data != _uncompressed) printf(" pp3.1: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);
      while (value >= 8)
      {
printf("  loop, value=%d, _pos=%d, BN=%d\n", value, _pos, BN);
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


printf(" leaving=%d\n", _pos);
  finishSlice();
printf(" leaving=%d\n", _pos);

  if (timingWord)
  {
    static unsigned long sync = 0xAAAAAA0000000000;
    static unsigned long prevTimeWord = 0;
    unsigned long tWord = ((unsigned long *)&wp[nWords])[0] & 0x0000FFFFFFFFFFFF;

    if (verbose)
      printf("\n  Timing = %lu, deltaT=%lu\n", tWord, tWord - prevTimeWord);
    prevTimeWord = tWord;

if (_pos > 4096) printf("_pos @ timeword assert!!!!!!!!!\n");
    memcpy(&_uncompressed[_pos+8], (unsigned char*)&sync, 8);
    memcpy(&_uncompressed[_pos], (unsigned char*)&tWord, 8);
    _pos += 16;
  }
  else
    printf("\n  No timing word\n");

  if (verbose)
    printf(" end - %d/%d words, sliceCnt = %d/%d\n\n", i, nWords, sliceCnt, nSlices);

if (_output.data != _uncompressed) printf(" pp5: un != out %p - %p !!!!!!!\n", _output.data, _uncompressed);
printf(" leaving=%d\n", _pos);
}

