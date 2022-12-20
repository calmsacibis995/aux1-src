#ifndef lint	/* .../appleprint/dwb/text/eqn.d/over.c */
#define _AC_NAME over_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:51:47}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of over.c on 87/11/11 21:51:47";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
# include "e.h"

boverb(p1, p2)
	int p1, p2;
{
	int treg;
	float h, b, d;

	treg = salloc();
	yyval = p1;
	d = EM(0.3, ps);
	h = eht[p1] + eht[p2] + d;
	b = eht[p2] - d;
	dprintf(".\tS%d <- %d over %d; b=%g, h=%g\n", 
		yyval, p1, p2, b, h);
	nrwid(p1, ps, p1);
	nrwid(p2, ps, p2);
	printf(".nr %d \\n(%d\n", treg, p1);
	printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n", p2, treg, treg, p2);
	printf(".nr %d \\n(%d+.5m\n", treg, treg);
	printf(".ds %d \\v'%gm'\\h'\\n(%du-\\n(%du/2u'\\*(%d\\\n", 
		yyval, REL(eht[p2]-ebase[p2]-d,ps), treg, p2, p2);
	printf("\\h'-\\n(%du-\\n(%du/2u'\\v'%gm'\\*(%d\\\n", 
		p2, p1, REL(-(eht[p2]-ebase[p2]+d+ebase[p1]),ps), p1);
	printf("\\h'-\\n(%du-\\n(%du/2u+.1m'\\v'%gm'\\l'\\n(%du-.2m'\\h'.1m'\\v'%gm'\n", 
		 treg, p1, REL(ebase[p1]+d,ps), treg, REL(d,ps));
	ebase[yyval] = b;
	eht[yyval] = h;
	lfont[yyval] = rfont[yyval] = 0;
	sfree(p2);
	sfree(treg);
}
