/*
-------------------------------------------------------------------------
OBJECT NAME:	Probe.h

FULL NAME:	Probe Information

COPYRIGHT:	University Corporation for Atmospheric Research, 1997
-------------------------------------------------------------------------
*/

#ifndef PROBE_H
#define PROBE_H

#include "define.h"

#include <raf/hdrAPI.h>


/* ID values for the id field in each record header.
 */
// Traditional 32 diode PMS2D probes.
#define PMS2DC1         0x4331
#define PMS2DC2         0x4332
#define PMS2DP1         0x5031
#define PMS2DP2         0x5032

// 64 diode Fast 2DC, 25um.
#define PMS2DC4         0x4334
#define PMS2DC5         0x4335
// 64 diode Fast 2DC, 10um.
#define PMS2DC6         0x4336
// 64 diode Fast 2DP, 200um.
#define PMS2DP4         0x5034

// 64 diode DMT CIP, 25um.
#define PMS2DC8         0x4338
// 64 diode DMT PIP, 100um.
#define PMS2DP8         0x5038

// Greyscale which we never flew.
#define PMS2DG1         0x4731
#define PMS2DG2         0x4732

// SPEC HVPS
#define HVPS1           0x4831
#define HVPS2           0x4832
// SPEC 3V-CPI
#define SPEC2D3H        0x3348
#define SPEC2D3V        0x3356
// SPEC 2DS
#define SPEC2DSH        0x5348
#define SPEC2DSV        0x5356

#define maxDiodes	(256)

/* -------------------------------------------------------------------- */
class Probe {

public:
  enum ProbeType { UNKNOWN, PMS2D, HVPS, GREYSCALE, FAST2D, TWODS, CIP };

  /**
   * Probe conctructor for ADS2 header.
   */
  Probe(Header *hdr, const Pms2 *p, int cnt);
  /**
   * Probe constructor for new PMS2D data files.  New means starting
   * in 2007 with PACDEX project.
   */
  Probe(const char xml_string[], int recSize);
  /**
   * Probe constructor for no file header.  Univ Wyoming.
   */
  Probe(const char hdr[]);

  virtual ~Probe() { }

  const std::string & Name() const	{ return _name; }
  const std::string & SerialNum() const	{ return _serialNumber; }

  const char * Code() const		{ return _code; }

  /// @returns Number of Diodes.
  size_t Resolution() const	{ return _resolution; }
  /// @returns Probe resolution per diode.
  size_t nDiodes() const	{ return _nDiodes; }
  size_t nSlices() const	{ return _nSlices; }

  bool Display() const		{ return _displayed; }
  void setDisplay(bool b)	{ _displayed = b; }

  virtual struct recStats ProcessRecord(const P2d_rec * record, float version) = 0;

  virtual bool isSyncWord(const unsigned char *p) = 0;

  ProbeType Type() const	{ return _type; }

  /**
   * Set sample area based on user selected calculation type Entire-in
   * vs center-in, etc.
   */
  void SetSampleArea();


  /// @returns Logical-record length.  Exabyte hangover.
  long lrLen() const		{ return _lrLen; }
  /// @returns Logical-records per physical record.  Exabyte hangover.
  int lrPpr() const		{ return _lrPpr; }

  static const unsigned char BlankSlice[16];

  /// Statistics and derived data from current ProcessRecord()
  /// @TODO this should be refactored out of public.
  struct recStats stats;


protected:

  void computeDerived(double sv[], size_t nBins, double liveTime);
  size_t checkRejectionCriteria(Particle * cp, recStats & stats);

  std::string	_name;
  std::string	_serialNumber;

  char		_code[4];
  ProbeType	_type;
  size_t	_resolution;
  size_t	_nDiodes;
  /// Number of slices per 4k buffer.
  size_t	_nSlices;

  long	_lrLen;
  int	_lrPpr;

  bool	_displayed;

  static const float diodeDiameter;

  float sampleAreaC[maxDiodes], sampleAreaP[maxDiodes];

};

#endif
