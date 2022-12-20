#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/loop.c */
#define _AC_NAME loop_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:07:14}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of loop.c on 87/11/11 21:07:14";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*
 *	Streams loopback driver
 *
 *	Copyright 1986 Unisoft Corporation of Berkeley CA
 *
 *
 *	UniPlus Source Code. This program is proprietary
 *	with Unisoft Corporation and is not to be reproduced
 *	or used in any manner except as authorized in
 *	writing by Unisoft.
 *
 */

#include <sys/types.h>
#include <sys/stream.h>
#include <sys/sysmacros.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/mmu.h>
#ifdef PAGING
#include <sys/page.h>
#endif PAGING
#include <sys/seg.h>
#ifdef PAGING
#include <sys/region.h>
#endif PAGING
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#ifdef FEP
#include <fwd.h>
static loop_cancel();
static loop_timeout();
int loop_count = 0;
#endif FEP

#ifndef NULL
#define NULL	0
#endif NULL

static int loop_open();
static int loop_close();

static int loop_open();
static int loop_close();
static int loop_wsrvc();
static int loop_rsrvc();

extern nulldev();

static struct 	module_info loop_info = { 93, "loop", 0, 4096, 4096, 256, NULL };
static struct	qinit looprdata = { putq, loop_rsrvc, loop_open, loop_close,
			nulldev, &loop_info, NULL};
static struct	qinit loopwdata = { putq, loop_wsrvc, loop_open, loop_close, 
			nulldev, &loop_info, NULL};
struct	streamtab loopinfo = {&looprdata, &loopwdata, NULL, NULL};

static int
loop_open(q, dev, flag, sflag, err, devp
#ifdef FEP
	,acktags
#endif FEP
)
queue_t *q;
dev_t dev, *devp;
int *err, flag, sflag;
#ifdef FEP
fwd_acktags_t *acktags;
#endif FEP
{
#ifdef FEP
	register int s;

	if (minor(dev) == 9) {
		s = splclk();
		if (loop_count == 0) {
			printf("loop_open: setting up timer\n");
			timeout(loop_timeout, (int)q, 20*HZ);
		}
		loop_count++;
		splx(s);
		s = sleep(&loopinfo, 0);
		if (s)
			untimeout(loop_timeout, (int)q);
		printf("loop_open(%d): count = %d (%d)\n", acktags->id, loop_count, s);
	}
	if (minor(dev) == 10) {
		WR(q)->q_hiwat = 2;
		WR(q)->q_lowat = 1;
	}
	WR(q)->q_ptr = q->q_ptr = (char *)minor(dev);
#endif FEP
	return(93);
}

#ifdef FEP
static
loop_timeout(q)
queue_t *q;
{
	loop_count = 0;
	printf("loop_timeout: doing wakeup\n");
	wakeup(&loopinfo);
}

#endif FEP

static int
loop_close(q, flag)
queue_t *q;
{
	extern loop_timer10();

	untimeout(loop_timer10, WR(q));
	flushq(q, 1);
	flushq(WR(q), 1);
}

int loop_timing, loop_pass;

loop_timer10(q)
queue_t *q;
{
	loop_pass = 1;
	loop_timing = 0;
	qenable(q);
	printf("timer10 wakeup\n");
}

static int
loop_wsrvc(q)
queue_t *q;
{
	mblk_t *m;

	if ((int)q->q_ptr == 10) {
		printf("loop_wsrvc\n");
		if (loop_pass) {
			loop_pass = 0;
			while (m = getq(q))
				freemsg(m);
			printf("timer10 message got\n");
		} else
		if (!loop_timing) {
			timeout(loop_timer10, q, HZ*5);
			loop_timing = 1;
		}
		return;
	}
	while ((m = getq(q)) != NULL) {
		switch (m->b_datap->db_type) {
		case M_DATA:
			if (canput(OTHERQ(q)->q_next)) {
				putnext(OTHERQ(q), m);
			} else {
				putbq(q, m);
				return;
			}
			break;
		default:
			freemsg(m);
		}
	}
}

static
loop_rsrvc(q)
queue_t *q;
{
	qenable(OTHERQ(q));
}
