
#include <cstdio>


class Particle
{
public:
  Particle();
  ~Particle();

  void writeBuffer(FILE *out);

private:

  void printParticleHeader(uint16_t *hdr);

  void reset();

  char *uncompressed;
  int _pos, _nBits;

};

