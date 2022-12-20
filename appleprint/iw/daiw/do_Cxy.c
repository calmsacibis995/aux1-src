#ifndef lint	/* .../appleprint/iw/daiw/do_Cxy.c */
#define _AC_NAME do_Cxy_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:30}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_Cxy_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_Cxy.c on 87/11/11 21:43:30";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_Cxy(ifile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This handles the ditroff `Cxy' command, where `xy' are contigous
 *	characters from a troff \(xy character specification. Whitespace
 *	terminates the `xy' specification. Thus, in troff, the `\(bu'
 *	character specification produces ditroff output command of `Cbu'.
 *	Note that in current troff, `xy' is one or two characters, but I
 *	have provided for up to four characters.
 *
 *  Algorithm:
 *	For up to the maximum number of characters in the local holding
 *	buffer, sizeof of which is determined by CCNCHR,
 *	    Get a character into the local holding buffer.
 *	    If it is NOT a printable character,
 *		Zero out the non-printing character in the holding buffer.
 *		Break.
 *
 *	For up to the maximum number of characters in the local holding
 *	buffer, continuing with the next character after the last one
 *	gotten from the code above,
 *	    Zero out the character slot in the local holding buffer.
 *
 *	Assert our assumption about CCNCHR in the next statement.
 *
 *	Pass the ASSUMED four characters in the local holding buffer to
 *	stuffc(), which will place them into the current CC structure.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_Cxy(ifile)
	FILE   *ifile;
	{
	int	c[CCNCHR];
	int	i;

	for (i = 0;  i < CCNCHR;  i++)
	    {
  	    c[i] = eatc(ifile);
	    if (!(isprint(c[i])))
		{
		c[i] = 0;
		break;
		}
	    }
	for (; i < CCNCHR; i++)
	    {
	    c[i] = 0;
	    }
	assert(CCNCHR == 4);		/* We are assuming this!             */
	stuffc(c[0],c[1],c[2],c[3]);	/* So watch out if you change CCNCHR!*/
	}
