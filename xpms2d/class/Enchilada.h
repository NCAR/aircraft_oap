/*
-------------------------------------------------------------------------
OBJECT NAME:	Enchilada.h

FULL NAME:	View Enchilada

COPYRIGHT:	University Corporation for Atmospheric Research, 2000-2018
-------------------------------------------------------------------------
*/

#ifndef ENCHILADA_H
#define ENCHILADA_H

#include <raf/Particle.h>
#include <raf/TextWindow.h>


/* -------------------------------------------------------------------- */
class Enchilada : public TextWindow
{
public:
	Enchilada(const Widget parent);

  void	AddLineItem(int cnt, OAP::Particle *cp);

};

#endif
