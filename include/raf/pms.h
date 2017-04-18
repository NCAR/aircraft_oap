/*
-------------------------------------------------------------------------
OBJECT NAME:	pms.h

FULL NAME:	PMSspec header file.

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1995-2000
-------------------------------------------------------------------------
*/

#ifndef PMS_H
#define	PMS_H

#define PMS_SPEC_FILE	"%s/%s/%s/PMSspecs"

#ifdef __cplusplus
extern "C" {
#endif
void	InitPMSspecs(const char fileName[]), ReleasePMSspecs(void);
char	*GetPMSparameter(const char serialNumber[], const char parameter[]);
int	GetPMSlist(char *list[]);
#ifdef __cplusplus
}
#endif

#endif
