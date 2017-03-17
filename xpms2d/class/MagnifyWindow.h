/*
-------------------------------------------------------------------------
OBJECT NAME:	MagnifyWindow.h

FULL NAME:	Magnify window and Zoom

COPYRIGHT:	University Corporation for Atmospheric Research, 1997
-------------------------------------------------------------------------
*/

#ifndef MAGWINDOW_H
#define MAGWINDOW_H

#include <define.h>

#include <raf/Window.h>

#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>


/* -------------------------------------------------------------------- */
class MagnifyWindow  : public WinForm {

public:
	MagnifyWindow(Widget parent);

  Widget DrawingArea() const	{ return(drawA); }
 
 
private:
  Widget        drawA;

};	/* END MAGNIFYWINDOW.H */

#endif
