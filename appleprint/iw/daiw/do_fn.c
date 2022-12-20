#ifndef lint	/* .../appleprint/iw/daiw/do_fn.c */
#define _AC_NAME do_fn_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:46}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_fn_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_fn.c on 87/11/11 21:43:46";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_fn(ifile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This functions handles the ditroff output language `fn' command,
 *	(the change font command), where `n' is the font number to
 *	change to.
 *
 *  Algorithm:
 *	Save the current font number as the old font number.
 *
 *	Eat the digits following the `f' to get the new current
 *	font number.
 *
 *	If the font number is less than 1,
 *	    Issue error message.
 *	    Set font number to font number 1.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_fn(ifile)
	FILE   *ifile;
	{
	int	old_font;

	old_font = cur_font;
	cur_font = eatnnn(ifile);
	if (cur_font < 1)
	    {
	    softerr (214,"font number %d is less than 1",cur_font);
	    cur_font = 1;
	    }
	if (sw_debug[0])
	    {
	    (void) fprintf(stderr,
                           "old font = %d, current font = %d\n",
                           old_font,cur_font);
	    }
	}
