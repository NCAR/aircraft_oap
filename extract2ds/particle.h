
#include <cstdint>
#include <cstdio>

#include <raf/OAP.h>


/**
 * Class to decompress SPEC run length encoded data, place it in the RAF OAP
 * format and write that buffer out when it fills the 4k.  There is one instance
 * of this class for each probe array.  So one for an HVPS, but two for the
 * 2DS, one for H channel and one for V channel.
 */
class Particle
{
public:
  Particle(const char code[], FILE *outFile);
  ~Particle();

  void processParticle(uint16_t *wp, bool verbose);

  /**
   * Set timestamp.  This should be the stamp from the previous record, which
   * would coorespond to the start of the upcoming record.  We will then ADD
   * time words to get a final time stamp.
   */
  void setHeader(const OAP::P2d_hdr &hdr);
  void writeBuffer();

private:

  void particleHeaderSanityCheck(const uint16_t *hdr);
  void printParticleHeader(const uint16_t *hdr) const;
  void finishSlice();
  void fixupTimeStamp();
  bool diodeCountCheck();

  FILE *_out_fp;
  unsigned char _code[8];	// only really need 2 bytes
  OAP::P2d_rec _output;
  unsigned char *_uncompressed;

  size_t _pos;		// write position into output buffer.
  size_t _nBits;
  uint16_t _prevID;
  unsigned long _firstTimeWord, _lastTimeWord;

  static const size_t _nDiodes;
  static const unsigned long _syncWord;
  static const unsigned char _syncString[3];

};

