#include <vector>
#include <cstdlib>

/**
 * Class to contain and manage data blocks for computed variables/data.
 */
class ProbeData
{
public:
  struct derived
  {
    std::vector<float> accepted;	// Counts of accepted particles.
    std::vector<float> rejected;	// Counts of rejected particles.
    std::vector<float> total_conc, dbz, dbar, disp, lwc, eff_rad;
    std::vector<float> total_conc100, total_conc150;
  };

  ProbeData(size_t size);

  void ReplaceNANwithMissingData();

  int size() const { return _size; }

  std::vector<float> tas;
  std::vector<float> cpoisson1, cpoisson2, cpoisson3;
  std::vector<float> pcutoff, corrfac;

  struct derived all, round;

protected:
  // Number of seconds we are processing / writing into the netCDF file.
  int _size;
};
