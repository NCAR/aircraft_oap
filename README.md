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

To install these programs on your computer:
- git clone --recursive http://github.com/ncar/aircraft_oap
- cd aircraft_oap
- scons install

All programs will be installed in aircraft_oap/bin by default.

