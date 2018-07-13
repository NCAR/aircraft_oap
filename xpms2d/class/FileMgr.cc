/*
-------------------------------------------------------------------------
OBJECT NAME:	FileMgr.cc

FULL NAME:	File Manager Class

DESCRIPTION:	

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "FileMgr.h"

extern ADS_DataFile	*dataFile[];


/* -------------------------------------------------------------------- */
FileManager::FileManager()
{
  numberFiles = 0;
  currentFile = 0;

}	/* END CONTRUCTOR */

/* -------------------------------------------------------------------- */
void FileManager::NewFile(char fileName[], UserConfig &cfg)
{
  int	i;

  for (i = numberFiles-1; i >= 0; --i)
    delete dataFile[i];

  numberFiles = 1;
  currentFile = 0;

  dataFile[0] = new ADS_DataFile(fileName, cfg);

  // Save off this data path for subsequent calls.
  strcpy(DataPath, fileName);
  char *p = strrchr(DataPath, '/');
  if (p)
    strcpy(p, "/*2d*");
  else
    strcpy(DataPath, "*2d*");

}	/* END NEWFILE */

/* -------------------------------------------------------------------- */
void FileManager::AddFile(char fileName[], UserConfig &cfg)
{
  dataFile[numberFiles++] = new ADS_DataFile(fileName, cfg);

}	/* END ADDFILE */

/* END FILEMGR.CC */
