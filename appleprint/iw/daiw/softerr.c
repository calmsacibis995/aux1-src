#ifndef lint	/* .../appleprint/iw/daiw/softerr.c */
#define _AC_NAME softerr_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:59}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _softerr_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of softerr.c on 87/11/11 21:44:59";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	softerr(softnum,fmt,p1,p2,p3,p4,p5,p6,p7,p8)
 *
 *  Arguments:
 *	softnum	The error number.
 *	fmt	The printf format string.
 *	p1-p8	Up to eight arguments for use by with the printf string.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This function handles the soft errors.
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
void softerr(softnum,fmt,p1,p2,p3,p4,p5,p6,p7,p8)
	int	softnum;
	char   *fmt;
        int	p1,p2,p3,p4,p5,p6,p7,p8;
	{
	(void) fprintf(stderr,"%s: soft error %d\n",pgm,softnum);
	(void) fprintf(stderr,"%s: ",pgm);
	(void) fprintf(stderr,fmt,p1,p2,p3,p4,p5,p6,p7,p8);
	(void) fprintf(stderr,"\n");
	(void) fflush(stderr);
	}
