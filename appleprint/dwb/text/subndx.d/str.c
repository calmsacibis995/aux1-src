#ifndef lint	/* .../appleprint/dwb/text/subndx.d/str.c */
#define _AC_NAME str_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:13:02}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of str.c on 87/11/11 22:13:02";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
int
string (token, max)

char	*token;
int	max  ;		/*max. no. of non null characters allowed in token*/

{	
	char	*s;
	int	i;

	s = token;
	for (i = 0; i <= max; i++)
		if (*s++ == '\0')
			return (1);

	return (0);                   /*if no null found, return false*/
}


