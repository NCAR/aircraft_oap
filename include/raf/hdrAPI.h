/*
-------------------------------------------------------------------------
OBJECT NAME:	hdrAPI.h

DESCRIPTION:	Header file tape API package

COPYRIGHT:	University Corporation for Atmospheric Research, 1998-2005
-------------------------------------------------------------------------
*/

#ifndef HDRAPI_H
#define HDRAPI_H

#include "header.h"
#include "ntohf.h"

#include <string>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>


/* -------------------------------------------------------------------- */
/**
 * Class to read and parse an ADS header.
 */
class Header
{
public:
	Header(const char fName[]);
	Header(const std::string fName);
	~Header();

  void 	readHeader(const char fName[]);

  const void 	*HeaderPointer()	{ return((void *)_header); }

  const char	*ProjectName();		// May not work if no PROJ_DIR

  // Struct Fl fields.
  const char	*Version()		{ return(_header->version); }
  const char	*ProjectNumber()	{ return(_header->prnum); }
  const char	*FlightNumber()		{ return(_header->fltnum); }
  const char	*TapeNumber()		{ return(_header->tpnum); }
  const char	*FlightDate()		{ return(_header->date); }
  const char	*FlightTime()		{ return(_header->time); }
  const char	*TimeZone()		{ return(_header->tzone); }
  const char	*Aircraft()		{ return(_header->acraft); }

  long	NumberItems()			{ return(ntohl(_header->n_items)); }
  long	lrLength()			{ return(ntohl(_header->lrlen)); }
  long	lrPpr()				{ return(ntohl(_header->lrppr)); }
  long	HeaderLength()			{ return(ntohl(_header->thdrlen)); }

  const char	*ADStype()		{ return(_header->ads_type); }


  const void *GetFirst(), *GetFirst(const char blkName[]), *GetSDI(const char varName[]);
  const void *GetNext(), *GetNext(const char blkName[]);


  const char *ItemType(const void *vp)	{ return(((Blk *)vp)->item_type); }
  long	ItemLength(const void *vp)	{ return(ntohl(((Blk *)vp)->item_len)); }
  long	Start(const void *vp)		{ return(ntohl(((Blk *)vp)->start)); }
  long	SizeOf(const void *vp)		{ return(ntohl(((Blk *)vp)->length)); }

  long	SampleRate(const Dme *vp)	{ return(ntohl(vp->rate)); }
  long	SampleRate(const Ins *vp)	{ return(ntohl(vp->rate)); }
  long	SampleRate(const Masp *vp)	{ return(ntohl(vp->rate)); }
  long	SampleRate(const Pms1 *vp)	{ return(ntohl(vp->rate)); }
  long	SampleRate(const Pms1v2 *vp)	{ return(ntohl(vp->rate)); }
  long	SampleRate(const Serial *vp)	{ return(ntohl(vp->rate)); }
  long	SampleRate(const Sh *vp)	{ return(ntohl(vp->rate)); }

  // Which DSM is probe/sensor attached to.
  const char	*DSM(const Asy *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Blk *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Dme *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Evt *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Ins *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Irs *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Masp *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Pms1 *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Pms1v2 *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Pms2 *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Pms2h *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Serial *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Sh *vp)		{ return(vp->dsm_locn); }
  const char	*DSM(const Uvhyg *vp)		{ return(vp->dsm_locn); }

  // Probe/sensor mounting location.
  const char	*AircraftLocation(const Blk *vp)	{ return(vp->locn); }
  const char	*AircraftLocation(const Evt *vp)	{ return(vp->locn); }
  const char	*AircraftLocation(const Irs *vp)	{ return(vp->locn); }
  const char	*AircraftLocation(const Masp *vp)	{ return(vp->locn); }
  const char	*AircraftLocation(const Pms1 *vp)	{ return(vp->locn); }
  const char	*AircraftLocation(const Pms1v2 *vp)	{ return(vp->locn); }
  const char	*AircraftLocation(const Pms2 *vp)	{ return(vp->locn); }
  const char	*AircraftLocation(const Pms2h *vp)	{ return(vp->locn); }
  const char	*AircraftLocation(const Uvhyg *vp)	{ return(vp->locn); }


  // Struct Sh (SDI or Analog/Digital stuff).
  long	DSMaddress(const Sh *vp)		{ return(ntohl(vp->adsaddr)); }
  const char	*SDItype(const Sh *vp)		{ return(vp->type); }
  long	InterleaveOffset(const Sh *vp)		{ return(ntohl(vp->offset)); }

  float	AtoDconversionFactor(const Sh *vp)	{ return(ntohf(vp->convert)); }
  long  AtoDconversionOffset(const Sh *vp)	{ return(ntohl(vp->convert_offset)); }

  long  ChannelGain(const Sh *vp)		{ return(ntohl(vp->ch_gain)); }
  long  ChannelOffset(const Sh *vp)		{ return(ntohl(vp->ch_offset)); }


  const char	*VariableName(const Asy *vp)	{ return(vp->name); }
  const char	*VariableName(const Masp *vp)	{ return(vp->item_type); }
  const char	*VariableName(const Pms1 *vp)	{ return(vp->name); }
  const char	*VariableName(const Pms1v2 *vp)	{ return(vp->name); }
  const char	*VariableName(const Pms2 *vp)	{ return(vp->name); }
  const char	*VariableName(const Pms2h *vp)	{ return(vp->name); }
  const char	*VariableName(const Serial *vp)	{ return(vp->name); }
  const char	*VariableName(const Sh *vp)	{ return(vp->name); }

  long	CalibrationOrder(const Sh *vp)		{ return(ntohl(vp->order)); }
  float	CalibrationCoefficient(const Sh *vp, int i)	{ return(ntohf(vp->cof[i])); }

  const char	*SerialNumber(const Masp *vp)	{ return(vp->serial_num); }
  const char	*SerialNumber(const Pms1v2 *vp)	{ return(vp->serial_num); }
  const char	*SerialNumber(const Pms2 *vp)	{ return(vp->serial_num); }
  const char	*SerialNumber(const Pms2h *vp)	{ return(vp->serial_num); }
  const char	*SerialNumber(const Uvhyg *vp)	{ return(vp->serial_num); }

  long	InterfaceNumber(const Pms1v2 *vp)	{ return(ntohl(vp->intf_num)); }
  long	InterfaceChannel(const Pms1v2 *vp)	{ return(ntohl(vp->intf_chan)); }
  long	InterfaceChannel(const Pms2 *vp)	{ return(ntohl(vp->intf_chan)); }

  long	ParticleSpacingStart(const Pms1v2 *vp)  { return(ntohl(vp->ps_start)); }
  long	ParticleSpacingLength(const Pms1v2 *vp) { return(ntohl(vp->ps_length)); }
  float	ParticleSpacingGate(const Pms1v2 *vp)   { return(ntohf(vp->ps_gate)); }

  long	lrLength(const Pms2 *vp)	{ return(ntohl(vp->lrlen)); }
  long	lrLength(const Asy *vp)		{ return(ntohl(vp->lrlen)); }
  short	lrPpr(const Pms2 *vp)		{ return(ntohs(vp->lrppr)); }
  long	lrPpr(const Asy *vp)		{ return(ntohl(vp->lrppr)); }

  short	Resolution(const Pms2 *vp)	{ return(ntohs(vp->resolution)); }

  bool	isValid()			{ return(_valid); }

private:
  Fl	*_header;
  char	*_hdr;
  int	_currentIndx;
  char	*_currentPtr;

  char	_projName[80];

  bool	_valid;
};

#endif

/* END HDRAPI.H */
