/*
-------------------------------------------------------------------------
OBJECT NAME:	CanvasWindow.cc

FULL NAME:	

ENTRY POINTS:	CanvasWindow()

STATIC FNS:	

DESCRIPTION:	

NOTES:		

COPYRIGHT:	University Corporation for Atmospheric Research, 2003
-------------------------------------------------------------------------
*/

#include "CanvasWindow.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
#include <qregexp.h>

ADS_rtFile	*file;

/* -------------------------------------------------------------------- */
class PrintFilter: public QwtPlotPrintFilter
{
public:
	PrintFilter() {};

  virtual QFont font(const QFont &f, Item, int) const
  {
    QFont f2 = f;
    f2.setPointSize((int)(f.pointSize() * 0.75));
    return f2;
  }
};

/* -------------------------------------------------------------------- */
CanvasWindow::CanvasWindow(QApplication *qApp) : QMainWindow(0, "canvas")
{
  QToolBar *toolBar = new QToolBar(this, "options");
  toolBar->setLabel( "File Operations" );


  QToolButton *q = new QToolButton(toolBar);
  q->setTextLabel("Quit");
  q->setUsesTextLabel(true);
  q->setUsesBigPixmap(false);
  connect(q, SIGNAL(clicked()), qApp, SLOT(quit()));

  QToolButton *p = new QToolButton(toolBar);
  p->setTextLabel("Print");
  p->setUsesTextLabel(true);
  p->setUsesBigPixmap(false);
  connect(p, SIGNAL(clicked()), SLOT(Print()));


  QHBox *box = new QHBox(this);
  setCentralWidget((QWidget *)box);

  file = new ADS_rtFile();
  plot = new DataPlot(box, file);

  plot->show();

  statusBar()->message( "Ready", 2000 );

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void CanvasWindow::Print()
{
  QPrinter printer;

  QString docName = plot->title().text();
  if ( docName.isEmpty() )
  {
    docName.replace(QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
    printer.setDocName(docName);
  }

  printer.setCreator("hvpscal");
  printer.setOrientation(QPrinter::Landscape);
  printer.setPageSize(QPrinter::Letter);
  printer.setPrintProgram("lpr");

  if (printer.setup())
    plot->print(printer);

}	/* END PRINT */

/* END CANVASWINDOW.CC */
