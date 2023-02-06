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


int moreData(FILE *infp, unsigned char buffer[])
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

  return rc;
}


void printParticleHeader(uint16_t *hdr)
{
  int idx = 1;
  printf("  %x - ID=%6d nSlices=%3d ", hdr[0], hdr[3], hdr[4]);

  if ((hdr[idx] & 0x0FFF) == 0)
    ++idx;

  printf("%c nWords=%4d %s",
        idx == 1 ? 'H' : 'V',
        hdr[idx] & 0x0fff,
        hdr[idx] & 0x1000 ? "- NT" : "");
}


static unsigned char	uncompressed_h[32768];
static unsigned char	uncompressed_v[32768];

static int h_pos = 0;
static int v_pos = 0;


void processParticle(uint16_t *wp)
{
  int i, nSlices = wp[4], nWords, sliceCnt = 0, nBits = 0;

  unsigned char *out_buff = 0;
  int *pos, nBytes, residualClearBits;

  uint16_t value, id = wp[3];
  bool timingWord = true;
  static uint16_t prevID = 0;

  if (verbose)
    printParticleHeader(wp);

  if (id > 0 && id != prevID+1)
    printf("!!! Non sequential praticle ID : prev=%d, this=%d !!!\n", prevID, id);


  prevID = id;

  if (wp[2] != 0)
  {
    nWords = wp[2] & 0x0FFF;
    timingWord = !(wp[2] & 0x1000);
    out_buff = uncompressed_v;
    pos = v_pos;
  }
  else
  {
    nWords = wp[1] & 0x0FFF;
    timingWord = !(wp[1] & 0x1000);
    out_buff = uncompressed_h;
    pos = h_pos;
  }

  wp += 5;
  if (timingWord) nWords -= 3;

  for (i = 0; i < nWords; ++i)
  {
    if (wp[i] == 0x4000)	// Fully shadowed slice.
    {
      memset(out_buff, 0, 16);
      pos += 16;

      if (asciiArt)
      {
        printf("\n  %2d 1 0x", sliceCnt);
        for (int j = 0; j < 128; ++j)
          printf("1");
      }
      nBits = 0;
      ++sliceCnt;
      continue;
    }

    if (wp[i] == 0x7FFF)	// Uncompressed slice.
    {
      memcpy(&out_buff[pos], (char *)&wp[i+1], 16);
      pos += 16;

      if (asciiArt)
      {
        printf("\n  %2d u 0x", sliceCnt);
        for (int j = 0; j < 8; ++j)
          for (int k = 0; k < 16; ++k)
            printf("%d", !((wp[i+1+j] << k) & 0x8000));
      }
      i += 15;
      nBits = 0;
      ++sliceCnt;
      continue;
    }

    if (wp[i] & 0x4000)				// First word of slice
    {
      finishSlice(nBits);

      // Initialize to clear.
      memset(&out_buff[pos], 0xFF, 16);

      ++sliceCnt;
      nBits = 0;
      if (asciiArt)
        printf("\n  %2d 0x", sliceCnt);
    }

    if ((value = (wp[i] & 0x007F)) > 0)		// Number of clear pixels
    {
      pos += value / 8;
      residualClearBits = value % 8;

      if (asciiArt)
      {
        for (int j = 0; j < value; ++j)
          printf("0");
      }
      nBits += value;
    }

    if ((value = (wp[i] & 0x3F80) >> 7) > 0)	// Number of shadowed pixels
    {
      unsigned char byte = 0xff;
      shadedBits = 8 - residualClearBits;

      if (value < shadedBits) // if the whole shaded region resides in this byte
      {
        byte <<= value;
        for (int j = 0; j < shadedBits - value; ++j) {
          byte = (byte << 1) | 0x01;
        }
        out_buff[pos++] = byte;
      }
      else	// extends through the rest of this byte.
      {
        byte <<= shadedBits;
        out_buff[pos++] = byte;

        int v1 = value - shadedBits;
        nBytes = v1 / 8;
        shadedBits = v1 % 8;
        memset(&out_buff[pos], 0, nBytes);
        pos += nBytes;
        v1 -= 8 * nBytes;

// Now do the residual bits in the last byte.
        if (v1 > 0)
        {
          byte = 0x7f;
          byte >>= (v1-1);
          out_buff[pos++] = byte;
        }
      }


      if (asciiArt)
      {
        for (int j = 0; j < value; ++j)
          printf("1");
      }
      nBits += value;
    }
  }

// Finish Slice needs to just make sure pos is properly incremented to % 16 and put back into h_pos or v_pos

  if (wp[2] != 0)
  {
    v_pos += 16;
  }
  else
  {
    h_pos += 16;
  }

  finishSlice(nBits);

  if (timingWord)
  {
    static unsigned long prevTimeWord = 0;
    unsigned long tWord = ((unsigned long *)&wp[nWords])[0] & 0x0000FFFFFFFFFFFF;

    printf("\n  Timing = %lu, deltaT=%lu\n", tWord, tWord - prevTimeWord);
    prevTimeWord = tWord;

    memset(&out_buff[pos], 0x05, 8);
    memcpy(&out_buff[pos+8], (unsigned char*)&wp[nWords], 8);
    pos += 16;
  }
  else
    printf("\n  No timing word\n");

  if (verbose)
    printf(" end - %d/%d words, sliceCnt = %d/%d\n\n", i, nWords, sliceCnt, nSlices);
}



void processImageFile(FILE *infp, FILE *hkfp, FILE *outfp)
{
  static unsigned char	buffer[8192];
  static uint16_t	particle[8192];
  int	imCnt = 0, nlCnt = 0, partialPos = 0;

  struct imageBuf *tds = (struct imageBuf *)buffer;
  uint16_t *wp = ((uint16_t *)tds->rdf);
  OAP::P2d_rec outBuf;

  printf("processImageFile\n");

  while (moreData(infp, buffer) == 1)
  {
    int j = 0;

    outBuf.year = htons(tds->year);
    outBuf.month = htons(tds->month);
    outBuf.day = htons(tds->day);
    outBuf.hour = htons(tds->hour);
    outBuf.minute = htons(tds->minute);
    outBuf.second = htons(tds->second);
    outBuf.msec = htons(tds->msecond);

    outBuf.tas = findHouseKeeping(hkfp, tds);

    if (cfg.StoreCompressed())
    {
      memcpy(outBuf.data, tds->rdf, 4096);
      fwrite(&outBuf, sizeof(outBuf), 1, outfp);
      continue;
    }


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
          moreData(infp, buffer);
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
          moreData(infp, buffer);
          j = 0;
          n -= partialPos;
        }

//printf("    %d %d %d\n", partialPos, j, n);
        memcpy(&particle[5+partialPos], &wp[j], n * sizeof(uint16_t));
        j += (n-1);


        // OK, we can process particle
        imCnt++;
        processParticle((uint16_t *)particle);
      }
    }

  }

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
  fprintf(outfp, " suffix=\"%s\"/>\n", cfg.Suffix().c_str());

  if (cfg.Type().compare("F2DS") == 0)
  {
    fprintf(outfp, " <probe id=\"SV\"");
    fprintf(outfp, " type=\"%s\"", cfg.Type().c_str());
    fprintf(outfp, " resolution=\"%d\"", cfg.Resolution());
    fprintf(outfp, " nDiodes=\"%d\"", cfg.nDiodes());
    fprintf(outfp, " clockFreq=\"%d\"", cfg.ClockFrequency());
    fprintf(outfp, " laserWaveLength=\"%d\"", cfg.WaveLength());
    fprintf(outfp, " serialnumber=\"%s\"", cfg.SerialNumber().c_str());
    fprintf(outfp, " suffix=\"%s\"/>\n", cfg.Suffix().c_str());
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
