/*
-------------------------------------------------------------------------
OBJECT NAME:	DataFile.h

FULL NAME:	Data File Class

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#ifndef _OAP_DATAFILE_H_
#define _OAP_DATAFILE_H_

#ifdef PNG
#include <zlib.h>
#endif

#include <raf/PMS2D.h>
#include <raf/Fast2D.h>
#include <raf/TwoDS.h>
#include <raf/CIP.h>
#include <raf/HVPS.h>
#include <raf/hdrAPI.h>
#include <raf/portable.h>

#include <map>

// ADS image record types
#define ADS_WORD	0x4144
#define HDR_WORD	0x5448
#define SDI_WORD	0x8681
#define AVAPS_WORD	0x4156

#define PMS2_SIZE	4116
#define PMS2_RECSIZE	(0x8000 / PMS2_SIZE) * PMS2_SIZE

namespace OAP
{
  class UserConfig;
}


/**
 * Probe mapping uses the 2 byte key/id in every data record.  Once
 * you have a record, you can get the Probe info from the ProbeList.
 */
typedef std::map<uint16_t, OAP::Probe *> ProbeList;


/* -------------------------------------------------------------------- */
class ADS_DataFile {

public:
  ADS_DataFile(const char fName[], OAP::UserConfig &cfg);
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
  GetPosition() const { return(_indices.size() == 0 ? 0 : 100 * currPhys / _indices.size()); }

//  int	NextSyncRecord(char buff[]);
  bool	LocatePMS2dRecord(OAP::P2d_rec *buff, int h, int m, int s);
  bool	FirstPMS2dRecord(OAP::P2d_rec *buff);
  bool	NextPMS2dRecord(OAP::P2d_rec *buff);
  bool	PrevPMS2dRecord(OAP::P2d_rec *buff);

  int	NextPhysicalRecord(unsigned char buff[]);

  void	ToggleSyntheticData();

  bool	isValidProbe(const unsigned char *pr) const;

  OAP::Probe *ProbeP(uint32_t id)	{ return _probeList[id]; }

  const ProbeList&
  Probes() const { return _probeList; }


protected:
  enum HeaderType { NoHeader, ADS2, OAP };

  struct Index { uint64_t index; int16_t time[4]; };

  void	initADS2(OAP::UserConfig *cfg);
  void	initADS3(const char *hdrString, OAP::UserConfig *cfg);

  /* Add probe based on id word in old ADS2 header.  These are really
   * old files, or from University of Wyoming
   */
  void	AddToProbeList(const char *id, OAP::UserConfig *cfg);
  /* Add probe based on the XML entry in an OAP file.
   */
  void	AddToProbeListFromXML(const char *id, OAP::UserConfig *cfg);

  uint64_t	posOfPhysicalRecord(size_t i) {
	if (i > _indices.size()) fprintf(stderr, "currPhys exceeds nIndices\n");
	return _indices[i].index;
	}

  void	buildIndices(OAP::UserConfig *cfg), sort_the_table(int, int), SortIndices(int);
  time_t getFileModifyTime(const char *path);

  void	SwapPMS2D(OAP::P2d_rec *);
  void	check_rico_half_buff(OAP::P2d_rec *buff, size_t start, size_t end);

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
  uint64_t	_savePos;

  ProbeList	_probeList;

  bool		_gzipped, _useTestRecord;
  HeaderType	_fileHeaderType;

  /**
   * For ADS2, this will be the ADS2 header version number; up to 3.5.
   * For ADS3 and the new OAP file format we force this to start at 5.
   *   add the OAP version to 5.
   */
  char		_version[16];

  std::vector<Index>	_indices;
  int		currPhys;
  int		currLR;

  OAP::P2d_rec	physRecord[P2DLRPR], *testRecP;

};	/* END DATAFILE.H */

#endif
