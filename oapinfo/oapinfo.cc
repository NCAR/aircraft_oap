/*
-------------------------------------------------------------------------
OBJECT NAME:	oapinfo.cc

FULL NAME:	OAP File Info

DESCRIPTION:	Display information from a NCAR PMS2D/OAP file, including
		indivdual record headers.

COPYRIGHT:	University Corporation for Atmospheric Research, 2012
-------------------------------------------------------------------------
*/

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
#include <unistd.h>
#include <bitset>

#include <netinet/in.h>

#include <raf/OAP.h>

using namespace std;

char	buffer[50000];

class Config
{
public:
  Config() : hdrOnly(false), fullHex(false), histo(false), asciiArt(false), probe(0) { }

  bool	hdrOnly;
  bool	fullHex;
  bool	histo;
  bool	asciiArt;

  int16_t probe;	// 0 means all probes.

  string sourceFile;
} cfg;


class Stats
{
public:
  Stats() : totalRecords(0), totalParticles(0), dofRejected(0) { }

  unsigned long totalRecords;
  unsigned long totalParticles;
  unsigned long dofRejected;
} stats;


void Output(char buff[]);
void ParticleCount(OAP::P2d_rec *p2d, size_t nDiodes);


void Usage()
{
  cerr << "Usage: oapinfo [-h] [-x] [-c] [-p ID] file.2d" << endl;
  cerr << "  -h: XML File header only." << endl;
  cerr << "  -x: Full hex dump of each record." << endl;
  cerr << "  -c: histogram count totals, per record and per second." << endl;
  cerr << "  -p ID : Output a single probe where ID is probe ID, e.g. C1, P1, C4, 3H, 3V." << endl;
  cerr << "  -ascii or ascii art dump.\n"<< endl;
  exit(1);
}

/* -------------------------------------------------------------------- */
void processArgs(int argc, char *argv[])
{
  if (argc < 2)
    Usage();

  for (int aCnt = 1; aCnt < argc; ++aCnt)
  {
    if (argv[aCnt][0] == '-')
    {
      if (strcmp(argv[aCnt], "-h") == 0)
        cfg.hdrOnly = true;
      else
      if (strcmp(argv[aCnt], "-x") == 0)
        cfg.fullHex = true;
      else
      if (strcmp(argv[aCnt], "-c") == 0)
        cfg.histo = true;
      else
      if (strcmp(argv[aCnt], "-p") == 0)
        cfg.probe = ntohs(*((int16_t *)argv[++aCnt]));
      else
      if (strcmp(argv[aCnt], "-ascii") == 0)
        cfg.asciiArt = true;
      else
        Usage();
    }
    else
      cfg.sourceFile = argv[aCnt];
  }

  if (cfg.sourceFile.length() == 0)
    Usage();
}


/* -------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
  FILE *fp;

  processArgs(argc, argv);

  if ((fp = fopen(cfg.sourceFile.c_str(), "rb")) == NULL)
  {
    cerr << "Unable to open input file : " << cfg.sourceFile << endl;
    exit(1);
  }

  // Output file header.
  do
  {
    fgets(buffer, 5000, fp);
    cout << buffer;
  }
  while (strstr(buffer, "</OAP>") == 0);


  if (!cfg.hdrOnly)
    while (fread(buffer, sizeof(OAP::P2d_rec), 1, fp) == 1)
      Output(buffer);

  fclose(fp);

  cout << stats.totalRecords << " total records found.\n";
  if (cfg.histo)
  {
    cout << stats.totalParticles << " total particles found.\n";
    cout << stats.dofRejected << " particles DOF rejected.\n";
  }

  return 0;

}	/* END MAIN */

/* -------------------------------------------------------------------- */
void Output(char buff[])
{
  OAP::P2d_rec *p2d = (OAP::P2d_rec *)buffer;
  size_t nDiodes = 0;

  switch (ntohs(*(unsigned short *)buff))
  {
    case 0x4331:	// PMS2D C Probes.
    case 0x4332:
    case 0x5031:	// PMS2D P Probes.
    case 0x5032:
      nDiodes = 32;
      break;
    case 0x4334:	// USB 25um C probe.
    case 0x4335:	// USB 25um C probe (2nd one).
    case 0x4336:	// USB 10um C probe.
    case 0x4338:	// DMT CIP
    case 0x5034:	// USB 200um P Probe.
    case 0x5038:	// DMT PIP
      nDiodes = 64;
      break;
    case 0x4831:	// HVPS
//    case 0x4832:
    case 0x3348:	// 3V-CPI / 2DS Horizontal.
    case 0x3356:	// 3V-CPI / 2DS Vertical.
    case 0x5348:	// 2DS Horizontal.
    case 0x5356:	// 2DS Vertical.
      nDiodes = 128;
      break;
    default:
      cout << "Unrecognized record id=" << buff[0] << buff[1] << endl;
  }

  // If specific probe selected, then check here.
  if (cfg.probe != 0 && cfg.probe != ntohs(*(unsigned short *)buff))
    return;

  ++stats.totalRecords;

  if (cfg.histo)
    ParticleCount(p2d, nDiodes);
  else
  {
    cout << dec << "  " << ((char*)p2d)[0] << ((char*)p2d)[1] << " ";
    cout << setw(2) << setfill('0') << ntohs(p2d->year) << '/';
    cout << setw(2) << setfill('0') << ntohs(p2d->month) << '/';
    cout << setw(2) << setfill('0') << ntohs(p2d->day) << " ";
    cout << setw(2) << setfill('0') << ntohs(p2d->hour) << ':';
    cout << setw(2) << setfill('0') << ntohs(p2d->minute) << ':';
    cout << setw(2) << setfill('0') << ntohs(p2d->second);
    cout << '.' << setw(3) << setfill('0') << ntohs(p2d->msec);
    cout << ", tas=" << ntohs(p2d->tas) << endl;
  }

  if (cfg.fullHex)
  {
    size_t bytesPerSlice = nDiodes / 8;
    size_t nSlices = 4096 / bytesPerSlice;
    for (size_t i = 0; i < nSlices; ++i)
    {
      for (size_t j = 0; j < bytesPerSlice; ++j)
      {
        cout << hex << setw(2) << setfill('0') << (int)p2d->data[(i*bytesPerSlice) + j];
      }
      cout << endl;
    }
  }
  if (cfg.asciiArt)
  {
    size_t bytesPerSlice = nDiodes / 8;
    size_t nSlices = 4096 / bytesPerSlice;
    for (size_t i = 0; i < nSlices; ++i)
    {
      for (size_t j = 0; j < bytesPerSlice; ++j)
      {
        cout << setw(2) << setfill('0') << std::bitset<8>{p2d->data[(i*bytesPerSlice) + j]};
      }
      cout << endl;
    }
  }
}

/* -------------------------------------------------------------------- */
void OutputParticleCount(OAP::P2d_hdr *p, size_t counts[], size_t n, bool output_msec)
{
  int total = 0;

  cout << setw(2) << setfill('0') << ntohs(p->hour) << ':';
  cout << setw(2) << setfill('0') << ntohs(p->minute) << ':';
  cout << setw(2) << setfill('0') << ntohs(p->second);

  if (output_msec)
    cout << "." << setw(3) << setfill('0') << p->msec;
  cout << " - ";

  for (size_t i = 0; i < n; ++i)
  {
     cout << counts[i] << ' ';
     total += counts[i];
  }

  cout << " - total = " << total << endl;
}

/* -------------------------------------------------------------------- */
static const unsigned char syncWord[] = { 0xAA, 0xAA, 0xAA };
static const unsigned char dofRejected[] = { 0xAA, 0xAA, 0xAB };
static const size_t nSyncB = 3;


void ParticleCount(OAP::P2d_rec *p2d, size_t nDiodes)
{
  static bool firstTime = true;
  static size_t	perSecondCounts[512], sizeCounter = 0;
  static OAP::P2d_hdr prevRec;
  size_t recordCounts[512];

  if (firstTime)
  {
    firstTime = false;
    memset((void *)perSecondCounts, 0, sizeof(perSecondCounts));
  }

  memset(recordCounts, 0, sizeof(recordCounts));

  // Output 1 second totals.
  if (memcmp((void *)&p2d->hour, (void *)&prevRec.hour, 6) != 0)
  {
    OutputParticleCount(&prevRec, perSecondCounts, nDiodes, false);
    memset((void *)perSecondCounts, 0, sizeof(perSecondCounts));
    cout << endl;
  }

  size_t bytesPerSlice = nDiodes / 8;
  size_t nSlices = 4096 / bytesPerSlice;
  for (size_t i = 0; i < nSlices; ++i)
  {
    if (memcmp(&p2d->data[i*bytesPerSlice+bytesPerSlice], syncWord, 2) == 0)
    {
      ++stats.totalParticles;
    }
    if (memcmp(&p2d->data[i*bytesPerSlice+bytesPerSlice], dofRejected, 3) == 0)
    {
      ++stats.dofRejected;
    }


    // Check both little and big endian.
    if (memcmp(&p2d->data[i*bytesPerSlice+bytesPerSlice-3], syncWord, nSyncB) == 0 ||
        memcmp(&p2d->data[i*bytesPerSlice], syncWord, nSyncB) == 0)
    {
      if (sizeCounter > 511) sizeCounter = 511;
      ++perSecondCounts[sizeCounter];
      ++recordCounts[sizeCounter];
      sizeCounter = 0;
    }
    else
      ++sizeCounter;
  }

  cout << ((char*)p2d)[0] << ((char*)p2d)[1] << " - ";
  OutputParticleCount((OAP::P2d_hdr *)p2d, recordCounts, nDiodes, true);
  memcpy((void *)&prevRec, (void *)p2d, sizeof(OAP::P2d_hdr));
}
