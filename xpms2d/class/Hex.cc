/*
-------------------------------------------------------------------------
OBJECT NAME:	Hex.cc

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2009
-------------------------------------------------------------------------
*/

#include "Hex.h"

/* -------------------------------------------------------------------- */
Hex::Hex(const Widget parent) : TextWindow(parent, "hex")
{

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void Hex::Update(size_t nBuffs, P2d_rec sets[])
{
  Clear();

  /* Title */
  strcpy(buffer, "      ");
  for (size_t j = 0; j < nBuffs; ++j)
  {
    const char *p = (char *)&sets[j].id;

    if (((char *)&sets[j].id)[1] >= 'A')	// 128 diode (2DS)
      sprintf(&buffer[strlen(buffer)], "               %c%c                ", p[0], p[1]);
    else
    if (((char *)&sets[j].id)[1] >= '4')	// 64 diode data, hex long-long
      sprintf(&buffer[strlen(buffer)], "       %c%c        ", p[0], p[1]);
    else					// 32 diode data, hex long.
      sprintf(&buffer[strlen(buffer)], "   %c%c    ", p[0], p[1]);
  }

  strcat(buffer, "\n");
  Append(buffer);


  /* Records */
  for (size_t i = 0; i < nSlices_32bit; ++i)
  {
    int nBytes;
    size_t nSlices;

    sprintf(buffer, "%4zu  ", i);

    for (size_t j = 0; j < nBuffs; ++j)
    {
      if (((char *)&sets[j].id)[1] >= 'A')	// 128 diode (2DS)
      {
        nSlices = nSlices_128bit;
        nBytes = 16;
      }
      else
      if (((char *)&sets[j].id)[1] >= '4')	// 64 diode data, hex long-long
      {
        nSlices = nSlices_64bit;
        nBytes = 8;
      }
      else					// 32 diode data, hex long.
      {
        nSlices = nSlices_32bit;
        nBytes = 4;
      }

      if (i < nSlices)
      {
        for (int k = nBytes-1; k >= 0; --k)
          sprintf(&buffer[strlen(buffer)], "%02X",
		sets[j].data[(i*nBytes)+k]);
          strcat(buffer, " ");
      }
    }

    strcat(buffer, "\n");
    Append(buffer);
  }

  MoveTo(0);

}	/* END UPDATE */

/* END HEX.CC */
