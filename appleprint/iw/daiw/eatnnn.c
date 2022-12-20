#ifndef lint	/* .../appleprint/iw/daiw/eatnnn.c */
#define _AC_NAME eatnnn_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:16}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _eatnnn_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of eatnnn.c on 87/11/11 21:44:16";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	eatnnn(ifile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *  Returns:
 *	A numeric value.
 *
 *  Description:
 *	This functions `eats' the numeric digits from the next character
 *	in the input file until a non-numeric character is encountered.
 *	Any non-numeric digits in front of the numeric digits will be ignored.
 *
 *  Algorithm:
 *	Set the value to be returned to zero.
 *
 *	Eat any non-numeric digits, stopping after the first numeric
 *	digit has been eaten.
 *
 *	Set value to the numeric digit just eaten.
 *
 *	While eat a character, and it is not end of file,
 *	    If the character is not numeric, put it back
 *	    and break out of this while loop,
 *	    Else shift the value over a decade, and add to the value
 *	    the numeric value of the digit just eaten.
 *
 *	Return the numeric value.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

int eatnnn(ifile)
	register FILE  *ifile;
	{
	register int	c;
	register int	value;

	value = 0;
	while ((c = eatc(ifile)) != EOF)
	    {
	    if (isdigit(c))
		{
		break;
		}
	    }
	value = c - '0';
	while ((c = eatc(ifile)) != EOF)
	    {
	    if (!isdigit(c))
		{
		uneatc(c,ifile);
		break;
		}
	    else
		{
		value *= 10;
		value += c - '0';
		}
	    }
	return(value);
	}
