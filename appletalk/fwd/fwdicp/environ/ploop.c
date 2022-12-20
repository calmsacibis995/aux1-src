#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/ploop.c */
#define _AC_NAME ploop_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:07:20}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of ploop.c on 87/11/11 21:07:20";
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

#ifndef NULL
#define NULL	0
#endif NULL

static int ploop_open();
static int ploop_close();

static int ploop_open();
static int ploop_close();
static int ploop_putp();

extern nulldev();

static struct 	module_info ploop_info = { 93, "ploop", 0, 256, 256, 256, NULL };
static struct	qinit plooprdata = { putq, NULL, ploop_open, ploop_close,
			nulldev, &ploop_info, NULL};
static struct	qinit ploopwdata = { ploop_putp, NULL, ploop_open, ploop_close, 
			nulldev, &ploop_info, NULL};
struct	streamtab ploopinfo = {&plooprdata, &ploopwdata, NULL, NULL};

static int
ploop_open(q, dev, flag, sflag, err)
queue_t *q;
int *err;
{
	return(93);
}

static int
ploop_close(q, flag)
queue_t *q;
{
}

static int
ploop_putp(q, m)
queue_t *q;
mblk_t *m;
{
	switch (m->b_datap->db_type) {
	case M_DATA:
		putnext(OTHERQ(q), m);
		break;
	default:
		freemsg(m);
	}
}
