#ifndef lint	/* .../appleprint/iw/daiw/do_pn.c */
#define _AC_NAME do_pn_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:58}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_pn_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_pn.c on 87/11/11 21:43:58";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_pn(ifile,ofile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *	ofile	The stream I/O pointer a FILE structure. The output file
 *		for this program as it's turns it's input (ditroff output)
 *		into output that will make the device print and typeset and
 *		all manner of interesting things.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This functions handles the ditroff output language `pn' command,
 *	which is the begin a new page command, and `n' is the new page
 *	page number.
 *
 *  Algorithm:
 *	Call pageflush() to output the current page.
 *
 *	Eat the new page number.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_pn(ifile,ofile)
	FILE   *ifile;
	FILE   *ofile;
	{

	pageno = eatnnn(ifile);
	if (pageno > 1)
	    pageflush(ofile);
	if (sw_debug[0])
	    {
	    (void) fprintf(stderr,"starting page %d\n",pageno);
	    }
	}
