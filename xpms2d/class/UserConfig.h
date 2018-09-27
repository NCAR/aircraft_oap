/*
-------------------------------------------------------------------------
OBJECT NAME:    UserConfig.h

FULL NAME:      User Configuration

COPYRIGHT:      University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#ifndef _USERCONFIG_H_
#define _USERCONFIG_H_

enum Sizing { BASIC, ENTIRE_IN, CENTER_IN, RECONSTRUCTION };

/**
 * Class to contain user chosen processing options from the GUI interface.
 */
class UserConfig
{
public:

  UserConfig() : _rejectZeroAreaImage(true), _density(1.0), _areaRatioRej(0.1), _sizingAlgo(BASIC)
  { }


  float
  GetDensity() const		{ return _density; }

  float
  GetAreaRatioReject() const	{ return _areaRatioRej; }

  Sizing
  GetConcentration() const	{ return _sizingAlgo ; }

  bool
  RejectZeroAreaImage() const	{ return(true); }

  void
  SetWaterDensity(float dens)	{ _density = dens; }

  void
  SetAreaRatioReject(float ratio)	{ _areaRatioRej = ratio; }

  void
  SetSizingAlgo(Sizing algo)	{ _sizingAlgo = algo; }


protected:

  bool _rejectZeroAreaImage;

  float _density;
  float _areaRatioRej;
  Sizing _sizingAlgo;

};

#endif
