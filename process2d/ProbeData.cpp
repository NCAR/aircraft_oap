#include "ProbeData.h"

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
