/*
-------------------------------------------------------------------------
OBJECT NAME:	FileMgr.h

FULL NAME:	Data File Manager

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#ifndef FILEMGR_H
#define FILEMGR_H

#include <define.h>

#include <DataFile.h>

namespace OAP
{
  class UserConfig;
}

/* -------------------------------------------------------------------- */
class FileManager {

public:
	FileManager();

  void		NewFile(char fileName[], OAP::UserConfig &cfg);
  void		AddFile(char fileName[], OAP::UserConfig &cfg);

  void		SetCurrentFile(int newFile)	{ currentFile = newFile; }

  size_t	NumberOfFiles() const		{ return(numberFiles); }

  ADS_DataFile	*CurrentFile()
		{ return(numberFiles == 0 ? (ADS_DataFile *)NULL : dataFile[currentFile]); }

  ADS_DataFile	*dataFile[MAX_DATAFILES];


private:
  int	numberFiles;
  int	currentFile;

};	/* END FILEMGR.H */

#endif
