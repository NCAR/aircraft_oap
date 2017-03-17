#
# xpms2d C++ class makefile
#
RAFINC=	${JLOCAL}/include
I2D=	../src
O2D=	../obj

OBJS=	CanvasWindow.o Colors.o ControlWindow.o\
	DataFile.o Enchilada.o FileMgr.o Hex.o\
	Magnify.o MagnifyCanvas.o MagnifyWindow.o\
	MainCanvas.o Probe.o Histogram.o

SRCS=	CanvasWindow.cc Colors.cc ControlWindow.cc\
	DataFile.cc Enchilada.cc FileMgr.cc Hex.cc\
	Magnify.cc MagnifyCanvas.cc MagnifyWindow.cc\
	MainCanvas.cc Probe.cc Histogram.cc

HDRS=	${I2D}/define.h

all: ${OBJS}


${OBJS}: ${HDRS}

CanvasWindow.o:		CanvasWindow.h ${RAFINC}/raf/Window.h

ControlWindow.o:	ControlWindow.h FileMgr.h DataFile.h
Colors.o:		Colors.h MainCanvas.h

DataFile.o:		DataFile.h ${RAFINC}/raf/hdrAPI.h Probe.h

Enchilada.o:		Enchilada.h ${RAFINC}/raf/TextWindow.h
FileMgr.o:		FileMgr.h DataFile.h
Hex.o:			Hex.h ${RAFINC}/raf/TextWindow.h
Histogram.o:		Histogram.h ${RAFINC}/raf/TextWindow.h
Magnify.o:		Magnify.h MainCanvas.h MagnifyCanvas.h Colors.h
MagnifyCanvas.o:	MagnifyCanvas.h ${RAFINC}/raf/Canvas.h ${RAFINC}/raf/Cursor.h
MagnifyWindow.o:	MagnifyWindow.h ${RAFINC}/raf/Window.h
MainCanvas.o:		MainCanvas.h Colors.h ${RAFINC}/raf/Canvas.h
Probe.o:		Probe.h ${RAFINC}/raf/hdrAPI.h


clean:
	${RM} *.o
