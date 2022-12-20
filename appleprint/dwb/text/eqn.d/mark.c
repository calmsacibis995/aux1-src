#ifndef lint	/* .../appleprint/dwb/text/eqn.d/mark.c */
#define _AC_NAME mark_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:51:38}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of mark.c on 87/11/11 21:51:38";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#include "e.h"

mark(p1)
	int p1;
{
	markline = 1;
	printf(".ds %d \\k(09\\*(%d\n", p1, p1);
	yyval = p1;
	dprintf(".\tmark %d\n", p1);
}

lineup(p1)
{
	markline = 2;
	if (p1 == 0) {
		yyval = salloc();
		printf(".ds %d \\h'|\\n(09u'\n", yyval);
	}
	dprintf(".\tlineup %d\n", p1);
}
