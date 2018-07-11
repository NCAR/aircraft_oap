/*
-------------------------------------------------------------------------
OBJECT NAME:	Probe.h

FULL NAME:	Probe Information

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
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


/* -------------------------------------------------------------------- */
class Probe {

public:
  enum ProbeType { UNKNOWN, PMS2D, HVPS, GREYSCALE, FAST2D, TWODS, CIP };

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

  virtual struct recStats
  ProcessRecord(const P2d_rec * record, float version) = 0;

  virtual bool
  isSyncWord(const unsigned char *p) = 0;
  virtual bool
  isOverloadWord(const unsigned char *p)	{ return false; }
  virtual bool
  isBlankSlice(const unsigned char *p)	{ return memcmp(p, BlankSlice, nDiodes()/8) == 0; }

  ProbeType Type() const	{ return _type; }

  /**
   * Set sample area based on user selected calculation type Entire-in
   * vs Center-in, etc.  Output is sampleArea[] in mm^2
   */
  void SetSampleArea();

  /// Crude sample area based on armWidth, nDiodes, and resolution.
  float SampleArea() const	{ return _sampleArea; }

  /// @returns Logical-record length.  Exabyte hangover.
  long lrLen() const		{ return _lrLen; }
  /// @returns Logical-records per physical record.  Exabyte hangover.
  int lrPpr() const		{ return _lrPpr; }

  static const unsigned char BlankSlice[16];

  /// Statistics and derived data from current ProcessRecord()
  /// @TODO this should be refactored out of public.
  struct recStats stats;


protected:
  /**
   * Probe conctructor for ADS2 header.  This will only ever have 32 diode
   * PMS2D probes an HVPS once.
   */
  Probe(ProbeType type, Header *hdr, const Pms2 *p, int cnt, size_t ndiodes);
  /**
   * Probe constructor for new PMS2D data files.  New means starting
   * in 2007 with PACDEX project.  These files follow the OAPfiles standard.
   */
  Probe(ProbeType type, const char xml_string[], int recSize, size_t ndiodes);
  /**
   * Probe constructor for no file header.  Univ Wyoming.
   */
  Probe(ProbeType type, const char hdr[], size_t ndiodes);

  /// Common initialization for all constructors.
  void init();

  void ClearStats(const P2d_rec *record);
  void checkEdgeDiodes(Particle * cp, const unsigned char *p);
  size_t area(const unsigned char *p);
  size_t height(const unsigned char *p);
  void computeDerived(double sv[], size_t nBins, double liveTime);
  size_t checkRejectionCriteria(Particle * cp, recStats & stats);

  std::string	_name;
  std::string	_serialNumber;

  char		_code[4];
  ProbeType	_type;

  long	_lrLen;
  int	_lrPpr;

  /// Probe resolution per diode in micron
  size_t	_resolution;

  /// Number of diodes in the array
  size_t	_nDiodes;

  /// Physical distance between the two probe arms in mm.
  float		_armWidth;

  /// Effective Area Width in mm.
  float		_eaw;

  /// Depth of Field constant.
  float		_dof_const;

  /// Number of slices per 4k buffer.
  size_t	_nSlices;


  /// IS this probe being displayed.
  bool		_displayed;


  // Store prevtime and hdr for ProcessRecord, so we can compute elapsed time.
  P2d_hdr	_prevHdr;
  int		_prevTime;
  unsigned long long _prevTimeWord;

  /// Crude sample area - as opposed to the 'per bin' sample area.
  float		_sampleArea;

  /// Sample area per bin in mm^2
  float *sampleArea;

  /// Current Particle data.  Class member so it can carry across records.
  Particle	*cp;

  static const float diodeDiameter;

};

#endif
