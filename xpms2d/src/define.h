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
#include <iostream>

#include <Xm/Xm.h>

#include <raf/Queue.h>

#ifndef ERR
#define OK		(0)
#define ERR		(-1)
#endif
 
#define COMMENT		'#'	/* Comment character for textfiles  */
 
#define SecondsSinceMidnite(t)	(t[0] * 3600 + t[1] * 60 + t[2])


#define BUFFSIZE	4096
#define PATH_LEN	256
#define NAMELEN		16

#define	SPACE		' '

const size_t MAX_DATAFILES = 2;
const size_t MAX_DATASETS = 4;
const size_t MAX_PANELS = 4;
const size_t MAX_PROBES = 4;

const uint32_t StandardSyncWord = 0x55000000;
const uint32_t SyncWordMask = 0xff000000;

const unsigned long long CIP_Sync = 0xAAAAAAAAAAAAAAAALL;

const unsigned long long Fast2D_Sync = 0xAAAAAA0000000000LL;
const unsigned long long Fast2D_Overld = 0x5555AA0000000000LL;

/* Mask should be all 'F's, but there appears to be an occasional flipped bit
 * at the end.  So use 'E' for the last nibble.
 */
const unsigned long long Fast2D_Mask = 0xFFFFFE0000000000LL;

const size_t nSlices_32bit = 1024;
const size_t nSlices_64bit = 512;
const size_t nSlices_128bit = 256;

#define TITLESIZE	80

#define ALL_SETS	(-1)


class Particle
  {
public:
  Particle();	// in process.cc

  long time;           // Seconds since mid-night
  long msec;

  size_t w, h;
  size_t area;
  bool	vert_gap;		// Was there a blank slice (i.e. multiple parts)
  bool	horiz_gap;		//   and in the hozizontal dir?
  bool	reject;
  unsigned char	edge;		// particle touched either edge
  bool	timeReject;		// Nimbus will reject (because of bad timing bar)

  ushort	x1, x2;		// for particles that touch both edges.
  uint32_t	timeWord;
  uint32_t	deltaTime;	// Amount of time between prev & this particle
  uint32_t	liveTime;	// Amount of time consumed by particle
  };


struct recStats
  {
  uint32_t thisTime;		// Time of this record in secs since midnight.
  uint32_t DASelapsedTime;	// milliseconds.
  uint32_t tBarElapsedtime;	// milliseconds.

  int	nTimeBars;		// n Particles.
  int	accum[1024];
  int	nonRejectParticles;	// n Particles not rejected.
  uint32_t minBar, maxBar, meanBar;

  double	SampleVolume, DOFsampleVolume;
  double	tas, concentration, lwc, dbz;
  float		frequency;              // TAS clock.
  int		resolution, area;

  Queue		particles;

  // Return values in/for MainCanvas::draw()
  bool		duplicate;
  unsigned long	prevTime;
  };


/* Values for rejection/concentration stuff*/
enum { BASIC, ENTIRE_IN, CENTER_IN, RECONSTRUCTION };

/* Values for "displayLevel"		*/
enum { NORMAL, DIAGNOSTIC, ENCHILADA, RAW_RECORD };

/* Values for "HandleError"		*/
enum { RETURN, EXIT, IRET };

enum ProbeType { UNKNOWN, PMS2D, HVPS, GREYSCALE, FAST2D, TWODS, CIP };

extern char buffer[], *outFile, DataPath[], *timeSeg, pngPath[], psPath[];

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
	PageCurrent(), SetSampleArea(),
	SetCurrentFile(Widget, XtPointer, XtPointer),
	ViewHex(Widget, XtPointer, XtPointer),
	ViewEnchilada(Widget, XtPointer, XtPointer),
	ViewHistogram(Widget, XtPointer, XtPointer),
	SetDensity(Widget, XtPointer, XtPointer),
	SetAreaRatioRej(Widget, XtPointer, XtPointer),
	SetConcentration(Widget, XtPointer, XtPointer),
	ForkNetscape(Widget, XtPointer, XtPointer),
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
