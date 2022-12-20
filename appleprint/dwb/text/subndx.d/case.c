#ifndef lint	/* .../appleprint/dwb/text/subndx.d/case.c */
#define _AC_NAME case_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:10:42}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of case.c on 87/11/11 22:10:42";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
/*
	ulcase searches the input string for any upper case letters and
	changes them to lower case. It returns a 1 if a change is made,
	and 0 if none.
*/


ulcase (string)
char	*string;
{
	int	chg = 0;
	int	lcase = 'a' - 'A';
	char	*ch ;

	ch = string;					/*get first character*/
	while (*ch != '\0') {
		if (*ch >= 'A' && *ch <= 'Z')		/*if upper case */ {
			*ch += lcase;			/*change to lower case*/
			chg = 1;
		}
		ch++;					/*get next character in string*/
	}
	return (chg);
}


