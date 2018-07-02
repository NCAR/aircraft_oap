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
   * Probe constructor for new PMS2D data files.  New means starting
   * in 2007 with PACDEX project.
   */
  CIP(const char xml_string[], int recSize);

  /**
   * Probe constructor for no file header.  Would like to make this open 
   * a PADS file in the future.
   */
  CIP(const char name[]);


  struct recStats ProcessRecord(const P2d_rec * record, float version);

  bool isSyncWord(const unsigned char *p);

  /// DMT CIP/PIP probes are run length encoded.  Decode here.
  size_t uncompress(unsigned char *dest, const unsigned char src[], int nbytes);

  static const unsigned long long SyncWord;


protected:

  void init();

  long long TimeWord_Microseconds(unsigned long long slice);


};

#endif
