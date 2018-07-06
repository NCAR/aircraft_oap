#
# xpms2d C++ class makefile
#
RAFINC=	${JLOCAL}/include
O2D=	../obj

OBJS=	CanvasWindow.o Colors.o ControlWindow.o\
	DataFile.o Enchilada.o FileMgr.o Hex.o\
	Magnify.o MagnifyCanvas.o MagnifyWindow.o\
	MainCanvas.o Probe.o Histogram.o PMS2D.o\
	Fast2D.o TwoDS.o CIP.o HVPS.o Particle.o

SRCS=	CanvasWindow.cc Colors.cc ControlWindow.cc\
	DataFile.cc Enchilada.cc FileMgr.cc Hex.cc\
	Magnify.cc MagnifyCanvas.cc MagnifyWindow.cc\
	MainCanvas.cc Probe.cc Histogram.cc PMS2D.cc\
	Fast2D.cc TwoDS.cc CIP.cc HVPS.cc Particle.cc

HDRS=	define.h ${RAFINC}/raf/header.h

all: ${OBJS}


${OBJS}: ${HDRS}

CanvasWindow.o:		CanvasWindow.h ${RAFINC}/raf/Window.h

ControlWindow.o:	ControlWindow.h FileMgr.h DataFile.h PMS2D.h Fast2D.h TwoDS.h CIP.h HVPS.h
Colors.o:		Colors.h MainCanvas.h

DataFile.o:		DataFile.h Probe.h ${RAFINC}/raf/hdrAPI.h

Enchilada.o:		Enchilada.h ${RAFINC}/raf/TextWindow.h
FileMgr.o:		FileMgr.h DataFile.h
Hex.o:			Hex.h FileMgr.h ${RAFINC}/raf/TextWindow.h
Histogram.o:		Histogram.h ${RAFINC}/raf/TextWindow.h
Magnify.o:		Magnify.h MainCanvas.h MagnifyCanvas.h Colors.h
MagnifyCanvas.o:	MagnifyCanvas.h ${RAFINC}/raf/Canvas.h ${RAFINC}/raf/Cursor.h
MagnifyWindow.o:	MagnifyWindow.h ${RAFINC}/raf/Window.h
MainCanvas.o:		MainCanvas.h DataFile.h Colors.h ${RAFINC}/raf/Canvas.h ${RAFINC}/raf/PostScript.h

Probe.o:		Probe.h ${RAFINC}/raf/hdrAPI.h
PMS2D.o:		PMS2D.h Probe.h ControlWindow.h ${RAFINC}/raf/hdrAPI.h
Fast2D.o:		Fast2D.h Probe.h ControlWindow.h
TwoDS.o:		TwoDS.h Probe.h ControlWindow.h
CIP.o:			CIP.h Probe.h ControlWindow.h
HVPS.o:			HVPS.h Probe.h ControlWindow.h ${RAFINC}/raf/hdrAPI.h
Particle.o:		Particle.h

clean:
	${RM} *.o
