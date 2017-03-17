#include "DataPlot.h"

#include <qpen.h>

#include <unistd.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>

P2d_rec	hvpsRecord;


/* -------------------------------------------------------------------- */
DataPlot::DataPlot(QWidget *parent, ADS_rtFile *f) : QwtPlot(parent)
{
  file = f;
  freeze = false;

  setAxisTitle(QwtPlot::xBottom, "Diode #");
  setAxisTitle(QwtPlot::yLeft, "Volts");
  setMargin(10);
//  setAutoLegend(true);
  setCanvasBackground(QColor("white"));

  crv1 = new QwtPlotCurve("Masked");
  crv2 = new QwtPlotCurve("Valid");

  crv1->setPen(QPen(red));
  crv2->setPen(QPen(green));
  crv1->setStyle(QwtPlotCurve::Sticks);
  crv2->setStyle(QwtPlotCurve::Sticks);

  crv1->attach(this);
  crv2->attach(this);

  file->FirstPMS2dRecord(&hvpsRecord);

  (void)startTimer(10);

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void DataPlot::ToggleFreeze()
{
  freeze = 1 - freeze;

}	/* END TOGGLEFREEZE */

/* -------------------------------------------------------------------- */
void DataPlot::timerEvent(QTimerEvent *)
{   
  if (freeze || file->NextPMS2dRecord(&hvpsRecord) == false)
    return;    

  Hdr_blk	*hdr = (Hdr_blk *)&hvpsRecord;
  ushort	*data = (ushort *)hvpsRecord.data;

  if (ntohs(data[0]) != 0xcaaa)
    return;

  ushort	*mask = (ushort *)&hvpsRecord.data[4];
  double	xVal1[300], xVal2[300], yVal1[300], yVal2[300];
  int		rCnt, gCnt;

  sprintf(buffer, "UTC (%02d:%02d:%02d)",
	ntohs(hdr->hour), ntohs(hdr->minute), ntohs(hdr->second));
  setAxisTitle(QwtPlot::xBottom, buffer);

  rCnt = gCnt = 0;
  for (int i = 0; i < 256; ++i)
  {
//printf("mask[%d]=%x i%16=%d = %d\n", i/16, mask[i/16], i%16, (0x8000 >> (i%16)));
    if ( mask[i/16] & (0x8000 >> (i%16)) )
    {
      xVal2[gCnt] = i;
      yVal2[gCnt++] = data[28 + i];
    }
    else
    {
      xVal1[rCnt] = i;
      yVal1[rCnt++] = data[28 + i];
    }
  }

  crv1->setData(xVal1, yVal1, rCnt);
  crv2->setData(xVal2, yVal2, gCnt);

  replot();
  sleep(1);

}	/* END TIMEREVENT */

/* END DATAPLOT.CC */
