#ifndef lint	/* .../appleprint/dwb/text/neqn.d/integral.c */
#define _AC_NAME integral_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:55:13}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of integral.c on 87/11/11 21:55:13";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
# include "e.h"
# include "e.def"

integral(p, p1, p2) {
#ifndef	NEQN
	if (p1 != 0)
		printf(".ds %d \\h'-0.4m'\\v'0.4m'\\*(%d\\v'-0.4m'\n", p1, p1);
	if (p2 != 0)
		printf(".ds %d \\v'-0.3m'\\*(%d\\v'0.3m'\n", p2, p2);
#endif
	if (p1 != 0 && p2 != 0)
		shift2(p, p1, p2);
	else if (p1 != 0)
		bshiftb(p, SUB, p1);
	else if (p2 != 0)
		bshiftb(p, SUP, p2);
	if(dbg)printf(".\t\\\" integral: S%d; h=%d b=%d\n", 
		p, eht[p], ebase[p]);
	lfont[p] = ROM;
}

setintegral() {
	char *f;

	yyval = oalloc();
	f = "\\(is";
	printf(".ds %d %s\n", yyval, f);
	eht[yyval] = VERT(2);
	ebase[yyval] = 0;
	lfont[yyval] = rfont[yyval] = ROM;
}
