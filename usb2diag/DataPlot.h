#ifndef _DataPlot_h_
#define _DataPlot_h_

#include "DataUsb2d64.h"

#include <qwidget.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qevent.h>
#include <qdatetime.h>

 

/* -------------------------------------------------------------------- */
/**
 * Class to plot/draw PMS-2D ercords on a Qt Widget.
 */
class DataPlot: public QWidget
{
public:
  DataPlot(QWidget * parent, FILE * fp);
  virtual ~DataPlot() { if (dm !=NULL) {delete dm;}}

  void  Plot(bool pnew=true);
  void	ToggleFreeze();
  void	RstTimer(int s);
  void  Prt();
 
protected:
  virtual void timerEvent(QTimerEvent * e);

  virtual void paintEvent(QPaintEvent * e);


private:
  bool     _freeze;
  DataUsb2d64* dm;
  void _drawIt();

  QPrinter _printer;
  QPainter _painter;
  int      _count;
  int      _c;
  
};

#endif
