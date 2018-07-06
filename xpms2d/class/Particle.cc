/*
-------------------------------------------------------------------------
OBJECT NAME:    Probe.cc

FULL NAME:      Particle Information

COPYRIGHT:      University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#include "Particle.h"

Particle::Particle() :  time(0), msec(0), w(0), h(0), area(0), vert_gap(false),
                        horiz_gap(false), reject(false), edge(0), timeReject(false)
{
}

