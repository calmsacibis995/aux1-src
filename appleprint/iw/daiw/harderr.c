#ifndef lint	/* .../appleprint/iw/daiw/harderr.c */
#define _AC_NAME harderr_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:22}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _harderr_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of harderr.c on 87/11/11 21:44:22";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	harderr(hardnum,fmt,p1,p2,p3,p4,p5,p6,p7,p8)
 *
 *  Arguments:
 *	hardnum	The error number.
 *	fmt	The printf format string.
 *	p1-p8	Up to eight arguments for use by with the printf string.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This function handles the hard errors.
 *
 *  Algorithm:
 *	Really obvious, right?
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

/* VARARGS2 */
void harderr(hardnum,fmt,p1,p2,p3,p4,p5,p6,p7,p8)
	int	hardnum;
	char   *fmt;
        int	p1,p2,p3,p4,p5,p6,p7,p8;
	{
	(void) fprintf(stderr,"%s: hard error %d\n",pgm,hardnum);
	(void) fprintf(stderr,"%s: ",pgm);
	(void) fprintf(stderr,fmt,p1,p2,p3,p4,p5,p6,p7,p8);
	(void) fprintf(stderr,"\n");
	(void) fflush(stdout);
	exit(hardnum);
	}
