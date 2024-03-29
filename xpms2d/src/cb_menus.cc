/*
-------------------------------------------------------------------------
OBJECT NAME:	cb_menus.c

FULL NAME:	Callback Wrappers for menus off of main menu bar

ENTRY POINTS:	GetDataFileName()
		NewDataFile()
		AddDataFile()
		PrintSave()
		SavePNG()
		Quit()
		ToggleDisplay()
		ToggleSynthetic()

STATIC FNS:	PrintPS()
		Save()

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "define.h"
#include <cerrno>
#include <unistd.h>
#include <sstream>

#include <raf/Application.h>
#include <ControlWindow.h>
#include <Colors.h>
#include <raf/Cursor.h>
#include <FileMgr.h>
#include <MainCanvas.h>
#include <raf/PostScript.h>
#include <raf/Printer.h>
#include <raf/XmFile.h>
#include <raf/XmWarn.h>

static char	*PSoutFile = NULL, *pngOutFile = NULL;

extern Application	*application;
extern ControlWindow	*controlWindow;
extern Colors		*color;
extern XCursor		cursor;
extern FileManager	fileMgr;
extern MainCanvas	*mainPlot;
extern Printer		*printerSetup;
extern XmFile		*fileSel;

/* Contains current N 2d records (for re-use by things like ViewHex */
extern size_t nBuffs;
extern OAP::P2d_rec pgFbuff[];

extern OAP::UserConfig userConfig;

/* -------------------------------------------------------------------- */
void GetDataFileName(Widget w, XtPointer client, XtPointer call)
{
  fileSel->QueryFile("Enter Data file to read:", DataPath, (XtCallbackProc)client);
}

/* -------------------------------------------------------------------- */
void NewDataFile(Widget w, XtPointer client, XtPointer call)
{
  char *dataFile;

  cursor.WaitCursor(mainPlot->Wdgt());
  fileSel->ExtractFileName(
         ((XmFileSelectionBoxCallbackStruct *)call)->value, &dataFile);

  fileMgr.NewFile(dataFile, userConfig);
  controlWindow->SetFileNames();
  controlWindow->SetProbes();
  controlWindow->PositionTime(True);
  cursor.PointerCursor(mainPlot->Wdgt());

}	/* END NEWDATAFILE */

/* -------------------------------------------------------------------- */
void AddDataFile(Widget w, XtPointer client, XtPointer call)
{
  char * dataFile;

  if (fileMgr.NumberOfFiles() >= MAX_DATAFILES)
    {
    ErrorMsg("Currently at maximum allowed files.");
    return;
    }

  cursor.WaitCursor(mainPlot->Wdgt());
  fileSel->ExtractFileName(
         ((XmFileSelectionBoxCallbackStruct *)call)->value, &dataFile);

  fileMgr.AddFile(dataFile, userConfig);
  controlWindow->SetFileNames();
  cursor.PointerCursor(mainPlot->Wdgt());

}	/* END ADDDATAFILE */

/* -------------------------------------------------------------------- */
static void PrintPS(Widget w, XtPointer client, XtPointer call)
{
  bool          displayProbes = false;
  float         version;
  char		buffer[256];
  ADS_DataFile	*file = fileMgr.CurrentFile();

  const ProbeList& probes = fileMgr.CurrentFile()->Probes();
  ProbeList::const_iterator iter;
  for (iter = probes.begin(); iter != probes.end(); ++iter)
    if (iter->second->Display())
      displayProbes = true;

  if (!displayProbes)
    return;

  cursor.WaitCursor(mainPlot->Wdgt());
  mainPlot->reset(0, 0);
  version = atof(file->HeaderVersion());

  std::stringstream title;
  title << file->ProjectNumber() << ", " << file->FlightNumber() << " - "
        << file->FlightDate();

  PostScript pen(PSoutFile, "xpms2d", title.str().c_str(), 0.72);
  pen.SetFont(40);
  snprintf(buffer, 256, "%d (%s) stringwidth pop 2 div sub %d moveto\n",
                550, title.str().c_str(), (int)(800 * printerSetup->HeightRatio()));
  pen.Issue(buffer);
  pen.ShowStr(title.str().c_str());
  pen.SetFont(16);

  for (size_t i = 0; i < nBuffs; ++i)
    {
    iter = probes.begin();
    for (size_t j = 0; iter != probes.end(); ++iter, ++j)
      {
      if (!strncmp(iter->second->Code(), (char*)&pgFbuff[i], 2) && iter->second->Display())
        mainPlot->draw(&pgFbuff[i], iter->second, version, j+1, &pen);
      }
    }

  cursor.PointerCursor(mainPlot->Wdgt());

}	/* END POSTSCRIPT */

/* -------------------------------------------------------------------- */
static void SavePS(Widget w, XtPointer client, XtPointer call)
{
  char	*p, buffer[256];

  fileSel->ExtractFileName(
         ((XmFileSelectionBoxCallbackStruct *)call)->value, &PSoutFile);

  strcpy(psPath, PSoutFile);
  if ((p = strrchr(psPath, '/')))
    strcpy(p+1, "*.ps");

  if (access(PSoutFile, F_OK) == 0)
    {
    snprintf(buffer, 256, "Overwrite file %s", PSoutFile);
    WarnMsg(buffer, PrintPS, NULL);
    }
  else
    PrintPS(NULL, NULL, NULL);

}       /* END SAVEPS */

/* -------------------------------------------------------------------- */
void PrintSave(Widget w, XtPointer client, XtPointer call)
{
  if (fileMgr.NumberOfFiles() == 0)
    {
    ErrorMsg("No data.");
    return;
    }

  if (client)
    fileSel->QueryFile("Enter PostScript output file name:", psPath,
                       (XtCallbackProc)SavePS);
  else
    {
    PSoutFile = NULL;
    PrintPS(NULL, NULL, NULL);
    }

}	/* END POSTSCRIPT */

#ifdef PNG
/* -------------------------------------------------------------------- */
void SavePNG_OK(Widget w, XtPointer client, XtPointer call)
{
  XImage        *image;

  image = mainPlot->GetImage(0, 1, mainPlot->Width(), mainPlot->Height()-1);

  color->SavePNG(pngOutFile, image);

  XDestroyImage(image);

}       /* END SAVEPNG_OK */

/* -------------------------------------------------------------------- */
void confirmPNG(Widget w, XtPointer client, XtPointer call)
{
  char	*p, buffer[256];

  fileSel->ExtractFileName(((XmFileSelectionBoxCallbackStruct *)call)->value, &pngOutFile);

  strcpy(pngPath, pngOutFile);
  if ((p = strrchr(pngPath, '/')))
    strcpy(p+1, "*.png");

  if (access(pngOutFile, F_OK) == 0)
    {
    snprintf(buffer, 256, "Overwrite file %s", pngOutFile);
    new XmWarn(application->Shell(), buffer, SavePNG_OK, NULL);
    }
  else
    SavePNG_OK((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);

}

/* -------------------------------------------------------------------- */
void SavePNG(Widget w, XtPointer client, XtPointer call)
{
  fileSel->QueryFile("Enter PNG output file name:", pngPath,
                       (XtCallbackProc)confirmPNG);

}       /* END SAVEPNG */
#endif

/* -------------------------------------------------------------------- */
void Quit(Widget w, XtPointer client, XtPointer call)
{
  delete application;

  exit(0);

}	/* END QUIT */

/* -------------------------------------------------------------------- */
void ToggleTiming(Widget w, XtPointer client, XtPointer call)
{
  mainPlot->SetTimingWords();
  PageCurrent();

}       /* END TOGGLEDISPLAY */

/* -------------------------------------------------------------------- */
void ToggleWrap(Widget w, XtPointer client, XtPointer call)
{
  mainPlot->SetWrapDisplay();
  PageCurrent();

}       /* END TOGGLEDISPLAY */

/* -------------------------------------------------------------------- */
void ToggleDisplay(Widget w, XtPointer client, XtPointer call)
{
  mainPlot->SetDisplayMode((long)client);
  PageCurrent();

}       /* END TOGGLEDISPLAY */

/* -------------------------------------------------------------------- */
void ToggleHistogram(Widget w, XtPointer client, XtPointer call)
{
  // This is the histograms on the canvas, not the HistogramWindow.
  mainPlot->SetHistograms();
  PageCurrent();

}       /* END TOGGLEDISPLAY */

/* -------------------------------------------------------------------- */
void ToggleSynthetic(Widget w, XtPointer client, XtPointer call)
{
  fileMgr.CurrentFile()->ToggleSyntheticData();
  PageCurrent();

}       /* END TOGGLESYNTHETIC */

/* -------------------------------------------------------------------- */
#include "raf/opener.h"

void OpenURL(Widget w, XtPointer client, XtPointer call)
{
  switch ((long)client)
    {
    case 1:
      opener("http://www.eol.ucar.edu/research-aircraft");
      break;
    case 2:
      opener("http://www.eol.ucar.edu/raf/Software");
      break;
    case 3:
      opener("http://www.eol.ucar.edu/raf/Software/xpms2d.html");
      break;
    }

}       /* END OPENURL */

/* END CB_MENUS.CC */
