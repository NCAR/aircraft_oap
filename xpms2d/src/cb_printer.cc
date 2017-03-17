/*
-------------------------------------------------------------------------
OBJECT NAME:	cb_printer.cc

FULL NAME:	Callbacks to PrinterSetup

ENTRY POINTS:	EditPrintParms()
		ApplyPrintParms()
		SetPrinterName()

DESCRIPTION:	none

COPYRIGHT:	University Corporation for Atmospheric Research, 1997-8
-------------------------------------------------------------------------
*/

#include <raf/Printer.h>

extern Printer		*printerSetup;


/* -------------------------------------------------------------------- */
void EditPrintParms(Widget w, XtPointer client, XtPointer call)
{
  printerSetup->PopUp();

}	/* END EDITPRINTPARMS */

/* -------------------------------------------------------------------- */
void ApplyPrintParms(Widget w, XtPointer client, XtPointer call)
{
  printerSetup->ApplyParms();

}	/* END APPLYPRINTPARMS */

/* -------------------------------------------------------------------- */
void SetPrinterName(Widget w, XtPointer client, XtPointer call)
{
  printerSetup->SetPrinter((long)client);

}       /* END SETPRINTERNAME */

/* END CB_PRINTER.CC */
