#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/clock.c */
#define _AC_NAME clock_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:06:32}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of clock.c on 87/11/11 21:06:32";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)clock.c	UniPlus VVV.2.1.8	*/

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/param.h"
#include "sys/psl.h"
#include "sys/sysmacros.h"
#include "sys/callout.h"
#include "sys/time.h"
#include "sys/reg.h"
#include <sys/errno.h>
#endif lint
#include "fwdicp.h"

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 */

time_t	lbolt;
extern int switching;
extern char queueflag;

struct callout	callout[NCALL];

clock_init()
{

	/*
	 * Initialize callouts
	 */
	int	i;

	calltodo.c_next = NULL;
	callfree = &callout[0];
	for (i = 1; i < NCALL; i++)
		callout[i-1].c_next = &callout[i];
}


clock(ap)
struct args *ap;
{
	register a;
	int i;
	register struct callout *p1;

	/*
	 * Update real-time timeout queue.
	 * At front of queue are some number of events which are ``due''.
	 * The time to these is <= 0 and if negative represents the
	 * number of ticks which have passed since it was supposed to happen.
	 * The rest of the q elements (times > 0) are events yet to happen,
	 * where the time for each is given as a delta from the previous.
	 * Decrementing just the first of these serves to decrement the time
	 * to all events.
	 */
	p1 = calltodo.c_next;
	while(p1) {
		if(--p1->c_time >= 0)
			break;
		p1 = p1->c_next;
	}
	if (!queueflag)
		if(calltodo.c_next && calltodo.c_next->c_time <= 0)
			timein();
	lbolt++;	/* time in ticks */
}

int bkmscnt = 0;
timein()
{
	register struct callout *p1;
	register s;
	register caddr_t arg;
	register int (*func)();
	register int a;

	s = splclk();
	for (;;) {

		(void) spl7();
		if ((p1 = calltodo.c_next) == 0 || p1->c_time > 0)
			break;
		arg = p1->c_arg; func = p1->c_func; a = p1->c_time;
		calltodo.c_next = p1->c_next;
		p1->c_next = callfree;
		callfree = p1;
		(void) splclk();
		(*func)(arg, a);
	}
	(void) splx(s);
}
/*
 *  Arrange that (*fun)(arg) is called in t/hz seconds.
 */
timeout(fun, arg, t)
	int (*fun)();
	caddr_t arg;
	register int t;
{
	register struct callout *p1, *p2, *pnew;
	register int s = spl7();

	if (t == 0)
		t = 1;
	pnew = callfree;
	if (pnew == NULL)
		panic("timeout table overflow");
	callfree = pnew->c_next;
	pnew->c_arg = arg;
	pnew->c_func = fun;
	for (p1 = &calltodo; (p2 = p1->c_next) && p2->c_time < t; p1 = p2)
		if (p2->c_time > 0)
			t -= p2->c_time;
	p1->c_next = pnew;
	pnew->c_next = p2;
	pnew->c_time = t;
	if (p2)
		p2->c_time -= t;
	(void) splx(s);
}

/*
 * untimeout is called to remove a function timeout call
 * from the callout structure.
 */
untimeout(fun, arg)
	int (*fun)();
	caddr_t arg;
{
	register struct callout *p1, *p2;
	register int s;

	s = spl7();
	for (p1 = &calltodo; (p2 = p1->c_next) != 0; p1 = p2) {
		if (p2->c_func == fun && p2->c_arg == arg) {
			if (p2->c_next && p2->c_time > 0)
				p2->c_next->c_time += p2->c_time;
			p1->c_next = p2->c_next;
			p2->c_next = callfree;
			callfree = p2;
			break;
		}
	}
	(void) splx(s);
}

