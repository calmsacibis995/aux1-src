#ifndef lint	/* .../appleprint/iw/daiw/eatc.c */
#define _AC_NAME eatc_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:10}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _eatc_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of eatc.c on 87/11/11 21:44:10";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	eatc(ifile)
 *
 *  Arguments:
 *	ifile	The ditroff output file that is input to this program
 *		stream I/O pointer.
 *
 *  Description:
 *	This function does the character by character reading of the
 *	input file that came from the ditroff program (the ditroff output
 *	file). The major reason that this is a function, rather than having
 *	the other functions that need to read a character call getc()
 *	directly, is that the line number, column, and absolute byte offset
 *	are kept track of for error reporting purposes. This function and
 *	uneatc() should always be thought of or modified in sync.
 *
 *  Algorithm:
 *	Read a character.
 *	Increment the total characters read count.
 *	Increment the current column counter.
 *	If the character read is a newline character,
 *	    increment the line count,
 *	    make (remember) the current column number as the old column
 *	    number in case we ever have to back up via uneatc(),
 *	    and set the column number to zero.
 *	Return the character value read.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

int eatc(ifile)
	FILE   *ifile;
	{
	int	c;

	c = getc(ifile);
	inpcount++;
	linecol++;
	if (c == '\n')
	    {
	    linecnt++;
	    oldlcol = linecol;
	    linecol = 0;
	    }
	return(c);
	}
