/*
-------------------------------------------------------------------------
OBJECT NAME:	Hex.h

FULL NAME:	View Hex

COPYRIGHT:	University Corporation for Atmospheric Research, 2000
-------------------------------------------------------------------------
*/

#ifndef HEX_H
#define HEX_H

#include <define.h>
#include <raf/header.h>
#include <raf/TextWindow.h>


/* -------------------------------------------------------------------- */
class Hex : public TextWindow
{
public:
	Hex(const Widget parent);

  void	Update(size_t nBuffs, P2d_rec sets[]);

};

#endif
