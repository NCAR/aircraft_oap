
#include <cstdint>
#include <cstdio>


class Particle
{
public:
  Particle();
  ~Particle();

  void processParticle(uint16_t *wp, bool verbose);
  void writeBuffer(FILE *out);

private:

  void printParticleHeader(uint16_t *hdr);
  void finishSlice();

  void reset();

  char *uncompressed;
  int _pos, _nBits;

};

