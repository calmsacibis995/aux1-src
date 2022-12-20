#ifndef lint	/* .../appleprint/dwb/text/subndx.d/strr.c */
#define _AC_NAME strr_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:13:04}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of strr.c on 87/11/11 22:13:04";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
/*
	str_rev, given a string, returns a string which is the reverse.
	For example, given "word", "drow" is returned.
*/

#define DEBUG_STRR 0

strn_rev (str, rstr, n)

char	*str, *rstr;
int	n;					/*length of initial substring*/

{	
	int	i, j;

	j = 0;
	for (i = n - 1; i >= 0; i--) {
		*(rstr + j) = *(str + i);
		j++;
	}
	*(rstr + j) = '\0';
	if (DEBUG_STRR)
		printf ("\n strr.c: str: %s rstr: %s", str, rstr);
}


