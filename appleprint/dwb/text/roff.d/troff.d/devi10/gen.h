#ifndef lint	/* .../appleprint/dwb/text/roff.d/troff.d/devi10/gen.h */
#define _AC_NAME gen_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:04:41}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *gen_h_sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of gen.h on 87/11/11 22:04:41";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS

/*
 *
 * A few general purpose definitons used in most of the files in this
 * package.
 *
 */


#define NON_FATAL	0		/* error classification */
#define FATAL		1

#define OFF		0
#define ON		1

#define FALSE		0
#define TRUE		1

#define BYTE		8
#define BMASK		0377

#define PI		3.141592654


#define MAX(A, B)	((A > B) ? A : B)
#define MIN(A, B)	((A < B) ? A : B)
#define ABS(A)		((A) >= 0 ? (A) : -(A))

