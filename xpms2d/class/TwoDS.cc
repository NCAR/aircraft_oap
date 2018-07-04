/*
-------------------------------------------------------------------------
OBJECT NAME:	TwoDS.cc

FULL NAME:	SPEC TwoDS Probe Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "TwoDS.h"
#include "ControlWindow.h"

const unsigned char TwoDS::SyncString[] = { 0xaa, 0xaa, 0xaa };
const unsigned char TwoDS::OverldString[] = { 0x55, 0x55, 0xaa };


#define TimeWord_Microseconds(slice)      ((slice & 0x000000ffffffffffLL) / 12)


/* -------------------------------------------------------------------- */
TwoDS::TwoDS(const char xml_entry[], int recSize) : Probe(xml_entry, recSize)
{
  std::string XMLgetAttributeValue(const char s[], const char target[]);

  _lrLen = recSize;

  std::string id = XMLgetAttributeValue(xml_entry, "id");
  strcpy(_code, id.c_str());

  _name = XMLgetAttributeValue(xml_entry, "type");
  _name += XMLgetAttributeValue(xml_entry, "suffix");

  _resolution = atoi(XMLgetAttributeValue(xml_entry, "resolution").c_str());

printf("TwoDS:: id=%s, name=%s, resolution=%zu\n", _code, _name.c_str(), _resolution);
}

void TwoDS::init()
{
  _type = Probe::TWODS;
  _nDiodes = 128;
  _nSlices = P2D_DATA / _nDiodes * 8;
  _lrPpr = 1;
}


extern ControlWindow	*controlWindow;

/* -------------------------------------------------------------------- */
bool TwoDS::isSyncWord(const unsigned char *p)
{
  return *p & 0x55;
}

/* -------------------------------------------------------------------- */
struct recStats TwoDS::ProcessRecord(const P2d_rec *record, float version)
{
  char	*probeID = (char *)&record->id;

  stats.tBarElapsedtime = 0;
  stats.nTimeBars = 0;
  stats.nonRejectParticles = 0;
  stats.minBar = 10000000;
  stats.maxBar = 0;
  stats.area = 0;
  stats.DOFsampleVolume = 0.0;
  stats.duplicate = false;
  stats.particles.clear();
  stats.tas = (float)record->tas;
  if (version < 5.09)
    stats.tas = stats.tas * 125 / 255;

  stats.thisTime = (record->hour * 3600 + record->minute * 60 + record->second) * 1000 + record->msec; // in milliseconds

  stats.resolution = Resolution();

  if (probeID[0] == 'P')
    stats.SampleVolume = 261.0 * (stats.resolution * nDiodes() / 1000);
  else
  if (probeID[0] == 'C')
    stats.SampleVolume = 61.0 * (stats.resolution * nDiodes() / 1000);

  stats.frequency = stats.resolution / stats.tas;
  stats.SampleVolume *= stats.tas *
                        (stats.DASelapsedTime - record->overld) * 0.001;


  return stats;

}	// END PROCESSTWODS

// END TWODS.CC
