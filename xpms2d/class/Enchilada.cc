/*
-------------------------------------------------------------------------
OBJECT NAME:	Enchilada.cc

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2018
-------------------------------------------------------------------------
*/

#include "Enchilada.h"
#include <raf/OAPUserConfig.h>

extern OAP::UserConfig	userConfig;

/* -------------------------------------------------------------------- */
Enchilada::Enchilada(const Widget parent) : TextWindow(parent, "enchilada")
{
}

/* -------------------------------------------------------------------- */
void Enchilada::AddLineItem(int cnt, OAP::Particle *cp)
{
  int   h, m, s;
  char	buffer[512];

  if (cp == 0)
    return;

  if (cnt == 0) // Print title.
  {
    Append(" #     Time       timeWord  iy  ix  ia    dt  rj dofRej");
    switch (userConfig.GetConcentration())
    {
      case OAP::BASIC:
	Append("theoretical");
	break;

      case OAP::ENTIRE_IN:
	Append("Entire-in");
	break;

      case OAP::CENTER_IN:
	Append("Center-in");
	break;

      case OAP::RECONSTRUCTION:
        Append("Reconstruct");
	break;
    }
    Append("\n");
  }


  h = cp->time / 3600;
  m = (cp->time - (h*3600)) / 60;
  s = cp->time - (h*3600) - (m*60);

  // Particle #, time stamp, timeword, reject, h, w, a
  sprintf(buffer, "%03d %02d:%02d:%02d.%03ld  %8lu %3zu %3zu %3zu %6u %2d %2d\n",
        cnt, h, m, s, cp->msec, cp->timeWord, cp->h, cp->w, cp->area,
	cp->deltaTime, cp->reject, cp->dofReject);

  Append(buffer);

}	/* END ADDLINEITEM */

/* END ENCHILADA.CC */
