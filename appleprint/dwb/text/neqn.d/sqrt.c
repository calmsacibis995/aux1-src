#ifndef lint	/* .../appleprint/dwb/text/neqn.d/sqrt.c */
#define _AC_NAME sqrt_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:55:53}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of sqrt.c on 87/11/11 21:55:53";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
# include "e.h"

sqrt(p2) int p2; {
	yyval = p2;
	nrwid(p2, ps, p2);
	printf(".ds %d \\v'%du'\\e\\L'%du'\\l'\\n(%du'",
		p2, ebase[p2], -eht[p2], p2);
	printf("\\v'%du'\\h'-\\n(%du'\\*(%d\n", eht[p2]-ebase[p2], p2, p2);
	eht[p2] += VERT(1);
	if(dbg)printf(".\t\\\" sqrt: S%d <- S%d;b=%d, h=%d\n", 
		p2, p2, ebase[p2], eht[p2]);
}
