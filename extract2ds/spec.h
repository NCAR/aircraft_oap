/*
-------------------------------------------------------------------------
OBJECT NAME:    spec.h

FULL NAME:      Translate SPEC to OAP

DESCRIPTION:    Structs for SPEC probes.

COPYRIGHT:      University Corporation for Atmospheric Research, 2023
-------------------------------------------------------------------------
*/

#include <cstdint>


struct imageBuf
{
  int16_t       year, month, dow, day, hour, minute, second, msecond;
  char          rdf[4096];
  uint16_t      cksum;
};

struct hkBuf
{
  int16_t       year, month, dow, day, hour, minute, second, msecond;
  char          rdf[164];
  uint16_t      cksum;
};

struct maskBuf
{
  int16_t       year, month, dow, day, hour, minute, second, msecond;
  char          rdf[54];
  uint16_t      cksum;
};
