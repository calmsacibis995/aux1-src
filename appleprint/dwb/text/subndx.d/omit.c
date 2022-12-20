#ifndef lint	/* .../appleprint/dwb/text/subndx.d/omit.c */
#define _AC_NAME omit_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:12:10}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of omit.c on 87/11/11 22:12:10";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#include <stdio.h>
#include "cnst.h"

/*given a string, omit checks to see if it is on its list of words and
  returns the integer value TRUE or FALSE*/

omit (word)

char	*word;

#define	 longest  8				/*length of longest omit word*/
{
	int	omt_tot = 11;			/*number of omit words*/
	static char	*omt[] = {  
		"all", 		/*list of omit words*/
		"both",
		"each",
		"etc.",
		"figure",
		"many",
		"section",
		"some",
		"table",
		"these",
		"this"
	};
	char	term[longest];
	int	i, found;
	int	ulcase();			/*changes string to lower case*/

	found = FALSE;
	if (strlen(word) <= longest - 1) {
		strcpy (term, word);
		ulcase (term);
		for (i = 0; i < omt_tot; i++)
			if (strcmp (term, omt[i]) == 0)
				found = TRUE;
	}
	return (found);
}


