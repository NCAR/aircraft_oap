/* raf.h  -- RAF-specific hardwired.  
*/

#ifndef RAF_H
#define RAF_H

#define PROJ_C130	'1'
#define PROJ_KINGAIR	'2'
#define PROJ_P3		'3'
#define PROJ_NOAA_G4	'4'
#define PROJ_B57	'5'
#define PROJ_TADS	'6'
#define PROJ_SABRELINER	'7'
#define PROJ_ELECTRA	'8'
#define PROJ_SAILPLANE	'9'

/*
Define each airplane name and the name of one candidate host that is likely
to be on the plane. This hostname need NOT be the WINDS server, Control Host,
NFS server, or anything at all; just installed, running, and on the net. 
As well define the 1st digit, as implied from above PROJ_<airplane> defines,
for project numbers associated with given platform, following the hostname.

Scripts using the name of one candidate host for given airplane should also
deal with that host not being up on the plane (using ping, e.g.) and ask
user interactively for an alternate name...

PLEASE NOTE: the following lines (including commented line with calcart name 
and #defines using <airplane>_NAME) use a format that other scripts parse
to get hostname info. Please do not alter without contacting WINDS expert! 
Thank You! (10/21/94)
*/
/* 
CALCART_NAME: tonopah 
WindsMount: /home/local 
WindsMount: /usr/openwin 
*/
#define ELECTRA_NAME	"Electra"	/* ONBOARD: salsa 8	*/
#define KINGAIR_NAME	"KingAir"	/* ONBOARD: ozone 2	*/
#define SABRELINER_NAME	"Sabreliner"	/* ONBOARD: sherlock 7	*/
#define C130_NAME	"C-130"		/* ONBOARD: hercules 1	*/
#define SAILPLANE_NAME	"Sailplane"	/* ONBOARD: sailplane-unknown 9 */
#define INHOUSE_NAME	"In-House"	/* ONBOARD: orion 0	*/
#define NOAA_G4_NAME	"GIV"		/* ONBOARD: rainier 4	*/
#define TADS_NAME	"TADS"		/* ONBOARD: frik 6	*/
#define B57_NAME	"B-57"		/* ONBOARD: phoenix 5	*/
#define NRL_P3_NAME	"P-3"		/* ONBOARD: orion 3	*/
/* 
types of data records, sizes
*/
#ifndef MX_PHYS
#define MX_PHYS    65535              	/* max physical record size, 32768 */
#endif

#define PMS2DC1         0x4331
#define PMS2DC2         0x4332
#define PMS2DG1		0x4731
#define PMS2DG2		0x4732
#define PMS2DH1		0x4831
#define PMS2DH2		0x4832
#define PMS2DP1         0x5031
#define PMS2DP2         0x5032

#define SDIWRD          0x8681
#define AVAPS_WORD	0x4156
#define PMS2_SIZE       4116
#define PMS2_RECSIZE    (MX_PHYS/PMS2_SIZE)*PMS2_SIZE

#ifdef __cplusplus
extern "C" {
#endif

void *GetMemory(size_t nbytes);
char *strupr(char s[]), TrimTrailingBlanks(char s[]);

int	TapeAssign(int fd), TapeOpen(char name[]), TapeRead(char *record),
	TapeSeek(long start_rec), TapeClose(), TapeTell();

/* nc_time.c */
int	GetFlightRecordNumber(int ncid, const char userTime[]);
time_t	InitFlightTime(int ncid);
int	GetFlightYear(int currentTimeOffset),
	GetFlightMonth(int currentTimeOffset),
	GetFlightDay(int currentTimeOffset),
	GetFlightHour(int currentTimeOffset),
	GetFlightMinute(int currentTimeOffset),
	GetFlightSecond(int currentTimeOffset);

#ifdef __cplusplus
}
#endif

#endif
