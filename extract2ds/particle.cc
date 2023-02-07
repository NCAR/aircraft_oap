
#include "particle.h"


Particle::Particle() : pos(0)
{
  uncompressed = new char[32768];

}

Particle::~Particle()
{
  delete [] uncompressed;

}

void Particle::writeBuffer(FILE *out)
{
// break this up into 4k chunks and into a P2d

  P2d_rec output;

  while (pos > 0)
  {
    int len = std::min(pos, 4096);
    memset(output.data, 0, 4096);
    memcpy(output.data, uncompressed, len);
    fwrite(&output, sizeof(p2d_rec), 1, out);
    pos -= len;
  }

  // reset buffer.
  pos = 0;

}



void Particle::printParticleHeader(uint16_t *hdr)
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


void Particle::processParticle(uint16_t *wp)
{
  int i, nSlices = wp[4], nWords, sliceCnt = 0, nBits = 0;

  int nBytes, residualClearBits;

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
  }
  else
  {
    nWords = wp[1] & 0x0FFF;
    timingWord = !(wp[1] & 0x1000);
  }

  wp += 5;
  if (timingWord) nWords -= 3;

  for (i = 0; i < nWords; ++i)
  {
    if (wp[i] == 0x4000)	// Fully shadowed slice.
    {
      memset(uncompressed, 0, 16);
      pos += 16;

      if (verbose)
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
      memcpy(&uncompressed[pos], (char *)&wp[i+1], 16);
      pos += 16;

      if (verbose)
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
      memset(&uncompressed[pos], 0xFF, 16);

      ++sliceCnt;
      nBits = 0;
      if (verbose)
        printf("\n  %2d 0x", sliceCnt);
    }

    if ((value = (wp[i] & 0x007F)) > 0)		// Number of clear pixels
    {
      pos += value / 8;
      residualClearBits = value % 8;

      if (verbose)
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
        uncompressed[pos++] = byte;
      }
      else	// extends through the rest of this byte.
      {
        byte <<= shadedBits;
        uncompressed[pos++] = byte;

        int v1 = value - shadedBits;
        nBytes = v1 / 8;
        shadedBits = v1 % 8;
        memset(&uncompressed[pos], 0, nBytes);
        pos += nBytes;
        v1 -= 8 * nBytes;

// Now do the residual bits in the last byte.
        if (v1 > 0)
        {
          byte = 0x7f;
          byte >>= (v1-1);
          uncompressed[pos++] = byte;
        }
      }


      if (verbose)
      {
        for (int j = 0; j < value; ++j)
          printf("1");
      }
      nBits += value;
    }
  }

// Finish Slice needs to just make sure pos is properly incremented to % 16 and put back into h_pos or v_pos
/*
  if (wp[2] != 0)
  {
    v_pos += 16;
  }
  else
  {
    h_pos += 16;
  }
*/
  finishSlice(nBits);

  if (timingWord)
  {
    static unsigned long prevTimeWord = 0;
    unsigned long tWord = ((unsigned long *)&wp[nWords])[0] & 0x0000FFFFFFFFFFFF;

    printf("\n  Timing = %lu, deltaT=%lu\n", tWord, tWord - prevTimeWord);
    prevTimeWord = tWord;

    memset(&uncompressed[pos], 0x05, 8);
    memcpy(&uncompressed[pos+8], (unsigned char*)&wp[nWords], 8);
    pos += 16;
  }
  else
    printf("\n  No timing word\n");

  if (verbose)
    printf(" end - %d/%d words, sliceCnt = %d/%d\n\n", i, nWords, sliceCnt, nSlices);
}

