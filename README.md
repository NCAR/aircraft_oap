# aircraft_oap
A collection of programs for manipulating Optical Array Probe data.  Consists of several translators to convert native probe data into a common OAP file format, a processor, and a display program. 

The OAP file format can be found at http://www.eol.ucar.edu/raf/software/OAPfiles.html

Directories:

| Directory | Description |
| ----------- | ----------------------------------------------------------------------------------------- |
| cip | DMT CIP and PIP data translators. |
| doc | Documentation. |
| hvpsdiag | Old Qt3 program for SPEC HVPS diagnostics. |
| oapinfo | Program to give some information from OAP files. |
| process2d | 2D Processor to process 2D image data into size-distributions and other quantitative data. |
| translate2ds | SPEC 2DS data translator. |
| usb2diag | |
| xpms2d | X Windows / Motif program for viewing image data from most OAP probes. http://www.eol.ucar.edu/raf/software/xpms2d.html |

### Documentation ###

Documentation is available on a per-program basis.

xpms2d - The users manual can be found online at 
http://www.eol.ucar.edu/raf/software/xpms2d.html

The OAP file format can be found at http://www.eol.ucar.edu/raf/software/OAPfiles.html

### Environment ###
The aircraft OAP utilities are written in C. They require netCDF library 3.0 or later.

The utilities build and run on any Unix/Linux operating system, including Mac OS X.

### Dependencies ###

To install these programs on your computer, ensure the following libraries are installed:

 * openmotif
 * netcdf
 * python (needed by vardb)
 * log4cpp (needed by vardb)
 * xerces-c (needed by vardb)
 * boost (needed by vardb)
 
 If you are still getting compile errors, you can look here for other things you may need (like Xcode)
 https://github.com/ncar/aspen/wiki/MacOS-Build-Environment

### Installation ###

 * git clone --recursive https://github.com/ncar/aircraft_oap
 * cd aircraft_oap
 * scons
 * scons install
All programs will be installed in aircraft_oap/bin by default.

If you wish to compile just a single utility, cd to that utilities subdir (say xpms2d) and run
 * scons -u .

To install in a different directory, say your home dir, 
 * scons install --prefix=~
This will put compiled programs in ~/bin
