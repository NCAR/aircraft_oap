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

int findHouseKeeping(FILE *hkfp, imageBuf *imgRec);



bool cksum(const uint16_t buff[], int nWords, uint16_t ckSum)
{
  uint16_t sum = 0;

  for (int i = 0; i < nWords; ++i)
    sum += buff[i];

  if (sum != ckSum)
    printf("Checksum mis-match %u %u\n", sum, ckSum);

  return sum == ckSum;
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
  int pCnt = 0;

  for (int i = 0; i < 2048; ++i)
    if (((uint16_t *)tds->rdf)[i] == 0x3253)
      ++pCnt;

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

  oapHdr.tas = htons(findHouseKeeping(hkfp, tds));
  oapHdr.overld = 0;

  return rc;
}


void processImageFile(FILE *infp, FILE *hkfp, FILE *outfp)
{
  static unsigned char	buffer[8192];
  static uint16_t	particle[8192];
  int	imCnt = 0, nlCnt = 0, partialPos = 0;

  Particle *probe[2];
  probe[0] = new Particle("SH", outfp);
  probe[1] = new Particle("SV", outfp);

  struct imageBuf *tds = (struct imageBuf *)buffer;
  uint16_t	*wp = ((uint16_t *)tds->rdf);
  OAP::P2d_hdr	outHdr;

  printf("processImageFile\n");

  while (moreData(infp, buffer, outHdr, hkfp) == 1)
  {
    int j = 0;

    if (cfg.StoreCompressed())
    {
      OAP::P2d_rec outBuf;
      memcpy(&outBuf, &outHdr, sizeof(OAP::P2d_hdr));
      memcpy(outBuf.data, tds->rdf, 4096);
      fwrite(&outBuf, sizeof(outBuf), 1, outfp);
      continue;
    }

    probe[0]->setHeader(outHdr);
    probe[1]->setHeader(outHdr);

    for (; j < 2048; ++j)
    {
      if (wp[j] == 0x4e4c)		// NL flush buffer
      {
//        printf("%x - %d %d %d %d\n", wp[j], wp[j+1], wp[j+2], wp[j+3], wp[j+4]);
        if (verbose)
          printf("NL Flush, pos = %d\n", j);
        nlCnt++;
        break;
      }


      if (wp[j] == 0x3253)		// start particle
      {
        if (verbose)
          printf(" start particle, pos=%d\n", j);

        partialPos = 0;
        if (j > 2043)	// want particle 5 byte header
        {
          if (verbose) printf(" short header, j=%d\n", j);
          partialPos = 2048-j;
          memcpy(particle, &wp[j], partialPos * sizeof(uint16_t));
          if (moreData(infp, buffer, outHdr, hkfp) != 1) break;
          probe[0]->setHeader(outHdr);
          probe[1]->setHeader(outHdr);
          j = 0;
        }

        memcpy(&particle[partialPos], &wp[j], (5-partialPos) * sizeof(uint16_t));
        uint16_t nh = particle[1] & 0x0FFF;
        uint16_t nv = particle[2] & 0x0FFF;
        if (nh > 0 && nv > 0)	// One of these must be zero.
          continue;

        int n = std::max(nh, nv);
//printf("    %d %d %d\n", partialPos, j, n);
        j += (5-partialPos);

        partialPos = 0;
        if (j + n > 2048)
        {
          if (verbose) printf(" short image, j=%d, n=%d\n", j, n);
          partialPos = 2048-j;
          memcpy(&particle[5], &wp[j], partialPos * sizeof(uint16_t));
          if (moreData(infp, buffer, outHdr, hkfp) != 1) break;
          probe[0]->setHeader(outHdr);
          probe[1]->setHeader(outHdr);
          j = 0;
          n -= partialPos;
        }

//printf("    %d %d %d\n", partialPos, j, n);
        memcpy(&particle[5+partialPos], &wp[j], n * sizeof(uint16_t));
        j += (n-1);


        // OK, we can process particle
        imCnt++;
fflush(stdout);
        if (particle[4] < 256)
        {
          if (nh)
            probe[0]->processParticle((uint16_t *)particle, verbose);
          else
            probe[1]->processParticle((uint16_t *)particle, verbose);
        }
        else
        {
          if (verbose) printf(" not processing particle # %d - n=%d\n", particle[3], particle[4]);
        }
      }
    }

  }

  delete probe[0];
  delete probe[1];
  printf("Record cnt = %d, particle Cnt=%d nullCnt=%d\n", recordCnt, imCnt, nlCnt);
}


int findHouseKeeping(FILE *hkfp, imageBuf *imgRec)
{
  char	buffer[1024];
  uint32_t tas;
  float *tasf = (float *)&tas;

  struct hkBuf *hkb = (struct hkBuf *)buffer;
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

  printf("findHouseKeeping: we shouldn't hit this.  EOF?\n");

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

  fprintf(outfp, " <probe id=\"%s\"", cfg.PacketID().c_str());
  fprintf(outfp, " type=\"%s\"", cfg.Type().c_str());
  fprintf(outfp, " resolution=\"%d\"", cfg.Resolution());
  fprintf(outfp, " nDiodes=\"%d\"", cfg.nDiodes());
  fprintf(outfp, " clockFreq=\"%d\"", cfg.ClockFrequency());
  fprintf(outfp, " laserWaveLength=\"%d\"", cfg.WaveLength());
  fprintf(outfp, " serialnumber=\"%s\"", cfg.SerialNumber().c_str());
//  fprintf(outfp, " suffix=\"%s\"/>\n", cfg.Suffix().c_str());
  fprintf(outfp, " suffix=\"%s\"/>\n", "_2H");

  // Add entry for second array.  2DS is treated as two probes, horizontal and vertical.
  if (cfg.Type().compare("F2DS") == 0)
  {
    fprintf(outfp, " <probe id=\"SV\"");
    fprintf(outfp, " type=\"%s\"", cfg.Type().c_str());
    fprintf(outfp, " resolution=\"%d\"", cfg.Resolution());
    fprintf(outfp, " nDiodes=\"%d\"", cfg.nDiodes());
    fprintf(outfp, " clockFreq=\"%d\"", cfg.ClockFrequency());
    fprintf(outfp, " laserWaveLength=\"%d\"", cfg.WaveLength());
    fprintf(outfp, " serialnumber=\"%s\"", cfg.SerialNumber().c_str());
    fprintf(outfp, " suffix=\"%s\"/>\n", "_2V");
  }

  fprintf(outfp, "</OAP>\n");
}


int processArgs(int argc, char *argv[])
{
  int i;

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
  FILE *infp, *hkfp, *outfp;
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

  for (; indx < argc; ++indx)
  {
    strcpy(fileName, argv[indx]);

    if ((infp = fopen(fileName, "rb")) == 0)
    {
      fprintf(stderr, "Can't open 2DS image file %s, continuing to next file.\n", fileName);
      continue;
    }

    printf("\n\nOpening and processing file [%s]\n", fileName);

    strcat(fileName, "HK");
    if ((hkfp = fopen(fileName, "rb")) == 0)
    {
      fprintf(stderr, "Can't open file %s, continuing without housekeeping (for TAS).\n", fileName);
//      fclose(infp);
//      continue;
    }

    processImageFile(infp, hkfp, outfp);

    if (infp) fclose(infp);
    if (hkfp) fclose(hkfp);
  }

  fclose(outfp);

  return 0;
}
