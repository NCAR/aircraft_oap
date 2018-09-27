/*
-------------------------------------------------------------------------
OBJECT NAME:	Hex.cc

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2018
-------------------------------------------------------------------------
*/

#include "Hex.h"
#include "FileMgr.h"

extern FileManager      fileMgr;

/* -------------------------------------------------------------------- */
Hex::Hex(const Widget parent) : TextWindow(parent, "hex")
{

}	/* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void Hex::Update(size_t nBuffs, P2d_rec sets[])
{
  char buffer[1024];

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
  const ProbeList & probes = fileMgr.CurrentFile()->Probes();
  ProbeList::const_iterator iter;
  /* Display records in column format.  1024 is max slices per buffer
   * (old 32 diode 2d probe).
   */
  for (size_t i = 0; i < 1024; ++i)
  {
    sprintf(buffer, "%4zu  ", i);

    for (size_t j = 0; j < nBuffs; ++j)
    {
      size_t nSlices = probes.find(sets[j].id)->second->nSlices();
      size_t nBytes = P2D_DATA / nSlices;

      if (i < nSlices)
      {
        for (size_t k = 0; k < nBytes; ++k)
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
