#ifndef lint	/* .../appleprint/dwb/text/neqn.d/diacrit.c */
#define _AC_NAME diacrit_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:54:46}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of diacrit.c on 87/11/11 21:54:46";
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

diacrit(p1, type) int p1, type; {
	int c, t;

	c = oalloc();
	t = oalloc();
	nrwid(p1, ps, p1);
	printf(".nr 10 %du\n", max(eht[p1]-ebase[p1]-VERT(2),0));
	switch(type) {
		case VEC:	/* vec */
		case DYAD:	/* dyad */
			printf(".ds %d \\v'-1'_\\v'1'\n", c);
			break;
		case HAT:
			printf(".ds %d ^\n", c);
			break;
		case TILDE:
			printf(".ds %d ~\n", c);
			break;
		case DOT:
			printf(".ds %d \\v'-1'.\\v'1'\n", c);
			break;
		case DOTDOT:
			printf(".ds %d \\v'-1'..\\v'1'\n", c);
			break;
		case BAR:
			printf(".ds %d \\v'-1'\\l'\\n(%du'\\v'1'\n", 
				c, p1);
			break;
		case UNDER:
			printf(".ds %d \\l'\\n(%du'\n", c, p1);
			break;
		}
	nrwid(c, ps, c);
	printf(".as %d \\h'-\\n(%du-\\n(%du/2u'\\v'0-\\n(10u'\\*(%d", 
		p1, p1, c, c);
	printf("\\v'\\n(10u'\\h'-\\n(%du+\\n(%du/2u'\n", c, p1);
	if (type != UNDER)
		eht[p1] += VERT(1);
	if (dbg) printf(".\t\\\" diacrit: %c over S%d, h=%d, b=%d\n", type, p1, eht[p1], ebase[p1]);
	ofree(c); ofree(t);
}
