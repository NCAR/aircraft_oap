/*
-------------------------------------------------------------------------
OBJECT NAME:	global.cc

FULL NAME:	Global Variable Definitions

DESCRIPTION:	extern.h should look just like this.
-------------------------------------------------------------------------
*/

#include "define.h"

bool	DataChanged;	/* New data was read from file.			*/
bool	UTCseconds;	/* Seconds since midnight vs. HH:MM:SS		*/

char	DataPath[PATH_LEN], pngPath[PATH_LEN], psPath[PATH_LEN];


/* Time stuff */
int	UserStartTime[4], UserEndTime[4];	/* HH:MM:SS, 4th is seconds*/
int	MinStartTime[4], MaxEndTime[4];		/* since midnight.	*/


/* -------------------------------------------------------------------- */
long long ntohll(long long *p)
{
  union {
    long long v;
    char b[8];
  } u;
  const char* cp = (const char*)p;
  for (int i = 7; i >= 0; i--) u.b[i] = *cp++;
  return u.v;
}

/* END GLOBAL.CC */
