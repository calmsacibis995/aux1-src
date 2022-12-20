#ifndef lint	/* .../appleprint/iw/daiw/do_Vn.c */
#define _AC_NAME do_Vn_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:40}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_Vn_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_Vn.c on 87/11/11 21:43:40";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_Vn(ifile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	Function called after the ditroff 'V' command (Vn, absolute vertical
 *	positioning, n is increasingly positive as it travels down the page)
 *	has been encountered. Eat the number after the 'V' and use it to set
 *	the new value of the current vertical poistion.
 *
 *	The easiest way to know how long the page is (so we can go to
 *	end of the page at end of page) is to remember the High Water
 *	Mark, Vertical (hwm_vert) and advance to it at page flushing
 *	time. The ditroff output language always sets a vertical position
 *	that is effectively the top of the next page at end of page time.
 *	Therefore, tracking the vertical high water mark in this function
 *	will give the vertical page size at end of page time.
 *
 *  Algorithm:
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_Vn(ifile)
    FILE   *ifile;
{
    tpos_vert = eatnnn(ifile);
    pos_vert = tpos_vert / dev_scale;
    if (tpos_vert > hwm_vert) {
	hwm_vert = tpos_vert;
    }
}
