#ifndef lint	/* .../appleprint/iw/daiw/do_sn.c */
#define _AC_NAME do_sn_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:01}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_sn_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_sn.c on 87/11/11 21:44:01";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_sn(ifile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This functions handles the ditroff output language `sn' command,
 *	which changes the point size.
 *
 *  Algorithm:
 *	Remember the old point size by copying the current point size to it.
 *
 *	Eat the new point size and set the current point size equal to it.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_sn(ifile)
	FILE   *ifile;
	{
	int	old_ptsz;

	old_ptsz = pointsz;
	pointsz = eatnnn(ifile);
	if (sw_debug[0])
	    {
	    (void) fprintf(stderr,
                           "old point size = %d, current point size = %d\n",
                           old_ptsz,pointsz);
	    }
	}
