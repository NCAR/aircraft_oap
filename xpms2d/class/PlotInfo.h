/*
-------------------------------------------------------------------------
OBJECT NAME:	PlotInfo.h

FULL NAME:	Plot Information

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#ifndef PLOTINFO_H
#define PLOTINFO_H

#include <define.h>

#define TITLESIZE	80


/* -------------------------------------------------------------------- */
class PlotInfo {

public:
	PlotInfo();
	~PlotInfo();

  const char * Title() const	{ return(title); }
  const char * SubTitle() const	{ return(subTitle); }

  void	GenerateDefaultTitles();


private:
  char	title[TITLESIZE], subTitle[TITLESIZE];


};	/* END PLOTINFO.H */

#endif
