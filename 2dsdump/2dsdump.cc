
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>

const char hkTitle[] = " |  --------  Outside temperatures  --------   |           DSP_T   |  ----- Inside temperatures -----  |      -5      +5       RH   CanP   +7raw       |      ---------------   Diode Voltages   ---------------      |    Total Counts  TAS(m/s)";

struct recHdr
{
  int16_t	year, month, dow, day, hour, minute, second, msecond;
};

struct imageBuf
{
  int16_t	year, month, dow, day, hour, minute, second, msecond;
  char		rdf[4096];
  uint16_t	cksum;
};

struct hkBuf
{
  int16_t	year, month, dow, day, hour, minute, second, msecond;
  char		rdf[164];
  uint16_t	cksum;
};

struct maskBuf
{
  int16_t	year, month, dow, day, hour, minute, second, msecond;
  char		rdf[54];
  uint16_t	cksum;
};


// This is the index into the particle packet.
enum PKT_IDX { H_CHN=1, V_CHN=2, PID=3, nSLICES=4 };

const uint16_t SyncWord = 0x3253;	// Particle sync word.
const uint16_t FlushWord = 0x4e4c;	// NL Flush Buffer.

// Options
bool verbose = false;
bool timeOnly = false;
bool asciiArt = false;
char fileName[512];
int  recordCnt = 0, totalParticleCnt = 0, totalSliceCnt = 0;


/* ------------------------------------------------------------------------ */
bool cksum(const uint16_t buff[], int nWords, uint16_t ckSum)
{
  uint16_t sum = 0;

  for (int i = 0; i < nWords; ++i)
    sum += buff[i];

  if (sum != ckSum)
    printf("Checksum mis-match %u %u\n", sum, ckSum);

  return sum == ckSum;
}

/* ------------------------------------------------------------------------ */
float diffTimeStamps(struct recHdr &prevHdr, struct recHdr *thisHdr)
{
  struct tm tmp;
  double tpf, ttf;

  memset(&tmp, 0, sizeof(struct tm));
  tmp.tm_year = prevHdr.year - 1900;
  tmp.tm_mon = prevHdr.month-1;
  tmp.tm_mday = prevHdr.day;
  tmp.tm_hour = prevHdr.hour;
  tmp.tm_min = prevHdr.minute;
  tmp.tm_sec = prevHdr.second;
  tpf = (double)mktime(&tmp) + (double)prevHdr.msecond / 1000.0;

  memset(&tmp, 0, sizeof(struct tm));
  tmp.tm_year = thisHdr->year - 1900;
  tmp.tm_mon = thisHdr->month-1;
  tmp.tm_mday = thisHdr->day;
  tmp.tm_hour = thisHdr->hour;
  tmp.tm_min = thisHdr->minute;
  tmp.tm_sec = thisHdr->second;
  ttf = (double)mktime(&tmp) + (double)thisHdr->msecond / 1000.0;

  return(ttf-tpf);
}

/* ------------------------------------------------------------------------ */
int moreData(FILE *infp, unsigned char buffer[])
{
  static struct recHdr prevHdr;

  if (verbose)
    printf("  moreData\n");

  memcpy((void *)&prevHdr, buffer, sizeof(struct recHdr));
  int rc = fread(buffer, 4114, 1, infp);

  if (rc != 1)
    return rc;

  ++recordCnt;
  struct imageBuf *tds = (struct imageBuf *)buffer;
  uint16_t *wp = (uint16_t *)tds->rdf;
  int	pCnt = 0, nSlices = 0;

  for (int i = 0; i < 2043; ++i)
    if (wp[i] == SyncWord && (wp[i+H_CHN] == 0 || wp[i+V_CHN] == 0))
    {
      ++pCnt;
      nSlices += ((uint16_t *)tds->rdf)[i+nSLICES];
    }

  totalSliceCnt += nSlices;
  totalParticleCnt += pCnt;

  if (verbose || timeOnly)
    printf("%d/%d/%d %02d:%02d:%02d.%03d dt=%6.3f - 0x%04x - particle cnt=%3d, uncompressedSlices=%4d\n",
	tds->year, tds->month, tds->day, tds->hour,
	tds->minute, tds->second, tds->msecond, diffTimeStamps(prevHdr, (struct recHdr *)buffer),
	((uint16_t *)tds->rdf)[0], pCnt, nSlices);

  cksum((uint16_t *)tds->rdf, 2048, tds->cksum);

  return rc;
}

/* ------------------------------------------------------------------------ */
void finishSlice(int nBits)
{
  if (asciiArt)
  {
    if (nBits > 0)
      for (int j = nBits; j < 128; ++j)
        printf("0");
  }
}

/* ------------------------------------------------------------------------ */
void printParticleHeader(uint16_t *hdr, int idx)
{
  printf("  %x - ID=%6d nSlices=%3d ", hdr[0], hdr[PID], hdr[nSLICES]);

  printf("%c nWords=%4d %s",
	idx == H_CHN ? 'H' : 'V',
	hdr[idx] & 0x0fff,
	hdr[idx] & 0x1000 ? "- NT" : "");
}

/* ------------------------------------------------------------------------ */
void processParticle(uint16_t *wp)
{
  int i, nSlices = wp[nSLICES], nWords, sliceCnt = 0, nBits = 0;
  uint16_t value, id = wp[PID];
  bool timingWord = true;
  static uint16_t prevID[2] = { 0, 0 };
  int idx = 1;

  if ((wp[idx] & 0x0FFF) == 0)	// this is H vs V
    ++idx;

  printParticleHeader(wp, idx);

  if (id > 0)
  {
    if (id == prevID[idx-1])
      printf(" : multi packet particle\n");
    else
    if (id != prevID[idx-1]+1)
      printf("!!! Non sequential particle ID : prev=%d, this=%d !!!\n", prevID[idx-1], id);
  }

  prevID[idx-1] = id;

  if (wp[V_CHN] != 0)
  {
    nWords = wp[V_CHN] & 0x0FFF;
    timingWord = !(wp[V_CHN] & 0x1000);
  }
  else
  {
    nWords = wp[H_CHN] & 0x0FFF;
    timingWord = !(wp[H_CHN] & 0x1000);
  }

  wp += 5;
  if (timingWord) nWords -= 3;

  for (i = 0; i < nWords; ++i)
  {
    if (wp[i] == 0x4000)	// Fully shadowed slice.
    {
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
      if (verbose)
        printf("                 -- uncomressed slice -- \n");

      if (asciiArt)
      {
        printf("\n  %2d u 0x", sliceCnt);
        for (int j = 0; j < 8; ++j)
          for (int k = 0; k < 16; ++k)
            printf("%d", !((wp[i+1+j] << k) & 0x8000));
      }
      i += 7;
      nBits = 0;
      ++sliceCnt;
      continue;
    }

    if (wp[i] & 0x4000)		// First word of slice
    {
      finishSlice(nBits);

      ++sliceCnt;
      nBits = 0;
      if (asciiArt)
        printf("\n  %2d 0x", sliceCnt);
    }

    if ((value = (wp[i] & 0x007F)) > 0)		// Number of clear pixels
    {
      if (asciiArt)
      {
        for (int j = 0; j < value; ++j)
          printf("0");
      }
      nBits += value;
    }

    if ((value = (wp[i] & 0x3F80) >> 7) > 0)	// Number of shadowed pixels
    {
      if (asciiArt)
      {
        for (int j = 0; j < value; ++j)
          printf("1");
      }
      nBits += value;
    }
  }

  finishSlice(nBits);

  if (timingWord)
  {
    static unsigned long prevTimeWord = 0;
    unsigned long tWord = ((unsigned long *)&wp[nWords])[0] & 0x0000FFFFFFFFFFFF;

    printf("\n  Timing = %lu, deltaT=%lu\t - %.6fsec\n", tWord, tWord - prevTimeWord, (float)(tWord-prevTimeWord)/20000000);
    prevTimeWord = tWord;
  }
  else
    printf("\n  No timing word\n");

  printf(" end - %d/%d words, sliceCnt = %d/%d\n\n", i, nWords, sliceCnt, nSlices);
}

/* ------------------------------------------------------------------------ */
void processImageFile(FILE *infp)
{
  static unsigned char	buffer[8192];
  static uint16_t	particle[4096];
  int	imCnt = 0, tsCnt = 0, nlCnt = 0, partialPos = 0;

  struct imageBuf *tds = (struct imageBuf *)buffer;
  uint16_t *wp = (uint16_t *)tds->rdf;

  printf("processImageFile\n");

  while (moreData(infp, buffer) == 1)
  {
    int j = 0;

    if (timeOnly)
    {
      if (wp[0] == FlushWord)		// NL flush buffer
        nlCnt++;

      for (; j < 2043; ++j)
        if (wp[j] == SyncWord && (wp[j+1] == 0 || wp[j+2] == 0))		// start particle
        {
          ++imCnt;
          tsCnt += wp[j+4];
        }

      continue;
    }

    for (; j < 2048; ++j)
    {
      if (wp[j] == FlushWord)		// NL flush buffer
      {
//        printf("%x - %d %d %d %d\n", wp[j], wp[j+1], wp[j+2], wp[j+3], wp[j+4]);
        if (verbose)
          printf("NL Flush, pos = %d\n", j);
        nlCnt++;
        break;
      }


      if (wp[j] == SyncWord)		// start particle
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
        tsCnt += particle[4];
        processParticle((uint16_t *)particle);
      }
    }
  }

  printf("Record cnt = %d, nullCnt=%d\n", recordCnt, nlCnt);
  printf("   moreData: particle Cnt=%8d slice Cnt=%8d\n", totalParticleCnt, totalSliceCnt);
  printf("   process : particle Cnt=%8d slice Cnt=%8d\n", imCnt, tsCnt);
}

/* ------------------------------------------------------------------------ */
void processHouseKeepingFile(FILE *infp)
{
  char	buffer[1024];
  int	hkCnt = 0, mkCnt = 0, oCnt = 0;
  uint32_t tas;
  float *tasf = (float *)&tas;

  struct hkBuf *hkb = (struct hkBuf *)buffer;
  struct maskBuf *mkb = (struct maskBuf *)buffer;

  printf("processHouseKeepingFile\n");

  printf("%s\n", hkTitle);

  while (fread(buffer, 18, 1, infp) == 1)
  {
    if (verbose)
      printf("%d/%02d/%02d %02d:%02d:%02d.%03d - 0x%04x len=%d\n", hkb->year,
	hkb->month, hkb->day, hkb->hour, hkb->minute, hkb->second, hkb->msecond,
	((uint16_t *)&hkb->rdf)[0], ((uint16_t *)&hkb->rdf)[1]);

    switch (((uint16_t *)&hkb->rdf)[0])
    {
      case 0x484b:		// HK buffer
        fread(&buffer[18], 164, 1, infp);
        cksum((uint16_t *)hkb->rdf, 82, hkb->cksum);
        for (int i = 9; i < 22; ++i)
          printf("  %6.1f", (double)((int16_t *)&hkb->rdf)[i] * 0.07323 - 64.8);
        for (int i = 22; i < 24; ++i)
          printf("  %6.1f", (double)((int16_t *)&hkb->rdf)[i] * 0.0048828);		// +- 5Vdc
        printf(" - %6.1f", (double)((int16_t *)&hkb->rdf)[28] * 0.077547 - 26.24);	// RH in %
        printf(" %6.1f", (double)((int16_t *)&hkb->rdf)[29] * 0.018356 - 3.75);		// Canister pressure in psi
        printf(" %6.1f  - V", (double)((int16_t *)&hkb->rdf)[34] * 0.00488);
//        printf(" %6.1f  - V", (double)((int16_t *)&hkb->rdf)[34] * 0.000152588);
        for (int i = 36; i < 50; ++i)
          if (i != 43) // blank
            printf(" %4.1f", (double)((int16_t *)&hkb->rdf)[i] * 0.00244140625);
          else
            printf(" H");

        printf(" - %6d %6d", ((uint16_t *)&hkb->rdf)[57], ((uint16_t *)&hkb->rdf)[58]);
        tas = (uint32_t)((uint16_t *)&hkb->rdf)[75] << 16 | ((uint16_t *)&hkb->rdf)[76];
        printf("  %5.1f\n", *tasf);
        hkCnt++;
        break;

      case 0x4d4b:		// Mask buffer
        fread(&buffer[18], 54, 1, infp);
        cksum((uint16_t *)mkb->rdf, 27, mkb->cksum);
        mkCnt++;
        break;

      default:
        fread(&buffer[18], 2, 1, infp);
        printf(" no match %d/%d/%d %02d:%02d:%02d.%03d - %x\n", hkb->year,
		hkb->month, hkb->day, hkb->hour, hkb->minute, hkb->second, hkb->msecond,
		((uint16_t *)&hkb->rdf)[0]);
        oCnt++;
    }
  }

  printf("hkCnt=%d maskCnt=%d otherCnt=%d\n", hkCnt, mkCnt, oCnt);
}

/* ------------------------------------------------------------------------ */
void processArgs(int argc, char *argv[])
{

  for (int i = 1; i < argc; ++i)
  {
    if (strcmp(argv[i], "-v") == 0)
    {
      verbose = true;
    }
    else
    if (strcmp(argv[i], "-t") == 0)
    {
      timeOnly = true;
    }
    else
    if (strcmp(argv[i], "-ascii") == 0)
    {
      asciiArt = true;
    }
    else
      strcpy(fileName, argv[i]);
  }

}


/* ------------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
  FILE *infp;

  if (argc == 1)
  {
    fprintf(stderr, "Usage: 2dsdump [-v] [-t] [-ascii] file.F2DS[HK]\n");
    fprintf(stderr, "  -t for timestamps only.\n");
    fprintf(stderr, "  -v for verbose.\n");
    fprintf(stderr, "  -ascii for ascii art dump.\n");
    return 1;
  }

  processArgs(argc, argv);

  if ((infp = fopen(fileName, "rb")) == 0)
  {
    fprintf(stderr, "Can't open %s.\n", fileName);
    return(1);
  }

  putenv((char *)"TZ=UTC");

  if (strstr(fileName, "F2DSHK") )
    processHouseKeepingFile(infp);
  else
    processImageFile(infp);

  return 0;
}
