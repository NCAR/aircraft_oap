#include "probe.h"

void ProbeInfo::ComputeSamplearea(char eawmethod)
{
  for (int i = 0; i < numBins+1; ++i)
    bin_endpoints.push_back((i+0.5) * resolution);

  for (int i = 0; i < numBins; ++i)
  {
    float sa, prht, DoF, eff_wid, diam;

    diam = (bin_endpoints[i] + bin_endpoints[i+1]) / 2.0;

    prht = armWidth * 1.0e4;  //convert cm to microns
    DoF = std::min((dof_const * diam*diam), prht);  // in microns, limit on dof is physical distance between arms

    // Reconstruction, from eq 17 in Heymsfield & Parrish 1978
    if (eawmethod=='r') eff_wid = resolution*nDiodes+0.72*diam;
    //All-in, from eq 6 in HP78
    if (eawmethod=='a') eff_wid = std::max(resolution * (nDiodes-1)-diam, resolution);
    //Center-in
    if (eawmethod=='c') eff_wid = resolution*nDiodes;

    sa = DoF * eff_wid * 1e-12;  //compute sa and convert to m^2 

    bin_midpoints.push_back(diam);
    dof.push_back(DoF);
    eaw.push_back(eff_wid);
    samplearea.push_back(sa);
  }
}
