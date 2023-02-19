
#include <cstdint>
#include <cstdio>

#include <raf/OAP.h>


class Particle
{
public:
  Particle(const char code[], FILE *outFile);
  ~Particle();

  void processParticle(uint16_t *wp, bool verbose);
  void setHeader(OAP::P2d_hdr &hdr)	{ /* writeBuffer() ? */ memcpy(&_output, &hdr, sizeof(OAP::P2d_hdr)); }
  void writeBuffer();

private:

  void printParticleHeader(uint16_t *hdr, int idx);
  void finishSlice();
  bool diodeCountCheck();

  void reset();

  FILE *_out_fp;
  unsigned char _code[8];
  OAP::P2d_rec _output;
  unsigned char *_uncompressed;

  size_t _pos, _nBits;
  uint16_t _prevID[2];
  unsigned long _prevTimeWord;

  static const size_t _nDiodes;
  static const unsigned long _syncWord;
  static const unsigned char _syncString[3];

};

