#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/trans.c */
#define _AC_NAME trans_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:07:32}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of trans.c on 87/11/11 21:07:32";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

/*
 *	Streams translation driver
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

static int trans_open();
static int trans_close();
static int trans_rput();
static int trans_wput();

extern nulldev();

static struct 	module_info trans_info = { 93, "trans", 0, 256, 256, 256, NULL };
static struct	qinit transrdata = { trans_rput, NULL, trans_open, trans_close,
			nulldev, &trans_info, NULL};
static struct	qinit transwdata = { trans_wput, NULL, trans_open, trans_close, 
			nulldev, &trans_info, NULL};
struct	streamtab transinfo = {&transrdata, &transwdata, NULL, NULL};

static int
trans_open(q, dev, flag, sflag, err)
queue_t *q;
int *err;
dev_t dev;
{
	return(0);
}

static int
trans_close(q, flag)
queue_t *q;
{
}

static int
trans_rput(q, m)
queue_t *q;
mblk_t *m;
{
	putnext(q, m);
}

static int
trans_wput(q, m)
queue_t *q;
mblk_t *m;
{
	register mblk_t *mp;
	register char *cp;

	mp = m;
	while (mp) {
		cp = mp->b_rptr;
		while (cp < mp->b_wptr) {
			if (*cp >= 'a' && *cp <= 'z') {
				*cp = *cp + 'A' - 'a';
			} else
			if (*cp >= 'A' && *cp <= 'Z') {
				*cp = *cp + 'a' - 'A';
			}
			cp++;
		}
		mp = mp->b_next;
	}
	putnext(q, m);
}
