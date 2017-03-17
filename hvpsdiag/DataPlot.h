#include <qwt_plot.h>

#include "define.h"
#include <raf/adsIOrt.h>


class QwtPlotCurve;

/* -------------------------------------------------------------------- */
class DataPlot : public QwtPlot
{
public:
        DataPlot(QWidget *, ADS_rtFile *);

  void	ToggleFreeze();

protected:
  virtual void timerEvent(QTimerEvent *e);

private:
  QwtPlotCurve * crv1, * crv2;
  bool		freeze;
  ADS_rtFile	*file;

};

