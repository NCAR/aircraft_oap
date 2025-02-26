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

const char *NetCDF::ISO8601_Z = "%FT%T %z";
const char *NetCDF::Category = "CloudAndPrecip";


/* -------------------------------------------------------------------- */
NetCDF::NetCDF(Config & cfg) : _outputFile(cfg.outputFile), _file(0), _mode(NcFile::write)
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
  if (_file->isNull())
  {
    cerr << "Failed to open netCDF output file " << _outputFile << endl;
    exit(1);
  }

  readStartEndTime(cfg);

  // Check for existence of TASX variable.
  _tas = _file->getVar("TASX");

  if (!_tas.isNull())
    cout << "TASX variable found, will use instead of tas in 2D records.\n";

  InitVarDB();
}


/* -------------------------------------------------------------------- */
void NetCDF::InitVarDB()
{
  char *proj_dir = getenv("PROJ_DIR");

  if (proj_dir == 0)
  {
    cout << "Environment variable PROJ_DIR undefined." << endl;
    return;
  }

  char file[1024];
  snprintf(file, 1024, "%s/Configuration/VarDB", proj_dir);

  if (InitializeVarDB(file) == ERR)
  {
    cout << "Failed to initialize variable database file : " << file << endl;
    return;
  }
}

/* -------------------------------------------------------------------- */
NetCDF::~NetCDF()
{
  _file->close();
}

/* -------------------------------------------------------------------- */
void NetCDF::CreateNetCDFfile(const Config & cfg)
{
  cout << " " << cfg.outputFile << "...\n";

  if (_file && !_file->isNull())	// Was successfully opened in ctor, leave.
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
    _mode = NcFile::replace;
  }
  else {
    // User specified an output filname on command line, see if file exists.
    NcFile test(_outputFile.c_str(), NcFile::write);
    if (test.isNull())
      _mode = NcFile::newFile;
  }

  _file = new NcFile(_outputFile.c_str(), _mode);

  /* If we are creating a file from scratch, perform the following.
   */
  if (_mode != NcFile::write)
  {
    struct tm st, et;
    memset(&st, 0, sizeof(struct tm));
    memset(&et, 0, sizeof(struct tm));
    gmtime_r(&cfg.starttime, &st);
    gmtime_r(&cfg.stoptime, &et);

    putGlobalAttribute("institution", "NCAR Research Aviation Facility");
    putGlobalAttribute("Source", "NCAR/RAF Fast-2D Processing Software");
    putGlobalAttribute("Conventions", "NCAR-RAF/nimbus");
    putGlobalAttribute("ConventionsURL", "http://www.eol.ucar.edu/raf/Software/netCDF.html");
    putGlobalAttribute("ProjectName", cfg.project);
    putGlobalAttribute("FlightNumber", cfg.flightNumber);
    putGlobalAttribute("FlightDate", cfg.flightDate);
    putGlobalAttribute("date_created", dateProcessed());
    char tmp[128];
    snprintf(tmp, 128, "%02d:%02d:%02d-%02d:%02d:%02d",
	st.tm_hour, st.tm_min, st.tm_sec, et.tm_hour, et.tm_min, et.tm_sec);
    putGlobalAttribute("TimeInterval", tmp);
    putGlobalAttribute("Raw_2D_Data_File", cfg.inputFile.c_str());
  }
}

/* -------------------------------------------------------------------- */
void NetCDF::readTrueAirspeed(float tas[], size_t n)
{
  assert (n == _tas.getDim(0).getSize());
  _tas.getVar(tas);
}


std::string NetCDF::dateProcessed()
{
  time_t        t;
  struct tm     tm;
  char buffer[64];

  t = time(0);
  memset(&tm, 0, sizeof(struct tm));
  tm = *localtime(&t);
  strftime(buffer, 64, ISO8601_Z, &tm);

  return(std::string(buffer));
}


/* -------------------------------------------------------------------- */
void NetCDF::readStartEndTime(Config & cfg)
{
  NcVar var;

  if ((var = _file->getVar("Time")).isNull())
    var = _file->getVar("time");

  /* If we found a Time variable, acquire the start/end time from the netCDF
   * file.  This is the time frame we want to process against.
   */
  if (!var.isNull())
  {
    if (var.getDim(0).getSize() == 0)
      return;

    int nValues = var.getDim(0).getSize();
    float *timeValues = new float[nValues];
    var.getVar(timeValues);
    NcVarAtt unitsAtt, frmtAtt;
    char frmt[64], units[64];

    unitsAtt = var.getAtt("units");
    frmtAtt = var.getAtt("strptime_format");

    if (unitsAtt.isNull())
    {
      cerr << "No units for Time variable, fatal." << endl;
      exit(1);
    }

    unitsAtt.getValues(units);
    if (frmtAtt.isNull())
    {
      strcpy(frmt, "seconds since %F %T %z");
    }
    else
      frmtAtt.getValues(frmt);

    struct tm tm;
    time_t st, et;
    memset(&tm, 0, sizeof(struct tm));
    strptime(units, frmt, &tm);
    st = et = mktime(&tm);
    st += (time_t)timeValues[0];
    et += (time_t)timeValues[nValues-1];
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

    delete [] timeValues;
  }
}

/* -------------------------------------------------------------------- */
NcDim NetCDF::addDimension(const char name[], int size)
{
  NcDim dim = _file->getDim(name);

  if (!dim.isNull()) {
    cout << "Found dimension " << name << endl;
  }
  else {
    dim = _file->addDim(name, size);
    if (dim.isNull())
      cout << "Failed to create dimension " << name << endl;
    else
      cout << "Created dimension " << name << endl;
  }

  return dim;
}

/* -------------------------------------------------------------------- */
NcVar NetCDF::addTimeVariable(const Config & cfg, int size)
{
  _timevar = _file->getVar("Time");

  if (!_timevar.isNull())
    return _timevar;

  char timeunits[70];
  int year, month, day;
  struct tm st;
  memset(&st, 0, sizeof(struct tm));
  gmtime_r(&cfg.starttime, &st);

  sscanf(cfg.flightDate.c_str(), "%d/%d/%d", &month, &day, &year);
  snprintf(timeunits, 70, "seconds since %04d-%02d-%02d %02d:%02d:%02d +0000", year,month,day,st.tm_hour,st.tm_min,st.tm_sec);
  _timevar = _file->addVar("Time", ncInt, _timedim);

  if (_timevar.isNull())
  {
    cout << "Failed to create Time variable.\n";
    return _timevar;
  }
  putVarAttribute(_timevar, "long_name", "time of measurement");
  putVarAttribute(_timevar, "standard_name", "time");
  putVarAttribute(_timevar, "units", timeunits);
  putVarAttribute(_timevar, "strptime_format", "seconds since %F %T %z");

  int time[size];
  for (int i = 0; i < size; i++) time[i] = i;
  _timevar.putVar((const int *)time);

  return _timevar;
}


/* -------------------------------------------------------------------- */
void NetCDF::CreateDimensions(int numtimes, ProbeInfo &probe, const Config &cfg)
{
  char tmp[128], coord_name[128];
  NcVar var;

  // Define the dimensions.
  _timedim = addDimension("Time", numtimes);
  _spsdim = addDimension("sps1", 1);
  _bndsdim = addDimension("bnds", 2);

  strcpy(coord_name, probe.serialNumber.c_str());
  strcat(coord_name, "_P2D");
  _bindim = addDimension(coord_name, probe.numBins);

  // Create coordinate variable, if it doesn't exist
  if ((var = _file->getVar(coord_name)).isNull())
    var = _file->addVar(coord_name, ncFloat, _bindim);

  if (!var.isNull())
  {
    putVarAttribute(var, "units", "um");
    snprintf(tmp, 128, "%s arithmetic midpoint bin size in diameter", probe.type.c_str());
    putVarAttribute(var, "long_name", tmp);
    putVarAttribute(var, "bounds", coord_name);
  }

  // Create bounds variable, if it doesn't exist
  if ((var = _file->getVar(coord_name)).isNull())
  {
    strcat(coord_name, "_bnds");
    std::vector< NcDim > dimes;
    dimes.push_back(_bindim);
    dimes.push_back(_bndsdim);
    if (!(var = _file->addVar(coord_name, ncFloat, dimes)).isNull())
    {
      putVarAttribute(var, "units", "um");
      snprintf(tmp, 128, "lower and upper bounds for %s", probe.type.c_str());
      putVarAttribute(var, "long_name", tmp);
    }
  }

  snprintf(tmp, 128, "histogram%d", probe.numBins+1);
  _bindim_plusone = addDimension(tmp, probe.numBins+1);
  _intbindim = addDimension("interarrival_endpoints", cfg.nInterarrivalBins);
//  _intbindim = addDimension("interarrival_endpoints", cfg.nInterarrivalBins+1);
}


/* -------------------------------------------------------------------- */
NcVar NetCDF::addHistogram(string& varname, const ProbeInfo &probe, int binoffset)
{
  NcVar var;
  NcDim len_dim = _bindim;
  char str[128];

  const char *units = VarDB_GetUnits(varname.c_str());
  // Make an attempt at units if VarDB returned 'Unk'.
  if (strcmp(units, "Unk") == 0)
  {
    if (varname.compare(0, 3, "A2D") == 0) units = "count";
    if (varname.compare(0, 3, "C2D") == 0) units = "L";
  }

  /* Inter-arrival time array has a different histogram length,
   * kind of cheesy way to handle it...
   */
  if (varname.compare(0, 3, "I2D") == 0) len_dim = _intbindim;

  if ((var = _file->getVar(varname)).isNull())
  {
    std::vector< NcDim > dimes;
    dimes.push_back(_timedim);
    dimes.push_back(_spsdim);
    dimes.push_back(len_dim);
    if ((var = _file->addVar(varname, ncFloat, dimes)).isNull())
    {
      cerr << "addHistogram: - Failed to create new variable " << varname << endl;
      return var;
    }
    putVarAttribute(var, "_FillValue", (float)(-32767.0));
  }

  putVarAttribute(var, "units", units);
  putVarAttribute(var, "long_name", VarDB_GetTitle(varname.c_str()));
  putVarAttribute(var, "Category", Category);
  putVarAttribute(var, "SerialNumber", probe.serialNumber.c_str());
  putVarAttribute(var, "DataQuality", "Good");

  if (varname.compare(0, 3, "A2D") == 0) {
    putVarAttribute(var, "Resolution", (int)probe.resolution);
    putVarAttribute(var, "nDiodes", probe.nDiodes);
    putVarAttribute(var, "ResponseTime", (float)0.4);
    putVarAttribute(var, "ArmDistance", probe.armWidth * 10);
  }
  if (varname.compare(0, 3, "C2D") == 0) {
    putVarAttribute(var, "probe_technique", "oap");
    if (probe.resolution < 100)
      putVarAttribute(var, "size_distribution_type", "cloud_drop");
    else
      putVarAttribute(var, "size_distribution_type", "cloud_precipitation");
    snprintf(str, 128, "2 %c%s TASX", 'A', &varname.c_str()[1]);
    putVarAttribute(var, "Dependencies", str);
    putVarAttribute(var, "FirstBin", probe.firstBin);
    putVarAttribute(var, "LastBin", probe.numBins+binoffset-1);
    putVarAttribute(var, "DepthOfField", probe.dof);
    putVarAttribute(var, "EffectiveAreaWidth", probe.eaw);
    putVarAttribute(var, "CellSizes", probe.bin_endpoints);
    putVarAttribute(var, "CellSizeUnits", "micrometers");
    putVarAttribute(var, "Density", (float)1.0);
    if (binoffset)
    {
      putVarAttribute(var, "CellSizeNote", "CellSizes are upper bin limits as particle size.");
      putVarAttribute(var, "HistogramNote", "Zeroth data bin is an unused legacy placeholder.");
    }
    else
      putVarAttribute(var, "CellSizeNote", "CellSizes are lower bin limits as particle size.");
  }

  putVarAttribute(var, "DateProcessed", dateProcessed().c_str());

  return var;
}

/* -------------------------------------------------------------------- */
NcVar NetCDF::addVariable(string& varname, string& serialNumber)
{
  NcVar var;

  const char *units = VarDB_GetUnits(varname.c_str());
  const char *std_name = VarDB_GetStandardNameName(varname.c_str());

  // Make an attempt at units if VarDB returned 'Unk'.
  if (strcmp(units, "Unk") == 0)
  {
    if (varname.compare(0, 4, "CONC") == 0) units = "L";
    if (varname.compare(0, 4, "PLWC") == 0) units = "g m-3";
    if (varname.compare(0, 4, "DBZ") == 0) units = "dBz";
    if (varname.compare(0, 4, "DBAR") == 0) units = "um";
  }

  if ((var = _file->getVar(varname)).isNull())
  {
    if (!(var = _file->addVar(varname, ncFloat, _timedim)).isNull())
    {
      putVarAttribute(var, "_FillValue", (float)(-32767.0));
      putVarAttribute(var, "units", units);
      putVarAttribute(var, "long_name", VarDB_GetTitle(varname.c_str()));
      if (std_name && strcmp(std_name, "None"))
        putVarAttribute(var, "standard_name", std_name);
      putVarAttribute(var, "Category", Category);
      putVarAttribute(var, "SerialNumber", serialNumber.c_str());
      putVarAttribute(var, "DataQuality", "Good");
    }
  }
  return var;
}


/* -------------------------------------------------------------------- */
int NetCDF::WriteData(ProbeInfo& probe, ProbeData& data)
{
  NcVar vconca, vconcr, vplwa, vplwr;
  NcVar vdbara, vdbarr, vdispa, vdispr;
  NcVar vdbza, vdbzr, vreffa, vreffr;
  NcVar vnacca, vnaccr, vnreja, vnrejr;
  NcVar vconc100a, vconc100r, vconc150a, vconc150r;
  string varname;

  //Total counts and LWC
  varname="CONC2DCA"+probe.suffix; varname[6] = probe.id[0];
  vconca = addVariable(varname, probe.serialNumber);

  varname="CONC2DCR"+probe.suffix; varname[6] = probe.id[0];
  vconcr = addVariable(varname, probe.serialNumber);

  /* output 100 micron and 150 micron and above total concentrations.
   * this should be some command line flag at some point.
   */
  if (false)
  {
    varname="CONC2DCA100"+probe.suffix; varname[6] = probe.id[0];
    vconc100a = addVariable(varname, probe.serialNumber);

    varname="CONC2DCR100"+probe.suffix; varname[6] = probe.id[0];
    vconc100r = addVariable(varname, probe.serialNumber);

    varname="CONC2DCA150"+probe.suffix; varname[6] = probe.id[0];
    vconc150a = addVariable(varname, probe.serialNumber);

    varname="CONC2DCR150"+probe.suffix; varname[6] = probe.id[0];
    vconc150r = addVariable(varname, probe.serialNumber);

    vconc100a.putVar(&data.all.total_conc100[0]);
    vconc100r.putVar(&data.round.total_conc100[0]);
    vconc150a.putVar(&data.all.total_conc150[0]);
    vconc150r.putVar(&data.round.total_conc150[0]);
  }

  varname="PLWC2DCR"+probe.suffix; varname[6] = probe.id[0];
  vplwr = addVariable(varname, probe.serialNumber);

  varname="PLWC2DCA"+probe.suffix; varname[6] = probe.id[0];
  vplwa = addVariable(varname, probe.serialNumber);

  varname="DBAR2DCR"+probe.suffix; varname[6] = probe.id[0];
  vdbarr = addVariable(varname, probe.serialNumber);

  varname="DBAR2DCA"+probe.suffix; varname[6] = probe.id[0];
  vdbara = addVariable(varname, probe.serialNumber);

  varname="DISP2DCR"+probe.suffix; varname[6] = probe.id[0];
  vdispr = addVariable(varname, probe.serialNumber);

  varname="DISP2DCA"+probe.suffix; varname[6] = probe.id[0];
  vdispa = addVariable(varname, probe.serialNumber);

  varname="DBZ2DCR"+probe.suffix; varname[5] = probe.id[0];
  vdbzr = addVariable(varname, probe.serialNumber);

  varname="DBZ2DCA"+probe.suffix; varname[5] = probe.id[0];
  vdbza = addVariable(varname, probe.serialNumber);

  varname="REFF2DCR"+probe.suffix; varname[6] = probe.id[0];
  vreffr = addVariable(varname, probe.serialNumber);

  varname="REFF2DCA"+probe.suffix; varname[6] = probe.id[0];
  vreffa = addVariable(varname, probe.serialNumber);

  varname="NACCEPT2DCR"+probe.suffix; varname[9] = probe.id[0];
  vnaccr = addVariable(varname, probe.serialNumber);

  varname="NACCEPT2DCA"+probe.suffix; varname[9] = probe.id[0];
  vnacca = addVariable(varname, probe.serialNumber);

  varname="NREJECT2DCR"+probe.suffix; varname[9] = probe.id[0];
  vnrejr = addVariable(varname, probe.serialNumber);

  varname="NREJECT2DCA"+probe.suffix; varname[9] = probe.id[0];
  vnreja = addVariable(varname, probe.serialNumber);

  if (!vconca.isNull())  vconca.putVar(&data.all.total_conc[0]);
  if (!vconcr.isNull())  vconcr.putVar(&data.round.total_conc[0]);
  if (!vplwr.isNull())   vplwr.putVar(&data.round.lwc[0]);
  if (!vplwa.isNull())   vplwa.putVar(&data.all.lwc[0]);
  if (!vdbarr.isNull())  vdbarr.putVar(&data.round.dbar[0]);
  if (!vdbara.isNull())  vdbara.putVar(&data.all.dbar[0]);
  if (!vdispr.isNull())  vdispr.putVar(&data.round.disp[0]);
  if (!vdispa.isNull())  vdispa.putVar(&data.all.disp[0]);
  if (!vdbzr.isNull())   vdbzr.putVar(&data.round.dbz[0]);
  if (!vdbza.isNull())   vdbza.putVar(&data.all.dbz[0]);
  if (!vreffr.isNull())  vreffr.putVar(&data.round.eff_rad[0]);
  if (!vreffa.isNull())  vreffa.putVar(&data.all.eff_rad[0]);
  if (!vnaccr.isNull())  vnaccr.putVar(&data.round.accepted[0]);
  if (!vnacca.isNull())  vnacca.putVar(&data.all.accepted[0]);
  if (!vnrejr.isNull())  vnrejr.putVar(&data.round.rejected[0]);
  if (!vnreja.isNull())  vnreja.putVar(&data.all.rejected[0]);

  /* These variables are only output when generating a stand alone netCDF file.
   * i.e. They are not ouput if the -o command line is specified and it finds
   * an existing TAS.
   */
  if (_tas.isNull())
  {
    NcVar var;

    varname="poisson_coeff1"+probe.suffix;
    if ((var = _file->getVar(varname)).isNull()) {
      if ((var = _file->addVar(varname, ncFloat, _timedim)).isNull()) return NetCDF::NC_ERR;
      putVarAttribute(var, "units", "unitless");
      putVarAttribute(var, "long_name", "Interarrival Time Fit Coefficient 1");
    }
    var.putVar(&data.cpoisson1[0]);

    varname="poisson_coeff2"+probe.suffix;
    if ((var = _file->getVar(varname)).isNull()) {
      if ((var = _file->addVar(varname, ncFloat, _timedim)).isNull()) return NetCDF::NC_ERR;
      putVarAttribute(var, "units", "1/seconds");
      putVarAttribute(var, "long_name", "Interarrival Time Fit Coefficient 2");
    }
    var.putVar(&data.cpoisson2[0]);

    varname="poisson_coeff3"+probe.suffix;
    if ((var = _file->getVar(varname)).isNull()) {
      if ((var = _file->addVar(varname, ncFloat, _timedim)).isNull()) return NetCDF::NC_ERR;
      putVarAttribute(var, "units", "1/seconds");
      putVarAttribute(var, "long_name", "Interarrival Time Fit Coefficient 3");
    }
    var.putVar(&data.cpoisson3[0]);

    varname="poisson_cutoff"+probe.suffix;
    if ((var = _file->getVar(varname)).isNull()) {
      if ((var = _file->addVar(varname, ncFloat, _timedim)).isNull()) return NetCDF::NC_ERR;
      putVarAttribute(var, "units", "seconds");
      putVarAttribute(var, "long_name", "Interarrival Time Lower Limit");
    }
    var.putVar(&data.pcutoff[0]);

    varname="poisson_correction"+probe.suffix;
    if ((var = _file->getVar(varname)).isNull()) {
      if ((var = _file->addVar(varname, ncFloat, _timedim)).isNull()) return NetCDF::NC_ERR;
      putVarAttribute(var, "units", "unitless");
      putVarAttribute(var, "long_name", "Count/Concentration Correction Factor for Interarrival Rejection");
    }
    var.putVar(&data.corrfac[0]);

    varname="TAS"+probe.suffix;
    if ((var = _file->getVar(varname)).isNull()) {
      if ((var = _file->addVar(varname, ncFloat, _timedim)).isNull()) return NetCDF::NC_ERR;
      putVarAttribute(var, "units", "m/s");
      putVarAttribute(var, "long_name", "True Air Speed");
    }
    var.putVar(&data.tas[0]);

    varname="SA"+probe.suffix;
    if ((var = _file->getVar(varname)).isNull()) {
      if ((var = _file->addVar(varname, ncFloat, _bindim)).isNull()) return NetCDF::NC_ERR;
      putVarAttribute(var, "units", "m2");
      putVarAttribute(var, "long_name", "Sample area per channel");
    }
    var.putVar(&probe.samplearea[0]);

    //Bins
    varname="bin_endpoints"+probe.suffix;
    if ((var = _file->getVar(varname)).isNull()) {
      if ((var = _file->addVar(varname, ncFloat, _bindim_plusone)).isNull()) return NetCDF::NC_ERR;
      putVarAttribute(var, "units", "microns");
    }
    var.putVar(&probe.bin_endpoints[0]);

    varname="bin_midpoints"+probe.suffix;
    if ((var = _file->getVar(varname)).isNull()) {
      if ((var = _file->addVar(varname, ncFloat, _bindim)).isNull()) return NetCDF::NC_ERR;
      putVarAttribute(var, "units", "microns");
      putVarAttribute(var, "long_name", "Size Channel Midpoints");
    }
    var.putVar(&probe.bin_midpoints[0]);
  }

  return 0;
}

/* -------------------------------------------------------------------- */
void NetCDF::putGlobalAttribute(const char attrName[], float value)
{
  _file->putAtt(std::string(attrName), ncFloat, value);
}

void NetCDF::putGlobalAttribute(const char attrName[], const char *value)
{
  _file->putAtt(attrName, value);
}

void NetCDF::putGlobalAttribute(const char attrName[], const std::string value)
{
  putGlobalAttribute(attrName, value.c_str());
}


void NetCDF::putVarAttribute(NcVar & var, const char attrName[], int value)
{
  var.putAtt(std::string(attrName), ncInt, value);
}

void NetCDF::putVarAttribute(NcVar & var, const char attrName[], std::vector<float> values)
{
  var.putAtt(std::string(attrName), ncFloat, values.size(), &values[0]);
}

void NetCDF::putVarAttribute(NcVar & var, const char attrName[], float value)
{
  var.putAtt(std::string(attrName), ncFloat, value);
}

void NetCDF::putVarAttribute(NcVar & var, const char attrName[], const char *value)
{
  var.putAtt(attrName, value);
}

void NetCDF::putVarAttribute(NcVar & var, const char attrName[], const std::string value)
{
  putVarAttribute(var, attrName, value.c_str());
}

