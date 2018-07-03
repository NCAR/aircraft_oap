/*
-------------------------------------------------------------------------
OBJECT NAME:	Fast2D.cc

FULL NAME:	Fast2D Probe Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "Fast2D.h"
#include "ControlWindow.h"

const unsigned char Fast2D::SyncString[] = { 0xaa, 0xaa, 0xaa };
const unsigned char Fast2D::OverldString[] = { 0x55, 0x55, 0xaa };


#define TimeWord_Microseconds(slice)      ((slice & 0x000000ffffffffffLL) / 12)


/* -------------------------------------------------------------------- */
Fast2D::Fast2D(const char xml_entry[], int recSize) : Probe(xml_entry, recSize)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

  init();

printf("Fast2D:: id=%s, name=%s, resolution=%zu\n", _code, _name.c_str(), _resolution);
}

void Fast2D::init()
{
  _type = Probe::FAST2D;
  _nDiodes = 64;
  _lrPpr = 1;

  SetSampleArea();
}


extern ControlWindow	*controlWindow;

/* -------------------------------------------------------------------- */
bool Fast2D::isSyncWord(const unsigned char *p)
{
  return *p & 0x55;
}

/* -------------------------------------------------------------------- */
struct recStats Fast2D::ProcessRecord(const P2d_rec *record, float version)
{
  char	*probeID = (char *)&record->id;

  output.tBarElapsedtime = 0;
  output.nTimeBars = 0;
  output.nonRejectParticles = 0;
  output.minBar = 10000000;
  output.maxBar = 0;
  output.area = 0;
  output.DOFsampleVolume = 0.0;
  output.duplicate = false;
  output.particles.clear();
  output.tas = (float)record->tas;
  if (version < 5.09)
    output.tas = output.tas * 125 / 255;

  output.thisTime = (record->hour * 3600 + record->minute * 60 + record->second) * 1000 + record->msec; // in milliseconds

  output.resolution = Resolution();

  if (probeID[0] == 'P')
    output.SampleVolume = 261.0 * (output.resolution * nDiodes() / 1000);
  else
  if (probeID[0] == 'C')
    output.SampleVolume = 61.0 * (output.resolution * nDiodes() / 1000);

  output.frequency = output.resolution / output.tas;
  output.SampleVolume *= output.tas *
                        (output.DASelapsedTime - record->overld) * 0.001;



  return output;

}	// END PROCESSFAST2D

// END FAST2D.CC
