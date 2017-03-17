/*
-------------------------------------------------------------------------
OBJECT NAME:    MainCanvas.cc
 
FULL NAME:      Main canvas
 
COPYRIGHT:	University Corporation for Atmospheric Research, 1997-2006
-------------------------------------------------------------------------
*/

#include "MainCanvas.h"
#include "Enchilada.h"
#include "Histogram.h"
#include "Colors.h"
#include <raf/XFonts.h>
#include <raf/XPen.h>

ProbeType ProbeType(P2d_rec *record);

extern Enchilada	*enchiladaWin; 
extern Histogram	*histogramWin; 
extern Colors	*color;
extern XFonts	*fonts;
extern XPen	*pen;

#include <algorithm>
#include <cassert>
#include <sstream>

static const int	TOP_OFFSET = 35;
static const int	LEFT_MARGIN = 5;
// RAF HVPS, 40 pixels masked at each end of diode array.
static const int	HVPS_MASKED = 40;

static bool debug = false;

static bool part1[512][64];
static int part1slice = 0, part2slice = 0;

/* -------------------------------------------------------------------- */
MainCanvas::MainCanvas(Widget w) : Canvas(w)
{
  SetDisplayMode(NORMAL);

  wrap = false;
  timingWord = true;

  reset(NULL);

}       /* END CONSTRUCTOR */

/* -------------------------------------------------------------------- */
void MainCanvas::SetDisplayMode(int mode)
{
  displayMode = mode;

  switch (displayMode)
    {
    case NORMAL:
    case DIAGNOSTIC:
      PIX_PER_Y = 70;
      break;

    case ENCHILADA:
      PIX_PER_Y = 600;
      break;
    }

  maxRecs = (Height() - TOP_OFFSET) / PIX_PER_Y;

}	/* END SETDISPLAYMODE */

/* -------------------------------------------------------------------- */
void MainCanvas::reset(ADS_DataFile *file)
{
  maxRecs = (Height() - TOP_OFFSET) / PIX_PER_Y;
  y = TOP_OFFSET;

  if (file)
    {
    std::stringstream title;
    title << file->ProjectNumber() << ", " << file->FlightNumber() << " - "
	<< file->FlightDate();

    pen->SetFont(fonts->Font(0));
    pen->DrawText(Surface(), 300, 25, title.str().c_str());
    }

}	/* END RESET */

/* -------------------------------------------------------------------- */
void MainCanvas::draw(P2d_rec *record, struct recStats &stats, float version, int probeNum, PostScript *ps)
{
  char		buffer[256];
  Particle	*cp;
  bool		old2d = false;

  static short  prevOverLoad = 0;

  // Ok, now go back and display the data.
  pen->SetFont(fonts->Font(3));


  if (ps) ps->SetColor(color->GetColorPS(BLACK));
  else pen->SetColor(color->GetColor(BLACK));

  if (ps) { ps->MoveTo(35, 750-(y-2)); ps->LineTo(1060, 750-(y-2)); }
  else pen->DrawLine(Surface(), 0, y-2, Width(), y-2);

  if (ps) { ps->MoveTo(547, 750-(y-5)); ps->LineTo(547, 750-y); }
  else pen->DrawLine(Surface(), 523, y-5, 523, y);


part1slice = 0;
part2slice = 0;
memset(part1, 0, sizeof(part1));

  if (ProbeType(record) == FAST2D)
    drawFast2D(record, stats, version, probeNum, ps);
  else
  if (ProbeType(record) == TWODS)
    draw2DS(record, stats, version, probeNum, ps);
  else
  if (ProbeType(record) == HVPS)
    drawHVPS(record, stats, version, probeNum, ps);
  else
  if (ProbeType(record) == CIP)
    drawCIP(record, stats, version, probeNum, ps);
  else
    {
    old2d = true;
    drawPMS2D(record, stats, version, probeNum, ps);
    }

  // In the old ADS2, overld gets stamped in the one record early,
  // not in the actual record that was interupted.  Compensate here.
  size_t overLoad = old2d ? prevOverLoad : record->overld;
 
  if (stats.duplicate)
    {
    strcpy(buffer, " DUPLICATE RECORD! ");
    if (ps)
      {
      ps->SetColor(color->GetColorPS(RED));
      ps->MoveTo(10, 750-(y+15)); ps->ShowStr(buffer);
      ps->MoveTo(300, 750-(y+15)); ps->ShowStr(buffer);
      ps->MoveTo(600, 750-(y+15)); ps->ShowStr(buffer);
      }
    else
      {
      pen->SetColor(color->GetColor(RED));
      pen->DrawText(Surface(), 10, y+15, buffer);
      pen->DrawText(Surface(), 300, y+15, buffer);
      pen->DrawText(Surface(), 600, y+15, buffer);
      }
    }

  // Draw text info at bottom of record.
  if (ps) ps->SetColor(color->GetColorPS(BLACK));
  else pen->SetColor(color->GetColor(BLACK));

  if (stats.thisTime < stats.prevTime) {
    if (ps) ps->SetColor(color->GetColorPS(RED));
    else pen->SetColor(color->GetColor(RED));
    }

  sprintf(buffer, "%02d:%02d:%02d.%03d",
	record->hour, record->minute, record->second, record->msec);

  if (ps) {
    ps->MoveTo(200, 750-(y+50)); ps->ShowStr(buffer);
    ps->SetColor(color->GetColorPS(BLACK));
    }
  else {
    pen->DrawText(Surface(), 200, y+50, buffer);
    pen->SetColor(color->GetColor(BLACK));
    }

  sprintf(buffer, ", TAS=%5.1f, overLoad=", stats.tas);
  if (ps) { ps->MoveTo(283, 750-(y+50)); ps->ShowStr(buffer); }
  else pen->DrawText(Surface(), 283, y+50, buffer);

  if (overLoad > 90) {
    if (ps) ps->SetColor(color->GetColorPS(RED));
    else pen->SetColor(color->GetColor(RED));
    }

  sprintf(buffer, "%.3f,", (float)overLoad / 1000);
  if (ps) {
    ps->MoveTo(445, 750-(y+50)); ps->ShowStr(buffer);
    ps->SetColor(color->GetColorPS(BLACK));
    }
  else {
    pen->DrawText(Surface(), 445, y+50, buffer);
    pen->SetColor(color->GetColor(BLACK));
    }

  sprintf(buffer, "nParticles = %d, elapsed time = ", stats.nTimeBars);
  if (ps) { ps->MoveTo(490, 750-(y+50)); ps->ShowStr(buffer); }
  else pen->DrawText(Surface(), 490, y+50, buffer);

  if (stats.DASelapsedTime / 1000 > 36000) {
    if (ps) ps->SetColor(color->GetColorPS(RED));
    else pen->SetColor(color->GetColor(RED));

    sprintf(buffer, "%6u", stats.DASelapsedTime / 1000);
    }
  else {
    sprintf(buffer, "%6.3f", (float)stats.DASelapsedTime / 1000);
    }

  if (ps) { ps->MoveTo(706, 750-(y+50)); ps->ShowStr(buffer); }
  else pen->DrawText(Surface(), 706, y+50, buffer);

  if (ps) ps->SetColor(color->GetColorPS(BLACK));
  else pen->SetColor(color->GetColor(BLACK));

  sprintf(buffer,", timeBarTotal = %6.3f", (float)stats.tBarElapsedtime / 1000);
  if (ps) { ps->MoveTo(750, 750-(y+50)); ps->ShowStr(buffer); }
  else pen->DrawText(Surface(), 750, y+50, buffer);


  if (stats.DASelapsedTime == 0.0)
    sprintf(buffer, "%.1f%% %.1f%%", 0.0, 0.0);
  else
    {
    float olap = (float)(stats.tBarElapsedtime+overLoad) /
			stats.DASelapsedTime * 100;

    if (olap < 75.0 || olap > 125.0) {
      if (ps) ps->SetColor(color->GetColorPS(RED));
      else pen->SetColor(color->GetColor(RED));
      }

    sprintf(buffer, "%.1f%% %.1f%%",
	(float)stats.tBarElapsedtime / stats.DASelapsedTime * 100, olap);
    }

  if (ps) { ps->MoveTo(950, 750-(y+50)); ps->ShowStr(buffer); }
  else pen->DrawText(Surface(), 950, y+50, buffer);
  if (ps) ps->SetColor(color->GetColorPS(BLACK));
  else pen->SetColor(color->GetColor(BLACK));


{
  int nCols, colWidth;
  float		res2;

nCols = 5;
colWidth = Width() / nCols;

  res2 = (float)stats.resolution * stats.resolution / 1000000;

  switch (displayMode)
    {
    case ENCHILADA:
    case NORMAL:
      sprintf(buffer,
	"sv: act = %.2lfL, used = %.3lfL, area = %.2fmm2, conc = %.3lfN/L, lw = %.3lfg/M3, z = %.3lfdb",
	stats.SampleVolume / 1000, stats.DOFsampleVolume / 1000,
	res2 * stats.area, stats.concentration, stats.lwc, stats.dbz);

      if (ps) { ps->MoveTo(315, 750-(y+62)); ps->ShowStr(buffer); }
      else pen->DrawText(Surface(), 315, y+62, buffer);

      for (size_t i = 0; (cp = (Particle *)stats.particles.DeQueue()); ++i)
        delete cp;
//printf("Normal: %d unremoved particles\n", i);
      break;

    case DIAGNOSTIC:
      sprintf(buffer, "freq = %f,  mean=%u min=%u max=%u",
	stats.frequency * 1000, stats.meanBar, stats.minBar, stats.maxBar);

      if (ps) { ps->MoveTo(505, 750-(y+62)); ps->ShowStr(buffer); }
      else pen->DrawText(Surface(), 505, y+62, buffer);

      for (size_t i = 0; (cp = (Particle *)stats.particles.DeQueue()) ; ++i)
        delete cp;
//printf("Diagnostic: %d unremoved particles\n", i);
      break;
    }
}

  prevOverLoad = record->overld;
  y += PIX_PER_Y;

  if (histogramWin)
    histogramWin->AddLineItem(record, stats);

}       /* END DRAW */
 
/* -------------------------------------------------------------------- */
void MainCanvas::drawPMS2D(P2d_rec *record, struct recStats &stats, float version, int probeNum, PostScript *ps)
{
  int		nextColor, cntr = 0;
  uint32_t	*p, slice, pSlice, ppSlice, syncWord;
  bool		colorIsBlack = false;
  Particle	*cp;
  char		buffer[256];

  static uint32_t	prevSlice[2];
  static uint32_t	prevTime;
  static P2d_rec	prevRec;

  p = (uint32_t *)record->data;
  ppSlice = prevSlice[0];
  pSlice = prevSlice[1];

  if (memcmp((void *)record, (void *)&prevRec, sizeof(P2d_rec)) == 0)
    stats.duplicate = true;

  if (version < 3.35)
    syncWord = SyncWordMask;
  else
    syncWord = StandardSyncWord;

  if (displayMode == RAW_RECORD)
    {
    p = (uint32_t *)record->data;
    for (size_t i = 0; i < nSlices_32bit; ++i, ++p)		/* 2DC and/or 2DP	*/
      drawSlice(ps, i, (unsigned char *)p, 32);
//      drawSlice(ps, i, *p);

    y += 34;
    }

  p = (uint32_t *)record->data;
  for (size_t i = 0; i < nSlices_32bit; ++p)		/* 2DC and/or 2DP	*/
    {
    slice = *p;

    /* If sync word and word before is not -1, then sum up timing words
     * into milliseconds.
     */
    if (slice == syncWord && ppSlice == 0xffffffff && pSlice != 0xffffffff)
      {
      if ((cp = (Particle *)stats.particles.DeQueue()) == NULL)
        break;

if (debug) { if (cp) printf("dq: %06x %zu %zu\n", cp->timeWord, cp->h, cp->w); else printf("NULL\n"); }

      uint32_t timeWord = (uint32_t)((float)cp->timeWord * stats.frequency);

      if (timeWord >= stats.DASelapsedTime)
        {
        sprintf(buffer, "%u", timeWord);

        if (ps) {
          ps->SetColor(color->GetColorPS(RED));
          ps->MoveTo(i+10, 750-(y+(PIX_PER_Y-30)));
          ps->ShowStr(buffer);
          }
        else {
          pen->SetColor(color->GetColor(RED));
          pen->DrawText(Surface(), i+10, y+(PIX_PER_Y-30), buffer);
          }
        }


      // Draw timing & sync words in yellow (or some other color).
      if (cp && cp->reject) {
        if (cp->h == 1 && cp->w == 1)	// zero area image.
          if (ps) ps->SetColor(color->GetColorPS(YELLOW));
          else pen->SetColor(color->GetColor(YELLOW));
        else
          if (ps) ps->SetColor(color->GetColorPS(RED));
          else pen->SetColor(color->GetColor(RED));

        nextColor = 0;
        }
      else {	// Green for good particle
        if (ps) ps->SetColor(color->GetColorPS(GREEN));
        else pen->SetColor(color->GetColor(GREEN));

        nextColor = probeNum;
        }

      if (timingWord)
        {
        drawSlice(ps, i++, (unsigned char *)&pSlice, 32);
        drawSlice(ps, i++, (unsigned char *)&slice, 32);
        }
      else
        i += 2;

      if (ps) ps->SetColor(color->GetColorPS(nextColor));
      else pen->SetColor(color->GetColor(nextColor));

      if (nextColor == 0)	// if Black
        colorIsBlack = true;
      else
        colorIsBlack = false;

      for (++p; i < nSlices_32bit && *p != 0xffffffff; ++p)
        drawSlice(ps, i++, (unsigned char *)p, 32);
//        drawSlice(ps, i++, *p);

      if (enchiladaWin)
        enchiladaWin->AddLineItem(cntr++, cp);

      delete cp;
      }
    else
    if (p[1] != syncWord)
      {
      if (!colorIsBlack)
        {
        if (ps) ps->SetColor(color->GetColorPS(BLACK));
        else pen->SetColor(color->GetColor(BLACK));

        colorIsBlack = true;
        }

      drawSlice(ps, i++, (unsigned char *)&slice, 32);
//      drawSlice(ps, i++, slice);
      }
    else
      ++i;

    ppSlice = pSlice;
    pSlice = slice;
    }

  if (displayMode == DIAGNOSTIC)
    drawDiodeHistogram(record, syncWord);
  else
    drawAccumHistogram(stats);

  p = (uint32_t *)record->data;
  prevSlice[0] = p[1022];
  prevSlice[1] = p[1023];
  stats.prevTime = prevTime;
  prevTime = stats.thisTime;
  memcpy((void *)&prevRec, (void *)record, sizeof(P2d_rec));
}

/* -------------------------------------------------------------------- */
void MainCanvas::drawFast2D(P2d_rec *record, struct recStats &stats, float version, int probeNum, PostScript *ps)
{
  Particle	*cp;
  int		nextColor, cntr = 0;
  bool		colorIsBlack = false;
  unsigned long long *p;

  static unsigned long prevTime;
  static P2d_rec prevRec;

  if (memcmp((void *)record, (void *)&prevRec, sizeof(P2d_rec)) == 0)
    stats.duplicate = true;

  /*
   * If using the View->Raw Data menu item, or if no particles were detected
   * in the record, then come in here and do a raw display of the record.
   * Only color code timing (green) and overload (blue) words.
   */
  if (displayMode == RAW_RECORD || (cp = (Particle *)stats.particles.Front()) == NULL)
  {
    p = (unsigned long long *)record->data;
    for (size_t i = 0; i < nSlices_64bit; ++i, ++p)         /* 2DC and/or 2DP       */
    {
      if ((*p & Fast2D_Mask) == Fast2D_Sync)
      {
        if (ps) ps->SetColor(color->GetColorPS(GREEN));
        else pen->SetColor(color->GetColor(GREEN));
      }
      if ((*p & Fast2D_Mask) == Fast2D_Overld)
      {
        if (ps) ps->SetColor(color->GetColorPS(BLUE));
        else pen->SetColor(color->GetColor(BLUE));
      }

//      drawSlice(ps, i, *p);
      drawSlice(ps, i, (unsigned char *)p, 64);
      if (ps) ps->SetColor(color->GetColorPS(BLACK));
      else pen->SetColor(color->GetColor(BLACK));
    }

    if (displayMode == RAW_RECORD)
      y += 66;	// Add enough room for a second copy of this record.
    else
    {
      y += 32;	// Bail out (no particles detected from process.cc).
      return;
    }
  }

//  if ((cp = (Particle *)stats.particles.Front()) == NULL)
//    return;

  p = (unsigned long long *)record->data;
  for (size_t i = 0; i < nSlices_64bit; )
  {
    if (cp == 0 || cp->reject)
      nextColor = 0;	// black.
    else
      nextColor = probeNum;

    if ((*p & Fast2D_Mask) == Fast2D_Sync || (*p & Fast2D_Mask) == Fast2D_Overld)
    {
      /**
       * Color code timing words:
       * 	Green = accepted.
       * 	Yellow = zero area image (i.e. timing bar with no visible particle).
       * 	Red = rejected.
       * 	Blue = overload word, also rejected.
       */
      if ((*p & Fast2D_Mask) == Fast2D_Overld)
      {
        if (ps) ps->SetColor(color->GetColorPS(BLUE));
        else pen->SetColor(color->GetColor(BLUE));
      }
      else
      if (cp && cp->reject) {
        if (cp->h == 0 || cp->w == 0)
          if (ps) ps->SetColor(color->GetColorPS(YELLOW));
          else pen->SetColor(color->GetColor(YELLOW));
        else
          if (ps) ps->SetColor(color->GetColorPS(RED));
          else pen->SetColor(color->GetColor(RED));
      }
      else {	// Green for good particle
        if (ps) ps->SetColor(color->GetColorPS(GREEN));
        else pen->SetColor(color->GetColor(GREEN));
      }

      if (timingWord)
        drawSlice(ps, i, (unsigned char *)p, 64);
//        drawSlice(ps, i, *p);
      ++i; ++p;

      // Get next particle.
      cp = (Particle *)stats.particles.DeQueue();
      delete cp;
      cp = (Particle *)stats.particles.Front();

      draw_2DC_as_2DP(record);
    }
    else
    {
      if (ps) ps->SetColor(color->GetColorPS(nextColor));
      else pen->SetColor(color->GetColor(nextColor));

      if (nextColor == 0)	// if Black
        colorIsBlack = true;
      else
        colorIsBlack = false;

      for (; i < nSlices_64bit && (*p & Fast2D_Mask) != Fast2D_Sync
		&& (*p & Fast2D_Mask) != Fast2D_Overld; ++p)
        drawSlice(ps, i++, (unsigned char *)p, 64);
//        drawSlice(ps, i++, *p);

      if (enchiladaWin)
        enchiladaWin->AddLineItem(cntr++, cp);
    }
  }

/*
// For diagnostics, display record as is on 2nd half of screen/window.
  for (size_t i = 0; i < nSlices_64bit; ++i)
{ if (((unsigned long long *)record->data)[i] == 0xffffffffffffffffLL) { pen->SetColor(color->GetColor(YELLOW)); ((unsigned long long *)record->data)[i] = 0; }
    drawSlice(ps, nSlices_64bit+i, ((unsigned long long *)record->data)[i]);
pen->SetColor(color->GetColor(0)); }
*/

  if (displayMode == DIAGNOSTIC)
    drawDiodeHistogram(record);
  else
    drawAccumHistogram(stats);

  y += 32;
  stats.prevTime = prevTime;
  prevTime = stats.thisTime;
  memcpy((void *)&prevRec, (void *)record, sizeof(P2d_rec));
}

/* -------------------------------------------------------------------- */
void MainCanvas::draw2DS(P2d_rec *record, struct recStats &stats, float version, int probeNum, PostScript *ps)
{
  Particle	*cp;
  int		nextColor, cntr = 0;
  bool		colorIsBlack = false;
  unsigned char *p;

  static unsigned long prevTime;
  static P2d_rec prevRec;
  static int x = 0;

  if (memcmp((void *)record, (void *)&prevRec, sizeof(P2d_rec)) == 0)
    stats.duplicate = true;

  if (memcmp((void *)record, (void *)&prevRec, 18) == 0 && x < 800)
  {
//    printf("dup time\n");
    y -= (PIX_PER_Y + 96);
//    x += 5;
  }
  else
    x = 0;

  /**
   * If using the View->Raw Data menu item, or if no particles were detected
   * in the record, then come in here and do a raw display of the record.
   * Only color code timing (green) and overload (blue) words.
   */
//  if (displayMode == RAW_RECORD || (cp = (Particle *)stats.particles.Front()) == NULL)
  {
    p = (unsigned char *)record->data;
    for (size_t i = 0; i < nSlices_128bit; ++i, p += 16)
    {
      if (memchrcmp(p+13, 0xAA, 3) == 0)
      {
//unsigned long long *word = (unsigned long long *)p;
//printf("%llu\n", *word & 0x0000ffffffffffff);
        if (ps) ps->SetColor(color->GetColorPS(GREEN));
        else pen->SetColor(color->GetColor(GREEN));
      }
      if ((*p & Fast2D_Mask) == Fast2D_Overld)
      {
        if (ps) ps->SetColor(color->GetColorPS(BLUE));
        else pen->SetColor(color->GetColor(BLUE));
      }

      drawSlice(ps, x++, p, 128);
      if (ps) ps->SetColor(color->GetColorPS(BLACK));
      else pen->SetColor(color->GetColor(BLACK));
    }
/*
    if (displayMode == RAW_RECORD)
      y += 130;  // Add enough room for a second copy of this record.
    else
    {
      y += 32;  // Bail out (no particles detected from process.cc).
      return;
    }
*/
  }



  y += 96;
  stats.prevTime = prevTime;
  prevTime = stats.thisTime;
  memcpy((void *)&prevRec, (void *)record, sizeof(P2d_rec));
}

/* -------------------------------------------------------------------- */
// DMT CIP/PIP probes are run length encoded.  Decode here.
// Duplicated in src/process.cc, not consolidated at this time due to static variables.
size_t MainCanvas::uncompressCIP(unsigned char *dest, const unsigned char src[], int nbytes)
{
  int d_idx = 0, i = 0;

  static size_t nResidualBytes = 0;
  static unsigned char residualBytes[16];

  if (nResidualBytes)
  {
    memcpy(dest, residualBytes, nResidualBytes);
    d_idx = nResidualBytes;
    nResidualBytes = 0;
  }

  for (; i < 4096; ++i)
  {
    unsigned char b = src[i];

    int nBytes = (b & 0x1F) + 1;

    if ((b & 0x20))     // This is a dummy byte; for alignment purposes.
    {
      continue;
    }

    if ((b & 0xE0) == 0)
    {
      memcpy(&dest[d_idx], &src[i+1], nBytes);
      d_idx += nBytes;
      i += nBytes;
    }

    if ((b & 0x80))
    {
      memset(&dest[d_idx], 0, nBytes);
      d_idx += nBytes;
    }
    else
    if ((b & 0x40))
    {
      memset(&dest[d_idx], 0xFF, nBytes);
      d_idx += nBytes;
    }
  }

  // Align data.  Find a sync word and put record on mod 8.
  for (i = 0; i < d_idx; ++i)
  {
     if (memcmp(&dest[i], &CIP_Sync, 8) == 0)
     {
       int n = (&dest[i] - dest) % 8;
       if (n > 0)
       {
         memmove(dest, &dest[n], d_idx);
         d_idx -= n;
       }
       break;
     }
  }

  if (d_idx % 8)
  {
    size_t idx = d_idx / 8 * 8;
    nResidualBytes = d_idx % 8;
    memcpy(residualBytes, &dest[idx], nResidualBytes);
  }

  return d_idx / 8;     // return number of slices.
}

/* -------------------------------------------------------------------- */
void MainCanvas::drawCIP(P2d_rec *record, struct recStats &stats, float version, int probeNum, PostScript *ps)
{
  Particle	*cp;
  int		nextColor, cntr = 0;
  bool		colorIsBlack = false;
  unsigned long long *p;

  static unsigned long prevTime;
  static P2d_rec prevRec;

  if (memcmp((void *)record, (void *)&prevRec, sizeof(P2d_rec)) == 0)
    stats.duplicate = true;

  unsigned char image[16000];
  size_t nSlices = uncompressCIP(image, record->data, 4096);

  /**
   * If using the View->Raw Data menu item, or if no particles were detected
   * in the record, then come in here and do a raw display of the record.
   * Only color code timing (green) and overload (blue) words.
   */
  if (displayMode == RAW_RECORD || (cp = (Particle *)stats.particles.Front()) == NULL)
  {
    p = (unsigned long long *)image;
    for (size_t i = 0; i < nSlices; ++i, ++p)
    {
      if (*p  == CIP_Sync)
      {
        if (ps) ps->SetColor(color->GetColorPS(GREEN));
        else pen->SetColor(color->GetColor(GREEN));
      }

      drawSlice(ps, i, *p);
      if (ps) ps->SetColor(color->GetColorPS(BLACK));
      else pen->SetColor(color->GetColor(BLACK));
    }

    if (displayMode == RAW_RECORD)
      y += 66;	// Add enough room for a second copy of this record.
    else
    {
      y += 32;	// Bail out (no particles detected from process.cc).
      return;
    }
  }

//  if ((cp = (Particle *)stats.particles.Front()) == NULL)
//    return;

  p = (unsigned long long *)image;
  for (size_t i = 0; i < nSlices; )
  {
    if (cp == 0 || cp->reject)
      nextColor = 0;	// black.
    else
      nextColor = probeNum;

    if (*p == CIP_Sync)
    {
      /**
       * Color code timing words:
       * 	Green = accepted.
       * 	Yellow = zero area image (i.e. timing bar with no visible particle).
       * 	Red = rejected.
       * 	Blue = overload word, also rejected.
       */
      if (cp && cp->reject) {
        if (cp->h == 0 || cp->w == 0)
          if (ps) ps->SetColor(color->GetColorPS(YELLOW));
          else pen->SetColor(color->GetColor(YELLOW));
        else
          if (ps) ps->SetColor(color->GetColorPS(RED));
          else pen->SetColor(color->GetColor(RED));
      }
      else {	// Green for good particle
        if (ps) ps->SetColor(color->GetColorPS(GREEN));
        else pen->SetColor(color->GetColor(GREEN));
      }

      if (timingWord)
        drawSlice(ps, i, *p);
      ++i; ++p;

      // Get next particle.
      cp = (Particle *)stats.particles.DeQueue();
      delete cp;
      cp = (Particle *)stats.particles.Front();
    }
    else
    {
      if (ps) ps->SetColor(color->GetColorPS(nextColor));
      else pen->SetColor(color->GetColor(nextColor));

      if (nextColor == 0)	// if Black
        colorIsBlack = true;
      else
        colorIsBlack = false;

      for (; i < nSlices && *p != CIP_Sync; ++p)
        drawSlice(ps, i++, *p);

      if (enchiladaWin)
        enchiladaWin->AddLineItem(cntr++, cp);
    }
  }

/*
// For diagnostics, display record as is on 2nd half of screen/window.
  for (size_t i = 0; i < nSlices_64bit; ++i)
{ if (((unsigned long long *)record->data)[i] == 0xffffffffffffffffLL) { pen->SetColor(color->GetColor(YELLOW)); ((unsigned long long *)record->data)[i] = 0; }
    drawSlice(ps, nSlices_64bit+i, ((unsigned long long *)record->data)[i]);
pen->SetColor(color->GetColor(0)); }
*/

  if (displayMode == DIAGNOSTIC)
    drawDiodeHistogram(record);
  else
    drawAccumHistogram(stats);

  y += 32;
  stats.prevTime = prevTime;
  prevTime = stats.thisTime;
  memcpy((void *)&prevRec, (void *)record, sizeof(P2d_rec));
}

/* -------------------------------------------------------------------- */
void MainCanvas::drawHVPS(P2d_rec *record, struct recStats &stats, float version, int probeNum, PostScript *ps)
{
  size_t	y1 = 0, cntr = 0, shaded, unshaded, line = LEFT_MARGIN;
  unsigned short	*sp = (unsigned short *)record->data;
  Particle	*cp;

  static uint32_t	prevTime;
  static P2d_rec	prevRec;

  if (memcmp((void *)record, (void *)&prevRec, sizeof(P2d_rec)) == 0)
    stats.duplicate = true;

  if (*sp == 0xcaaa)
    {
//      printf("MainCanvas:draw: diagnostic record.\n");
    return;
    }

//printf("--------------------------------\n");
  pen->SetColor(color->GetColor(RED));
//    pen->DrawLine(Surface(), line, y+HVPS_MASKED, 1600, y+HVPS_MASKED);
  pen->DrawLine(Surface(), line, y+(128-HVPS_MASKED), 1600, y+(128-HVPS_MASKED));
//    pen->DrawLine(Surface(), line, y+256-HVPS_MASKED, 1600, y+256-HVPS_MASKED);

  for (size_t i = 0; i < 2048; )
    {
    if (wrap && line >= Width()-(LEFT_MARGIN<<1))
      {
      line = LEFT_MARGIN;
      y += 190 + PIX_PER_Y - (HVPS_MASKED<<1);

      if (ps) {
        ps->SetColor(color->GetColorPS(RED));
        }
      else {
        pen->SetColor(color->GetColor(RED));
        }

      pen->DrawLine(Surface(), line, y+(128-HVPS_MASKED), line+1500, y+(128-HVPS_MASKED));
      }

    if (sp[i] & 0x8000)
      {
//      if (!(sp[i+1] & 0x8000))
//        printf("Missaligned timing words.\n");

      if ((cp = (Particle *)stats.particles.DeQueue()) == NULL)
        break;

      line += 2;

      if (timingWord)
        {
        if (cp && cp->reject) {
          if (ps) ps->SetColor(color->GetColorPS(RED));
          else pen->SetColor(color->GetColor(RED));
          }
        else {
          if (ps) ps->SetColor(color->GetColorPS(GREEN));
          else pen->SetColor(color->GetColor(GREEN));
          }

        pen->DrawLine(Surface(), line, y, line, y+256-(HVPS_MASKED<<1));
        }

      if (ps) ps->SetColor(color->GetColorPS(BLUE));
      else pen->SetColor(color->GetColor(BLUE));

//printf("\ntiming: %d, ovrld=%x - %x", (((uint32_t)sp[i] << 15) & 0x3fff0000)+(uint32_t)(sp[i+1] & 0x3fff), sp[i] & 0xc000, sp[i+1] & 0xc000);

      i += 2;

      for (; !(sp[i] & 0x8000) && i < 2048; ++i)
        {
        shaded = (sp[i] & 0x3f80) >> 7;
        unshaded = sp[i] & 0x007f;

        if (sp[i] & 0x4000)
          {
          y1 = y;

          if (sp[i] == 0x4000)
            y1 += 128;

          line += 1;	// Max airspeed is 45 m/s.  Cheesy multiply by 2.
          unshaded -= HVPS_MASKED;
//printf("\n %4d", i);
          }

//if (sp[i] == 0x4000)
//  printf(" 128   0");
//else
//  printf(" %3d %3d", unshaded, shaded);

        if (shaded > 0)
          {	// Max airspeed is 45 m/s.  Cheesy multiply by 2.  Draw lines twice.
          pen->DrawLine(Surface(), line, y1 + unshaded,
			line, y1 + unshaded + shaded);
//          pen->DrawLine(Surface(), line + 1, y1 + unshaded,
//			line + 1, y1 + unshaded + shaded);
          }

        y1 += unshaded + shaded;
        }

      if (enchiladaWin)
        enchiladaWin->AddLineItem(cntr++, cp);

      delete cp;
      }
    else
      ++i;
    }

  y += 224 - (HVPS_MASKED<<1);
  stats.prevTime = prevTime;
  prevTime = stats.thisTime;
  memcpy((void *)&prevRec, (void *)record, sizeof(P2d_rec));
}

/* -------------------------------------------------------------------- */
void MainCanvas::draw_2DC_as_2DP(P2d_rec *record)
{
  if (((char *)&record->id)[0] != 'C')
    return;

  XPoint pts[512];
  int p_cnt = 0;

  for (size_t i = 0; i < 8; ++i)
  {
    for (size_t j = 0; j < 8; ++j)
    {
      int cntr = 0;
      for (size_t k = 0; k < 8; ++k)
      {
        for (size_t l = 0; l < 8; ++l)
          if (part1[i*8+k][j*8+l])
            ++cntr;
      }

      if (cntr > 33)
      {
        pts[p_cnt].x = 530 + part2slice;
        pts[p_cnt].y = y + (7-j);
        ++p_cnt;
      }
    }
    if (p_cnt)
      ++part2slice;
  }

  if (p_cnt)
  {
    pen->SetColor(color->GetColor(BLUE));
    pen->DrawPoints(Surface(), pts, p_cnt);
    ++part2slice;
  }

  // Reset data for next particle.
  part1slice = 0;
  memset(part1, 0, sizeof(part1));
}

/* -------------------------------------------------------------------- */
void MainCanvas::drawAccumHistogram(struct recStats &stats)
{
  pen->SetColor(color->GetColor(BLACK));

  for (size_t i = 0; i < 128; ++i)
  {
    if (stats.accum[i] > 0)
      pen->DrawLine(Surface(), 800+i, y+64, 800+i, y+(64-(stats.accum[i]*2)));
  }
}

/* -------------------------------------------------------------------- */
void MainCanvas::drawDiodeHistogram(P2d_rec *record, uint32_t syncWord)
{
  int histo[32];
  memset(histo, 0, sizeof(histo));

  unsigned long *p = (unsigned long *)record->data;
  for (size_t i = 0; i < nSlices_64bit; ++i, ++p)         /* 2DC and/or 2DP       */
  {
    if ((*p & SyncWordMask) == syncWord)
      continue;

    for (size_t j = 0; j < 32; ++j)
      if ( !((*p >> j) & 0x00000001) )
        histo[j]++;
  }

  pen->SetColor(color->GetColor(BLACK));

  for (size_t i = 0; i < 32; ++i)
  {
    if (histo[i] > 0)
      pen->DrawLine(Surface(), 1050, y+(31-i), 1050+histo[i], y+(31-i));
  }
}

/* -------------------------------------------------------------------- */
void MainCanvas::drawDiodeHistogram(P2d_rec *record)
{
  int histo[64];
  memset(histo, 0, sizeof(histo));

  unsigned long long *p = (unsigned long long *)record->data;
  for (size_t i = 0; i < nSlices_64bit; ++i, ++p)         /* 2DC and/or 2DP       */
  {
    if ((*p & Fast2D_Mask) == Fast2D_Sync || (*p & Fast2D_Mask) == Fast2D_Overld)
      continue;

    for (size_t j = 0; j < 64; ++j)
      if ( !((*p >> j) & 0x00000001) )
        histo[j]++;
  }

  pen->SetColor(color->GetColor(BLACK));

  for (size_t i = 0; i < 64; ++i)
  {
    if (histo[i] > 0)
      pen->DrawLine(Surface(), 800, y+(63-i), 800+histo[i], y+(63-i));
  }
}

/* -------------------------------------------------------------------- */
void MainCanvas::enchiladaLineItem(PostScript *ps, int i, int cnt, Particle *cp)
{
  int	h, m, s;

  if (ps) ps->SetColor(color->GetColorPS(BLACK));
  else pen->SetColor(color->GetColor(BLACK));

  if (cnt == 0)	// Print title.
    {
    strcpy(buffer, " #     Time       timeWord  rj  iy  ix  ia    dt   live");

    if (ps) {
       ps->MoveTo(5, 750); ps->ShowStr(buffer); }
    else
       pen->DrawText(Surface(), 5, 120, buffer);
    }


  h = cp->time / 3600;
  m = (cp->time - (h*3600)) / 60;
  s = cp->time - (h*3600) - (m*60);

  // Particle #, time stamp, timeword, reject, h, w, a
  sprintf(buffer, "%03d %02d:%02d:%02d.%03ld  %8u %2d %3zu %3zu %3zu %6u %6u",
	cnt, h, m, s, cp->msec, cp->timeWord, cp->reject, cp->h, cp->w,
	cp->area, cp->deltaTime, cp->liveTime);

  if (ps) {
    ps->MoveTo(5, 750-(y+80+(cnt*15))); ps->ShowStr(buffer); }
  else
    pen->DrawText(Surface(), 5, 137+(cnt*15), buffer);
}


/* -------------------------------------------------------------------- */
int MainCanvas::memchrcmp(const void *s1, const int c, size_t n)
{
  for (size_t i = 0; i < n; ++i)
    if ((((char *)s1)[i] & 0xff) != c)
      return 1;

  return 0;
}

/* -------------------------------------------------------------------- */
void MainCanvas::drawSlice(PostScript *ps, int x, const unsigned char *slice, size_t nDiodes)
{
  assert(nDiodes >= 32 && (nDiodes % 32) == 0);

  size_t	cnt = 0;
  size_t	nBytes = nDiodes / 8;
  XPoint	pts[nDiodes];

  if (memchrcmp(slice, 0xff, nBytes) == 0)
    return;

  if (ps)
    {
    size_t first;

    ps->MoveTo(x+37, 750-y-nDiodes);

    if (memchrcmp(slice, 0x00, nBytes) == 0)
      ps->rLineTo(0, nDiodes);
    else
    for (int i = nBytes-1; i >= 0; ++i)
      {
      for (size_t j = 0; j < 8; )
        {
        cnt = 1;
        first = (slice[i] >> j) & 0x01;

        while (((slice[i] >> ++j) & 0x01) == first && j < 8)
          ++cnt;

        if (first)
          ps->rMoveTo(0, cnt);
        else
          ps->rLineTo(0, cnt);
        }
      }
    }
  else
    {
    size_t y1 = y;
    // Count down due to byte swapping.
    for (int i = nBytes-1; i >= 0; --i, y1 += 8)
      {

      for (size_t j = 0; j < 8; ++j)
        if ( !((slice[i] >> j) & 0x01) )
          {
          pts[cnt].x = x + 12;
          pts[cnt].y = y1 + (7-j);
          ++cnt;
          }
      }

    pen->DrawPoints(Surface(), pts, cnt);
    }
}

/* -------------------------------------------------------------------- */
void MainCanvas::drawSlice(PostScript *ps, int i, uint32_t slice)
{
  if (slice == 0xffffffff)
    return;

  size_t	cnt = 0;
  XPoint	pts[32];

  if (ps)
    {
    size_t first;

    ps->MoveTo(i+37, 750-y-32);

    if (slice == 0x00000000)
      ps->rLineTo(0, 32);
    else
    for (size_t j = 0; j < 32; )
      {
      cnt = 1;
      first = (slice >> j) & 0x00000001;

      while (((slice >> ++j) & 0x00000001) == first && j < 32)
        ++cnt;

      if (first)
        ps->rMoveTo(0, cnt);
      else
        ps->rLineTo(0, cnt);
      }
    }
  else
    {
    for (size_t j = 0; j < 32; ++j)
      if ( !((slice >> j) & 0x00000001) )
        {
        pts[cnt].x = i + 12;
        pts[cnt].y = y + (31-j);
        ++cnt;
        }

    pen->DrawPoints(Surface(), pts, cnt);
    }
}

/* -------------------------------------------------------------------- */
void MainCanvas::drawSlice(PostScript *ps, int i, unsigned long long slice)
{
//  if (slice == 0xffffffffffffffffLL)
//    return;

  size_t	cnt = 0;
  XPoint	pts[64];

  if (ps)
    {
    size_t first;

    ps->MoveTo(i+37, 750-y-64);

    if (slice == 0x00000000)
      ps->rLineTo(0, 64);
    else
    for (size_t j = 0; j < 64; )
      {
      cnt = 1;
      first = (slice >> j) & 0x00000001;

      while (((slice >> ++j) & 0x00000001) == first && j < 64)
        ++cnt;

      if (first)
        ps->rMoveTo(0, cnt);
      else
        ps->rLineTo(0, cnt);
      }
    }
  else
    {
    for (size_t j = 0; j < 64; ++j)
      if ( !((slice >> j) & 0x00000001) )
        {
        pts[cnt].x = i + 12;
        pts[cnt].y = y + (63-j);
        ++cnt;
if ((slice & Fast2D_Mask) != Fast2D_Sync && (slice & Fast2D_Mask) != Fast2D_Overld)
  part1[part1slice][j] = true;
        }

    pen->DrawPoints(Surface(), pts, cnt);
if ((slice & Fast2D_Mask) != Fast2D_Sync && (slice & Fast2D_Mask) != Fast2D_Overld)
  ++part1slice;
    }
}

/* END MAINCANVAS.CC */
