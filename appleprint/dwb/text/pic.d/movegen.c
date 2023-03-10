#ifndef lint	/* .../appleprint/dwb/text/pic.d/movegen.c */
#define _AC_NAME movegen_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:57:18}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of movegen.c on 87/11/11 21:57:18";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#include	<stdio.h>
#include	"pic.h"
#include	"y.tab.h"

obj *movegen(type)
{
	static float prevdx, prevdy;
	int i, some;
	float defx, defy, dx, dy;
	obj *p;
	obj *ppos;
	static int xtab[] = { 1, 0, -1, 0 };	/* R=0, U=1, L=2, D=3 */
	static int ytab[] = { 0, 1, 0, -1 };
	Attr *ap;

	defx = getfval("movewid");
	defy = getfval("moveht");
	dx = dy = some = 0;
	for (i = 0; i < nattr; i++) {
		ap = &attr[i];
		switch (ap->a_type) {
		case TEXTATTR:
			savetext(ap->a_sub, ap->a_val.p);
			break;
		case SAME:
			dx = prevdx;
			dy = prevdy;
			some++;
			break;
		case LEFT:
			dx -= (ap->a_sub==DEFAULT) ? defx : ap->a_val.f;
			some++;
			hvmode = L_DIR;
			break;
		case RIGHT:
			dx += (ap->a_sub==DEFAULT) ? defx : ap->a_val.f;
			some++;
			hvmode = R_DIR;
			break;
		case UP:
			dy += (ap->a_sub==DEFAULT) ? defy : ap->a_val.f;
			some++;
			hvmode = U_DIR;
			break;
		case DOWN:
			dy -= (ap->a_sub==DEFAULT) ? defy : ap->a_val.f;
			some++;
			hvmode = D_DIR;
			break;
		case TO:
			ppos = ap->a_val.o;
			dx = ppos->o_x - curx;
			dy = ppos->o_y - cury;
			some++;
			break;
		case BY:
			ppos = ap->a_val.o;
			dx = ppos->o_x;
			dy = ppos->o_y;
			some++;
			break;
		case FROM:
		case AT:
			ppos = ap->a_val.o;
			curx = ppos->o_x;
			cury = ppos->o_y;
			break;
		}
	}
	if (some) {
		defx = dx;
		defy = dy;
	} else {
		defx *= xtab[hvmode];
		defy *= ytab[hvmode];
	}
	prevdx = defx;
	prevdy = defy;
	extreme(curx, cury);
	curx += defx;
	cury += defy;
	extreme(curx, cury);
	p = makenode(MOVE, 0);
	dprintf("M %g %g\n", curx, cury);
	return(p);
}
