
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

  void printParticleHeader(uint16_t *hdr);
  void finishSlice();
  bool diodeCountCheck();

  void reset();

  unsigned char _code[8];
  FILE *_out_fp;
  OAP::P2d_rec _output;
  unsigned char *_uncompressed;
  int _pos, _nBits;
  uint16_t _prevID;
  unsigned long _prevTimeWord;

  static const unsigned long _syncWord;
  static const unsigned char _syncString[3];

};

