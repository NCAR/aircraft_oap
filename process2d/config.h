#ifndef _config_h_
#define _config_h_

#include <string>
#include <ctime>

/**
 * Command line options / program configuration
 */
class Config
{
public:
  Config() : nOutputBins(128), nInterarrivalBins(40), firstBin(0), shattercorrect(true), eawmethod('r'), smethod('c'), verbose(false), debug(false) {}

  std::string inputFile;
  std::string outputFile;

  std::string platform;
  std::string project;
  std::string flightDate;
  std::string flightNumber;

  std::string user_starttime;
  std::string user_stoptime;

  time_t starttime;
  time_t stoptime;

  int nOutputBins;
  int nInterarrivalBins;

  int firstBin;

  bool shattercorrect;
  char eawmethod;           // Particle reconstruction, all-in, center-in
  char smethod;
  bool verbose;
  bool debug;
};

#endif
