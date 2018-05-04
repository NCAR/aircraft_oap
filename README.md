# aircraft_oap
A collection of programs for manipulating Optical Array Probe data.  Consists of several translators to convert native probe data into a common OAP file format, a processor, and a display program. See the [Wiki](https://github.com/NCAR/aircraft_oap/wiki) page associated with this GitHub repo for dependencies and installation instructions.

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
 * netcdf-cxx-devel (will pull in netcdf-cxx, netcdf-devel, and netcdf)
 * python-devel (needed by vardb)
 * log4cpp-devel (needed by vardb)
 * xerces-c-devel (needed by vardb)
 * boost-devel (needed by vardb)

### Installation ###

 * git clone --recursive https://github.com/ncar/aircraft_oap
 * cd aircraft_oap
 * scons --site-dir=vardb/site_scons
 * scons install --site-dir=vardb/site_scons

All programs will be installed in aircraft_oap/bin by default.

If you wish to compile just a single utility, cd to that utilities subdir (say xpms2d) and run
 * scons -u . --site_dir=vardb/site_scons

To install in a different directory, say your home dir, 
 * scons --site-dir=vardb/site_scons
 * scons install --site-dir=vardb/site_scons --prefix=~
(One should be able to run the --prefix without building first, but so far I haven't fixed that.)
This will put compiled programs in ~/bin
