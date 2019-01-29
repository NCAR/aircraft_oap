/*
-------------------------------------------------------------------------
OBJECT NAME:	Histogram.cc

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2018
-------------------------------------------------------------------------
*/

#include "Histogram.h"
#include <netinet/in.h>


/* -------------------------------------------------------------------- */
Histogram::Histogram(const Widget parent) : TextWindow(parent, "histogram")
{
}

/* -------------------------------------------------------------------- */
void Histogram::AddLineItem(OAP::P2d_rec * record, struct OAP::recStats & stats)
{
  char buffer[2000];

  sprintf(buffer, "%c%c %02d:%02d:%02d.%03d  ",
        ((char *)&record->id)[0], ((char *)&record->id)[1],
        record->hour, record->minute, record->second, record->msec
	);
  Append(buffer);

  size_t n = ((char *)&record->id)[1] > 51 ? 64 : 32;

  for (size_t i = 1; i <= n; ++i)
  {
    sprintf(buffer, "%4d", stats.accum[i]);
    Append(buffer);
  }
  sprintf(buffer, ", total=%6d, accepted=%6d\n", stats.nTimeBars, stats.nonRejectParticles);
  Append(buffer);
}
