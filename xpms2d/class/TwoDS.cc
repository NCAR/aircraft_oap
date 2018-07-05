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

  init();

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
  ClearStats(record);
  stats.DASelapsedTime = stats.thisTime - _prevTime;

  stats.SampleVolume = 50.8 * (Resolution() * nDiodes() / 1000);

  stats.frequency = Resolution() / stats.tas;
  stats.SampleVolume *= stats.tas *
                        (stats.DASelapsedTime - record->overld) * 0.001;




  _prevTime = stats.thisTime;
  memcpy((char *)&_prevHdr, (char *)record, sizeof(P2d_hdr));
  return stats;

}	// END PROCESSTWODS

// END TWODS.CC
