#ifndef lint	/* .../appleprint/iw/daiw/badinput.c */
#define _AC_NAME badinput_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:13}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _badinput_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of badinput.c on 87/11/11 21:43:13";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	badinput(c,reason,ifile)
 *
 *  Arguments:
 *	c	The bad character, i.e., the character in question.
 *	reason	A character pointer to a string describing why the
 *		character in question is in question.
 *	ifile	The input file that came from ditroff (ditroff output file).
 *
 *  Description:
 *	This function is called when a character in the input file (the
 *	output file from ditroff) is encountered that seems bad. Example,
 *	a numeric character is expected, but a non-numeric if found.
 *	For error recovery that is better than nothing, input is flushed
 *	until the next newline character is found. Of course, this is not
 *	much in the way of error recovery, but it is better than nothing.
 *
 *  Algorithm:
 *	Print out error message.
 *	Count the number of times this function has been called.
 *	If this function has been called more than 5 times,
 *	    Call the hard error routine and give up.
 *	else
 *	    Call the soft error routine.
 *	    Flush input until a newline character is found.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void badinput(c,reason,ifile)
	int	c;
	char   *reason;
	FILE   *ifile;
	{
	int	   c2;
	static int times_called = 0;

	c &= 0377;
	softerr(222,"bad input character %c (\\%03o) at line %d column %d (%s)",
		('\041' <= c && c <= '\176') ? c : 040,
                c,linecnt,linecol,reason);
	times_called++;
	if (times_called > 5)
	    {
	    harderr(223,"too many badinput() calls, I give up");
	    }
	else
	    {
	    softerr(224,"flushing until \\n");
	    while ((c2=eatc(ifile)) != '\n'  &&  c2 != EOF)
		{
		}
	    }
	}
