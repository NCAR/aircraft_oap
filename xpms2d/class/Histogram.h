/*
-------------------------------------------------------------------------
OBJECT NAME:	Histogram.h

FULL NAME:	View Histogram Data

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2009
-------------------------------------------------------------------------
*/

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <define.h>
#include <raf/header.h>
#include <raf/TextWindow.h>


/* -------------------------------------------------------------------- */
class Histogram : public TextWindow
{
public:
	Histogram(const Widget parent);

  void	AddLineItem(P2d_rec * record, recStats & output);

};

#endif
