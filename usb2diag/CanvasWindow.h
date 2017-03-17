/*
-------------------------------------------------------------------------
OBJECT NAME:	CanvasWindow.h

COPYRIGHT:	University Corporation for Atmospheric Research, 2007
-------------------------------------------------------------------------
*/

#ifndef _CanvasWindow_h_
#define _CanvasWindow_h_

#include "DataPlot.h"

#include <qapplication.h>
#include <qmainwindow.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qtoolbutton.h>
#include <qinputdialog.h> 
#include <qmessagebox.h> 
#include <qcursor.h>
#include <iostream>


/* -------------------------------------------------------------------- */
class CanvasWindow : public QMainWindow
{
  Q_OBJECT

public:
  CanvasWindow(QApplication *, const char * file_name);

protected slots:
  void	_print();
  void	_openf();
  void  _rstTimer();
 
private:
  FILE * _fp;
  QApplication*  _app;
  char*  _fn;
  QPrinter _printer;
  
  DataPlot * _plot;
  void _startplot();
  
};

#endif
