/*
-------------------------------------------------------------------------
OBJECT NAME:    spec.h

FULL NAME:      Translate SPEC to OAP

DESCRIPTION:    Structs for SPEC probes.

COPYRIGHT:      University Corporation for Atmospheric Research, 2023
-------------------------------------------------------------------------
*/

#ifndef _SPEC_H_
#define _SPEC_H_

#include <cstdint>

enum PacketFormatType { Type32, Type48 };

// Key words in SPEC RLE input data files.
static const uint16_t SyncWord = 0x3253;	// Particle sync word '2S'.
static const uint16_t MaskData = 0x4d4b;	// MK Flush Buffer.
static const uint16_t FlushWord = 0x4e4c;	// NL Flush Buffer.
static const uint16_t HousekeepWord = 0x484b;	// HK Flush Buffer.

//static const uint64_t Type32_TimingWordMask = 0x00000000FFFFFFFFL; // do I need this?
static const uint64_t Type48_TimingWordMask = 0x0000FFFFFFFFFFFFL;


struct imageBuf
{
  int16_t       year, month, dow, day, hour, minute, second, msecond;
  char          rdf[4096];
  uint16_t      cksum;
};

struct hk48Buf
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

// This is the index into the particle packet.
enum PKT_IDX { H_CHN=1, V_CHN=2, PID=3, nSLICES=4 };

#endif
