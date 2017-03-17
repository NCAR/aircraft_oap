/*
-------------------------------------------------------------------------
OBJECT NAME:	usb2d.c

COPYRIGHT:	University Corporation for Atmospheric Research, 2007
-------------------------------------------------------------------------
*/

#include "CanvasWindow.h"

#include <qapplication.h>


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  CanvasWindow * canvasWindow = new CanvasWindow(&app, argc > 1 ? argv[1] : 0);
  canvasWindow->show();

  app.setMainWidget(canvasWindow);
  canvasWindow->setCaption("USB_2d");
  canvasWindow->resize(1054, 800);

  app.exec();
  return 0;
}
