/*
-------------------------------------------------------------------------
OBJECT NAME:	DataFile.cc

FULL NAME:	ADS Data File Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#define _LARGEFILE64_SOURCE


#include "DataFile.h"

#define htonll	ntohll

#include <unistd.h>
#include <algorithm>
#include <vector>

const size_t nSlices_32bit = 1024;
const size_t nSlices_64bit = 512;
const size_t nSlices_128bit = 256;

static P2d_rec PtestRecord, CtestRecord, HtestRecord;

static uint32_t PtestParticle[] = {
0xffffffff,
0xff000134,
0x55000000,

0xfffc3fff,
0xfff81fff,
0xfff00fff,
0xfff00fff,

0xfff00fff,
0xfff00fff,
0xfff81fff,
0xfffc3fff,

0xffffffff,
0xffffffff
 };

static uint32_t CtestParticle[] = {
0xffffffff,
0xff000e00,
0x55000000,

0xfff81fff,
0xffe007ff,
0xffc003ff,
0xff8001ff,
0xff8001ff,
0xff8001ff,
0xff0000ff,
0xff0000ff,

0xff0000ff,
0xff0000ff,
0xff8001ff,
0xff8001ff,
0xff8001ff,
0xffc003ff,
0xffe007ff,
0xfff81fff,

0xffffffff,
0xffffffff
 };

static unsigned short HtestParticle[] = {
  0x8000,
  0x807a,

  0x427e,
  0x437d,
  0x447c,
  0x447c,
  0x447c,
  0x447c,
  0x437d,
  0x427e,
  };

static const size_t P2dLRpPR = 1;


/* -------------------------------------------------------------------- */
ADS_DataFile::ADS_DataFile(const char fName[])
{
  unsigned char buffer[4096];

  _hdr = 0;
  _fileName = fName;
  _version[0] = '\0';

  if (_fileName.find(".gz", 0) != std::string::npos)
    {
    _gzipped = true;
    printf("We have gzipped file.\n");
    }
  else
    _gzipped = false;

#ifdef PNG
  if (_gzipped)
    gz_fd = gzopen(_fileName.c_str(), "rb");
  else
#endif
    fp = fopen(_fileName.c_str(), "rb");

  if ((_gzipped && gz_fd) || (!_gzipped && fp == NULL))
    {
    sprintf((char *)buffer, "Can't open file %s", _fileName.c_str());
    ErrorMsg((char *)buffer);
    return;
    }

  _useTestRecord = false;

#ifdef PNG
  if (_gzipped)
    {
    gzread(gz_fd, buffer, 4096);
    gzseek(gz_fd, 0, SEEK_SET);	// gzrewind does not seem to work on
    }
  else
#endif
    {
    fread(buffer, 4096, 1, fp);
    rewind(fp);
    }

  // Briefly I had the start tag for the 2D universal file as "PMS2D",
  // switched to "OAP" (Optical Array Probe).
  if (strstr((char *)buffer, "<OAP") || strstr((char *)buffer, "<PMS2D>"))
    {
    printf("initADS3(%s)\n", _fileName.c_str());
    initADS3((const char *)buffer);	// the XML header file.
    }
  else
  if (isValidProbe(buffer))
    {
    _fileHeaderType = NoHeader;
    printf("No RAF header found, assuming raw PMS2D file.\n");
    }
  else
    {
    printf("initADS2(%s)\n", _fileName.c_str());
    initADS2();
    if (_hdr) strcpy(_version, _hdr->Version());
    }

  buildIndices();

  currLR = -1; currPhys = 0;

  if (ProjectNumber().compare("812") == 0)
    fprintf(stderr, "DataFile.cc: Lake-ICE - Bit shift correction will be performed on all records.\n");

  if (ProjectNumber().compare("135") == 0)
    fprintf(stderr, "DataFile.cc: RICO - Stuck bit correction will be performed.\n");

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void ADS_DataFile::initADS2()
{
  int	Ccnt, Pcnt, Hcnt;
  Ccnt = Pcnt = Hcnt = 0;

  _fileHeaderType = ADS2;

  _hdr = new Header(_fileName.c_str());
  _projectName = _hdr->ProjectNumber();
  _flightNumber = _hdr->FlightNumber();
  _flightDate = _hdr->FlightDate();

  if (_hdr->isValid() == false)
    return;

  for (const void *p = _hdr->GetFirst("PMS2D"); p; p = _hdr->GetNext("PMS2D"))
    {
    const char *name = _hdr->VariableName((Pms2 *)p);

    if (name[3] == 'P')
      {
      PMS2D *p = new PMS2D(_hdr, (Pms2 *)p, ++Pcnt);
      _probeList[*(uint16_t *)p->Code()] = p;
      }
    else
    if (name[3] == 'C')
      {
      PMS2D *p = new PMS2D(_hdr, (Pms2 *)p, ++Ccnt);
      _probeList[*(uint16_t *)p->Code()] = p;
      }
    else
    if (name[3] == 'H')
      {
      HVPS *p = new HVPS(_hdr, (Pms2 *)p, ++Hcnt);
      _probeList[*(uint16_t *)p->Code()] = p;
      }
    }
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::initADS3(const char *hdrString)
{
  std::string XMLgetElementValue(const char s[]);

  _fileHeaderType = OAP;

  // Extract version number.  ADS2 versions are less than 5.0.
  strcpy(_version, "5");
  char *endHdr = strstr((char *)hdrString, "</OAP>") + strlen("</OAP>\n");
  if (endHdr == 0)
    endHdr = strstr((char *)hdrString, "</PMS2D>") + strlen("</PMS2D>\n");
  else
    {
    char *p = strstr((char *)hdrString, "<OAP version");
    int version = 1;
    if (p)
      version = atoi(strstr((char *)hdrString, "<OAP version")+14);
    sprintf(_version, "5.%d", version);
    }

  if (endHdr == 0)
    {
    fprintf(stderr, "No end of ADS3 header???\n");
    exit(1);
    }

  // Position at first record for buildIndices.
  fseeko(fp, endHdr - hdrString, SEEK_SET);


  // Read pertinent meta-data from header.
  char *p;
  for (p = strtok((char *)hdrString, "\n"); p; p = strtok(NULL, "\n"))
    {
    if ( strstr(p, "</OAP>\n") || strstr(p, "</PMS2D>\n") )
      break;
    if ( strstr(p, "<Project>") )
      _projectName = XMLgetElementValue(p);
    else
    if ( strstr(p, "<FlightNumber>") )
      _flightNumber = XMLgetElementValue(p);
    else
    if ( strstr(p, "<FlightDate>") )
      _flightDate = XMLgetElementValue(p);
    else
    if ( strstr(p, "<probe") )
      {
      AddToProbeListFromXML(p);
      }
    }
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::AddToProbeListFromXML(const char xml_entry[])
{
  char *id = strstr((char *)xml_entry, "id=") + 4;
  if (id == 0)
    return;

  switch (ProbeType((const unsigned char *)id))
  {
    case Probe::PMS2D:
      _probeList[*(uint16_t *)id] = new PMS2D(xml_entry, PMS2_SIZE);
      break;
    case Probe::FAST2D:
      _probeList[*(uint16_t *)id] = new Fast2D(xml_entry, PMS2_SIZE);
      break;
    case Probe::TWODS:
      _probeList[*(uint16_t *)id] = new TwoDS(xml_entry, PMS2_SIZE);
      break;
    case Probe::HVPS:
      _probeList[*(uint16_t *)id] = new HVPS(xml_entry, PMS2_SIZE);
      break;
    case Probe::CIP:
      _probeList[*(uint16_t *)id] = new CIP(xml_entry, PMS2_SIZE);
      break;
    default:
      fprintf(stderr, "DataFile::initOAP, Unknown probe type, [%c%c]\n", id[0], id[1]);
      fprintf(stderr, "DataFile:: [%s]\n", xml_entry);
  }
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::AddToProbeList(const char *id)
{
  if (id == 0)
    return;

  switch (ProbeType((const unsigned char *)id))
  {
    case Probe::PMS2D:
      _probeList[*(uint16_t *)id] = new PMS2D(id);
      break;
    case Probe::HVPS:
      _probeList[*(uint16_t *)id] = new HVPS(id);
      break;
    case Probe::CIP:
      _probeList[*(uint16_t *)id] = new CIP(id);
      break;
    default:
      fprintf(stderr, "DataFile::initOAP, Unknown probe type, [%c%c]\n", id[0], id[1]);
  }
}

/* -------------------------------------------------------------------- */
Probe::ProbeType ADS_DataFile::ProbeType(const unsigned char *id)
{
  if (id[0] == 'C' || id[0] == 'P')
  {
    if (id[1] >= '4' && id[1] <= '7')
      return Probe::FAST2D;

    if (id[1] == '8')
      return Probe::CIP;

    return Probe::PMS2D;
  }

  if (id[0] == '2' || id[0] == '3' || id[0] == 'S')
    return Probe::TWODS;

  if (id[0] == 'H')
    return Probe::HVPS;

  if (id[0] == 'G')
    return Probe::GREYSCALE;

  return Probe::UNKNOWN;
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::ToggleSyntheticData()
{
  int		i, j;
  uint32_t		*p;
  unsigned short	*s;

  _useTestRecord = 1 - _useTestRecord;

  strcpy((char *)&CtestRecord.id, "C1");
  CtestRecord.hour = 12;
  CtestRecord.minute = 30;
  CtestRecord.second = 30;
  CtestRecord.tas = 100 * 255 / 125;
  CtestRecord.msec = 100;
  CtestRecord.overld = 36;

  p = (uint32_t *)CtestRecord.data;
  for (i = 0; i < 48; ++i)
    for (j = 0; j < 21; ++j)
      *p++ = CtestParticle[j];

  for (j = 0; j < 16; ++j)
    *p++ = CtestParticle[j];


  strcpy((char *)&PtestRecord.id, "P1");
  PtestRecord.hour = 12;
  PtestRecord.minute = 30;
  PtestRecord.second = 30;
  PtestRecord.tas = 100 * 255 / 125;
  PtestRecord.msec = 100;
  PtestRecord.overld = 30;

  p = (uint32_t *)PtestRecord.data;
  for (i = 0; i < 78; ++i)
    for (j = 0; j < 13; ++j)
      *p++ = PtestParticle[j];

  for (j = 0; j < 10; ++j)
    *p++ = PtestParticle[j];


  strcpy((char *)&HtestRecord.id, "H1");
  HtestRecord.hour = 12;
  HtestRecord.minute = 30;
  HtestRecord.second = 30;
  HtestRecord.tas = 100 * 255 / 125;
  HtestRecord.msec = 100;
  HtestRecord.overld = 30;

  s = (unsigned short *)HtestRecord.data;
  for (i = 0; i < 204; ++i)
    for (j = 0; j < 10; ++j)
      *s++ = HtestParticle[j];

  for (j = 0; j < 8; ++j)
    *s++ = HtestParticle[j];

}	/* END TOGGLESYNTHETICDATA */

/* -------------------------------------------------------------------- */
void ADS_DataFile::SetPosition(int position)
{
  size_t	pos;
  P2d_rec	buff;

  pos = nIndices * position / 100;

  if (pos == nIndices)
    --pos;

  int hour = ntohs(indices[pos].time[0]);

  if (ntohs(indices[0].time[0]) > 12 && hour < 12)
    hour += 24;

  LocatePMS2dRecord(&buff, hour,
	ntohs(indices[pos].time[1]), ntohs(indices[pos].time[2]));

}	/* END SETPOSITION */

/* -------------------------------------------------------------------- */
bool ADS_DataFile::LocatePMS2dRecord(P2d_rec *buff, int hour, int minute, int second)
{
  int	i;
  bool	rc, startPreMidnight = False;

  if (ntohs(indices[0].time[0]) > 12)
    startPreMidnight = True;

  for (i = 1; indices[i].index > 0 && ntohs(indices[i].time[0]) < hour; ++i)
    if (startPreMidnight && ntohs(indices[i].time[0]) < 12 && hour > 12)
      hour -= 24;

  for (; indices[i].index > 0; ++i)
    {
    if (ntohs(indices[i].time[0]) < hour)
      continue;
    if (ntohs(indices[i].time[1]) < minute)
      continue;

    break;
    }

  for (; indices[i].index > 0 && ntohs(indices[i].time[2]) < second; ++i)
    ;

  if (indices[i].index == -1)
    return(false);

  currPhys = std::max(0, i - 2);
  currLR = P2dLRpPR;
  rc = NextPMS2dRecord(buff);

  while (rc && ntohs(buff->second) < second)
    rc = NextPMS2dRecord(buff);

  SwapPMS2D(buff);
  return(rc);

}	/* END LOCATEPMS2DRECORD */

/* -------------------------------------------------------------------- */
bool ADS_DataFile::FirstPMS2dRecord(P2d_rec *buff)
{
  currPhys = currLR = 0;

  if (_useTestRecord)
    {
    memcpy((char *)buff, (char *)&CtestRecord, sizeof(P2d_rec));
    return(true);
    }

  if (indices[0].index == -1)	// No 2d records in file.
    return(false);

#ifdef PNG
  if (_gzipped)
    {
    gzseek(gz_fd, (z_off_t)indices[0].index, SEEK_SET);
    gzread(gz_fd, physRecord, sizeof(P2d_rec) * P2dLRpPR);
    }
  else
#endif
    {
    fseeko(fp, indices[0].index, SEEK_SET);
    fread(physRecord, sizeof(P2d_rec), P2dLRpPR, fp);
    }

  memcpy((char *)buff, (char *)physRecord, sizeof(P2d_rec));
  SwapPMS2D(buff);
  return(true);

}	/* END FIRSTPMS2DRECORD */

/* -------------------------------------------------------------------- */
bool ADS_DataFile::NextPMS2dRecord(P2d_rec *buff)
{
  if (_useTestRecord)
    {
    if (testRecP == &CtestRecord)
      testRecP = &PtestRecord;
    else
    if (testRecP == &PtestRecord)
      testRecP = &HtestRecord;
    else
      testRecP = &CtestRecord;

    testRecP->msec += 80;
    if (testRecP->msec >= 1000)
      {
      testRecP->msec -= 1000;
      if (++testRecP->second > 59)
        ++testRecP->minute;
      }

    memcpy((char *)buff, (char *)testRecP, sizeof(P2d_rec));
    return(true);
    }

  if (indices[currPhys].index == -1)
    return(false);

  if (++currLR >= P2dLRpPR)
    {
    currLR = 0;

    if (indices[++currPhys].index == -1)
      {
      --currPhys;
      return(false);
      }

#ifdef PNG
    if (_gzipped)
      {
      gzseek(gz_fd, (z_off_t)indices[currPhys].index, SEEK_SET);
      gzread(gz_fd, physRecord, sizeof(P2d_rec) * P2dLRpPR);
      }
    else
#endif
      {
      fseeko(fp, indices[currPhys].index, SEEK_SET);
      fread(physRecord, sizeof(P2d_rec), P2dLRpPR, fp);
      }
    }

  memcpy((char *)buff, (char *)&physRecord[currLR], sizeof(P2d_rec));
  SwapPMS2D(buff);
  return(true);

}	/* END NEXTPMS2DRECORD */

/* -------------------------------------------------------------------- */
bool ADS_DataFile::PrevPMS2dRecord(P2d_rec *buff)
{
  if (_useTestRecord)
    {
    if (testRecP == &CtestRecord)
      testRecP = &PtestRecord;
    else
      testRecP = &CtestRecord;

    testRecP->msec -= 80;
    if (testRecP->msec < 0)
      {
      testRecP->msec += 1000;
      if (--testRecP->second < 0)
        --testRecP->minute;
      }

    memcpy((char *)buff, (char *)testRecP, sizeof(P2d_rec));
    return(false);
    }

  if (--currLR < 0)
    {
    currLR = P2dLRpPR-1;

    if (--currPhys < 0)
      {
      currLR = 0;
      currPhys = 0;
      return(false);
      }

#ifdef PNG
    if (_gzipped)
      {
      gzseek(gz_fd, (z_off_t)indices[currPhys].index, SEEK_SET);
      gzread(gz_fd, physRecord, sizeof(P2d_rec) * P2dLRpPR);
      }
    else
#endif
      {
      fseeko(fp, indices[currPhys].index, SEEK_SET);
      fread(physRecord, sizeof(P2d_rec), P2dLRpPR, fp);
      }
    }

  memcpy((char *)buff, (char *)&physRecord[currLR], sizeof(P2d_rec));
  SwapPMS2D(buff);
  return(true);

}	/* END PREVPMS2DRECORD */

/* -------------------------------------------------------------------- */
int ADS_DataFile::NextPhysicalRecord(unsigned char buff[])
{
  int	rc, idWord;
  int	size = sizeof(short);

#ifdef PNG
  if (_gzipped)
    {
    _savePos = gztell(gz_fd);
    rc = gzread(gz_fd, buff, size) / size;
    }
  else
#endif
    {
    _savePos = ftello(fp);
//  if ((nBytes = read(fileno(fp), buff, size)) == 0 && GetNextADSfile())
    rc = fread(buff, size, 1, fp);
    }

  if (rc != 1)
    return(0);

  idWord = ntohs(*((unsigned short *)buff));

  switch (idWord)
    {
    case SDI_WORD:
      if (_hdr)
        size = (_hdr->lrLength() * _hdr->lrPpr()) - sizeof(short);
      break;

    case ADS_WORD:
      size = 18;
      break;

    case HDR_WORD:
      if (_hdr)
        size = _hdr->HeaderLength() - sizeof(short);
      break;

    case 0x4d43:      // MCR
      size = sizeof(Mcr_rec) - sizeof(short);
      break;

    case PMS2DC1: case PMS2DC2: // Original 32 diode PMS2D C probes (25um).
    case PMS2DP1: case PMS2DP2:	// Original 32 diode PMS2D P probes (200um).
    case PMS2DC4: case PMS2DC5:	// 64 diode 25 um 2DC.
    case PMS2DC6:		// 64 diode 10 um 2DC.
    case PMS2DC8: case PMS2DP8:	// CIP / PIP
    case PMS2DP4:		// 64 bit 2DP.
    case SPEC2DSH: case SPEC2DSV:	// SPEC 2DS
    case HVPS1: case HVPS2:	// HVPS
      size = (P2dLRpPR * sizeof(P2d_rec)) - sizeof(short);
      break;

    case PMS2DG1: case PMS2DG2: /* GrayScale */
      size = 32000 - sizeof(short);
      break;

    default:
      size = 0;
    }

#ifdef PNG
  if (_gzipped)
    {
    if (gzread(gz_fd, &buff[sizeof(short)], size) != size)
      size = 0;
    }
  else
#endif
    {
    if (fread(&buff[sizeof(short)], size, 1, fp) != 1)
      size = 0;
    }

  return(size + sizeof(short));

}	/* END NEXTPHYSICALRECORD */

/* -------------------------------------------------------------------- */
void ADS_DataFile::buildIndices()
{
  size_t cnt = 0;
  int	rc;
  FILE	*fpI;
  unsigned char	buffer[0x8000];
  char	tmpFile[256], *p;


  strcpy(tmpFile, _fileName.c_str());

  if ((p = strstr(tmpFile, ".2d")) == NULL)
    strcat(tmpFile, ".2Didx");
  else
    strcpy(p, ".2Didx");


  // Check for indices file....
  if ( (fpI = fopen(tmpFile, "rb")) )
    {
    long	len;

    fseek(fpI, 0, SEEK_END);
    len = ftell(fpI);

    if ((indices = (Index *)malloc(len)) == NULL)
      {
      fprintf(stderr, "buildIndices: Memory allocation error, fatal.\n");
      exit(1);
      }

    printf("Reading indices file, %s.\n", tmpFile);

    rewind(fpI);
    fread(indices, len, 1, fpI);
    fclose(fpI);

    if (5 != ntohl(5))		// If Intel architecture, swap bytes.
      for (size_t i = 0; i < len / sizeof(Index); ++i)
        indices[i].index = ntohll(&indices[i].index);

    nIndices = (len / sizeof(Index)) - 1;
    return;
    }


  printf("Building indices...."); fflush(stdout);
  FlushEvents();

  if ((indices = (Index *)malloc(8000000 * sizeof(Index))) == NULL)
    {
    fprintf(stderr, "buildIndices: Memory allocation error, fatal.\n");
    exit(1);
    }

#ifdef PNG
  if (_gzipped)
    gzseek(gz_fd, 0, SEEK_SET);
  else
#endif


  for (cnt = 0; (rc = NextPhysicalRecord(buffer)); )
    {
    size_t i;
    ProbeList::const_iterator iter;

    if (_fileHeaderType != NoHeader)
      {
      for (iter = _probeList.begin(); iter != _probeList.end(); ++iter)
        if (memcmp(iter->second->Code(), buffer, 2) == 0)
          break;

      if (iter == _probeList.end())	// shouldn't get here?
        continue;
      }
    else
      {
      for (iter = _probeList.begin(); iter != _probeList.end(); ++iter)
        if (memcmp(iter->second->Code(), buffer, 2) == 0)
          break;

      // Sanity check.
      if (!isValidProbe(buffer))
        continue;

      if (iter == _probeList.end())
        AddToProbeList((const char *)buffer);
      }

    for (i = 0; i < 1; ++i)
      {
      indices[cnt].index = _savePos + (sizeof(P2d_rec) * i);
      memcpy(indices[cnt].time, &buffer[2], 6);
      ++cnt;
      }
    }

  if (_fileHeaderType != OAP)
    {
#ifdef PNG
    if (_gzipped)
      gzseek(gz_fd, 0, SEEK_SET);
    else
#endif
      rewind(fp);
    }

  printf("\n%zu 2d records were found.\n", cnt);

  indices[cnt].index = -1;

  if ((indices = (Index *)realloc(indices, (cnt+1) * sizeof(Index))) == NULL)
    {
    fprintf(stderr, "buildIndices: Memory re-allocation error, fatal.\n");
    exit(1);
    }

//  SortIndices(cnt);

/* Write new 2d file.  Used to fix IDEAS-III files that were out of time order.
    {

    }
 */


  // Write indices to a file for future use.
  if ( _fileHeaderType != NoHeader && (fpI = fopen(tmpFile, "w+b")) )
    {
    printf("Writing indices file, %s.\n", tmpFile);

    if (5 != ntohl(5))		// If Intel architecture, swap bytes.
      for (size_t i = 0; i < cnt+1; ++i)
        indices[i].index = htonll(&indices[i].index);

    fwrite(indices, (cnt+1) * sizeof(Index), 1, fpI);
    fclose(fpI);

    if (5 != ntohl(5))		// Swap em back for current run.
      for (size_t i = 0; i < cnt+1; ++i)
        indices[i].index = ntohll(&indices[i].index);
    }

  nIndices = cnt;

}	/* END BUILDINDICES */

/* -------------------------------------------------------------------- */
void ADS_DataFile::SwapPMS2D(P2d_rec *buff)
{
  // Perform byte swapping on whole [data] record if required.
  if (1 != ntohs(1))
  {
    unsigned short	*sp = (unsigned short *)buff;

    for (int i = 1; i < 10; ++i)	// Swap header
      sp[i] = ntohs(sp[i]);

    if (ProbeType((unsigned char *)buff) == Probe::TWODS)
    {
      unsigned char tmp[16], *cp = (unsigned char *)buff->data;
      // 256 slices at 16 bytes each.
      for (size_t i = 0; i < nSlices_128bit; ++i, cp += 16)
      {
        for (size_t j = 0; j < 16; ++j)
          tmp[j] = cp[15-j];
        memcpy(cp, tmp, 16);
      }
    }
    else
    if (ProbeType((unsigned char *)buff) == Probe::HVPS)
    {
      sp = (unsigned short *)buff->data;
      for (size_t i = 0; i < 2048; ++i, ++sp)
        *sp = ntohs(*sp);
    }
  }

  /* Code for Lake-ICE data, which was shifted 1 bit from overclocking
   * the data lines.  Clocking was fine, except the data lines from the
   * wing tips was too long, so we had to cut in half.  This was the
   * first project with ADS2 PMS2D card.
   */
  if (ProjectNumber().compare("812") == 0)
  {
    uint32_t *p = (uint32_t *)buff->data;

    for (size_t i = 0; i < nSlices_32bit; ++i, ++p)
    {
      // It only matters to fix the sync & timing words.
      if (*p == 0xff800000)
      {
        *p = PMS2D::SyncWordMask;
        *(p-1) <<= *(p-1);
      }
    }
  }

  /* RICO stuck bit cleanup.
   */
  if (ProjectNumber().compare("135") == 0)
  {
    check_rico_half_buff(buff, 0, nSlices_32bit/2);
    check_rico_half_buff(buff, nSlices_32bit/2, nSlices_32bit);
  }
}

/* -------------------------------------------------------------------- */
bool ADS_DataFile::isValidProbe(const unsigned char *pr) const
{
  // Sanity check.
  if ((pr[0] == 'C' || pr[0] == 'P' || pr[0] == 'H') && isdigit(pr[1]))
    return(true);

  // SPEC 2DS
  if ((pr[0] == '2' || pr[0] == '3' || pr[0] == 'S') && (pr[1] == 'V' || pr[1] == 'H'))
    return(true);

  return(false);
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::SortIndices(int cnt)
{
  sort_the_table(0, cnt-1);

}       /* SORTTABLE */

/* -------------------------------------------------------------------- */
void ADS_DataFile::sort_the_table(int beg, int end)
{
  Index	*mid, temp;
  int	x = beg, y = end;

  mid = &indices[(x + y) / 2];

  while (x <= y)
    {
    while (memcmp(indices[x].time, mid->time, 6) < 0)
      ++x;

    while (memcmp(indices[y].time, mid->time, 6) > 0)
      --y;

    if (x <= y)
      {
      temp = indices[x];
      indices[x] = indices[y];
      indices[y] = temp;

      ++x;
      --y;
      }
    }

  if (beg < y)
    sort_the_table(beg, y);

  if (x < end)
    sort_the_table(x, end);
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::check_rico_half_buff(P2d_rec *buff, size_t beg, size_t end)
{
  std::vector<size_t> spectra, sorted_spectra;

  for (size_t j = 0; j < 32; ++j)
    spectra.push_back(0);

  // Generate spectra.
  uint32_t *p = (uint32_t *)buff->data;
  bool firstSyncWord = beg == 0 ? false : true;

  for (size_t i = beg; i < end; ++i, ++p)
  {
    uint32_t slice = ntohl(*p);

    // There seemed to be lots of splatter at the start of the buffer,
    // skip until first sync word appears.
    if (!firstSyncWord)
    {
      if ((slice & PMS2D::SyncWordMask) == 0x55000000)
        firstSyncWord = true;
      else
        continue;
    }

    if ((slice & PMS2D::SyncWordMask) == 0x55000000 || slice == 0xffffffff)
      continue;

    slice = ~slice;
    for (size_t j = 0; j < 32; ++j)
      if (((slice >> j) & 0x01) == 0x01)
        ++spectra[j];
  }

  // Sort the spectra and compare the last bin to the one next too it and see
  // if there is a large descrepency.  If so, probably a bad 1/2 buffer.
  sorted_spectra = spectra;
  sort(sorted_spectra.begin(), sorted_spectra.end());

  if ((sorted_spectra[30] * 2 < sorted_spectra[31]))
  {
    int stuck_bin = -1;
    for (size_t j = 0; j < 32; ++j)
      if (spectra[j] == sorted_spectra[31])
        stuck_bin = j;

    fprintf(stderr,
	"DataFile.cc: %02d:%02d:%02d.%d - Performing stuck bit correction, bit %d, ",
	buff->hour, buff->minute, buff->second, buff->msec, stuck_bin);

    if (beg == 0)
      fprintf(stderr, "first half.\n");
    else
      fprintf(stderr, "second half.\n");

    if (stuck_bin == -1)
    {
      fprintf(stderr, "DataFile.cc:  Impossible.\n");
      exit(1);
    }

    uint32_t mask1 = (0x01 << stuck_bin);
    uint32_t mask2 = (0x07 << (stuck_bin-1));
    uint32_t *p = (uint32_t *)buff->data;
    for (size_t i = beg; i < end; ++i, ++p)
    {
      uint32_t slice = *p;
      if ((slice & PMS2D::SyncWordMask) == 0x55000000 || *p == 0xffffffff)
        continue;

      slice = ~slice;
      if ((slice & mask2) == mask1)
        *p = htonl(~(slice & ~mask1));
    }
  }
}

/* -------------------------------------------------------------------- */
ADS_DataFile::~ADS_DataFile()
{
  free(indices);

  delete _hdr;
  _hdr = 0;

  for (size_t i = 0; i < _probeList.size(); ++i)
    delete _probeList[i];

#ifdef PNG
  if (_gzipped)
    gzclose(gz_fd);
  else
#endif
    fclose(fp);

}	/* END DESTRUCTOR */

/* END DATAFILE.CC */
