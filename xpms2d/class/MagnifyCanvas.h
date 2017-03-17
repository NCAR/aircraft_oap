/*
-------------------------------------------------------------------------
OBJECT NAME:	MagnifyCanvas.h

FULL NAME:	Magnify canvas

COPYRIGHT:	University Corporation for Atmospheric Research, 1997
-------------------------------------------------------------------------
*/

#ifndef MAGCANVAS_H
#define MAGCANVAS_H

#include <define.h>
#include <raf/header.h>

#include <raf/Canvas.h>


/* -------------------------------------------------------------------- */
class MagnifyCanvas : public Canvas {

public:
		MagnifyCanvas(Widget w);

  void          draw();


private:

};	/* END MAGNIFYCANVAS.H */

#endif
