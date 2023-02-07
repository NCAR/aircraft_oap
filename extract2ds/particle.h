
#include <cstdio>


class Particle
{
public:
  Particle();
  ~Particle();

  void writeBuffer(FILE *out);

private:

  char *uncompressed;
  int pos;
}

