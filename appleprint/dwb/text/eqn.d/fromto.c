#ifndef lint	/* .../appleprint/dwb/text/eqn.d/fromto.c */
#define _AC_NAME fromto_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:51:12}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of fromto.c on 87/11/11 21:51:12";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
# include "e.h"

fromto(p1, p2, p3)
	int p1, p2, p3;
{
	float b, h1, b1, t;
	int subps;

	yyval = salloc();
	lfont[yyval] = rfont[yyval] = 0;
	h1 = eht[yyval] = eht[p1];
	b1 = ebase[p1];
	b = 0;
	subps = ps;
	ps += deltaps;
	nrwid(p1, ps, p1);
	printf(".nr %d \\n(%d\n", yyval, p1);
	if (p2 > 0) {
		nrwid(p2, subps, p2);
		printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n", p2, yyval, yyval, p2);
		eht[yyval] += eht[p2];
		b = eht[p2];
	}
	if (p3 > 0) {
		nrwid(p3, subps, p3);
		printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n", p3, yyval, yyval, p3);
		eht[yyval] += eht[p3];
	}
	printf(".ds %d ", yyval);	/* bottom of middle box */
	if (p2 > 0) {
		t = eht[p2]-ebase[p2]+b1;
		printf("\\v'%gm'\\h'\\n(%du-\\n(%du/2u'%s\\*(%d%s", 
			REL(t,ps), yyval, p2, DPS(ps,subps), p2, DPS(subps,ps));
		printf("\\h'-\\n(%du-\\n(%du/2u'\\v'%gm'\\\n", 
			yyval, p2, REL(-t,ps));
	}
	printf("\\h'\\n(%du-\\n(%du/2u'\\*(%d\\h'\\n(%du-\\n(%du/2u'\\\n", 
		yyval, p1, p1, yyval, p1);
	if (p3  >0) {
		t = h1-b1+ebase[p3];
		printf("\\v'%gm'\\h'-\\n(%du-\\n(%du/2u'%s\\*(%d%s\\h'\\n(%du-\\n(%du/2u'\\v'%gm'\\\n", 
			REL(-t,ps), yyval, p3, DPS(ps,subps), p3, DPS(subps,ps), yyval, p3, REL(t,ps));
	}
	printf("\n");
	ebase[yyval] = b + b1;
	dprintf(".\tS%d <- %d from %d to %d; h=%g b=%g\n", 
		yyval, p1, p2, p3, eht[yyval], ebase[yyval]);
	sfree(p1);
	if (p2 > 0)
		sfree(p2);
	if (p3 > 0)
		sfree(p3);
}
