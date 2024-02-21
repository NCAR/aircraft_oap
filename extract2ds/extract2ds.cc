/*
-------------------------------------------------------------------------
OBJECT NAME:    spec2oap.cc

FULL NAME:      Translate SPEC to OAP

DESCRIPTION:    Translate SPEC OAP data to RAF OAP format.

COPYRIGHT:      University Corporation for Atmospheric Research, 2023
-------------------------------------------------------------------------
*/

#include <algorithm>
#include <cstdio>
#include <cstring>

#include <arpa/inet.h>

#include <raf/OAP.h>

#include "config.h"
#include "spec.h"
#include "particle.h"

// Output OAP file version number.
static const int FILE_VERSION = 1;


Config cfg;

int  recordCnt = 0;
bool verbose = false;

const uint16_t SyncWord = 0x3253;	// Particle sync word '2S'.
const uint16_t MaskData = 0x4d4b;	// MK Flush Buffer.
const uint16_t FlushWord = 0x4e4c;	// NL Flush Buffer.
const uint16_t HousekeepWord = 0x484b;	// HK Flush Buffer.

const uint64_t Config::Type32_TimingWordMask = 0x00000000FFFFFFFFL; // do I need this?
const uint64_t Config::Type48_TimingWordMask = 0x0000FFFFFFFFFFFFL;


Particle *probe[2] = { 0, 0 };

int findHouseKeeping48(FILE *hkfp, imageBuf *imgRec);



bool cksum(const uint16_t buff[], int nWords, uint16_t ckSum)
{
  uint16_t sum = 0;

  for (int i = 0; i < nWords; ++i)
    sum += buff[i];

  if (sum != ckSum)
    printf("Checksum mis-match %u %u, record #%d\n", sum, ckSum, recordCnt);

  return sum == ckSum;
}


int findLastTimeWord(uint16_t *p)
{
  int pCnt = 0;
  unsigned long lastWord = 0;

  for (int i = 0; i < 2048; ++i)
  {
    if (p[i] == FlushWord)
      break;

    if (p[i] == SyncWord)
    {
      uint16_t n = 0, test = 0;
      if (p[i+1] > 0) { ++test; n = p[i+1]; }
      if (p[i+2] > 0) { ++test; n = p[i+2]; }

      if (test != 1)
        { printf("bad particle = 0\n"); continue; }

      ++pCnt;


      if (i < 2043)
      {
        // grab timingWord.
        if ((n & 0x1000) == 0)	// bit 12 not set
        {
          n &= 0x0fff;
printf("ID=%u - i=%d + 5=5 + n=%d = %d\n", p[i+3], i, n, i+5+n);
          if (i + 5 + n < 2048)
          {
            if (cfg.DataFormat() == Type48)
              lastWord = ((uint64_t *)&p[i+5+n-3])[0] & Type48_TimingWordMask;
            else
              lastWord = ((uint32_t *)&p[i+5+n-2])[0];
          }

          printf(" lastTWord=%lu\n", lastWord);
        }
        i += 5 + n - 1;	// -1 bacause +1 will happen as loop increments.
      }
    }
    else printf("skipping %d\n", i);
  }

  return(pCnt);
}


int moreData(FILE *infp, unsigned char buffer[], OAP::P2d_hdr &oapHdr, FILE *hkfp)
{
  if (verbose)
    printf("  moreData\n");

  int rc = fread(buffer, 4114, 1, infp);

  if (rc != 1)
    return rc;

  ++recordCnt;
  struct imageBuf *tds = (struct imageBuf *)buffer;
  int pCnt = findLastTimeingWord( (uint16_t *)tds->rdf );


  if (verbose)
    printf("%d/%02d/%02d %02d:%02d:%02d.%03d - pCnt=%d - 0x%04x\n", tds->year, tds->month,
	tds->day, tds->hour, tds->minute, tds->second, tds->msecond,
	pCnt, ((uint16_t *)tds->rdf)[0]);

  cksum((uint16_t *)tds->rdf, 2048, tds->cksum);

  oapHdr.year = htons(tds->year);
  oapHdr.month = htons(tds->month);
  oapHdr.day = htons(tds->day);
  oapHdr.hour = htons(tds->hour);
  oapHdr.minute = htons(tds->minute);
  oapHdr.second = htons(tds->second);
  oapHdr.msec = htons(tds->msecond);

  if (hkfp)	// If F2DS - type48
    oapHdr.tas = htons(findHouseKeeping48(hkfp, tds));

  oapHdr.overld = 0;

  if (probe[0]) probe[0]->setHeader(oapHdr);
  if (probe[1]) probe[1]->setHeader(oapHdr);

  return rc;
}


void processImageFile(FILE *infp, FILE *hkfp, FILE *outfp)
{
  static unsigned char	buffer[8192];
  static uint16_t	particle[8192];
  int	imCnt = 0, nlCnt = 0, partialPos = 0;
  int	rejectTooLong = 0, wrapsRecord = 0;

  if (probe[0] == 0)
  {
    if (cfg.Type().compare("F2DS") == 0)
    {
      probe[0] = new Particle("SV", outfp);
      probe[1] = new Particle("SH", outfp);
    }
    else
      probe[0] = new Particle("H1", outfp);
  }

  struct imageBuf *tds = (struct imageBuf *)buffer;
  uint16_t	*wp = ((uint16_t *)tds->rdf);
  OAP::P2d_hdr	outHdr;

  printf("processImageFile\n");

  moreData(infp, buffer, outHdr, hkfp);	// get first record, need to use timestamp and discard

  while (1)
  {
    if (moreData(infp, buffer, outHdr, hkfp) != 1) break;

    int j = 0;

    if (cfg.StoreCompressed())
    {
      OAP::P2d_rec outBuf;
      memcpy(&outBuf, &outHdr, sizeof(OAP::P2d_hdr));
      memcpy(outBuf.data, tds->rdf, OAP::OAP_BUFF_SIZE);
      fwrite(&outBuf, sizeof(outBuf), 1, outfp);
      continue;
    }

    for (; j < 2048; ++j)
    {
      if (wp[j] == FlushWord)		// NL flush buffer
      {
//        printf("%x - %d %d %d %d\n", wp[j], wp[j+1], wp[j+2], wp[j+3], wp[j+4]);
        if (verbose)
          printf(" NL Flush, pos = %d\n", j);
        nlCnt++;
        j = 2048;	// bail out, we are done with this record.
      }
      else
      if (wp[j] == MaskData)		// MK buffer
      {
        // ignoring MK at this time.
        if (verbose)
          printf(" MK, pos = %d\n", j);
        j += 22;
      }
      else
      if (wp[j] == HousekeepWord)	// HK buffer
      {
        if (j + 50 < 2048)
        {
          uint32_t tas;
          float *tasf = (float *)&tas;
          tas = (uint32_t)(wp[j+49] << 16) | wp[j+50];
          outHdr.tas = htons((uint16_t)*tasf);
          if (verbose)
            printf(" HK found, tas = %5.1f %d\n", *tasf, (int)*tasf);
        }
        else
          if (verbose)
            printf(" HK : tas location exceeds end of this record\n");

        j += 52;
      }
      else
      if (wp[j] == SyncWord)		// start particle
      {
        if (verbose)
          printf(" start particle, pos=%d\n", j);

        partialPos = 0;
        if (j > 2043)	// want particle 5 byte header
        {
          if (verbose) printf(" short header, j=%d\n", j);
          ++wrapsRecord;
          partialPos = 2048-j;
          memcpy(particle, &wp[j], partialPos * sizeof(uint16_t));
          if (moreData(infp, buffer, outHdr, hkfp) != 1) break;
          j = 0;
        }

        memcpy(&particle[partialPos], &wp[j], (5-partialPos) * sizeof(uint16_t));
        uint16_t nh = particle[H_CHN] & 0x0FFF;
        uint16_t nv = particle[V_CHN] & 0x0FFF;
        if (nh > 0 && nv > 0)	// One of these must be zero.
        {
          if (verbose) printf(" passing up on syncWord, nh&nv > 0\n");
          continue;
        }

        int n = std::max(nh, nv);
//printf("    %d %d %d\n", partialPos, j, n);
        j += (5-partialPos);

        partialPos = 0;
        if (j + n > 2048)
        {
          if (verbose) printf(" short image, j=%d, n=%d\n", j, n);
          ++wrapsRecord;
          partialPos = 2048-j;
          memcpy(&particle[5], &wp[j], partialPos * sizeof(uint16_t));
          if (moreData(infp, buffer, outHdr, hkfp) != 1) break;
          j = 0;
          n -= partialPos;
        }

//printf("    %d %d %d\n", partialPos, j, n);
        memcpy(&particle[5+partialPos], &wp[j], n * sizeof(uint16_t));
        j += (n-1);


        // OK, we can process particle
        imCnt++;
        if (particle[nSLICES] < 256)
        {
          if (nv)
            probe[0]->processParticle((uint16_t *)particle, verbose);
          else
            probe[1]->processParticle((uint16_t *)particle, verbose);
        }
        else
        {
          ++rejectTooLong;
          if (verbose) printf(" not processing particle # %d - n=%d\n", particle[3], particle[4]);
        }
      }
      else
        if (verbose)
          printf("Nothing found, j += 1\n");
    }
  }

  printf("Record cnt = %d, particle Cnt=%d, nullCnt=%d\n", recordCnt, imCnt, nlCnt);
  printf("                 rejected too long=%d, particle wraps to next record headers=%d\n",
		rejectTooLong, wrapsRecord);
}


int findHouseKeeping48(FILE *hkfp, imageBuf *imgRec)
{
  char	buffer[1024];
  uint32_t tas;
  float *tasf = (float *)&tas;

  struct hk48Buf *hkb = (struct hk48Buf *)buffer;
  struct maskBuf *mkb = (struct maskBuf *)buffer;

  while (fread(buffer, 18, 1, hkfp) == 1)
  {
    if (verbose)
      printf("%d/%02d/%02d %02d:%02d:%02d.%03d - 0x%04x len=%d\n", hkb->year,
	hkb->month, hkb->day, hkb->hour, hkb->minute, hkb->second, hkb->msecond,
	((uint16_t *)&hkb->rdf)[0], ((uint16_t *)&hkb->rdf)[1]);


    switch (((uint16_t *)&hkb->rdf)[0])
    {
      case 0x484b:		// HK buffer
        fread(&buffer[18], 164, 1, hkfp);
        cksum((uint16_t *)hkb->rdf, 82, hkb->cksum);

        // Locate houeskeeping with matching time stamp.
        if (hkb->year < imgRec->year) break;
        if (hkb->month < imgRec->month) break;
        if (hkb->day < imgRec->day) break;
        if (hkb->hour < imgRec->hour) break;
        if (hkb->minute < imgRec->minute) break;
        if (hkb->second < imgRec->second) break;

        tas = (uint32_t)((uint16_t *)&hkb->rdf)[75] << 16 | ((uint16_t *)&hkb->rdf)[76];
        if (verbose)
          printf(" tas = %5.1f\n", *tasf);
        fseek(hkfp, -182L, SEEK_CUR);
        return((int)*tasf);
        break;

      case 0x4d4b:		// Mask buffer
        fread(&buffer[18], 54, 1, hkfp);
        cksum((uint16_t *)mkb->rdf, 27, mkb->cksum);
        break;

      default:
        fread(&buffer[18], 2, 1, hkfp);
        printf(" no match %d/%d/%d %02d:%02d:%02d.%03d - %x\n", hkb->year,
		hkb->month, hkb->day, hkb->hour, hkb->minute, hkb->second, hkb->msecond,
		((uint16_t *)&hkb->rdf)[0]);
    }
  }

  printf("findHouseKeeping48: we shouldn't hit this.  EOF?\n");

  return(0);
}


void outputXMLheader(FILE *outfp)
{
  fprintf(outfp, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  fprintf(outfp, "<OAP version=\"%d\">\n", FILE_VERSION);
  fprintf(outfp, " <Source>ncar.ucar.edu</Source>\n");
  fprintf(outfp, " <FormatURL>http://www.eol.ucar.edu/raf/Software/OAPfiles.html</FormatURL>\n");
  fprintf(outfp, " <Project>%s</Project>\n", cfg.ProjectName().c_str());
  fprintf(outfp, " <Platform>%s</Platform>\n", cfg.Platform().c_str());
  fprintf(outfp, " <FlightNumber>%s</FlightNumber>\n", cfg.FlightNumber().c_str());
  fprintf(outfp, " <FlightDate>%s</FlightDate>\n", cfg.FlightDate().c_str());

  fprintf(outfp, "  <probe id=\"%s\"", cfg.PacketID().c_str());
  fprintf(outfp, " type=\"%s\"", cfg.Type().c_str());
  fprintf(outfp, " resolution=\"%d\"", cfg.Resolution());
  fprintf(outfp, " nDiodes=\"%d\"", cfg.nDiodes());
  fprintf(outfp, " clockFreq=\"%d\"", cfg.ClockFrequency());
  fprintf(outfp, " laserWaveLength=\"%d\"", cfg.WaveLength());
  fprintf(outfp, " serialnumber=\"%s\"", cfg.SerialNumber().c_str());
//  fprintf(outfp, " suffix=\"%s\"/>\n", cfg.Suffix().c_str());

  // Add entry for second array.  2DS is treated as two probes, horizontal and vertical.
  if (cfg.Type().compare("F2DS") == 0)
  {
    fprintf(outfp, " suffix=\"%s\"/>\n", "_2H");
    fprintf(outfp, "  <probe id=\"SV\"");
    fprintf(outfp, " type=\"%s\"", cfg.Type().c_str());
    fprintf(outfp, " resolution=\"%d\"", cfg.Resolution());
    fprintf(outfp, " nDiodes=\"%d\"", cfg.nDiodes());
    fprintf(outfp, " clockFreq=\"%d\"", cfg.ClockFrequency());
    fprintf(outfp, " laserWaveLength=\"%d\"", cfg.WaveLength());
    fprintf(outfp, " serialnumber=\"%s\"", cfg.SerialNumber().c_str());
    fprintf(outfp, " suffix=\"%s\"/>\n", "_2V");
  }
  else
    fprintf(outfp, " suffix=\"%s\"/>\n", "_H1");

  fprintf(outfp, "</OAP>\n");
}


int processArgs(int argc, char *argv[])
{
  int i;

  char *p = strstr(argv[argc-1], ".");
  if (p)	// ...and this should equal HVPS or F2DS
    cfg.SetType(p+1);

  for (i = 1; i < argc && argv[i][0] == '-'; ++i)
  {
    if (strcmp(argv[i], "-v") == 0)
    {
      verbose = true;
    }
    else
    if (strcmp(argv[i], "-project") == 0)
    {
      cfg.SetProjectName(argv[++i]);
    }
    else
    if (strcmp(argv[i], "-platform") == 0)
    {
      cfg.SetPlatform(argv[++i]);
    }
    else
    if (strcmp(argv[i], "-flight") == 0)
    {
      cfg.SetFlightNumber(argv[++i]);
    }
    else
    if (strcmp(argv[i], "-type") == 0)
    {
      cfg.SetType(argv[++i]);
    }
    else
    if (strcmp(argv[i], "-sn") == 0)
    {
      cfg.SetSerialNumber(argv[++i]);
    }
    else
    if (strcmp(argv[i], "-suffix") == 0)
    {
      cfg.SetSuffix(argv[++i]);
    }
    else
    if (strcmp(argv[i], "-offset") == 0)
    {
      cfg.SetTimeOffset(argv[++i]);
    }
    else
    if (strcmp(argv[i], "-o") == 0)
    {
      cfg.SetOutputFilename(argv[++i]);
    }
//    else
//      strcpy(fileName, argv[i]);
  }

  return i;
}


int main(int argc, char *argv[])
{
  FILE *infp, *hkfp = 0, *outfp;
  char fileName[512];
  int indx;

  if (argc == 1)
  {
    fprintf(stderr, "Usage: extract2ds [-v] [-o outfile] input_file(s).F2DS\n");
    fprintf(stderr, "  -project proj_name\tto set project name.\n");
    fprintf(stderr, "  -platform tail_num\tto set platform name.\n");
    fprintf(stderr, "  -flight flight_num\tto set flight number.\n");
    fprintf(stderr, "  -sn serial_num\tto set probe serial number.\n");
    fprintf(stderr, "  -offset seconds\tto add a time offset in seconds.\n");
    fprintf(stderr, "  -o output_file\tto set output file name.\n");
    fprintf(stderr, "  -v for verbose.\n");
    return 1;
  }


  indx = processArgs(argc, argv);

  strcpy(fileName, cfg.OutputFilename().c_str());
  if (strlen(fileName) == 0)
    strcpy(fileName, "extract2ds.2d");
  if ((outfp = fopen(fileName, "w+b")) == 0)
  {
    fprintf(stderr, "Can't open output file %s, fatal.\n", fileName);
    return 1;
  }


  outputXMLheader(outfp);
  putenv((char *)"TZ=UTC");

  for (; indx < argc; ++indx)
  {
    strcpy(fileName, argv[indx]);

    if ((infp = fopen(fileName, "rb")) == 0)
    {
      fprintf(stderr, "Can't open 2DS image file %s, continuing to next file.\n", fileName);
      continue;
    }

    printf("\n\nOpening and processing file [%s]\n", fileName);

    if (cfg.Type() == "F2DS")
    {
      strcat(fileName, "HK");
      if ((hkfp = fopen(fileName, "rb")) == 0)
      {
        fprintf(stderr, "Can't open file %s, continuing without housekeeping (for TAS).\n", fileName);
      }
    }

    processImageFile(infp, hkfp, outfp);

    if (infp) fclose(infp);
    if (hkfp) fclose(hkfp);
  }

  fclose(outfp);

  return 0;
}
