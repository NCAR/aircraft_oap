#include "probe.h"

void ProbeInfo::SetBinEndpoints(std::string input)
{
  for(std::string::size_type p0 = 0, p1 = input.find(',');
        p1!=std::string::npos || p0!=std::string::npos;
        (p0=(p1==std::string::npos) ? p1 : ++p1),p1 = input.find(',', p0) )
  {
    bin_endpoints.push_back( atof(input.c_str()+p0) );
  }

  numBins = bin_endpoints.size() - 1;
}


void ProbeInfo::ComputeSamplearea(Config::Method eawmethod)
{
  // if size greater than zero, then we've already set bin_endpoints.
  if (bin_endpoints.size() == 0)
    for (int i = 0; i < numBins+1; ++i)
      bin_endpoints.push_back((i+0.5) * resolution);

  for (int i = 0; i < numBins; ++i)
  {
    float sa, prht, DoF, eff_wid, diam;

    diam = (bin_endpoints[i] + bin_endpoints[i+1]) / 2.0;

    prht = armWidth * 1.0e4;  //convert cm to microns
    DoF = std::min((dof_const * diam*diam), prht);  // in microns, limit on dof is physical distance between arms

    // Reconstruction, from eq 17 in Heymsfield & Parrish 1978
    if (eawmethod == Config::RECONSTRUCTION) eff_wid = resolution*nDiodes+0.72*diam;
    //All-in, from eq 6 in HP78
    if (eawmethod == Config::ENTIRE_IN) eff_wid = std::max(resolution * (nDiodes-1)-diam, resolution);
    //Center-in
    if (eawmethod == Config::CENTER_IN) eff_wid = resolution*nDiodes;

    sa = DoF * eff_wid * 1e-12;  //compute sa and convert to m^2

    bin_midpoints.push_back(diam);
    dof.push_back(DoF);
    eaw.push_back(eff_wid);
    samplearea.push_back(sa);
  }
}
