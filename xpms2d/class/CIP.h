/*
-------------------------------------------------------------------------
OBJECT NAME:	CIP.h

FULL NAME:	DMT CIP/PIP probes

COPYRIGHT:	University Corporation for Atmospheric Research, 2018
-------------------------------------------------------------------------
*/

#ifndef _CIP_H_
#define _CIP_H_

#include "Probe.h"


/* -------------------------------------------------------------------- */
class CIP : public Probe
{

public:
  /**
   * Probe constructor for OAP files.  For CIP/PIP, this means the data has
   * been run through the pads2oap converter.
   */
  CIP(UserConfig *cfg, const char xml_string[], int recSize);

  /**
   * Probe constructor for no file header.  Would like to make this open 
   * a PADS file in the future.
   */
  CIP(UserConfig *cfg, const char name[]);


  struct recStats ProcessRecord(const P2d_rec * record, float version);

  bool isSyncWord(const unsigned char *p);

  /// DMT CIP/PIP probes are run length encoded.  Decode here.
  size_t uncompress(unsigned char *dest, const unsigned char src[], int nbytes);

  /**
   * Swap data to big endian after being uncompressed.
   */
  static void SwapBytes(unsigned char *cp, size_t nSlices);

  static const unsigned long long SyncWord;


protected:

  void dmt_init();

  long long TimeWord_Microseconds(const unsigned char *p) const;

  /**
   * These are for the decompress algorithm.  Bytes to be carried over to next
   * decompress call.  These are already uncompressed.
   */
  size_t	_nResidualBytes;
  unsigned char	residualBytes[16];

  /**
   * Used for when sync word is last word of a buffer, and timing word will
   * be first slice in the next buffer.
   */
  bool	_carryOver;



};

#endif
