/*
-------------------------------------------------------------------------
OBJECT NAME:	init.c

FULL NAME:	Initialize  Procedures

TYPE:		X11/R5 application, motif

DESCRIPTION:	Inializiation routines run once at the beginning of main
		Initialize is used to set all global variables.
		Process_args does just that.

AUTHOR:		websterc@ncar.ucar.edu
-------------------------------------------------------------------------
*/

#include "define.h"
#include <unistd.h>


/* --------------------------------------------------------------------- */
void Initialize()
{
  FlashMode = Grid = Frozen = false;
  NumberSeconds = 10;

}	/* END INITIALIZE */

/* --------------------------------------------------------------------- */
void process_args(char **argv)
{
  while (*++argv)
    if ((*argv)[0] == '-')
      switch ((*argv)[1])
        {
        case 's':   /* Number of seconds	*/
          if (*++argv == NULL)
            return;

          NumberSeconds = atoi(*argv);
          break;

        default:
          fprintf(stderr, "Invalid option, Usage:\n");
          fprintf(stderr, "  rtplot [-s Seconds]\n");
        }

}	/* END PROCESS_ARGS */

/* END INIT.CC */
