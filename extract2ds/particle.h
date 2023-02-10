
#include <cstdint>
#include <cstdio>

#include <raf/OAP.h>


class Particle
{
public:
  Particle();
  ~Particle();

  void processParticle(uint16_t *wp, bool verbose);
  void setHeader(OAP::P2d_hdr &hdr)	{ memcpy(&_output, &hdr, sizeof(OAP::P2d_hdr)); }
  void writeBuffer(FILE *out);

private:

  void printParticleHeader(uint16_t *hdr);
  void finishSlice();

  void reset();

  OAP::P2d_rec _output;
  unsigned char *_uncompressed;
  int _pos, _nBits;

};

