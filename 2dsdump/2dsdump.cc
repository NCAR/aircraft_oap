
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>


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


bool verbose = false;
bool asciiArt = false;


bool cksum(const uint16_t buff[], int nWords, uint16_t ckSum)
{
  uint16_t sum = 0;

  for (int i = 0; i < nWords; ++i)
    sum += buff[i];

  if (sum != ckSum)
    printf("Checksum mis-match %u %u\n", sum, ckSum);

  return sum == ckSum;
}


int moreData(FILE *infp, char buffer[])
{
  if (verbose)
    printf("  moreData\n");

  int rc = fread(buffer, 4114, 1, infp);

  if (rc != 1)
    return rc;

  struct imageBuf *tds = (struct imageBuf *)buffer;

  if (verbose)
    printf("%d/%d/%d %02d:%02d:%02d.%03d - %x\n", tds->year, tds->month,
	tds->day, tds->hour, tds->minute, tds->second, tds->msecond,
	((uint16_t *)tds->rdf)[0]);

  cksum((uint16_t *)tds->rdf, 2048, tds->cksum);

  return rc;
}


void finishSlice(int nBits)
{
  if (asciiArt)
  {
    if (nBits > 0)
      for (int j = nBits; j < 128; ++j)
        printf("0");
  }
}


void processParticle(uint16_t *wp)
{
  int i, nSlices = wp[4], nWords, sliceCnt = 0, nBits = 0;
  uint16_t value;
  bool timingWord = true;

  printf("%x - ID=%6d nSlices=%3d ", wp[0], wp[3], nSlices);

  if (wp[2] != 0)
  {
    nWords = wp[2] & 0x0FFF;
    timingWord = !(wp[2] & 0x1000);
    printf("H nWords=%4d - %s\n", nWords, timingWord ? "" : "NT");
  }
  else
  {
    nWords = wp[1] & 0x0FFF;
    timingWord = !(wp[1] & 0x1000);
    printf("V nWords=%4d - %s\n", nWords, timingWord ? "" : "NT");
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
            printf("%d", !((wp[1+j] << k) & 0x8000));
      }
      i += 15;
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

    printf("\n Timing = %lu, deltaT=%lu\n", tWord, tWord - prevTimeWord);
    prevTimeWord = tWord;
  }
  else
    printf("\n No timing word\n");

  printf(" end - %d/%d words, sliceCnt = %d/%d\n", i, nWords, sliceCnt, nSlices);
}


void processImageFile(FILE *infp)
{
  static char	buffer[8192], particle[8192];
  int	imCnt = 0, nlCnt = 0, oCnt = 0, partialPos = 0;

  struct imageBuf *tds = (struct imageBuf *)buffer;
  uint16_t *wp = ((uint16_t *)tds->rdf);

  printf("processImageFile\n");

  while (moreData(infp, buffer) == 1)
  {
    int j = 0;

    if (partialPos > 0)	// remainder of last particle from last record
    {
      for (j = 0; j < 2048; ++j)
      {
        if (wp[j] == 0x3253)	// we have end particle
          break;
      }
printf(" copy second part of partical started last record, partilaPos=%d %d\n", partialPos, j);
      memcpy(&particle[partialPos*sizeof(uint16_t)], &wp[0], (j+1) * sizeof(uint16_t));
      processParticle((uint16_t *)particle);
      partialPos = 0;
    }

    for (; j < 2048; ++j)
    {
      if (wp[j] == 0x4e4c)		// NL flush buffer
      {
//        printf("%x - %d %d %d %d\n", wp[j], wp[j+1], wp[j+2], wp[j+3], wp[j+4]);
        if (verbose)
          printf("NL Flush, pos = %d\n", j);
        nlCnt++;
        continue;
      }


      if (wp[j] == 0x3253)		// start particle
      {
        int k;

        partialPos = 0;

        // Search for next particle (we want end of this particle).
        for (k = j+1; k < 2048; ++k)
        {
          if (wp[k] == 0x3253)	// we have end particle
            break;
        }


        // This was a partial particle, break out for more data.
        if (k == 2048)
        {
          partialPos = k - j;
          memcpy(particle, &wp[j], (k-j) * sizeof(uint16_t));
          break;	// end of record, break out for more data.
        }

        memcpy(particle, &wp[j], (k-j+1) * sizeof(uint16_t));


        // OK, we can process particle
        imCnt++;
        processParticle((uint16_t *)particle);

        j = k - 1;
      }
    }
  }

  printf("2Scnt=%d nlCnt=%d otherCnt=%d\n", imCnt, nlCnt, oCnt);
}


void processHouseKeepingFile(FILE *infp)
{
  char	buffer[1024];
  int	hkCnt = 0, mkCnt = 0, oCnt = 0;

  struct hkBuf *hkb = (struct hkBuf *)buffer;
  struct maskBuf *mkb = (struct maskBuf *)buffer;

  printf("processHouseKeepingFile\n");

  while (fread(buffer, 18, 1, infp) == 1)
  {
    if (verbose)
      printf("%d/%d/%d %02d:%02d:%02d.%03d - %x\n", hkb->year,
	hkb->month, hkb->day, hkb->hour, hkb->minute, hkb->second, hkb->msecond,
	((uint16_t *)&hkb->rdf)[0]);

    switch (((uint16_t *)&hkb->rdf)[0])
    {
      case 0x484b:		// HK buffer
        fread(&buffer[18], 164, 1, infp);
        cksum((uint16_t *)hkb->rdf, 82, hkb->cksum);
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


int main(int argc, char *argv[])
{
  int	argCnt = 1;

  FILE *infp;

  if (argc == 1)
  {
    fprintf(stderr, "Usage: 2dsdump [-v] [-ascii] file.F2DS[HK]\n");
    fprintf(stderr, "  Use -ascii for ascii art dump.\n");
  }

  if (strcmp(argv[argCnt], "-v") == 0)
  {
    verbose = true;
    argCnt++;
  }

  if (strcmp(argv[argCnt], "-ascii") == 0)
  {
    asciiArt = true;
    argCnt++;
  }

  if ((infp = fopen(argv[argCnt], "rb")) == 0)
  {
    fprintf(stderr, "Can't open %s.\n", argv[argCnt]);
    return(1);
  }

  if (strstr(argv[argCnt], "F2DSHK") )
    processHouseKeepingFile(infp);
  else
    processImageFile(infp);

  return(0);
}
