
#ifndef _pads2oap_h_
#define _pads2oap_h_

#include <stdint.h>
#include <string>
#include <vector>

#define TWOD_BUFF_SIZE	4096

// Values currently in use for the 'id' field...at least by NCAR-RAF.
#define PMS2D_C1	0x4331		// First PMS-2DC
#define PMS2D_C2	0x4332		// Second PMS-2DC
#define PMS2D_C4	0x4334		// First RAF 64 diode 25 um Fast 2DC
#define PMS2D_C5	0x4335		// Second RAF 64 diode 25 um Fast 2DC
#define PMS2D_C6	0x4336		// First RAF 64 diode 10 um Fast 2DC
#define PMS2D_C8	0x4338		// First DMT CIP probe.
#define PMS2D_G1	0x4731		// First 2D Greyscale; unused to date
#define PMS2D_H1	0x4831		// SPEC HVPS
#define PMS2D_P1	0x5031		// First PMS-2DP
#define PMS2D_P2	0x5032		// Second PMS-2DP
#define PMS2D_P4	0x5034		// First RAF 64 diode 200 um Fast 2DP
#define PMS2D_P8	0x5038		// First DMT PIP probe
#define PMS2D_3V	0x3348		// 3V-CPI 2DS Vertical
#define PMS2D_3H	0x3356		// 3V-CPI 2DS Horizontal


struct _oap_rec {
  int16_t id;				// 'P1','C1','P2','C2', H1, etc
  int16_t hour;
  int16_t minute;
  int16_t second;
  int16_t year;				// starting in 2007 w/ PACDEX
  int16_t month;			// starting in 2007 w/ PACDEX
  int16_t day;				// starting in 2007 w/ PACDEX
  int16_t tas;				// true air speed, m/s
  int16_t msec;				// msec of this record
  int16_t overld;			// overload time, msec
  unsigned char data[TWOD_BUFF_SIZE];	// image buffer
  };
typedef struct _oap_rec OAP_rec;

struct _pads_rec {
  uint16_t year;
  uint16_t month;
  uint16_t day;
  uint16_t hour;
  uint16_t minute;
  uint16_t second;
  uint16_t msec;
  uint16_t wday;
  unsigned char data[TWOD_BUFF_SIZE];
  };
typedef struct _pads_rec PADS_rec;


class Probe
{
public:
  Probe() : id(0), resolution(0), nDiodes(0) { }

  uint16_t id;
  int resolution;
  int nDiodes;
  std::string serialnumber;
  std::string suffix;
};



class OAPfile
{
public:
  OAPfile() : _out(0) { };
  ~OAPfile();

  void open(const std::string& file_name);

  /**
   * @param id Probe ID, used to distinguish between multiple probes in the same file.
   * @param resolution probe resoultion in micrometers.
   * @param nDiodes is the number of diodes.
   * @param serialnumber is the probe serial number.  Strictly meta-data.
   * @param suffix is used when merging data into a netCDF.  All generated variables
   * will have this suffix.  See process2d(1) program
   */
  void AddProbe(uint16_t id, int resolution, int nDiodes, std::string serialnumber, std::string suffix);

  Probe ProbeData(size_t indx)		{ return _probelist[indx]; }

  void WriteHeader();
  void WriteRecord(const OAP_rec&);

  void SetProject(std::string s)	{ _project = s; }
  void SetPlatform(std::string s)	{ _platform = s; }
  void SetFlightNumber(std::string s)	{ _flightnumber = s; }
  void SetFlightDate(std::string s)	{ _flightdate = s; }

protected:
  FILE *_out;

  std::string _project;
  std::string _platform;
  std::string _flightnumber;
  std::string _flightdate;

  std::vector<Probe> _probelist;
};


#endif
