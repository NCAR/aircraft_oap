

#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include <cstdint>
#include <cstdio>

#include <raf/OAP.h>

#include "spec.h"


class Config;

/**
 * Class to decompress SPEC run length encoded data, place it in the RAF OAP
 * format and write that buffer out when it fills the 4k.  There is one instance
 * of this class for each probe array.  So one for an HVPS, but two for the
 * 2DS, one for H channel and one for V channel.
 */
class Particle
{
public:
  Particle(const char code[], FILE *outFile, Config *cfg);
  ~Particle();

  void processParticle(uint16_t *wp, bool verbose);

  /**
   * Set timestamp.  This should be the stamp from the previous record, which
   * would coorespond to the start of the upcoming record.  We will then ADD
   * time words to get a final time stamp.
   */
  void setHeader(const OAP::P2d_hdr &hdr, uint64_t ltw);
  void writeBuffer();


private:

  void particleHeaderSanityCheck(const uint16_t *hdr);
  void printParticleHeader(const uint16_t *hdr) const;
  void finishSlice();
  void fixupTimeStamp();
  bool diodeCountCheck();

  Config *_config;

  FILE *_out_fp;
  unsigned char _code[8];	// only really need 2 bytes
  OAP::P2d_rec _compressedTime;	// Header/timestamp from current compressed record.
  OAP::P2d_rec _output;		//   ...which gets compied into here and modified.
  unsigned char *_uncompressed;

  size_t _pos;		// write position into output buffer.
  size_t _nBits;

  // Previous particle ID or seq number.  For determining multi-packet particles
  uint16_t _prevID;
  // Timing word of current particle we are processing
  uint64_t _thisTimeWord;
  // Last timing word in the _compressedBuffer
  uint64_t _lastTimeWord;

  // positions in output buffer of first and last timing words...so can add padding
  // to deltaT of leading and trailing slices.
  int  _posFTW, _posLTW;


  // Previous and this time stamps by datasystem.  Providing bounding start
  // and end time records we generate.
  time_t _prevDAQtime, _thisDAQtime;
  int _prevMsec, _thisMsec;

  size_t _resolution;

  static const size_t _nDiodes;
  static const uint64_t _syncWord;
  static const unsigned char _syncString[3];

};

#endif
