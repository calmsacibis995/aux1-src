#ifndef lint	/* .../appleprint/iw/daiw/do_nnc.c */
#define _AC_NAME do_nnc_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:55}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_nnc_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_nnc.c on 87/11/11 21:43:55";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_do_nnc(ifile)
 *
 *  Arguments:
 *	c1	This character is a numeric digit. Since this function
 *		is a `shorthand' notation for ditroff output, i.e., this
 *		command does NOT start with a letter, but with two digits,
 *		process() must pass the digit that was encountered to
 *		this function since it could be from `0' to `9'.
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This functions handles the ditroff output language `nnc' command,
 *	where `nn' is exactly two digits indicating the amout to move
 *	right for, and `c' is the character to print.
 *
 *  Algorithm:
 *	Eat the second digit.
 *
 *	Check the first digit for `numericness'.
 *
 *	Check the second digit for `numericness'.
 *
 *	Convert the two digits into the binary value for right
 *	motion, then update the current horizontal position by thr value.
 *
 *	Eat the next character, it is the character to be printed.
 *
 *	Stuff the character into a CC structure.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_nnc(c1,ifile)
	int	c1;
	FILE   *ifile;
	{
	int	c2;
	int	c3;
	int	value;

	c2 = eatc(ifile);
	if (!isdigit(c1))
	    {
	    badinput(c1,"do_nnc(): n[0] not numeric",ifile);
	    }
	else if (!isdigit(c2))
	    {
	    badinput(c2,"do_nnc(): n[1] not numeric",ifile);
	    }
	else
	    {
	    value  = (c1 - '0') * 10;	/* NOT PORTABLE! */
	    value +=  c2 - '0';		/* NOT PORTABLE! */
	    tpos_horz += value;
	    pos_horz = tpos_horz / dev_scale;
	    c3 = eatc(ifile);
	    stuffc(c3,0,0,0);
	    }
	}
