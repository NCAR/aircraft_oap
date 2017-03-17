/*
-------------------------------------------------------------------------
OBJECT NAME:	Enchilada.cc

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2009
-------------------------------------------------------------------------
*/

#include "Enchilada.h"
#include "ControlWindow.h"

extern ControlWindow	*controlWindow;

/* -------------------------------------------------------------------- */
Enchilada::Enchilada(const Widget parent) : TextWindow(parent, "enchilada")
{
}

/* -------------------------------------------------------------------- */
void Enchilada::AddLineItem(int cnt, Particle *cp)
{
  int   h, m, s;

  if (cp == 0)
    return;

  if (cnt == 0) // Print title.
  {
    Append(" #     Time       timeWord  rj  iy  ix  ia    dt  ");
    switch (controlWindow->GetConcentration())
    {
      case BASIC:
	Append("theoretical");
	break;

      case ENTIRE_IN:
	Append("Entire-in");
	break;

      case CENTER_IN:
	Append("Center-in");
	break;

      case RECONSTRUCTION:
        Append("Reconstruct");
	break;
    }
    Append("\n");
  }


  h = cp->time / 3600;
  m = (cp->time - (h*3600)) / 60;
  s = cp->time - (h*3600) - (m*60);

  // Particle #, time stamp, timeword, reject, h, w, a
  sprintf(buffer, "%03d %02d:%02d:%02d.%03ld  %8u %2d %3zu %3zu %3zu %6u\n",
        cnt, h, m, s, cp->msec, cp->timeWord, cp->reject, cp->h, cp->w,
        cp->area, cp->deltaTime);

  Append(buffer);

}	/* END ADDLINEITEM */

/* END ENCHILADA.CC */
