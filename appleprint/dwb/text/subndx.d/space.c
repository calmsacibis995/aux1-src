#ifndef lint	/* .../appleprint/dwb/text/subndx.d/space.c */
#define _AC_NAME space_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:12:59}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of space.c on 87/11/11 22:12:59";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#include	<stdio.h>
#include	"dstructs.h"

#define DEBUGs 0

#define BLOCKMAX 100

struct word *w_malloc ()

{
	static struct word *wblock;
	static int	wcurr = BLOCKMAX;
	unsigned	size;
	char	*malloc();

	if (DEBUGs)    
		printf ("w_alloc: called\n");
	 {
		if (wcurr == BLOCKMAX) {
			size = BLOCKMAX * (sizeof(struct word ));
			wblock = (struct word *) malloc (size);
			if (wblock == NULL)
				fprintf(stderr, "\nw_alloc: out of space");
			wcurr = 0;
		}
		if (DEBUGs)
			printf ("w_alloc: wcurr=%d\n", wcurr);
		wcurr++;
		return (wblock++);
	}
}


struct subj *s_malloc ()

{
	static struct subj *sblock;
	static int	scurr = BLOCKMAX;
	unsigned	size;
	char	*malloc();

	if (DEBUGs)    
		printf ("s_alloc: called\n");
	 {
		if (scurr == BLOCKMAX) {
			size = BLOCKMAX * (sizeof(struct subj ));
			sblock = (struct subj *) malloc (size);
			if (sblock == NULL)
				fprintf(stderr, "\ns_alloc: out of space");
			scurr = 0;
		}
		if (DEBUGs)
			printf ("s_alloc: scurr=%d\n", scurr);
		scurr++;
		return (sblock++);
	}
}


