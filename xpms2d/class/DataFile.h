/*
-------------------------------------------------------------------------
OBJECT NAME:	DataFile.h

FULL NAME:	Data File Class

COPYRIGHT:	University Corporation for Atmospheric Research, 1997
-------------------------------------------------------------------------
*/

#ifndef DATAFILE_H
#define DATAFILE_H

#ifdef PNG
#include <zlib.h>
#endif

#include <define.h>

#include "Probe.h"
#include <raf/hdrAPI.h>

#include <vector>

/* ADS image record types */
#define ADS_WORD	0x4144
#define HDR_WORD	0x5448
#define SDI_WORD	0x8681
#define AVAPS_WORD	0x4156

/* ID values for the id field in each record header.
 */
// Traditional 32 diode PMS2D probes.
#define PMS2DC1		0x4331
#define PMS2DC2		0x4332
#define PMS2DP1		0x5031
#define PMS2DP2		0x5032

// 64 diode Fast 2DC, 25um.
#define PMS2DC4		0x4334
#define PMS2DC5		0x4335
// 64 diode Fast 2DC, 10um.
#define PMS2DC6		0x4336
// 64 diode Fast 2DP, 200um.
#define PMS2DP4		0x5034

// 64 diode DMT CIP, 25um.
#define PMS2DC8		0x4338

// Greyscale which we never flew.
#define PMS2DG1		0x4731
#define PMS2DG2		0x4732
// SPEC HVPS
#define HVPS1		0x4831
#define HVPS2		0x4832
// SPEC 2DS / 3V-CPI
#define SPEC2DSH	0x3348
#define SPEC2DSV	0x3356
 

#define PMS2_SIZE	4116
#define PMS2_RECSIZE	(0x8000 / PMS2_SIZE) * PMS2_SIZE

typedef std::vector<Probe *> ProbeList;


/* -------------------------------------------------------------------- */
class ADS_DataFile {

public:
  ADS_DataFile(const char fName[]);
  ~ADS_DataFile();

  const std::string &
  FileName() const	{ return _fileName; }

  /**
   * For ADS2, this will be the ADS2 header version number; up to 3.5.
   * For ADS3 and the new OAP file format we force this to start at 5.
   *   add the OAP version to 5.
   */
  const char *
  HeaderVersion() const	{ return _version; }

  const std::string &
  ProjectNumber() const	{ return _projectName; }

  const std::string &
  FlightNumber() const	{ return _flightNumber; }

  const std::string &
  FlightDate() const	{ return _flightDate; }

  void	SetPosition(int position);

  int
  GetPosition() const { return(nIndices == 0 ? 0 : 100 * currPhys / nIndices); }

//  int	NextSyncRecord(char buff[]);
  bool	LocatePMS2dRecord(P2d_rec *buff, int h, int m, int s);
  bool	FirstPMS2dRecord(P2d_rec *buff);
  bool	NextPMS2dRecord(P2d_rec *buff);
  bool	PrevPMS2dRecord(P2d_rec *buff);

  int	NextPhysicalRecord(char buff[]);

  void	ToggleSyntheticData();

  bool	isValidProbe(const char *pr) const;

  const ProbeList&
  Probes() const { return _probeList; }

protected:
  enum HeaderType { NoHeader, ADS2, OAP };

  typedef struct { long long index; int16_t time[4]; } Index;

  void		initADS2();
  void		initADS3(char *hdrString);

  long long	posOfPhysicalRecord(size_t i) {
	if (i > nIndices) fprintf(stderr, "currPhys exceeds nInices\n");
	return indices[i].index;
	}
  long long	ntohll(long long * p) const;

  void		buildIndices(), sort_the_table(int, int), SortIndices(int);
  void		SwapPMS2D(P2d_rec *);
  void		check_rico_half_buff(P2d_rec *buff, size_t start, size_t end);

  std::string	_fileName;

  std::string	_projectName;
  std::string	_flightNumber;
  std::string	_flightDate;

  Header	*_hdr;
#ifdef PNG
  gzFile	gz_fd;
#else
  int		gz_fd;
#endif
  FILE		*fp;
  long long	_savePos;

  ProbeList	_probeList;

  bool		_gzipped, _useTestRecord;
  HeaderType	_fileHeaderType;

  /**
   * For ADS2, this will be the ADS2 header version number; up to 3.5.
   * For ADS3 and the new OAP file format we force this to start at 5.
   *   add the OAP version to 5.
   */
  char		_version[16];

  Index		*indices;
  int		currPhys;
  int		currLR;
  size_t	nIndices;

  P2d_rec	physRecord[P2DLRPR], *testRecP;

};	/* END DATAFILE.H */

#endif
