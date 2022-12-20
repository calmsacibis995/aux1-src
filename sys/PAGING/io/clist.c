#ifndef lint	/* .../sys/PAGING/io/clist.c */
#define _AC_NAME clist_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.3 87/11/11 21:22:35}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of clist.c on 87/11/11 21:22:35";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)clist.c	UniPlus VVV.2.1.1	*/

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/param.h"
#include "sys/ioctl.h"
#include "sys/tty.h"
#endif lint

getc(p)
register struct clist *p;
{
	register struct cblock *cp;
	register int c, s;

	s = spl6();
	if (p->c_cc > 0) {
		p->c_cc--;
		cp = p->c_cf;
		c = cp->c_data[cp->c_first++]&0377;
		if (cp->c_first == cp->c_last) {
			if ((p->c_cf = cp->c_next) == NULL)
				p->c_cl = NULL;
			cp->c_next = cfreelist.c_next;
			cfreelist.c_next = cp;
			if(cfreelist.c_flag) {
				cfreelist.c_flag = 0;
				wakeup((caddr_t)&cfreelist);
			}
		}
	} else {
		if((cp = p->c_cf) != NULL) {
			if((p->c_cf = cp->c_next) == NULL)
				p->c_cl = NULL;
			cp->c_next = cfreelist.c_next;
			cfreelist.c_next = cp;
			if(cfreelist.c_flag) {
				cfreelist.c_flag = 0;
				wakeup((caddr_t)&cfreelist);
			}
		}
		c = -1;
	}
	splx(s);
	return(c);
}

putc(c, p)
register struct clist *p;
{
	register struct cblock *cp, *ocp;
	register s;

	s = spl6();
	if ((cp = p->c_cl) == NULL || cp->c_last == cfreelist.c_size) {
		ocp = cp;
		if (cfreelist.c_next == NULL) {
			splx(s);
			return(-1);
		}
		cp = cfreelist.c_next;
		cfreelist.c_next = cp->c_next;
		cp->c_next = NULL;
		cp->c_first = cp->c_last = 0;
		if (ocp == NULL)
			p->c_cf = cp;
		else
			ocp->c_next = cp;
		p->c_cl = cp;
	}
	cp->c_data[cp->c_last++] = c;
	p->c_cc++;
	splx(s);
	return(0);
}


struct cblock *
getcf()
{
	register struct cblock *cp;
	register int s;

	s = spl6();
	if ((cp = cfreelist.c_next) != NULL) {
		cfreelist.c_next = cp->c_next;
		cp->c_next = NULL;
		cp->c_first = 0;
		cp->c_last = cfreelist.c_size;
	}
	splx(s);
	return(cp);
}

putcf(cp)
register struct cblock *cp;
{
	register int s;

	s = spl6();
	cp->c_next = cfreelist.c_next;
	cfreelist.c_next = cp;
	if(cfreelist.c_flag) {
		cfreelist.c_flag = 0;
		wakeup((caddr_t)&cfreelist);
	}
	splx(s);
}

struct cblock *
getcb(p)
register struct clist *p;
{
	register struct cblock *cp;
	register int s;

	s = spl6();
	if ((cp = p->c_cf) != NULL) {
		p->c_cc -= cp->c_last - cp->c_first;
		if (p->c_cc < 0)
			panic("bad clist count\n");
		if ((p->c_cf = cp->c_next) == NULL)
			p->c_cl = NULL;
	}
	splx(s);
	return(cp);
}

putcb(cp, p)
register struct cblock *cp;
register struct clist *p;
{
	register struct cblock *ocp;
	register int s;

	s = spl6();
	if ((ocp = p->c_cl) == NULL)
		p->c_cf = cp;
	else
		ocp->c_next = cp;
	p->c_cl = cp;
	cp->c_next = NULL;
	p->c_cc += cp->c_last - cp->c_first;
	splx(s);
	return(0);
}

/* <@(#)clist.c	6.1> */
