#ifndef lint	/* .../appleprint/iw/daiw/uneatc.c */
#define _AC_NAME uneatc_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:45:08}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _uneatc_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of uneatc.c on 87/11/11 21:45:08";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Copyright 6/3/87 by Apple Computer.
 *
 *  SCCS:
 *	Module:  uneatc.c
 *	Delta:   6/3/87
 *      SID:     1.2
 *
 *  Function:
 *	uneatc(c,ifile)
 *
 *  Arguments:
 *	c	The character to be put back.
 *	ifile	The ditroff output file that is input to this program
 *		stream I/O pointer.
 *
 *  Description:
 *	This function does the character by character unreading of the
 *	input file that came from the ditroff program (the ditroff output
 *	file). The major reason that this is a function, rather than having
 *	the other functions that need to put back a character call ungetc()
 *	directly, is that the line number, column, and absolute byte offset
 *	are kept track of for error reporting purposes. This function and
 *	eatc() should always be thought of or modified in sync.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void uneatc(c,ifile)
	int	c;
	FILE   *ifile;
	{

	(void) ungetc(c,ifile);
	inpcount--;
	if (c == '\n')
	    {
	    linecnt--;
	    linecol = oldlcol;
	    }
	}
