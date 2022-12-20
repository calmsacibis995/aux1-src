#ifndef lint	/* .../appleprint/iw/daiw/do_Hn.c */
#define _AC_NAME do_Hn_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:37}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_Hn_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_Hn.c on 87/11/11 21:43:37";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_Hn(ifile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This functions handles the ditroff output language `Hn' command,
 *	where `n' is one or more digits. The `Hn' command specifies
 *	an absolute horizontal position to go to. The absolute horizontal
 *	position is stored in a global variable. Whenever actual characters
 *	are then stored in a CC structure, the absolute horizontal position
 *	is picked up from the global variable at that time.
 *
 *  Algorithm:
 *	Global variable as returned by the eatnnn() function, which `eats'
 *	the numeric value from the ditroff output language input stream.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_Hn(ifile)
    FILE   *ifile;
{
    tpos_horz = eatnnn(ifile);
    pos_horz = tpos_horz / dev_scale;
}
