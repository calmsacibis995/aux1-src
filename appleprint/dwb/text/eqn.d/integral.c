#ifndef lint	/* .../appleprint/dwb/text/eqn.d/integral.c */
#define _AC_NAME integral_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:51:20}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of integral.c on 87/11/11 21:51:20";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#include "e.h"
#include "e.def"

integral(p, p1, p2)
{
	if (p1 != 0)
		printf(".ds %d \\h'-0.4m'\\v'0.2m'\\*(%d\\v'-0.2m'\n", p1, p1);
	if (p2 != 0)
		printf(".ds %d \\v'-0.1m'\\^\\*(%d\\v'0.1m'\n", p2, p2);
	if (p1 != 0 && p2 != 0)
		shift2(p, p1, p2);
	else if (p1 != 0)
		bshiftb(p, SUB, p1);
	else if (p2 != 0)
		bshiftb(p, SUP, p2);
	dprintf(".\tintegral: S%d; h=%g b=%g\n", p, eht[p], ebase[p]);
	lfont[p] = ROM;
}

setintegral()
{
	yyval = salloc();
	printf(".ds %d \\v'.1m'\\s+4\\(is\\s-4\\v'-.1m'\n", yyval);
	eht[yyval] = EM(1.15, ps+4);
	ebase[yyval] = EM(0.3, ps);
	eps[yyval] = ps;
	lfont[yyval] = rfont[yyval] = ROM;
}
