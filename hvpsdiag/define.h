/*
-------------------------------------------------------------------------
OBJECT NAME:	define.h

FULL NAME:	Include File to Include the Include Files

DESCRIPTION:	
-------------------------------------------------------------------------
*/

#ifndef DEFINE_H
#define DEFINE_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <netinet/in.h>

#define CONSTANTS_H
#define NAMLEN          13

#define BUFFSIZE	2048
#define PATH_LEN	1024

#define	SPACE		' '

#define TITLESIZE	128
#define LEGENDSIZE	(TITLESIZE<<1)

#define NUMTICMARKS	10

/* Defines for 'Mode'	*/
#define COUNTS		0
#define VOLTS		1
#define ENGINEERING	2

#include "extern.h"

#endif

/* END DEFINE.H */
