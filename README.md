# aircraft_oap
A collection of programs for manipulating Optical Array Probe data.  Consists of several translators to convert native probe data into a common OAP file format, a processor, and a display program. 

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
| xpms2d | X Windows / Motif program for viewing image data from most OAP probes. |

### Documentation ###

Documentation is available on a per-program basis.

[xpms2d User's Guide](http://www.eol.ucar.edu/raf/software/xpms2d.html)\
[The OAP File Format](http://www.eol.ucar.edu/raf/software/OAPfiles.html)\
See doc/procfd2c.doc for description of processing algorithms.

### Environment ###
The aircraft OAP utilities are written in C/C++.  The utilities build and run on any Unix/Linux operating system, including Mac OS X.

### MacOS

aircraft_oap is available via [MacPorts](https://www.macports.org/) or follow the [MacOS build environment](https://github.com/ncar/aircraft_oap/wiki/MacOS-Build-Environment) directions to build from source.

### Dependencies ###

To install these programs on your computer, ensure the following packages (and the corresponding -devel versions for Linux) are installed:
```
 openmotif (starting to show up as just "motif" on RHEL8 & Fedora).
 netcdf, netcdf-cxx
 python (needed by vardb)
 log4cpp (needed by vardb)
 xerces-c (needed by vardb)
 boost (needed by vardb)
 xorg-x11-fonts-ISO8859-1-75dpi
 xorg-x11-fonts-ISO8859-1-100dpi
```
 
 Ubuntu Focal (20.04)
```
libxt-dev
libmotif-dev
libnetcdf-dev
libnetcdf-cxx-legacy-dev
liblog4cpp5-dev
libxerces-c-dev
libboost-regex-dev
```

 If you are still getting compile errors, you can look here for other things you may need (like Xcode)
 https://github.com/ncar/aircraft_oap/wiki/MacOS-Build-Environment

### Installation ###
```
 git clone --recursive https://github.com/ncar/aircraft_oap
 cd aircraft_oap
 scons
 scons install
 ```
All programs will be installed in /usr/local on MacOS, otherwise /opt/local.  You may override the install directory, e.g. "scons INSTALL_DIRECTORY=/home/local install" and the binaries will be place in /home/local/bin.

If you just want xpms2d, you can type '**scons xpms2d**'.  The only dependencies for xpms2d are the openmotif and fonts listed above.
