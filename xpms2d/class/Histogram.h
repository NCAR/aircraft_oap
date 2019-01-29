/*
-------------------------------------------------------------------------
OBJECT NAME:	Histogram.h

FULL NAME:	View Histogram Data

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2018
-------------------------------------------------------------------------
*/

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <define.h>
#include <raf/TextWindow.h>

#include <raf/Probe.h>

/* -------------------------------------------------------------------- */
class Histogram : public TextWindow
{
public:
	Histogram(const Widget parent);

  void	AddLineItem(OAP::P2d_rec * record, struct OAP::recStats & stats);

};

#endif
