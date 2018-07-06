#
# Makefile for xpms2d
#
BIN	= ${JLOCAL}/bin
WWW	= /net/www/docs/raf/Software
RELEASE = ${JLOCAL}/xpms2d

INC	= ../class
RAFINC	= ${JLOCAL}/include
C2D	= ../class
O2D	= ../class

#       Linux
#
DEFINES	= -D_FILE_OFFSET_BITS=64 -DPNG
INCLUDES= -I../class -I${RAFINC}
LIB_DIRS= -L${JLOCAL}/lib
LIBS   = -lraf -lXm -lXt -lX11 -lpng -lz
ANONFTP	= /net/ftp/pub/archive/RAF-src/bin.RHEL6

# Mac OS X
#
# Requires XCode from Mac App Store, use Macports/porticus to get libpng and netcdf
# and get OpenMotif from here (OpenMotif from Macports was not working for us):
#  http://www.ist-inc.com/downloads/motif_download.html
#
# Mountain Lion update.  Need to get XQuartz download for X11, it is no longer bundled
# with MacOS.  Using Homebrew instead of MacPorts now.
#
# setenv JLOCAL /usr/local (or 'export JLOCAL=/usr/local').
#
# Build libraf first.
#
#DEFINES	= -D_FILE_OFFSET_BITS=64 -DPNG -DPNG15
#INCLUDES= -I../class -I${INC} -I${RAFINC} -I/opt/X11/include -I/usr/OpenMotif/include
#LIB_DIRS= -L/opt/X11/lib -L${JLOCAL}/lib -L/usr/OpenMotif/lib
#LIBS   = -lraf -lXm -lXt -lICE -lX11 -lpng -lz

# Don't use optimization with gcc (2.95.2 and earlier) and C++.
CXX	= g++
CXXFLAGS= -Wall -Wno-write-strings -g -O ${INCLUDES} ${DEFINES}

PROG	= xpms2d
HDRS	= ../class/define.h

SRCS=	xpms2d.cc global.cc ccb.cc cb_canvas.cc cb_control.cc cb_mag.cc\
	cb_menus.cc cb_printer.cc cb_text.cc xml.cc

OBJS=	xpms2d.o global.o ccb.o cb_canvas.o cb_control.o cb_mag.o\
	cb_menus.o cb_printer.o cb_text.o xml.o


O2DOB=	${O2D}/CanvasWindow.o\
	${O2D}/ControlWindow.o ${O2D}/Colors.o ${O2D}/Enchilada.o\
	${O2D}/DataFile.o ${O2D}/FileMgr.o ${O2D}/Hex.o\
	${O2D}/Magnify.o ${O2D}/MagnifyCanvas.o ${O2D}/MagnifyWindow.o\
	${O2D}/MainCanvas.o ${O2D}/Probe.o ${O2D}/Histogram.o ${O2D}/PMS2D.o\
	${O2D}/Fast2D.o ${O2D}/TwoDS.o ${O2D}/CIP.o ${O2D}/HVPS.o ${O2D}/Particle.o


${PROG}: ${OBJS} classDir
	${CXX} ${CXXFLAGS} ${LIB_DIRS} ${OBJS} ${O2DOB} ${LIBS} -o $@

classDir:
	cd ${O2D}; make "CXXFLAGS=${CXXFLAGS}"

install: ${PROG}
	test -d ${BIN} || mkdir ${BIN}
	cp ${PROG} ${BIN}

publish:
	cp ${PROG} ${ANONFTP}
	cp ${PROG}.html ${WWW}

clean:
	(cd ../class; make clean)
	${RM} core* ${OBJS} ${PROG}

print:
	enscript -2Gr -b${PROG} ${HDRS} ${SRCS}

${OBJS}:	${HDRS}

xpms2d.o:	${RAFINC}/raf/Application.h ${INC}/CanvasWindow.h ${INC}/Colors.h ${INC}/ControlWindow.h ${RAFINC}/raf/Cursor.h ${INC}/FileMgr.h ${INC}/DataFile.h ${RAFINC}/raf/XFonts.h ${RAFINC}/raf/XPen.h ${INC}/MagnifyWindow.h ${INC}/MainCanvas.h ${RAFINC}/raf/Printer.h ${RAFINC}/raf/XmError.h ${RAFINC}/raf/XmFile.h ${RAFINC}/raf/XmWarn.h fbr.h

cb_canvas.o:	${INC}/MainCanvas.h ${INC}/Magnify.h
cb_control.o:	${RAFINC}/raf/Cursor.h ${INC}/FileMgr.h ${INC}/MainCanvas.h ${INC}/ControlWindow.h ${INC}/Colors.h
cb_mag.o:	${INC}/MagnifyCanvas.h
cb_menus.o:	${RAFINC}/raf/Application.h ${INC}/ControlWindow.h ${RAFINC}/raf/Cursor.h ${INC}/FileMgr.h ${INC}/MainCanvas.h ${RAFINC}/raf/PostScript.h ${RAFINC}/raf/Printer.h ${RAFINC}/raf/XmFile.h
cb_printer.o:	${RAFINC}/raf/Printer.h
cb_text.o:	${RAFINC}/raf/TextWindow.h
process.o:	${INC}/ControlWindow.h
