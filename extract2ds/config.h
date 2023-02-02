/*
-------------------------------------------------------------------------
OBJECT NAME:	config.h

DESCRIPTION:	Header File declaring Variable and associated processing
		functions.

COPYRIGHT:	University Corporation for Atmospheric Research, 2023
-------------------------------------------------------------------------
*/

#ifndef _config_h_
#define _config_h_

#include <string>

class Config
{
public:

  Config();

  const std::string& ProjectDirectory() const	{ return _projectDirectory; }
  const std::string& ProjectName() const	{ return _projectName; }
  const std::string& Platform() const		{ return _platform; }
  const std::string& FlightNumber() const	{ return _flightNumber; }
  const std::string& FlightDate() const		{ return _flightDate; }
  const std::string& Type() const		{ return _type; }
  const std::string& SerialNumber() const	{ return _serialNumber; }
  const std::string& Suffix() const		{ return _suffix; }
  const std::string& OutputFilename() const	{ return _outputFilename; }
  int TimeOffset() const			{ return _timeOffset; }
  bool StoreCompressed() const			{ return _storeCompressed; }

  int Resolution() const			{ return _resolution; }
  int nDiodes() const				{ return _nDiodes; }
  int ClockFrequency() const			{ return _clockFreq; }
  int WaveLength() const			{ return _waveLength; }


  void SetProjectDirectory(const std::string s)	{ _projectDirectory = s; }
  void SetProjectName(const std::string s)	{ _projectName = s; }
  void SetPlatform(const std::string s)		{ _platform = s; }
  void SetFlightNumber(const std::string s)	{ _flightNumber = s; }
  void SetFlightDate(const std::string s)	{ _flightDate = s; }
  void SetType(const std::string s);
  void SetSerialNumber(const std::string s)	{ _serialNumber = s; }
  void SetSuffix(const std::string s)		{ _suffix = s; }
  void SetTimeOffset(const std::string s)	{ _timeOffset = atoi(s.c_str()); }
  void SetOutputFilename(const std::string s)	{ _outputFilename = s; }

  void SetResolution(int r)			{ _resolution = r; }
  void SetDiodes(int n)				{ _nDiodes = n; }
  void SetWaveLength(int l)			{ _waveLength = l; }
  void SetClockFreq(int f)			{ _clockFreq = f; }

private:

  std::string _projectDirectory;
  std::string _projectName;
  std::string _platform;
  std::string _flightNumber;
  std::string _flightDate;
  std::string _type;
  std::string _serialNumber;
  std::string _suffix;
  std::string _outputFilename;

  int _timeOffset;

  bool _storeCompressed;

  int _resolution;
  int _nDiodes;
  int _clockFreq;
  int _waveLength;

};

#endif

// END CONFIG.H
