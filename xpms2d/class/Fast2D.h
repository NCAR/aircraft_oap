/*
-------------------------------------------------------------------------
OBJECT NAME:	Fast2D.h

FULL NAME:	NCAR/RAF Fast2D.

COPYRIGHT:	University Corporation for Atmospheric Research, 2018
-------------------------------------------------------------------------
*/

#ifndef _FAST2D_H_
#define _FAST2D_H_

#include "Probe.h"


/* -------------------------------------------------------------------- */
class Fast2D : public Probe
{

public:
  Fast2D(const char xml_string[], int recSize);

  struct recStats ProcessRecord(const P2d_rec * record, float version);

  bool isSyncWord(const unsigned char *p);
  bool isOverloadWord(const unsigned char *p);

  static const unsigned char SyncString[3];
  static const unsigned char OverldString[3];


protected:

  void f2d_init();

  unsigned long long
  TimeWord_Microseconds(const unsigned char *p);

  /**
   * Clock speed of the probe.  Version one was 12Mhz.  Versions 2 was 33Mhz
   * This is used to turn the timing word into millisconds.
   */
  unsigned int	_clockMhz;

};

#endif
