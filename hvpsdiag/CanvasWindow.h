/*
-------------------------------------------------------------------------
OBJECT NAME:	CanvasWindow.h

FULL NAME:	Create Main Canvas Window

TYPE:		

DESCRIPTION:	

NOTES:		

COPYRIGHT:	University Corporation for Atmospheric Research, 2003
-------------------------------------------------------------------------
*/

#ifndef CANWINDOW_H
#define CANWINDOW_H

#include <qapplication.h>
#include <qmainwindow.h>
#include <qlistbox.h>
#include <qhbox.h>
#include <qmenubar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qprinter.h>
#include <qwt_plot.h>

#include "define.h"
#include "DataPlot.h"


/* -------------------------------------------------------------------- */
class CanvasWindow : public QMainWindow
{
  Q_OBJECT

public:
	CanvasWindow(QApplication *);

protected slots:
  void	Print();

private:
  DataPlot	*plot;

};

#endif
