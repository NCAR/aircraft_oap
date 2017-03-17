/*
-------------------------------------------------------------------------
OBJECT NAME:	rtplot.c

FULL NAME:	Real Time Plot (Counts & Volts).

TYPE:		X11/R6 Qt

DESCRIPTION:	See man page

INPUT:		Command line options

COPYRIGHT:	University Corporation for Atmospheric Research, 2003
-------------------------------------------------------------------------
*/

#include "CanvasWindow.h"

#include <qapplication.h>

CanvasWindow	*canvasWindow;

/* --------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  canvasWindow = new CanvasWindow(&app);
  canvasWindow->show();

  app.setMainWidget(canvasWindow);
  canvasWindow->setCaption("hvpsdiag");
  canvasWindow->resize(900, 525);

  app.exec();

  return(0);

}	/* END MAIN */

/* END RTPLOT.CC */
