/*
-------------------------------------------------------------------------
OBJECT NAME:	CanvasWindow.cc

COPYRIGHT:	University Corporation for Atmospheric Research, 2007
-------------------------------------------------------------------------
*/

#include "CanvasWindow.h"

#include <cstdlib>

/* -------------------------------------------------------------------- */
CanvasWindow::CanvasWindow(QApplication *qApp, const char * file_name) : QMainWindow(0, "canvas"), _app(qApp), _fn((char*)file_name)
{
  
  QToolBar *toolBar = new QToolBar(this, "options");
  toolBar->setLabel( "" );

  
  QToolButton *f = new QToolButton(toolBar);
  f->setTextLabel("OpenFile");
  f->setUsesTextLabel(true);
  f->setUsesBigPixmap(false);
  connect(f, SIGNAL(clicked()), SLOT(_openf()));

  //QPopupMenu *o = new QPopupMenu( toolBar, "Option");
  QToolButton *t = new QToolButton(toolBar);
  t->setTextLabel("TimeInterval");
  t->setUsesTextLabel(true);
  t->setUsesBigPixmap(false);
  connect(t, SIGNAL(clicked()), SLOT(_rstTimer()));


  QToolButton *p = new QToolButton(toolBar);
  p->setTextLabel("Print");
  p->setUsesTextLabel(true);
  p->setUsesBigPixmap(false);
  connect(p, SIGNAL(clicked()), SLOT(_print()));


  QToolButton *q = new QToolButton(toolBar);
  q->setTextLabel("Quit");
  q->setUsesTextLabel(true);
  q->setUsesBigPixmap(false);
  connect(q, SIGNAL(clicked()), qApp, SLOT(quit()));

  _startplot();
  statusBar()->message( "Ready", 2000 );

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void CanvasWindow::_openf()
{
  QFileDialog *dlg = new QFileDialog(getenv("DATA_DIR"), QString::null, 0, 0, TRUE );
  dlg->setCaption( QFileDialog::tr( "Open Data File" ) );
  dlg->setMode( QFileDialog::ExistingFile );
  
  QString file_name;
  if ( dlg->exec() == QDialog::Accepted ) {
    file_name = dlg->selectedFile();
    std::cout << "selected name:" << file_name <<"\n"<<std::endl;
  }
  delete dlg;

  if (!file_name.isEmpty()) {
    _fn = (char*)file_name.ascii();
    _startplot();
  }
  
}	/* END file open */

/* -------------------------------------------------------------------- */
void CanvasWindow::_startplot()
{
  if ((_fp = fopen64(_fn, "r")) == 0)
  {
    std::cerr << "No file \n\n" << _fn << "\n" << std::endl;
    return;
  } 
             
 // QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  setCursor(QCursor(Qt::WaitCursor));
  _plot = new DataPlot(this, _fp);
  setCentralWidget(_plot);
  _plot->show();
  unsetCursor();
 // QApplication::restoreOverrideCursor();

}
        /* END _startplot */

/* -------------------------------------------------------------------- */
void CanvasWindow::_rstTimer()
{
  if (_fp==NULL)
  {
    QMessageBox::warning( this, "Time Interval",
        "Please open a data file first.\n"
        "", 0, 0, 1 ) ;
    return; 
  } 

  bool ok;
  int res = QInputDialog::getInteger(
        "Reset Timer", "Please enter a time interval (seconds):", 2, 0, 500, 2,
            &ok, this );
  if ( !ok ) {
    std::cerr << "No Timer Interval entered. \n"<< std::endl;
    return;
  } 
  std::cout << "Input t_interval:" << res <<"\n"<< std::endl;
  _plot->RstTimer(res);
}	/* END RstTimer */


/* -------------------------------------------------------------------- */
void CanvasWindow::_print()
{
  if (_fp==NULL)
  {
    QMessageBox::warning( this, "  Print",
        "Please open a data file first.\n"
        "", 0, 0, 1 ) ;
    return; 
  } 

  _plot->Prt();
}	/* END PRINT */



/* END CANVASWINDOW.CC */
