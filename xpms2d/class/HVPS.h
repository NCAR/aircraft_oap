/*
-------------------------------------------------------------------------
OBJECT NAME:	PMS2D.h

FULL NAME:	Old School 32 diode PMS2D probe data class

COPYRIGHT:	University Corporation for Atmospheric Research, 2018
-------------------------------------------------------------------------
*/

#ifndef _HVPS_H_
#define _HVPS_H_

#include "Probe.h"


/* -------------------------------------------------------------------- */
class HVPS : public Probe
{

public:
  /**
   * Probe conctructor for ADS2 header.
   */
  HVPS(Header *hdr, const Pms2 *p, int cnt);
  /**
   * Probe constructor for new PMS2D data files.  New means starting
   * in 2007 with PACDEX project.
   */
  HVPS(const char xml_string[], int recSize);
  /**
   * Probe constructor for no file header.  Univ Wyoming.
   */
  HVPS(const char hdr[]);


  struct recStats ProcessRecord(const P2d_rec * record, float version);

  bool isSyncWord(const unsigned char *p);


protected:

  void init();

  static const size_t lowerMask, upperMask;

};

#endif
