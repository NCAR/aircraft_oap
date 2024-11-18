/*
 * Probe List:
 *
 * Fast2DC, Fast2DP (C4, C6 P4)
 * Fast2DC_v2, Fast2DP_v2
 * 2DS (SH, SV)
 * 3V-CPI (3H, 3V)
 * CIP, PIP (C8, P8)
 */

#ifndef _probeinfo_h_
#define _probeinfo_h_

#include <iostream>
#include <string>
#include <vector>

#include "config.h"


/**
 * Probe information.
 */
class ProbeInfo
{
public:
  enum ProbeType { UNKNOWN, PMS2D, FAST2D, F2DS, HVPS, GREYSCALE, CIP };
  enum ClockType { FIXED, TAS };

  ProbeInfo(std::string tp, std::string new_id, std::string sn, int ndiodes, float res,
		std::string sfx, int binoffset, int numbins)

  : type(tp), id(new_id), serialNumber(sn), suffix(sfx), nDiodes(ndiodes), resolution(res),
	numBins(numbins), firstBin(binoffset), lastBin(numbins), armWidth(6.1),
	dof_const(2.37), clockMhz(1.0), dofMask(0x01), rle(false), clockType(TAS)
  {

    if (type.find("Fast2D") != std::string::npos)
    {
      clockType = FIXED;
      clockMhz = 12.0e+06;
      timingMask = 0x000000ffffffffffULL;

      if (type.find("_v2") != std::string::npos)
      {
        clockMhz = 33.33333333e+06;
        timingMask = 0x000003ffffffffffULL;
      }
    }

    if (id[0] == 'P') {		// 2DP (P1 - P4)
      armWidth = 26.1;
    }

    if (id[0] == '3')		// 3V-CPI
    {
      armWidth = 5.08;
      dof_const = 5.13;
      clockMhz = 20.0e+06;
      timingMask = 0x0000ffffffffffffULL;
    }

    if (id[0] == 'S')		// 2DS
    {
      armWidth = 6.3;
      dof_const = 5.13;
      clockMhz = 20.0e+06;
      timingMask = 0x0000ffffffffffffULL;
    }

    if (id[0] == 'H')		// HVPS
    {
      armWidth = 16.25;
      dof_const = 5.13;
      clockMhz = 20.0e+06;
      timingMask = 0x00000000ffffffffULL;
    }

    if (type.find("CIP") != std::string::npos ||
        type.find("PIP") != std::string::npos)	// DMT CIP/PIP
    {
      clockType = FIXED;
      clockMhz = 1.0e+06;
      rle = true;
      armWidth = 10.0;
//armWidth = 7.0;	// FAAM 100um CIP.
      if (type.find("PIP") != std::string::npos)
      {
        resolution = 100.0;
        armWidth = 26.0;	// PIP (P8)
      }
    }
  }


  /**
   * User supplied BIN_EDGES, found in the PMSspecs file most likely.
   */
  void SetBinEndpoints(std::string input);

  /**
   * Compute sample area of a probe for given diameter.
   * The sample area unit should be meter^2 for all probes.
   * @param eawmethod Are we doing particle reconstruction, center-in, or all-in.
   */
  void ComputeSamplearea(Config::Method eawmethod);

  std::string type;		// string probe name/type
  std::string id;		// Two byte ID at the front of the data-record.
  std::string serialNumber;
  std::string suffix;
  int nDiodes;			// 32, 64, or 128 at this time.
  float resolution;		// micrometers
  int numBins;
  int firstBin, lastBin;
  float armWidth;       // cm
  float dof_const;
  float clockMhz;	// for timing words.
  unsigned long long timingMask;
  unsigned char dofMask;// Mask for dofReject flag.  CIP/PIP, and Fast2D will use.
  bool rle;		// Data buffers are Run Length Encoded.  CIP/PIP are.
  ClockType clockType;	// Timing words use fixed clock or tas based frequency.

  std::vector<float> bin_endpoints;
  std::vector<float> bin_midpoints;
  std::vector<float> dof;    // Depth Of Field
  std::vector<float> eaw;    // Effective Area Width
  std::vector<float> samplearea;

  // Blank these times per $PROJ_DIR/$PROJECT/$PLATFORM/Production/BlankOAP_rf##
  std::vector<std::pair<time_t, time_t> > blank_out;
};

#endif
