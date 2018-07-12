/*
-------------------------------------------------------------------------
OBJECT NAME:	cb_control.cc

FULL NAME:	Callback Wrappers

ENTRY POINTS:	ApplyTimeChange()
		ModifyActiveVars()
		PageForward()
		PageCurrent()
		PageBackward()
		SetCurrentFile()
		SetProbe()
		SetDensity()
		SetConcentration()
		SetScaleTime()
		SetScaleSpeed()
		StartMovie()
		StopMovie()

STATIC FNS:	

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1994-2018
-------------------------------------------------------------------------
*/

#include "define.h"
#include <unistd.h>

#include <ControlWindow.h>
#include <Colors.h>
#include <raf/Cursor.h>
#include <Enchilada.h>
#include <Histogram.h>
#include <FileMgr.h>
#include <raf/XPen.h>
#include <MainCanvas.h>

extern Enchilada	*enchiladaWin;
extern Histogram	*histogramWin;
extern FileManager	fileMgr;
extern ControlWindow	*controlWindow;
extern MainCanvas	*mainPlot;
extern XCursor		cursor;
extern Colors		*color;
extern XPen		*pen;


/* Contains current N 2d records (for re-use by things like ViewHex */
size_t	nBuffs = 0;
P2d_rec	pgFbuff[20];


/* -------------------------------------------------------------------- */
void ApplyTimeChange(Widget w, XtPointer client, XtPointer call)
{
  int		h, m, s;
  char		*p;
  float		version;

  if (fileMgr.CurrentFile() == NULL ||
      fileMgr.CurrentFile()->Probes().size() == 0)
    return;


  cursor.WaitCursor(mainPlot->Wdgt());
  version = atof(fileMgr.CurrentFile()->HeaderVersion());
  p = XmTextFieldGetString(controlWindow->StartTime());

  if (strlen(p) == 0) {
    cursor.PointerCursor(mainPlot->Wdgt());
    XtFree(p);
    return;
    }

  sscanf(p, "%d:%d:%d", &h, &m, &s);

  if (!fileMgr.CurrentFile()->LocatePMS2dRecord(&pgFbuff[0], h, m, s))
    {
    fprintf(stderr, "No 2d records after %s.\n", p);
    cursor.PointerCursor(mainPlot->Wdgt());
    XtFree(p);
    return;
    }

  XtFree(p);

  PageForward(NULL, NULL, NULL);

}	/* END APPLYTIMECHANGE */

/* -------------------------------------------------------------------- */
void PageForward(Widget w, XtPointer client, XtPointer call)
{
  char		*buff_p;
  bool		displayProbes = false;
  float		version;
  struct recStats p7;

  if (fileMgr.NumberOfFiles() == 0)
    return;

  const ProbeList& probes = fileMgr.CurrentFile()->Probes();
  ProbeList::const_iterator iter;
  for (iter = probes.begin(); iter != probes.end(); ++iter)
    if (iter->second->Display())
      displayProbes = true;

  if (!displayProbes)
    return;

  cursor.WaitCursor(mainPlot->Wdgt());

  mainPlot->Clear();
  mainPlot->reset(fileMgr.CurrentFile(), 0);
  version = atof(fileMgr.CurrentFile()->HeaderVersion());

  if (enchiladaWin)
    enchiladaWin->Clear();

  if (histogramWin)
    histogramWin->Clear();

  for (nBuffs = 0; nBuffs < mainPlot->maxRecords() &&
              fileMgr.CurrentFile()->NextPMS2dRecord(&pgFbuff[nBuffs]); )
    {
    buff_p = (char *)&pgFbuff[nBuffs];

    if (((buff_p[0] == 'H' && mainPlot->SpaceAvailable() < 294) ||
	 (buff_p[0] != 'H' && mainPlot->SpaceAvailable() < 70)) )
      break;

    if (nBuffs == 0)
      {
      controlWindow->UpdateStartTime(&pgFbuff[0]);
      mainPlot->reset(fileMgr.CurrentFile(), &pgFbuff[0]);
      }

    iter = probes.begin();
    for (int j = 0; iter != probes.end(); ++j, ++iter)
      {
      if (!memcmp(iter->second->Code(), buff_p, 2) && iter->second->Display())
        {
        pen->SetColor(color->GetColor(j+1));
        mainPlot->draw(&pgFbuff[nBuffs], iter->second, version, j+1, NULL);
        pen->SetColor(color->GetColor(0));
        if (++nBuffs >= 20)
          {
          fprintf(stderr, "cb_control.ccL::PageForward: nBuffs exceeds max of 20.\n");
          exit(1);
          }
        break;
        }
      }
    }

  mainPlot->ExposeAll();
  cursor.PointerCursor(mainPlot->Wdgt());
  controlWindow->UpdateTimeScale();

}	/* END PAGEFORWARD */

/* -------------------------------------------------------------------- */
void PageCurrent()
{
  size_t	i;
  P2d_rec	pgBuff;

  if (fileMgr.NumberOfFiles() == 0)
    return;

  const ProbeList& probes = fileMgr.CurrentFile()->Probes();
  cursor.WaitCursor(mainPlot->Wdgt());

  // Loop back and find the record prior to first record to display
  for (i = 0; i < mainPlot->maxRecords() &&
              fileMgr.CurrentFile()->PrevPMS2dRecord(&pgBuff); )
    {
    if (probes.find(*(uint16_t *)&pgBuff)->second->Display())
      ++i;
    }

  // and process it so stats on the first displayed records work.
  if (i > 0) probes.find(*(uint16_t *)&pgBuff)->second->ProcessRecord(&pgBuff, -1);
  cursor.PointerCursor(mainPlot->Wdgt());
  PageForward(NULL, NULL, NULL);

}	/* END PAGECURRENT */

/* -------------------------------------------------------------------- */
void PageBackward(Widget w, XtPointer client, XtPointer call)
{
  size_t	i;
  P2d_rec	pgBbuff;

  if (fileMgr.NumberOfFiles() == 0)
    return;

  const ProbeList& probes = fileMgr.CurrentFile()->Probes();
  cursor.WaitCursor(mainPlot->Wdgt());

  for (i = 0; i < (mainPlot->maxRecords() << 1) &&
              fileMgr.CurrentFile()->PrevPMS2dRecord(&pgBbuff); )
    {
    if (probes.find(*(uint16_t *)&pgBbuff)->second->Display())
      ++i;
    }

  if (i > 0) probes.find(*(uint16_t *)&pgBbuff)->second->ProcessRecord(&pgBbuff, -1);
  cursor.PointerCursor(mainPlot->Wdgt());
  PageForward(NULL, NULL, NULL);
  controlWindow->UpdateTimeScale();

}	/* END PAGEBACKWARD */

/* -------------------------------------------------------------------- */
void SetCurrentFile(Widget w, XtPointer client, XtPointer call)
{
  fileMgr.SetCurrentFile((long)client);
}

/* -------------------------------------------------------------------- */
void SetProbe(Widget w, XtPointer client, XtPointer call)
{
  long probeNum = (long)client;
  fileMgr.CurrentFile()->Probes().find((uint16_t)probeNum)->second->setDisplay(((XmToggleButtonCallbackStruct *)call)->set);
  PageCurrent();
}

/* -------------------------------------------------------------------- */
void SetDensity(Widget w, XtPointer client, XtPointer call)
{
  // skip the unset of other button.
  if (((XmToggleButtonCallbackStruct *)call)->set)
    {
    controlWindow->SetWaterDensity((long)client);
    PageCurrent();
    }
}

/* -------------------------------------------------------------------- */
void SetAreaRatioRej(Widget w, XtPointer client, XtPointer call)
{
  if (((XmToggleButtonCallbackStruct *)call)->set)
    {
    controlWindow->SetAreaRatioReject((long)client);
    PageCurrent();
    }
}

/* -------------------------------------------------------------------- */
void SetConcentration(Widget w, XtPointer client, XtPointer call)
{
  // skip the unset of other button.
  if (((XmToggleButtonCallbackStruct *)call)->set == false)
    return;

  controlWindow->SetConcentrationCalc((long)client);

  const ProbeList & probes = fileMgr.CurrentFile()->Probes();
  ProbeList::const_iterator iter;
  for (iter = probes.begin(); iter != probes.end(); ++iter)
    iter->second->SetSampleArea();

  PageCurrent();
}

/* -------------------------------------------------------------------- */
void StartMovie(Widget w, XtPointer client, XtPointer call)
{
  controlWindow->Start();
  controlWindow->Stop();
}

/* -------------------------------------------------------------------- */
void StopMovie(Widget w, XtPointer client, XtPointer call)
{
  controlWindow->Stop();
}

/* -------------------------------------------------------------------- */
void SetScaleTime(Widget w, XtPointer client, XtPointer call)
{
  controlWindow->PositionTime(False);
  PageForward(NULL, NULL, NULL);
}

/* -------------------------------------------------------------------- */
void SetScaleSpeed(Widget w, XtPointer client, XtPointer call)
{
  controlWindow->SetDelay();
}

/* END CB_CONTROL.CC */
