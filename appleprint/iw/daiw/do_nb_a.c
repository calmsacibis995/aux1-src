#ifndef lint	/* .../appleprint/iw/daiw/do_nb_a.c */
#define _AC_NAME do_nb_a_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:52}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_nb_a_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_nb_a.c on 87/11/11 21:43:52";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_nb_a(ifile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *  Returns:
 *	(void)
 *
 *  Description:
 *	This functions handles the ditroff output language `nb a' command,
 *	which apparently does nothing! But we will scan the input and throw
 *	it away. Ahh - the mysteries of troff and ditroff continue .....
 *
 *  Algorithm:
 *	Eat the two numbers and ignore them. You figure it out.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_nb_a(ifile)
	FILE   *ifile;
	{

	(void) eatnnn(ifile);
	(void) eatnnn(ifile);
	}
