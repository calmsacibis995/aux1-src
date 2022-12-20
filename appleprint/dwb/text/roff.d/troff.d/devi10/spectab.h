#ifndef lint	/* .../appleprint/dwb/text/roff.d/troff.d/devi10/spectab.h */
#define _AC_NAME spectab_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:06:23}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *spectab_h_sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of spectab.h on 87/11/11 22:06:23";
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
 * Control strings used by the post-processor when it is building up
 * characters. The format of the strings is, an optional motion followed
 * by a character that we want to print. The ordered pair (dx,dy) defines
 * a motion. dx and dy specify relative horizontal and vertical movements
 * in terms of a percentage of the width of the character we are trying to
 * build up. Both dx and dy must be integers. If you decide to change this
 * syntax, routine dostring() will have to be rewritten.
 *
 */

char *spectab[] = {

"fi", "f(55,0)i",
"fl", "f(60,0)l",
"ff", "f(50,0)f",
"Fi", "f(33,0)f(45,0)i",
"Fl", "f(33,0)f(45,0)l",
0, 0,

};

