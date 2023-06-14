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
#include "raf/OAProbeFactory.h"

#include <unistd.h>
#include <algorithm>

using namespace OAP;

static OAP::P2d_rec PtestRecord, CtestRecord, HtestRecord;

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

using namespace OAP;

/* -------------------------------------------------------------------- */
ADS_DataFile::ADS_DataFile(const char fName[], UserConfig &cfg)
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
    fprintf(stderr, "Can't open file %s", _fileName.c_str());
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
    initADS3((const char *)buffer, &cfg);	// the XML header file.
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
    initADS2(&cfg);
    if (_hdr) strcpy(_version, _hdr->Version());
    }

  buildIndices(&cfg);

  currLR = -1; currPhys = 0;

  if (ProjectNumber().compare("812") == 0)
    fprintf(stderr, "DataFile.cc: Lake-ICE - Bit shift correction will be performed on all records.\n");

  if (ProjectNumber().compare("135") == 0)
    fprintf(stderr, "DataFile.cc: RICO - Stuck bit correction will be performed.\n");

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void ADS_DataFile::initADS2(UserConfig *cfg)
{
  int	Ccnt, Pcnt, Hcnt;
  Ccnt = Pcnt = Hcnt = 0;

  _fileHeaderType = ADS2;

  _hdr = new Header(_fileName.c_str());
  _projectName = _hdr->ProjectNumber();
  _flightNumber = _hdr->FlightNumber();
  _flightDate = _hdr->FlightDate();
printf("ADS2 hdr length = %d\n", _hdr->HeaderLength());
  if (_hdr->isValid() == false)
    return;

  for (const void *blk = _hdr->GetFirst("PMS2D"); blk; blk = _hdr->GetNext("PMS2D"))
    {
    const char *name = _hdr->VariableName((Pms2 *)blk);

    if (name[3] == 'P')
      {
      PMS2D *p = new PMS2D(cfg, _hdr, (Pms2 *)blk, ++Pcnt);
      _probeList[*(uint16_t *)p->Code()] = p;
      }
    else
    if (name[3] == 'C')
      {
      PMS2D *p = new PMS2D(cfg, _hdr, (Pms2 *)blk, ++Ccnt);
      _probeList[*(uint16_t *)p->Code()] = p;
      }
    else
    if (name[3] == 'H')
      {
      HVPS *p = new HVPS(cfg, _hdr, (Pms2 *)blk, ++Hcnt);
      _probeList[*(uint16_t *)p->Code()] = p;
      }
    }
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::initADS3(const char *hdrString, UserConfig *cfg)
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
    snprintf(_version, 16, "5.%d", version);
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
      AddToProbeListFromXML(p, cfg);
      }
    }
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::AddToProbeListFromXML(const char xml_entry[], UserConfig *cfg)
{
  char *id = strstr((char *)xml_entry, "id=") + 4;
  if (id == 0)
    return;

  OAProbeFactory *factory = OAProbeFactory::getFactory();

  _probeList[*(uint16_t *)id] = factory->create(xml_entry, cfg);
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::AddToProbeList(const char *id, UserConfig *cfg)
{
  if (id == 0)
    return;

  switch (probeType((const unsigned char *)id))
  {
    case PMS2D_T:
      _probeList[*(uint16_t *)id] = new PMS2D(cfg, id);
      break;
    case HVPS_T:
      _probeList[*(uint16_t *)id] = new HVPS(cfg, id);
      break;
    case CIP_T:
      _probeList[*(uint16_t *)id] = new CIP(cfg, id);
      break;
    default:
      fprintf(stderr, "DataFile::initOAP, Unknown probe type, [%c%c]\n", id[0], id[1]);
  }
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
  OAP::P2d_rec	buff;

  pos = _indices.size() * position / 100;

  if (pos == _indices.size())
    --pos;

  int hour = ntohs(_indices[pos].time[0]);

  if (ntohs(_indices[0].time[0]) > 12 && hour < 12)
    hour += 24;

  LocatePMS2dRecord(&buff, hour,
	ntohs(_indices[pos].time[1]), ntohs(_indices[pos].time[2]));

}	/* END SETPOSITION */

/* -------------------------------------------------------------------- */
bool ADS_DataFile::LocatePMS2dRecord(OAP::P2d_rec *buff, int hour, int minute, int second)
{
  size_t i;
  bool	rc, startPreMidnight = false;

  if (ntohs(_indices[0].time[0]) > 12)
    startPreMidnight = true;

  for (i = 1; _indices[i].index > 0 && ntohs(_indices[i].time[0]) < hour; ++i)
    if (startPreMidnight && ntohs(_indices[i].time[0]) < 12 && hour > 12)
      hour -= 24;

  for (; _indices[i].index > 0; ++i)
    {
    if (ntohs(_indices[i].time[0]) < hour)
      continue;
    if (ntohs(_indices[i].time[1]) < minute)
      continue;

    break;
    }

  for (; i < _indices.size() && ntohs(_indices[i].time[2]) < second; ++i)
    ;

  if (i == _indices.size())
    return(false);

  currPhys = std::max(0, (int)i - 2);
  currLR = P2dLRpPR;
  rc = NextPMS2dRecord(buff);

  while (rc && ntohs(buff->second) < second)
    rc = NextPMS2dRecord(buff);

  SwapPMS2D(buff);
  return(rc);

}	/* END LOCATEPMS2DRECORD */

/* -------------------------------------------------------------------- */
bool ADS_DataFile::FirstPMS2dRecord(OAP::P2d_rec *buff)
{
  currPhys = currLR = 0;

  if (_useTestRecord)
    {
    memcpy((char *)buff, (char *)&CtestRecord, sizeof(OAP::P2d_rec));
    return(true);
    }

  if (_indices.size() == 0)	// No 2d records in file.
    return(false);

#ifdef PNG
  if (_gzipped)
    {
    gzseek(gz_fd, (z_off_t)_indices[0].index, SEEK_SET);
    gzread(gz_fd, physRecord, sizeof(OAP::P2d_rec) * P2dLRpPR);
    }
  else
#endif
    {
    fseeko(fp, _indices[0].index, SEEK_SET);
    fread(physRecord, sizeof(OAP::P2d_rec), P2dLRpPR, fp);
    }

  memcpy((char *)buff, (char *)physRecord, sizeof(OAP::P2d_rec));
  SwapPMS2D(buff);
  return(true);

}	/* END FIRSTPMS2DRECORD */

/* -------------------------------------------------------------------- */
bool ADS_DataFile::NextPMS2dRecord(OAP::P2d_rec *buff)
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

    memcpy((char *)buff, (char *)testRecP, sizeof(OAP::P2d_rec));
    return(true);
    }

  if (currPhys >= _indices.size())
//  if (_indices[currPhys].index == -1)
    return(false);

  if (++currLR >= P2dLRpPR)
    {
    currLR = 0;

    if (++currPhys >= _indices.size())
//    if (_indices[++currPhys].index == -1)
      {
      --currPhys;
      return(false);
      }

#ifdef PNG
    if (_gzipped)
      {
      gzseek(gz_fd, (z_off_t)_indices[currPhys].index, SEEK_SET);
      gzread(gz_fd, physRecord, sizeof(OAP::P2d_rec) * P2dLRpPR);
      }
    else
#endif
      {
      fseeko(fp, _indices[currPhys].index, SEEK_SET);
      fread(physRecord, sizeof(OAP::P2d_rec), P2dLRpPR, fp);
      }
    }

  memcpy((char *)buff, (char *)&physRecord[currLR], sizeof(OAP::P2d_rec));
  SwapPMS2D(buff);
  return(true);

}	/* END NEXTPMS2DRECORD */

/* -------------------------------------------------------------------- */
bool ADS_DataFile::PrevPMS2dRecord(OAP::P2d_rec *buff)
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

    memcpy((char *)buff, (char *)testRecP, sizeof(OAP::P2d_rec));
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
      gzseek(gz_fd, (z_off_t)_indices[currPhys].index, SEEK_SET);
      gzread(gz_fd, physRecord, sizeof(OAP::P2d_rec) * P2dLRpPR);
      }
    else
#endif
      {
      fseeko(fp, _indices[currPhys].index, SEEK_SET);
      fread(physRecord, sizeof(OAP::P2d_rec), P2dLRpPR, fp);
      }
    }

  memcpy((char *)buff, (char *)&physRecord[currLR], sizeof(OAP::P2d_rec));
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
      size = (P2dLRpPR * sizeof(OAP::P2d_rec)) - sizeof(short);
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
time_t ADS_DataFile::getFileModifyTime(const char *path)
{
  struct stat attr;
  if (stat(path, &attr) == 0)
    return attr.st_mtime;

  return 0;
}

/* -------------------------------------------------------------------- */
void ADS_DataFile::buildIndices(UserConfig *cfg)
{
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
  if (getFileModifyTime(_fileName.c_str()) < getFileModifyTime(tmpFile) &&
      (fpI = fopen(tmpFile, "rb")) )
    {
    long	len;

    fseek(fpI, 0, SEEK_END);
    len = ftell(fpI);

    _indices.resize(len / sizeof(Index));

    printf("Reading indices file, %s.\n", tmpFile);

    rewind(fpI);
    fread(&_indices[0], len, 1, fpI);
    fclose(fpI);

    if (5 != ntohl(5))		// If Intel architecture, swap bytes.
      for (size_t i = 0; i < _indices.size(); ++i)
        _indices[i].index = ntohll(&_indices[i].index);

    return;
    }


  printf("Building indices...."); fflush(stdout);

#ifdef PNG
  if (_gzipped)
    gzseek(gz_fd, 0, SEEK_SET);
  else
#endif


  for (; (rc = NextPhysicalRecord(buffer)); )
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
        AddToProbeList((const char *)buffer, cfg);
      }

    for (i = 0; i < 1; ++i)
      {
      Index newOne;
      newOne.index = _savePos + (sizeof(OAP::P2d_rec) * i);
      memcpy(newOne.time, &buffer[2], 6);
      _indices.push_back(newOne);
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

  printf("\n%zu 2d records were found.\n", _indices.size());

//  SortIndices(cnt);

  // Write indices to a file for future use.
  if ( _fileHeaderType != NoHeader && (fpI = fopen(tmpFile, "w+b")) )
    {
    printf("Writing indices file, %s.\n", tmpFile);

    if (5 != ntohl(5))		// If Intel architecture, swap bytes.
      for (size_t i = 0; i < _indices.size(); ++i)
        _indices[i].index = htonll(&_indices[i].index);

    fwrite(&_indices[0], sizeof(Index), _indices.size(), fpI);
    fclose(fpI);

    if (5 != ntohl(5))		// Swap em back for current run.
      for (size_t i = 0; i < _indices.size(); ++i)
        _indices[i].index = ntohll(&_indices[i].index);
    }

}	/* END BUILDINDICES */

/* -------------------------------------------------------------------- */
void ADS_DataFile::SwapPMS2D(OAP::P2d_rec *buff)
{
  swapPMS2D(buff);

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
  sort_the_table(0, _indices.size());

}       /* SORTTABLE */

/* -------------------------------------------------------------------- */
void ADS_DataFile::sort_the_table(int beg, int end)
{
  Index	temp;
  int	x = beg, y = end, mid;

  mid = (beg + end) / 2;

  while (x <= y)
    {
    while (memcmp(_indices[x].time, _indices[mid].time, 6) < 0)
      ++x;

    while (memcmp(_indices[y].time, _indices[mid].time, 6) > 0)
      --y;

    if (x <= y)
      {
      temp = _indices[x];
      _indices[x] = _indices[y];
      _indices[y] = temp;

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
void ADS_DataFile::check_rico_half_buff(OAP::P2d_rec *buff, size_t beg, size_t end)
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
  delete _hdr;

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
