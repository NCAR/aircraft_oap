/*
-------------------------------------------------------------------------
OBJECT NAME:	config.cc

COPYRIGHT:	University Corporation for Atmospheric Research, 2023
-------------------------------------------------------------------------
*/

#include "config.h"

#include <cstdio>


/* -------------------------------------------------------------------- */
Config::Config() : _type("F2DS"), _timeOffset(0), _storeCompressed(true), _resolution(10), _nDiodes(128), _clockFreq(20), _waveLength(785)

{

}

void Config::SetType(std::string s)
{
  _type = s;

  if (_type.compare("F2DS") == 0)
  {
    _code = "SH";
    _resolution = 10;
    _nDiodes = 128;
    _clockFreq = 20;
    _waveLength = 785;
  }

  if (_type.compare("HVPS") == 0)
  {
    _code = "H1";
    _resolution = 200;
    _nDiodes = 128;
    _clockFreq = 20;
    _waveLength = 785;
  }

}
