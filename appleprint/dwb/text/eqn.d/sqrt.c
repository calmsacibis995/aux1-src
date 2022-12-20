#ifndef lint	/* .../appleprint/dwb/text/eqn.d/sqrt.c */
#define _AC_NAME sqrt_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:52:23}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of sqrt.c on 87/11/11 21:52:23";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
# include "e.h"

sqrt(p2)
	int p2;
{
	int nps;

	nps = ps * 0.95 * eht[p2] / EM(1.0,ps) + 0.99;	/* kludgy */
	nps = max(EFFPS(nps), ps);
	yyval = p2;
	if (ttype == DEVCAT || ttype == DEVAPS || ttype == DEVPSC || ttype == DEVIW)
		eht[yyval] = EM(1.2, nps);
	else if (ttype == DEV202)
		eht[yyval] = EM(1.1, nps);
	dprintf(".\tS%d <- sqrt S%d;b=%g, h=%g, nps=%d\n", 
		yyval, p2, ebase[yyval], eht[yyval], nps);
	printf(".as %d \\|\n", yyval);
	nrwid(p2, ps, p2);
	printf(".ds %d \\v'%gm'%s", yyval, REL(ebase[p2],ps), DPS(ps,nps));	/* proper position for sqrt */
	if (ttype == DEVCAT || ttype == DEVAPS || ttype == DEVPSC || ttype == DEVIW)
		printf("\\v'-.2m'\\(sr\\l'\\n(%du\\(rn'\\v'.2m'", p2);
	else
		printf("\\(sr\\l'\\n(%du\\(rn'", p2);
	printf("%s\\v'%gm'\\h'-\\n(%du'\\*(%d\n", DPS(nps,ps), REL(-ebase[p2],ps), p2, p2);
	lfont[yyval] = rfont[yyval] = ROM;
}
