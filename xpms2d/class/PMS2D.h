/*
-------------------------------------------------------------------------
OBJECT NAME:	PMS2D.h

FULL NAME:	Old School 32 diode PMS2D probe data class

COPYRIGHT:	University Corporation for Atmospheric Research, 2018
-------------------------------------------------------------------------
*/

#ifndef _PMS2D_H_
#define _PMS2D_H_

#include "Probe.h"


/* -------------------------------------------------------------------- */
class PMS2D : public Probe
{

public:
  /**
   * Probe conctructor for ADS2 header.
   */
  PMS2D(UserConfig *cfg, Header *hdr, const Pms2 *p, int cnt);
  /**
   * Probe constructor for new PMS2D data files.  New means starting
   * in 2007 with PACDEX project.
   */
  PMS2D(UserConfig *cfg, const char xml_string[], int recSize);
  /**
   * Probe constructor for no file header.  Univ Wyoming.
   */
  PMS2D(UserConfig *cfg, const char hdr[]);


  struct recStats ProcessRecord(const P2d_rec * record, float version);

  bool isSyncWord(const unsigned char *p);

  static const uint32_t StandardSyncWord;
  static const uint32_t SyncWordMask;


protected:

  void pms2d_init();

  uint32_t	_prevSlice;

};

#endif
