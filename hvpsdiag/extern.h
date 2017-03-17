/*
-------------------------------------------------------------------------
OBJECT NAME:	extern.h

FULL NAME:	Global Variable External Declarations

DESCRIPTION:	

AUTHOR:		websterc@ncar
-------------------------------------------------------------------------
*/

#ifndef EXTERN_H
#define EXTERN_H

extern bool	Grid, Frozen, FlashMode;

extern char	buffer[], DataPath[], *ProjectName, FlightNumber[];

extern int	NumberDataSets, NumberSeconds, NumberElements[], CurrentDataSet;
extern int	Aircraft;

extern double	*plotData, *xData;

#endif

/* END EXTERN.H */
