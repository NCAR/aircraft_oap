/*
-------------------------------------------------------------------------
OBJECT NAME:	ControlWindow.h

FULL NAME:	Create Main Control Window

COPYRIGHT:	University Corporation for Atmospheric Research, 1997
-------------------------------------------------------------------------
*/

#ifndef CONWINDOW_H
#define CONWINDOW_H

#include <define.h>
#include <raf/header.h>

#include <raf/Application.h>
#include <raf/Window.h>

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>


/* -------------------------------------------------------------------- */
class ControlWindow : public WinForm {

public:
		ControlWindow(Widget parent);
  void		PopUp();

  void		SetFileNames();
  void		SetProbes();

  Widget	StartTime() const	{ return(timeText); }

  void		UpdateStartTime(P2d_rec *buff);
  void		UpdateTimeScale();

  float		GetDensity() const	{ return(density[densIdx].density); }
  float		GetAreaRatioReject() const	{ return(ratio[ratioIdx].density); }
  int		GetConcentration() const	{ return(concIdx); }

  bool		RejectZeroAreaImage() const	{ return(true); }

  void		SetWaterDensity(int idx)	{ densIdx = idx; }
  void		SetAreaRatioReject(int idx)	{ ratioIdx = idx; }
  void		SetConcentrationCalc(int idx)	{ concIdx = idx; }
  void		SetUserDensity();
  void		SetDelay(), PositionTime(bool), Start(), Stop();


private:
  Widget	timeText, fileB[MAX_DATAFILES], butt[8],
		probeB[MAX_PROBES], densB[4], densTxt,
		delayScale, timeScale, cncB[4], ratioB[7];

  int		concIdx, densIdx, delay, ratioIdx;
  bool		movieRunning;

struct _dens {
  const char *label;
  float	density;
  } density[4];

struct _dens ratio[7];

};	/* END CONTROLWINDOW.H */

#endif
