#ifndef lint	/* .../appleprint/dwb/text/roff.d/troff.d/devi10/editrast.h */
#define _AC_NAME editrast_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:04:35}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *editrast_h_sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of editrast.h on 87/11/11 22:04:35";
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
 * A few definitions needed by routines that handle the editing expressions.
 * Stuff's primarily used by the scanner.
 *
 */


#define START		0		/* just starting next statment */
#define ASSIGNOP	1
#define LPAREN		2
#define RPAREN		3
#define PLUS		4
#define MINUS		5
#define TIMES		6
#define DIVIDE		7
#define UMINUS		8
#define CONSTANT	9
#define XREF		10
#define YREF		11
#define HEIGHT		12
#define WIDTH		13
#define CHWIDTH		14
#define ENDEDIT		15		/* found "end" string - done editing */
#define BUILD		16		/* found a "build" command - ends edit */


typedef struct  {

	char	*str;			/* when we find this string */
	int	code;			/* scanner returns this value */

} Tokens;

