/*
-------------------------------------------------------------------------
OBJECT NAME:	define.h

FULL NAME:	Include File to Include the Include Files
-------------------------------------------------------------------------
*/

#ifndef DEFINE_H
#define DEFINE_H

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <stdint.h>
#include <vector>
#include <iostream>

#include <Xm/Xm.h>

#include <raf/OAP.h>

#define COMMENT		'#'	/* Comment character for textfiles  */

#define SecondsSinceMidnite(t)	(t[0] * 3600 + t[1] * 60 + t[2])


#define BUFFSIZE	4096
#define PATH_LEN	1024

const size_t MAX_DATAFILES = 2;
const size_t MAX_PROBES = 4;


#include <raf/Particle.h>


/* Values for "displayLevel"		*/
enum { NORMAL, DIAGNOSTIC, RAW_RECORD };

/* Values for "HandleError"		*/
enum { RETURN, EXIT, IRET };

extern char *outFile, DataPath[], *timeSeg, pngPath[], psPath[];

extern bool	Interactive, DataChanged, UTCseconds;

void	GetDataFileName(Widget, XtPointer, XtPointer),
	NewDataFile(Widget, XtPointer, XtPointer),
	AddDataFile(Widget, XtPointer, XtPointer),
	EditPrintParms(Widget, XtPointer, XtPointer),
	PrintSave(Widget, XtPointer, XtPointer),
	SavePNG(Widget, XtPointer, XtPointer),
	Quit(Widget, XtPointer, XtPointer);

void	CanvasExpose(Widget, XtPointer, XtPointer),
	CanvasInput(Widget, XtPointer, XtPointer),
	CanvasResize(Widget, XtPointer, XtPointer);

void	MagnifyExpose(Widget, XtPointer, XtPointer),
	MagnifyInput(Widget, XtPointer, XtPointer),
	MagnifyResize(Widget, XtPointer, XtPointer);

void	ApplyTimeChange(Widget, XtPointer, XtPointer),
	ModifyActiveVars(Widget, XtPointer, XtPointer),
	PageForward(Widget, XtPointer, XtPointer),
	PageBackward(Widget, XtPointer, XtPointer),
	ToggleWrap(Widget, XtPointer, XtPointer),
	ToggleTiming(Widget, XtPointer, XtPointer),
	ToggleHistogram(Widget, XtPointer, XtPointer),
	PageCurrent(),
	SetCurrentFile(Widget, XtPointer, XtPointer),
	ViewHex(Widget, XtPointer, XtPointer),
	ViewEnchilada(Widget, XtPointer, XtPointer),
	ViewHistogram(Widget, XtPointer, XtPointer),
	SetDensity(Widget, XtPointer, XtPointer),
	SetAreaRatioRej(Widget, XtPointer, XtPointer),
	SetConcentration(Widget, XtPointer, XtPointer),
	OpenURL(Widget, XtPointer, XtPointer),
	SetScaleTime(Widget, XtPointer, XtPointer),
	StartMovie(Widget, XtPointer, XtPointer),
	StopMovie(Widget, XtPointer, XtPointer),
	SetScaleSpeed(Widget, XtPointer, XtPointer),
	SetProbe(Widget, XtPointer, XtPointer),
	SetPanel(Widget, XtPointer, XtPointer);

void	DismissWindow(Widget, XtPointer, XtPointer),
	DestroyWindow(Widget, XtPointer, XtPointer),
	ToggleDisplay(Widget, XtPointer, XtPointer),
	ToggleSynthetic(Widget, XtPointer, XtPointer),

	ValidateTime(Widget w, XtPointer client, XtPointer call),
        ValidateFloat(Widget w, XtPointer client, XtPointer call),
        ValidateInteger(Widget w, XtPointer client, XtPointer call);

void	ErrorMsg(const char msg[]), FlushEvents(),
	WarnMsg(const char msg[], XtCallbackProc okCB, XtCallbackProc cancelCB);

#endif

/* END DEFINE.H */
