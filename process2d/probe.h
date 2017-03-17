#ifndef _probeinfo_h_
#define _probeinfo_h_

#include <iostream>
#include <string>
#include <vector>

/**
 * Probe information.
 */
class ProbeInfo
{
public:
  enum ProbeType { UNKNOWN, PMS2D, HVPS, GREYSCALE, FAST2D, CIP };

  ProbeInfo(std::string new_id, std::string sn, int ndiodes, float res,
		std::string sfx, int binoffset, int numbins)

  : id(new_id), serialNumber(sn), suffix(sfx), nDiodes(ndiodes), resolution(res),
	numBins(numbins), firstBin(binoffset), lastBin(numbins), armWidth(6.1),
	dof_const(2.37), rle(false)
  {
    if (id[0] == 'P') {		// 2DP (P1 - P4)
      armWidth = 26.1;
    }
    if (id[0] == '3') {		// 3V-CPI
      armWidth = 5.08;
      dof_const = 5.13;
    }
    if (id[1] == '8') {		// CIP (C8)
      rle = true;
      armWidth = 10.0;
//armWidth = 7.0;	// FAAM 100um CIP.
      if (id[0] == 'P')
      {
        resolution = 100.0;
        armWidth = 26.0;	// PIP (P8)
      }
//      dof_const = 5.13;
    }
  }

  /**
   * Compute sample area of a probe for given diameter.
   * The sample area unit should be meter^2 for all probes.
   * @param reconstruct Are we doing particle reconstruction, or all-in.
   */
  void ComputeSamplearea(bool reconstruction);

  std::string id;            // Two byte ID at the front of the data-record.
  std::string serialNumber;
  std::string suffix;
  int nDiodes;          // 32, 64, or 128 at this time.
  float resolution;     // micrometers
  int numBins;
  int firstBin, lastBin;
  float armWidth;       // cm
  float dof_const;
  bool rle;		// Data buffers are Run Length Encoded.  CIP/PIP are.

  std::vector<float> bin_endpoints;
  std::vector<float> bin_midpoints;
  std::vector<float> dof;    // Depth Of Field
  std::vector<float> eaw;    // Effective Area Width
  std::vector<float> samplearea;
};

#endif
