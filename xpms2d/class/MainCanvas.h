/*
-------------------------------------------------------------------------
OBJECT NAME:	MainCanvas.h

FULL NAME:	Main canvas

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2018
-------------------------------------------------------------------------
*/

#ifndef MAINCANVAS_H
#define MAINCANVAS_H

#include <define.h>
#include <raf/header.h>

#include <raf/Canvas.h>
#include <DataFile.h>
#include <raf/PostScript.h>


/* -------------------------------------------------------------------- */
class MainCanvas : public Canvas {

public:
	MainCanvas(Widget w);

  void	SetDisplayMode(int mode);
  void	SetWrapDisplay()	{ _wrap = 1 - _wrap; }
  void	SetTimingWords()	{ _timingWord = 1 - _timingWord; }

  void	reset(ADS_DataFile *file);
  void	draw(P2d_rec *record, Probe *probe, float hdrVer, int probeNum, PostScript *ps);
  size_t maxRecords() const	{ return(maxRecs); }
  int	SpaceAvailable() const	{ return(Height() - y); }

  /**
   * Test if string s1 is all of same charcter c.  Similar to memcmp, but testing
   * for a fixed value as if s1 was created by memset(3).
   * @returns 0 if nBytes of s1 matches c, else returns 1 for non-match.
   */
  int memchrcmp(const void *s1, const int c, size_t n);


protected:
  void	drawPMS2D(P2d_rec *record, Probe *probe, float hdrVer, int probeNum, PostScript *ps);
  void	drawFast2D(P2d_rec *record, Probe *probe, float hdrVer, int probeNum, PostScript *ps);
  void	draw2DS(P2d_rec *record, Probe *probe, float hdrVer, int probeNum, PostScript *ps);
  void	drawHVPS(P2d_rec *record, Probe *probe, float hdrVer, int probeNum, PostScript *ps);
  void	drawCIP(P2d_rec *record, Probe *probe, float hdrVer, int probeNum, PostScript *ps);

  void	drawSlice(PostScript *ps, int i, const unsigned char *slice, size_t nDiodes);
  void	drawSlice(PostScript *ps, int i, uint32_t slice);

  /**
   * Degrade a 25um Fast2DC probe to a 200um 2DP looking data.  Done byr
   * looking at each 8x8 square and if more than 35 pixels are shadowed
   * then generate a pixel in the degraded image.  This particles are shown
   * to the right of the regular data.
   */
  void draw_2DC_as_2DP(P2d_rec *record);

  /**
   * Count all shadowed diodes across the flight track and display histogram.
   */
  void drawDiodeHistogram(P2d_rec *record, size_t nDiodes);
  void drawDiodeHistogram(P2d_rec *record, size_t nDiodes, uint32_t sync);

  void drawAccumHistogram(struct recStats &stats, size_t xOffset);

  size_t uncompressCIP(unsigned char *dest, const unsigned char src[], int nbytes);

  int	y, maxRecs;
  int	_displayMode;
  int	PIX_PER_Y;

  bool	_wrap,		// Wrap display around (HVPS mostly)
	_timingWord;	// Toggle timing words on/off.

};	/* END MAINCANVAS.H */

#endif
