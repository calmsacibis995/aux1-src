#ifndef lint	/* .../appleprint/iw/daiw/do_cx.c */
#define _AC_NAME do_cx_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:43}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_cx_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_cx.c on 87/11/11 21:43:43";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_cx(ifile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 * 	This functions handles the ditroff output language `cx' command,
 *	where `x' is a single ASCII character.
 *
 *  Algorithm:
 *	Get the character.
 *	Call stuffc() to put it into the current CC structure.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_cx(ifile)
	FILE   *ifile;
	{
	int	c;

	c = eatc(ifile);
	stuffc(c,0,0,0);
	}
