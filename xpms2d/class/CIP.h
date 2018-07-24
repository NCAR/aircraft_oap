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


};

#endif
