#ifndef lint	/* .../appleprint/iw/daiw/pageflush.c */
#define _AC_NAME pageflush_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:49}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _pageflush_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of pageflush.c on 87/11/11 21:44:49";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	pageflush(ofile)
 *
 *  Arguments:
 *	ofile	The FILE stream pointer for the output file where this
 *		program is placing the output onto. Not used in this
 *		function, but is passed on for subordinate functions
 *		to use.
 *
 *  Description:
 *	This function is called when the end of a page or the end of
 *	processing has been encountered in the ditroff output file.
 *	The pf_lp() function is called to run through the linked list
 *	of characters-for-the-page, and then the linked list is freed
 *	upon return from pf_lp().
 *
 *	This function is called from do_pn() when the end of a page is
 *	sensed in the ditroff output file, or from do_x() when the end of
 *	ditroff output file is sensed.
 *
 *  Algorithm:
 *	If debug switch is on,
 *	    call the debug output function.
 *
 *	Call pf_lp() to output all the characters for this page. Note
 *	that pf_lp() gets an output stream name, and the address of the
 *	head of the linked list. Someday, we may want to have multiple
 *	linked lists to give to pf_lp(), but for now we only have the one.
 *
 *	While there are still elements on the linked list,
 *	    free the linked list
 *	    and advance to the next element in the linked list.
 *
 *	Now that the linked list is empty, set the head of the linked
 *	list to empty also.
 *
 *  History:
 *	Coded October 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void pageflush(ofile)
	register FILE  *ofile;
	{
	register CC    *cc1;
	register CC    *cc2;

	if (sw_debug[1])
	    {
	    dbg_cc(cchead);
	    }
	pf_lp(ofile,cchead);
	cc1 = cchead;
	while (cc1)
	    {
	    cc2 = cc1->cc_next;
	    free(cc1);
	    cc1 = cc2;
	    }
	cchead = NULL;
	}
