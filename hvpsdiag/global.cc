/*
-------------------------------------------------------------------------
OBJECT NAME:	global.c

FULL NAME:	Global Variable Definitions

DESCRIPTION:	extern.h should look just like this.
-------------------------------------------------------------------------
*/

#include "define.h"

bool	Grid,				/* Overlay grid on graph?	*/
	Frozen,
	FlashMode;

char	buffer[BUFFSIZE], DataPath[1024], *ProjectName, FlightNumber[32];


/* Vars related to data sets	*/
int	NumberDataSets, NumberSeconds, CurrentDataSet, Aircraft;

/* Data record pointers
 */
double	*plotData, *xData;

float	HDRversion;

int	FlightDate[3];	// Dummy.  Here to shut linker up.

/* END GLOBAL.CC */
