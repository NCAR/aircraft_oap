/*
 * Program to convert PADS 2D probe files into RAF OAP file format.  This does not uncompress
 * the 4096 data block, rather it just encapsulates it differently.  Adds an XML header to the
 * front of the file with useful meta-data and reworks the header in front of each 4096 PADS
 * buffer.
 */
#include "pads2oap.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

// for byte swapping routines ntohs() & htons()
#include <arpa/inet.h>


void ProcessArgs(char *argv[], OAPfile& oap, std::string& pfn, std::string& ofn);
void ConvertPADStoOAP(const PADS_rec&, OAP_rec&, uint16_t);
std::string DateStamp(const PADS_rec&), TimeStamp(const PADS_rec&);


/* ----------------------------------------------------------------------- */
void defineProbes(OAPfile& oap)
{
  // Need a way to pass in probe list on command line....

  // DYNAMO setup; P4, 100um, 64 diodes, guessing serial#.
//  oap.AddProbe(PMS2D_P4, 100, 64, "2DP100", "_RWI");


//  oap.AddProbe(PMS2D_C8, 25, 64, "CIP001", "_CIP");
  oap.AddProbe(PMS2D_P8, 100, 64, "PIP001", "_PIP");

  // NCAR RAL - processing workshop.
//  oap.AddProbe(PMS2D_C8, 25, 64, "CIP001", "_RWI");

  // IDEAS-4
//  oap.AddProbe(PMS2D_C4, 25, 64, "F2DC001", "_LWI");
//  oap.AddProbe(PMS2D_C6, 10, 64, "F2DC002", "_RWO");

}

/* ----------------------------------------------------------------------- */
void usage()
{
  std::cerr << "pads2oap [-flight num] [-project name] [-platform name] -i pads_file -o oap_file\n";
  std::cerr << "  -project : is for project name.  e.g. TORERO\n";
  std::cerr << "  -platform : is for platform name.  e.g. GV_N677F or P3_N43RF\n";
  std::cerr << "  -flight : is for flight number, this is a string, not necessarily a number.  e.g. rf03\n";
  std::cerr << "  -i : input filename; PADS format.\n";
  std::cerr << "  -o : output filename; will be OAP format.\n";
  exit(1);
}

/* ----------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
  FILE *in;
  OAPfile oap;
  std::string pads_filename, oap_filename;

  ProcessArgs(argv, oap, pads_filename, oap_filename);

  if (pads_filename.length() == 0 || oap_filename.length() == 0)
    usage();


  if ((in = fopen(pads_filename.c_str(), "rb")) == NULL)
  {
    std::cerr << "Can't open input file " << argv[1] << std::endl;
    exit(1);
  }

  oap.open(oap_filename);
  defineProbes(oap);


  int cnt = 0;
  PADS_rec pads_rec;
  OAP_rec oap_rec;

  for (cnt = 0; fread(&pads_rec, sizeof(PADS_rec), 1, in); ++cnt)
  {
    ConvertPADStoOAP(pads_rec, oap_rec, oap.ProbeData(0).id);

    if (cnt == 0)
    {
      std::string s = DateStamp(pads_rec);
      oap.SetFlightDate(s);  // This was outside the loop, but here for auto-detect of date.
      oap.WriteHeader();
      std::cout << "First record " << DateStamp(pads_rec) << " " << TimeStamp(pads_rec) << std::endl;
    }

    oap.WriteRecord(oap_rec);
  }

  fclose(in);

  std::cout << "Last record " << DateStamp(pads_rec) << " " << TimeStamp(pads_rec) << std::endl;
  std::cout << cnt << " records transferred." << std::endl;
}

/* ----------------------------------------------------------------------- */
void ConvertPADStoOAP(const PADS_rec& pads_rec, OAP_rec& oap_rec, uint16_t id)
{
  memset(&oap_rec, 0, sizeof(OAP_rec));
  oap_rec.id	= htons(id);
  oap_rec.year	= htons(pads_rec.year);
  oap_rec.month	= htons(pads_rec.month);
  oap_rec.day	= htons(pads_rec.day);
  oap_rec.hour	= htons(pads_rec.hour);
  oap_rec.minute= htons(pads_rec.minute);
  oap_rec.second= htons(pads_rec.second);
  oap_rec.msec	= htons(pads_rec.msec);
  memcpy(oap_rec.data, pads_rec.data, TWOD_BUFF_SIZE);
}

/* ----------------------------------------------------------------------- */
std::string DateStamp(const PADS_rec& pads_rec)
{
  std::string s;
  char msg[1024];
  sprintf(msg, "%04d/%02d/%02d", pads_rec.year, pads_rec.month, pads_rec.day);
  s = msg;
  return s;
}

/* ----------------------------------------------------------------------- */
std::string TimeStamp(const PADS_rec& pads_rec)
{
  std::string s;
  char msg[1024];
  sprintf(msg, "%02d:%02d:%02d.%d", pads_rec.hour, pads_rec.minute, pads_rec.second, pads_rec.msec);
  s = msg;
  return s;
}


/* ----------------------------------------------------------------------- */
void OAPfile::open(const std::string& filename)
{
  if (filename.length() == 0)
    usage();

  if ((_out = fopen(filename.c_str(), "w+b")) == NULL)
  {
    std::cerr << "Can't open output file " << filename << std::endl;
    exit(1);
  }
}

OAPfile::~OAPfile()
{
  if (_out)
    fclose(_out);
}

void OAPfile::WriteRecord(const OAP_rec& oap_rec)
{
  if (!_out)
    return;

  if (fwrite(&oap_rec, sizeof(OAP_rec), 1, _out) != 1)
  {
    std::cerr << "Write error, exiting." << std::endl;
    fclose(_out);
    exit(1);
  }
}

/* ----------------------------------------------------------------------- */
void OAPfile::WriteHeader()
{
  std::stringstream header;

  header
	<< "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
  	<< "<OAP version=\"1\">\n"
	<< " <Institution>NOAA Aircraft Operations Center</Institution>\n"
	<< " <FormatURL>http://www.eol.ucar.edu/raf/Software/OAPfiles.html</FormatURL>\n";

  if (_project.length())
    header << " <Project>" << _project << "</Project>\n";

  if (_platform.length())
    header << " <Platform>" << _platform << "</Platform>\n";

  if (_flightnumber.length())
    header << " <FlightNumber>" << _flightnumber << "</FlightNumber>\n";

  if (_flightdate.length())
    header << " <FlightDate>" << _flightdate << "</FlightDate>\n";

  for (size_t i = 0; i < _probelist.size(); ++i)
  {
    const char *id = (char *)&_probelist[i].id;
    header << "  <probe id=\""
	<< id[1] << id[0]  << "\" type=\"CIP\" resolution=\""
	<< _probelist[i].resolution << "\" nDiodes=\""
	<< _probelist[i].nDiodes << "\" serialnumber=\""
	<< _probelist[i].serialnumber << "\" suffix=\""
	<< _probelist[i].suffix << "\" endian=\"little\" rle=\"true\"/>\n";
  }

  header << "</OAP>\n";

  if (_out)
    fwrite(header.str().c_str(), header.str().length(), 1, _out);
}

/* ----------------------------------------------------------------------- */
void OAPfile::AddProbe(uint16_t id, int resolution, int nDiodes, std::string serialnumber, std::string suffix)
{
  Probe p;

  p.id = id;
  p.resolution = resolution;
  p.nDiodes = nDiodes;
  p.serialnumber = serialnumber;
  p.suffix = suffix;

  _probelist.push_back(p);
}

/* ----------------------------------------------------------------------- */
void ProcessArgs(char *argv[], OAPfile& oap, std::string& pfn, std::string& ofn)
{
  while (*++argv)
  {
    if (strcmp(*argv, "-project") == 0)
    {
      oap.SetProject(*++argv);
    }
    else
    if (strcmp(*argv, "-platform") == 0)
    {
      oap.SetPlatform(*++argv);
    }
    else
    if (strcmp(*argv, "-flight") == 0)
    {
      oap.SetFlightNumber(*++argv);
    }
    else
    if (strcmp(*argv, "-i") == 0)
    {
      pfn = *++argv;
    }
    else
    if (strcmp(*argv, "-o") == 0)
    {
      ofn = *++argv;
    }
    else
      usage();
  }
}

