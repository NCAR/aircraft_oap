#include <ncFile.h>
#include <ncDim.h>
#include <ncVar.h>
#include <ncAtt.h>
#include <ncType.h>

class Config;
class ProbeInfo;
class ProbeData;

using namespace netCDF;

/**
 * Class to handle opening/creating the netCDF file and writing
 * variable data.
 */
class NetCDF
{
public:
  NetCDF(Config & cfg);
  ~NetCDF();

  NcFile *ncid() const { return _file; }

  void CreateNetCDFfile(const Config & cfg);

  void CreateDimensions(int numtimes, ProbeInfo &probe, const Config &cfg);

  /**
   * Add dimension to netCDF file.  If dimension already exists,
   * then a pointer to that dimension is returned.
   */
  NcDim addDimension(const char name[], int size);

  NcVar addTimeVariable(const Config & cfg, int size);

  bool hasTASX()
  { return _tas.isNull() ? false : true; }

  /**
   * Check for the existence of TASX.  Read it into provided space.
   */
  void readTrueAirspeed(float tas[], size_t n);

  int WriteData(ProbeInfo & probe, ProbeData & data);

/*
  NcDim *timedim() const { return _timedim; }
  NcDim *spsdim() const { return _spsdim; }
  NcDim *bindim() const { return _bindim; }
  NcDim *bindim_plusone() const { return _bindim_plusone; }
*/
  NcDim intbindim() const { return _intbindim; }

  NcVar addHistogram(std::string& vname, const ProbeInfo& probe, int binoffset);
  NcVar addVariable(std::string& name, std::string& serialNumber);

  void  putGlobalAttribute(const char attrName[], float value);
  void  putGlobalAttribute(const char attrName[], const char *value);
  void  putGlobalAttribute(const char attrName[], const std::string value);
  void  putVarAttribute(NcVar & var, const char attrName[], const char *value);
  void  putVarAttribute(NcVar & var, const char attrName[], float value);
  void  putVarAttribute(NcVar & var, const char attrName[], int value);
  void  putVarAttribute(NcVar & var, const char attrName[], std::vector<float> values);
  void  putVarAttribute(NcVar & var, const char attrName[], const std::string value);

  static const int NC_ERR = 2;

private:
  void InitVarDB();

  void readStartEndTime(Config & cfg);

  std::string dateProcessed();

  std::string _outputFile;

  NcFile *_file;
  NcFile::FileMode _mode;

  NcDim _timedim, _spsdim, _bindim, _bndsdim, _bindim_plusone, _intbindim;
  NcVar _timevar;
  NcVar _tas;

  static const char *ISO8601_Z;
  static const char *Category;
};
