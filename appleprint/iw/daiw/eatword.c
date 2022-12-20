#ifndef lint	/* .../appleprint/iw/daiw/eatword.c */
#define _AC_NAME eatword_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:19}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _eatword_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of eatword.c on 87/11/11 21:44:19";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	eatword(ifile,buffer)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *
 *	buffer	Pointer to a place to put the characters that make up the
 *		word being eaten.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This functions eats a text word from the input file ifile. Ignores
 *	leading blanks, looking for printable ASCII. Places ASCII into the
 *	buffer (pointed to by buffer). Terminates buffer with a null.
 *	Stops eating (and placing into the buffer) when encountering
 *	the first non-printable character.
 *
 *  Algorithm:
 *	Eat past any leading blanks, stopping at first non-blank character.
 *
 *	Save first non-blank character.
 *
 *	While getting characters,
 *	    If the character is in the ASCII range, save it,
 *	    else break out of this for loop.
 *
 *	Put a null at the end of the buffer.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void eatword(ifile,buffer)
	register FILE  *ifile;
	register char  *buffer;
	{
	register int	c;

	while ((c = eatc(ifile)) == ' ')
	    {
	    }
	*buffer++ = c;
	while ((c = eatc(ifile)) != EOF)
	    {
	    if ('\041' <= c  &&  c <= '\176')
		{
		*buffer++ = c;
		}
	    else
		{
		break;
		}
	    }
	*buffer = '\0';
	}
