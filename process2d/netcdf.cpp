#include "config.h"
#include "netcdf.h"
#include "probe.h"
#include "ProbeData.h"

#include <raf/vardb.h>

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace std;

const char *netCDF::ISO8601_Z = "%FT%T %z";


netCDF::netCDF(Config & cfg) : _outputFile(cfg.outputFile), _file(0), _mode(NcFile::Write), _tas(0)
{
  // No file to pre-open or file does not exist.  Bail out.
  if (_outputFile.size() == 0 || access(_outputFile.c_str(), F_OK))
    return;

  // Check for write permission.
  if (access(_outputFile.c_str(), W_OK))
  {
    cerr << "Write permission failure on output file "
	<< _outputFile << endl;
    exit(1);
  }

  // Attempt to open for writing.
  _file = new NcFile(_outputFile.c_str(), _mode);

  // Failed to open netCDF file for writing....
  if (!_file->is_valid())
  {
    cerr << "Failed to open netCDF output file "
	<< _outputFile << endl;
    exit(1);
  }

  readStartEndTime(cfg);

  // Check for existence of TASX variable.
  _tas = _file->get_var("TASX");

  if (_tas)
    cout << "TASX variable found, will use instead of tas in 2D records.\n";

  InitVarDB();
}

void netCDF::InitVarDB()
{
  char *proj_dir = getenv("PROJ_DIR");

  if (proj_dir == 0)
  {
    cout << "Environment variable PROJ_DIR undefined." << endl;
    return;
  }

  char file[1024];
  sprintf(file, "%s/Configuration/raf/VarDB", proj_dir);

  if (InitializeVarDB(file) == ERR)
  {
    cout << "Failed to initialize variable database file : " << file << endl;
    return;
  }
}

netCDF::~netCDF()
{
  _file->close();
}

void netCDF::CreateNetCDFfile(const Config & cfg)
{
  if (_file && _file->is_valid())	// Was successfully opened in ctor, leave.
    return;

  /* If user did not specify output file, then create base + .nc of input file.
   * And destroy an existing file if it exists.
   */
  if (_outputFile.size() == 0) {
    string ncfilename;
    size_t pos = cfg.inputFile.find_last_of("/\\");
    ncfilename = cfg.inputFile.substr(pos+1);
    pos = ncfilename.find_last_of(".");
    _outputFile = ncfilename.substr(0,pos) + ".nc";
    _mode = NcFile::Replace;
  }
  else {
    // User specified an output filname on command line, see if file exists.
    NcFile test(_outputFile.c_str(), NcFile::Write);
    if (!test.is_valid())
      _mode = NcFile::New;
  }

  _file = new NcFile(_outputFile.c_str(), _mode);

  /* If we are creating a file from scratch, perform the following.
   */
  if (_mode != NcFile::Write)
  {
    struct tm st, et;
    gmtime_r(&cfg.starttime, &st);
    gmtime_r(&cfg.stoptime, &et);

    _file->add_att("institution", "NCAR Research Aviation Facility");
    _file->add_att("Source", "NCAR/RAF Fast-2D Processing Software");
    _file->add_att("Conventions", "NCAR-RAF/nimbus");
    _file->add_att("ConventionsURL", "http://www.eol.ucar.edu/raf/Software/netCDF.html");
    _file->add_att("ProjectName", cfg.project.c_str());
    _file->add_att("FlightNumber", cfg.flightNumber.c_str());
    _file->add_att("FlightDate", cfg.flightDate.c_str());
    _file->add_att("date_created", dateProcessed().c_str());
    char tmp[128];
    sprintf(tmp, "%02d:%02d:%02d-%02d:%02d:%02d",
	st.tm_hour, st.tm_min, st.tm_sec, et.tm_hour, et.tm_min, et.tm_sec);
    _file->add_att("TimeInterval", tmp);
    _file->add_att("ReconstructionMethod", cfg.eawmethod);
    _file->add_att("SizeMethod", cfg.smethod);
    _file->add_att("Raw_2D_Data_File", cfg.inputFile.c_str());
  }
}

void netCDF::readTrueAirspeed(float tas[], size_t n)
{
  assert ((int)n == _tas->num_vals());

  NcValues *data = _tas->values();
  for (size_t i = 0; i < n; ++i)
    tas[i] = data->as_float(i);

  delete data;
}


std::string netCDF::dateProcessed()
{
  time_t        t;
  struct tm     tm;
  char buffer[64];

  t = time(0);
  tm = *localtime(&t);
  strftime(buffer, 64, ISO8601_Z, &tm);

  return(std::string(buffer));
}


void netCDF::readStartEndTime(Config & cfg)
{
  NcVar *var;

  if ((var = _file->get_var("Time")) == 0)
    var = _file->get_var("time");

  /* If we found a Time variable, acquire the start/end time from the netCDF
   * file.  This is the time frame we want to process against.
   */
  if (var)
  {
    if (var->num_vals() == 0)
      return;

    NcValues *v = var->values();
    NcAtt *units, *frmt_att;
    char *frmt;

    units = var->get_att("units");
    frmt_att = var->get_att("strptime_format");

    if (units == 0)
    {
      cerr << "No units for Time variable, fatal." << endl;
      exit(1);
    }

    if (frmt_att == 0)
    {
      frmt = (char *)"seconds since %F %T %z";
    }
    else
      frmt = frmt_att->as_string(0);

    struct tm tm;
    time_t st, et;
    strptime(units->as_string(0), frmt, &tm);
    st = mktime(&tm) + v->as_int(0);
    et = mktime(&tm) + v->as_int(var->num_vals()-1);
    cout << "NetCDF start time : " << ctime(&st);
    cout << "         end time : " << ctime(&et);


    if (cfg.user_starttime.length())
    {
      int h, m, s;
      sscanf(cfg.user_starttime.c_str(), "%02d%02d%02d", &h, &m, &s);
      tm.tm_hour = h;
      tm.tm_min = m;
      tm.tm_sec = s;
      st = mktime(&tm);
    }

    if (cfg.user_stoptime.length())
    {
      int h, m, s;
      sscanf(cfg.user_stoptime.c_str(), "%02d%02d%02d", &h, &m, &s);
      tm.tm_hour = h;
      tm.tm_min = m;
      tm.tm_sec = s;
      et = mktime(&tm);
    }

    cout << "User start time : " << ctime(&st);
    cout << "       end time : " << ctime(&et);

    cfg.starttime = st;
    cfg.stoptime = et;

    delete v;
  }
}

NcDim *netCDF::addDimension(const char name[], int size)
{
  NcDim *dim = _file->get_dim(name);

  if (dim) {
    cout << "Found dimension " << name << endl;
  }
  else {
    dim = _file->add_dim(name, size);
    if (dim)
      cout << "Created dimension " << name << endl;
    else
      cout << "Failed to create dimension " << name << endl;
  }

  return dim;
}

NcVar *netCDF::addTimeVariable(const Config & cfg, int size)
{
  _timevar = _file->get_var("Time");

  if (_timevar)
    return _timevar;

  char timeunits[70];
  int year, month, day;
  struct tm st;
  gmtime_r(&cfg.starttime, &st);

  sscanf(cfg.flightDate.c_str(), "%d/%d/%d", &month, &day, &year);
  sprintf(timeunits,"seconds since %04d-%02d-%02d %02d:%02d:%02d +0000", year,month,day,st.tm_hour,st.tm_min,st.tm_sec);
  _timevar = _file->add_var("Time", ncInt, _timedim);

  if (_timevar == 0)
  {
    cout << "Failed to create Time variable.\n";
    return _timevar;
  }
  _timevar->add_att("long_name", "time of measurement");
  _timevar->add_att("standard_name", "time");
  _timevar->add_att("units", timeunits);
  _timevar->add_att("strptime_format", "seconds since %F %T %z");

  int time[size];
  for (int i = 0; i < size; i++) time[i] = i;
  if (!_timevar->put(time, size))
    cout << "Failed to write Time data to file.\n";

  return _timevar;
}

void netCDF::CreateDimensions(int numtimes, ProbeInfo &probe, const Config &cfg)
{
  char tmp[1024];

  // Define the dimensions.
  _timedim = addDimension("Time", numtimes);
  _spsdim = addDimension("sps1", 1);
  sprintf(tmp, "Vector%d", probe.numBins);
  _bindim = addDimension(tmp, probe.numBins);
  sprintf(tmp, "Vector%d", probe.numBins+1);
  _bindim_plusone = addDimension(tmp, probe.numBins+1);
  _intbindim = addDimension("interarrival_endpoints", cfg.nInterarrivalBins+1);
}


NcVar *netCDF::addHistogram(string& varname, string& serialNumber)
{
  NcVar *var;
  NcDim *len_dim = _bindim_plusone;

  const char *units = VarDB_GetUnits(varname.c_str());
  // Make an attempt at units if VarDB returned 'Unk'.
  if (strcmp(units, "Unk") == 0)
  {
    if (varname.compare(0, 3, "A2D") == 0) units = "count";
    if (varname.compare(0, 3, "C2D") == 0) units = "#/L";
  }

  /* Inter-arrival time array has a different histogram length,
   * kind of cheesy way to handle it...
   */
  if (varname.compare(0, 3, "I2D") == 0) len_dim = _intbindim;

  if ((var = _file->get_var(varname.c_str())) == 0)
  {
    if ((var = _file->add_var(varname.c_str(), ncFloat, _timedim, _spsdim, len_dim)))
    {
      var->add_att("_FillValue", (float)(-32767.0));
      var->add_att("units", units);
      var->add_att("long_name", VarDB_GetTitle(varname.c_str()));
      var->add_att("Category", "PMS Probe");
      var->add_att("SerialNumber", serialNumber.c_str());
      var->add_att("DataQuality", "Good");
    }
  }
  if (var) var->add_att("DateProcessed", dateProcessed().c_str());

  return var;
}

NcVar *netCDF::addVariable(string& varname, string& serialNumber)
{
  NcVar *var;

  const char *units = VarDB_GetUnits(varname.c_str());
  // Make an attempt at units if VarDB returned 'Unk'.
  if (strcmp(units, "Unk") == 0)
  {
    if (varname.compare(0, 4, "CONC") == 0) units = "#/L";
    if (varname.compare(0, 4, "PLWC") == 0) units = "gram/m3";
    if (varname.compare(0, 4, "DBZ") == 0) units = "dBz";
    if (varname.compare(0, 4, "DBAR") == 0) units = "um";
  }

  if ((var = _file->get_var(varname.c_str())) == 0)
  {
    if ((var = _file->add_var(varname.c_str(), ncFloat, _timedim)))
    {
      var->add_att("_FillValue", (float)(-32767.0));
      var->add_att("units", units);
      var->add_att("long_name", VarDB_GetTitle(varname.c_str()));
      var->add_att("Category", "PMS Probe");
      var->add_att("SerialNumber", serialNumber.c_str());
      var->add_att("DataQuality", "Good");
    }
  }
  return var;
}


int netCDF::WriteData(ProbeInfo& probe, ProbeData& data)
{
  NcVar *var;
  string varname;

  //Total counts and LWC
  varname="CONC2DCA"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.all.total_conc[0], data.size());

  varname="CONC2DCR"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.round.total_conc[0], data.size());

  /* output 100 micron and 150 micron and above total concentrations.
   * this should be some command line flag at some point.
   */
  if (false)
  {
    varname="CONC2DCA100"+probe.suffix; varname[6] = probe.id[0];
    if ((var = addVariable(varname, probe.serialNumber)))
      var->put(&data.all.total_conc100[0], data.size());

    varname="CONC2DCR100"+probe.suffix; varname[6] = probe.id[0];
    if ((var = addVariable(varname, probe.serialNumber)))
      var->put(&data.round.total_conc100[0], data.size());

    varname="CONC2DCA150"+probe.suffix; varname[6] = probe.id[0];
    if ((var = addVariable(varname, probe.serialNumber)))
      var->put(&data.all.total_conc150[0], data.size());

    varname="CONC2DCR150"+probe.suffix; varname[6] = probe.id[0];
    if ((var = addVariable(varname, probe.serialNumber)))
      var->put(&data.round.total_conc150[0], data.size());
  }

  varname="PLWC2DCR"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.round.lwc[0], data.size());

  varname="PLWC2DCA"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.all.lwc[0], data.size());

  varname="DBAR2DCR"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.round.dbar[0], data.size());

  varname="DBAR2DCA"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.all.dbar[0], data.size());

  varname="DISP2DCR"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.round.disp[0], data.size());

  varname="DISP2DCA"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.all.disp[0], data.size());

  varname="DBZ2DCR"+probe.suffix; varname[5] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.round.dbz[0], data.size());

  varname="DBZ2DCA"+probe.suffix; varname[5] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.all.dbz[0], data.size());

  varname="REFF2DCR"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.round.eff_rad[0], data.size());

  varname="REFF2DCA"+probe.suffix; varname[6] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.all.eff_rad[0], data.size());

  varname="NACCEPT2DCR"+probe.suffix; varname[9] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.round.accepted[0], data.size());

  varname="NACCEPT2DCA"+probe.suffix; varname[9] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.all.accepted[0], data.size());

  varname="NREJECT2DCR"+probe.suffix; varname[9] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.round.rejected[0], data.size());

  varname="NREJECT2DCA"+probe.suffix; varname[9] = probe.id[0];
  if ((var = addVariable(varname, probe.serialNumber)))
    var->put(&data.all.rejected[0], data.size());
  
  
  /* These variables are only output when generating a stand alone netCDF file.
   * i.e. They are not ouput if the -o command line is specified and it finds
   * an existing TAS.
   */
  if (_tas == 0)
  {
    varname="poisson_coeff1"+probe.suffix;
    if ((var = _file->get_var(varname.c_str())) == 0) {
      if (!(var = _file->add_var(varname.c_str(), ncFloat, _timedim))) return netCDF::NC_ERR;
      if (!var->add_att("units", "unitless")) return netCDF::NC_ERR;
      if (!var->add_att("long_name", "Interarrival Time Fit Coefficient 1")) return netCDF::NC_ERR;
    }
    if (!var->put(&data.cpoisson1[0], data.size())) return netCDF::NC_ERR;

    varname="poisson_coeff2"+probe.suffix;
    if ((var = _file->get_var(varname.c_str())) == 0) {
      if (!(var = _file->add_var(varname.c_str(), ncFloat, _timedim))) return netCDF::NC_ERR;
      if (!var->add_att("units", "1/seconds")) return netCDF::NC_ERR;
      if (!var->add_att("long_name", "Interarrival Time Fit Coefficient 2")) return netCDF::NC_ERR;
    }
    if (!var->put(&data.cpoisson2[0], data.size())) return netCDF::NC_ERR;

    varname="poisson_coeff3"+probe.suffix;
    if ((var = _file->get_var(varname.c_str())) == 0) {
      if (!(var = _file->add_var(varname.c_str(), ncFloat, _timedim))) return netCDF::NC_ERR;
      if (!var->add_att("units", "1/seconds")) return netCDF::NC_ERR;
      if (!var->add_att("long_name", "Interarrival Time Fit Coefficient 3")) return netCDF::NC_ERR;
    }
    if (!var->put(&data.cpoisson3[0], data.size())) return netCDF::NC_ERR;

    varname="poisson_cutoff"+probe.suffix;
    if ((var = _file->get_var(varname.c_str())) == 0) {
      if (!(var = _file->add_var(varname.c_str(), ncFloat, _timedim))) return netCDF::NC_ERR;
      if (!var->add_att("units", "seconds")) return netCDF::NC_ERR;
      if (!var->add_att("long_name", "Interarrival Time Lower Limit")) return netCDF::NC_ERR;
    }
    if (!var->put(&data.pcutoff[0], data.size())) return netCDF::NC_ERR;

    varname="poisson_correction"+probe.suffix;
    if ((var = _file->get_var(varname.c_str())) == 0) {
      if (!(var = _file->add_var(varname.c_str(), ncFloat, _timedim))) return netCDF::NC_ERR;
      if (!var->add_att("units", "unitless")) return netCDF::NC_ERR;
      if (!var->add_att("long_name", "Count/Concentration Correction Factor for Interarrival Rejection")) return netCDF::NC_ERR;
    }
    if (!var->put(&data.corrfac[0], data.size())) return netCDF::NC_ERR;

    varname="TAS"+probe.suffix;
    if ((var = _file->get_var(varname.c_str())) == 0) {
      if (!(var = _file->add_var(varname.c_str(), ncFloat, _timedim))) return netCDF::NC_ERR;
      if (!var->add_att("units", "m/s")) return netCDF::NC_ERR;
      if (!var->add_att("long_name", "True Air Speed")) return netCDF::NC_ERR;
    }
    if (!var->put(&data.tas[0], data.size())) return netCDF::NC_ERR;
  
    varname="SA"+probe.suffix;
    if ((var = _file->get_var(varname.c_str())) == 0) {
      if (!(var = _file->add_var(varname.c_str(), ncFloat, _bindim))) return netCDF::NC_ERR;
      if (!var->add_att("units", "m2")) return netCDF::NC_ERR;
      if (!var->add_att("long_name", "Sample area per channel")) return netCDF::NC_ERR;
    }
    if (!var->put(&probe.samplearea[0], probe.numBins)) return netCDF::NC_ERR;
  
    //Bins  
    varname="bin_endpoints"+probe.suffix;
    if ((var = _file->get_var(varname.c_str())) == 0) {
      if (!(var = _file->add_var(varname.c_str(), ncFloat, _bindim_plusone))) return netCDF::NC_ERR;
      if (!var->add_att("units", "microns")) return netCDF::NC_ERR;
    }
    if (!var->put(&probe.bin_endpoints[0], probe.numBins+1)) return netCDF::NC_ERR; 

    varname="bin_midpoints"+probe.suffix;
    if ((var = _file->get_var(varname.c_str())) == 0) {
      if (!(var = _file->add_var(varname.c_str(), ncFloat, _bindim))) return netCDF::NC_ERR;
      if (!var->add_att("units", "microns")) return netCDF::NC_ERR;
      if (!var->add_att("long_name", "Size Channel Midpoints")) return netCDF::NC_ERR;
    }
    if (!var->put(&probe.bin_midpoints[0], probe.numBins)) return netCDF::NC_ERR;
  }

  return 0;
}
