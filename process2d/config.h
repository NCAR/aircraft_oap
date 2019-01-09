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

  /**
   * Effective Area Width computation method, or rejection algorithm.
   */
  enum Method	{ ENTIRE_IN, CENTER_IN, RECONSTRUCTION };

  /**
   * Sizing method: along X, or Y axis, or perform circle fit (the default).
   */
  enum SizeMethod	{ CIRCLE, X, Y, EQUIV_AREA_DIAM };

  Config() : nInterarrivalBins(40), firstBin(0), shattercorrect(true), eawmethod(CENTER_IN), smethod(CIRCLE), verbose(false), debug(false) {}

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

  int	nInterarrivalBins;

  int	firstBin;

  bool	shattercorrect;
  Method	eawmethod;	// Particle reconstruction, all-in, center-in
  SizeMethod	smethod;	// Sizing method.

  bool	verbose;
  bool	debug;
};

#endif
