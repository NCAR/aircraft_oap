#include "ProbeData.h"

#include <cmath>

ProbeData::ProbeData(size_t size) : _size(size)
{
  tas.resize(size, 0.0);
  cpoisson1.resize(size, 0.0);
  cpoisson2.resize(size, 0.0);
  cpoisson3.resize(size, 0.0);
  pcutoff.resize(size, 0.0);
  corrfac.resize(size, 0.0);

  all.accepted.resize(size, 0.0);
  all.rejected.resize(size, 0.0);
  all.total_conc.resize(size, 0.0);
  all.total_conc100.resize(size, 0.0);
  all.total_conc150.resize(size, 0.0);
  all.dbz.resize(size, -100.0);
  all.dbar.resize(size, 0.0);
  all.disp.resize(size, 0.0);
  all.lwc.resize(size, 0.0);
  all.eff_rad.resize(size, 0.0);

  round.accepted.resize(size, 0.0);
  round.rejected.resize(size, 0.0);
  round.total_conc.resize(size, 0.0);
  round.total_conc100.resize(size, 0.0);
  round.total_conc150.resize(size, 0.0);
  round.dbz.resize(size, -100.0);
  round.dbar.resize(size, 0.0);
  round.disp.resize(size, 0.0);
  round.lwc.resize(size, 0.0);
  round.eff_rad.resize(size, 0.0);
}


void ProbeData::ReplaceNANwithMissingData()
{
  for (size_t i = 0; i < all.total_conc.size(); i++)
  {
    if (std::isnan(all.total_conc[i]))
      all.total_conc[i] = all.dbz[i] = all.dbar[i] = all.disp[i] =
		all.lwc[i] = all.eff_rad[i] = all.accepted[i] =
		all.rejected[i] = all.total_conc100[i] = all.total_conc150[i] = -32767.0;
    if (std::isnan(round.total_conc[i]))
      round.total_conc[i] = round.dbz[i] = round.dbar[i] = round.disp[i] =
		round.lwc[i] = round.eff_rad[i] = round.accepted[i] =
		round.rejected[i] = round.total_conc100[i] = round.total_conc150[i] = -32767.0;
  }
}
