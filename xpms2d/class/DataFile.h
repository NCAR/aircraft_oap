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

#include "PMS2D.h"
#include "Fast2D.h"
#include "TwoDS.h"
#include "CIP.h"
#include "HVPS.h"

#include <map>

// ADS image record types
#define ADS_WORD	0x4144
#define HDR_WORD	0x5448
#define SDI_WORD	0x8681
#define AVAPS_WORD	0x4156

#define PMS2_SIZE	4116
#define PMS2_RECSIZE	(0x8000 / PMS2_SIZE) * PMS2_SIZE

typedef std::map<uint32_t, Probe *> ProbeList;


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

  int	NextPhysicalRecord(unsigned char buff[]);

  void	ToggleSyntheticData();

  static Probe::ProbeType ProbeType(const unsigned char *id);

  bool	isValidProbe(const unsigned char *pr) const;

  const ProbeList&
  Probes() const { return _probeList; }

protected:
  enum HeaderType { NoHeader, ADS2, OAP };

  typedef struct { long long index; int16_t time[4]; } Index;

  void	initADS2();
  void	initADS3(const char *hdrString);

  /* Add probe based on id word in old ADS2 header.  These are really
   * old files, or from University of Wyoming
   */
  void	AddToProbeList(const char *id);
  /* Add probe based on the XML entry in an OAP file.
   */
  void	AddToProbeListFromXML(const char *id);

  long long	posOfPhysicalRecord(size_t i) {
	if (i > nIndices) fprintf(stderr, "currPhys exceeds nIndices\n");
	return indices[i].index;
	}

  void	buildIndices(), sort_the_table(int, int), SortIndices(int);
  void	SwapPMS2D(P2d_rec *);
  void	check_rico_half_buff(P2d_rec *buff, size_t start, size_t end);

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
