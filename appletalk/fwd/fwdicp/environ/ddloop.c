#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/ddloop.c */
#define _AC_NAME ddloop_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:06:36}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of ddloop.c on 87/11/11 21:06:36";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#define NODE	6
#define	NDDP	50
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

extern T_ddp;

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
#include <../at/appletalk.h>
#endif FEP
#include <sys/debug.h>

#ifndef NULL
#define NULL	0
#endif NULL

static int loop_open();
static int loop_close();
static int loop_wsrvc();

extern nulldev();

static struct 	module_info loop_info = { 97, "ddloop", 0, 4096, 4096, 256, NULL };
static struct	qinit looprdata = { putq, NULL, loop_open, loop_close,
			nulldev, &loop_info, NULL};
static struct	qinit loopwdata = { putq, loop_wsrvc, loop_open, loop_close, 
			nulldev, &loop_info, NULL};
struct	streamtab ddloopinfo = {&looprdata, &loopwdata, NULL, NULL};

struct ddloop {
	queue_t	*q;
	int	dev;
	int	close;
};

static struct ddloop ddloop[NDDP];
static int ddinc = 0x80;

static int
loop_open(q, dev, flag, sflag, err, devp
#ifdef FEP
	,acktags
#endif FEP
)
register queue_t *q;
dev_t dev, *devp;
int *err, flag, sflag;
#ifdef FEP
fwd_acktags_t *acktags;
#endif FEP
{
	register int i, j;
	register struct ddloop *ddp;
	
	if (q->q_ptr) {
		if (sflag == DEVOPEN) {
			return(0);
		} else {
			return(OPENFAIL);
		}
	}
	for (i = 0; i < NDDP; i++)
	if (ddloop[i].q == NULL) {
		ddp = &ddloop[i];
		ddp->close = 0;
		ddp->q = q;
		break;
	}
	if (ddp == NULL)
		return(OPENFAIL);
	dev = minor(dev)&0xff;
	if (sflag == CLONEOPEN) {
		for (;;) {
			j = ddinc++;
			if (ddinc > 0xff)
				ddinc = 0x80;
			for (i = 0; i < NDDP; i++)
			if (j == ddloop[i].dev) {
			if (ddloop[i].q) 
			if (ddp != &ddloop[i])
				j = -1;
				break;
			}
			if (j >= 0)
				break;
		}
		ddp->dev = j;
		WR(q)->q_ptr = q->q_ptr = (char *)ddp;
		return(j);
	} else {
		for (i = 0; i < NDDP; i++)
		if (dev == ddloop[i].dev) 
		if (ddloop[i].q) 
		if (ddp != &ddloop[i])
			return(OPENFAIL);
		ddp->dev = dev;
		WR(q)->q_ptr = q->q_ptr = (char *)ddp;
		return(0);
	}
}

static
loop_wake(q)
queue_t *q;
{
	wakeup(q);
}

static int
loop_close(q, flag)
queue_t *q;
{
	struct ddloop *ddp, *ddpx;
	int i, s;
	mblk_t *m;

	ddp = (struct ddloop *)q->q_ptr;
	if (ddp->close) {
		for (;;) {
			ddpx = &ddloop[0];
			for (i = 0; i < NDDP;i++,ddpx++)
			if (ddpx->q && ddpx->dev == 2)
				break;
			if (i >= NDDP) 
				break;
			m = allocb(sizeof(char), BPRI_HI);
			if (m) {
				m->b_datap->db_type = M_CTL;
				*m->b_wptr++ = ddp->dev;
				putnext(ddpx->q, m);
				break;
			}
			s = splclk();
			timeout(loop_wake, q, HZ/8);
			(void)sleep(q, PZERO);
			splx(s);
		}
	}
	ddp->q = NULL;
}

static int
loop_wsrvc(q)
register queue_t *q;
{
	register mblk_t *m;
	register struct ddloop *ddp;
	register struct ddloop *ddpx;
	register at_ddp_t *ddph;
	register int i, dev;
	at_ddp_cfg_t *cfgp;
	struct iocblk *iocbp;
	mblk_t *m2;

	ddp = (struct ddloop *)q->q_ptr;
	while ((m = getq(q)) != NULL) {
		switch (m->b_datap->db_type) {
		case M_IOCTL:
			iocbp = (struct iocblk *)m->b_rptr;
			switch(iocbp->ioc_cmd) {
			case AT_DDP_GET_CFG:
				if (iocbp->ioc_count != sizeof(at_ddp_cfg_t)) 
					goto iocnak;
				cfgp = (at_ddp_cfg_t *)m->b_cont->b_rptr;
				cfgp->network_up = 1;
				cfgp->this_net = 0;
				cfgp->this_node = NODE;
				m->b_datap->db_type = M_IOCACK;
				qreply(q, m);
				break;
			case AT_DDP_IS_OPEN:
				if (iocbp->ioc_count != sizeof(char))
					goto iocnak;
				dev = *(unsigned char *)m->b_cont->b_rptr;
				ddpx = &ddloop[0];
				for (i = 0; i < NDDP;i++,ddpx++){
					if (ddpx->q && ddpx->dev == dev)
						break;
				}
				if (i >= NDDP)
					goto iocnak;
				ddpx->close = 1;
				m->b_datap->db_type = M_IOCACK;
				qreply(q, m);
				break;
			default:
			iocnak:
				m->b_datap->db_type = M_IOCNAK;
				qreply(q, m);
				break;
			}
			break;

		case M_DATA:
			m2 = copymsg(m);
			freemsg(m);
			if (m2 == NULL) 
				break;
			m = m2;
			ddph = (at_ddp_t *)m->b_rptr;
			ddph->src_socket = ddp->dev;
			W16(ddph->src_net, 0);
			ddph->src_node = NODE;
			ddpx = &ddloop[0];
			if (ddph->dst_node != NODE || R16(ddph->dst_net) != 0) {
				freemsg(m);
				break;
			}
			dev = ddph->dst_socket;
			for (i = 0; i < NDDP;i++,ddpx++)
			if (ddpx->q && ddpx->dev == dev)
				break;
			if (i >= NDDP || !canput(ddpx->q->q_next)) {
				freemsg(m);
			} else {
				putnext(ddpx->q, m);
			}
			break;
		default:
			freemsg(m);
		}
	}
}
