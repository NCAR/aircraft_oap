/*
-------------------------------------------------------------------------
OBJECT NAME:	config.cc

COPYRIGHT:	University Corporation for Atmospheric Research, 2023
-------------------------------------------------------------------------
*/

#include "config.h"

#include <cstdio>



/* -------------------------------------------------------------------- */
Config::Config() : _code("SH"), _type("F2DS"), _timeOffset(0), _stuckBitRejection(true), _storeCompressed(false), _resolution(10), _nDiodes(128), _clockFreq(20), _waveLength(785)

{

}

void Config::SetType(std::string s)
{
  _type = s;

  printf("Setting SPEC probe type to %s\n", s.c_str());

  if (_type.compare("F2DS") == 0)
  {
    _code = "SH";
    _resolution = 10;
    _nDiodes = 128;
    _clockFreq = 20;
    _waveLength = 785;
    _dataFormat = Type48;
  }

  if (_type.compare("HVPS") == 0)
  {
    _code = "H1";
    _resolution = 150;
    _nDiodes = 128;
    _clockFreq = 20;
    _waveLength = 785;
    _dataFormat = Type32;
  }

}
