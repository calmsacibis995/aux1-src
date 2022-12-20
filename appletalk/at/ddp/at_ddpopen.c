#ifndef lint	/* .../appletalk/at/ddp/at_ddpopen.c */
#define _AC_NAME at_ddpopen_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:57:08}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_ddpopen.c on 87/11/11 20:57:08";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)at_ddpopen.c		UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	The ddp module is multiplexing which means, there is one connection
 *	downstream (lap type 1 and 2) and multiple connection upstream. The
 *	conections upstream are called demultiplexing queues and are viewed from
 *	the user interface as files in the appletalk directory with
 *	the name "socket1" thru "socket254".
 *
 *	There are two parts to the ddp module code. That which is common to
 *	all ddp modules on the unix machine, no matter which network it is 
 *	connected to. And, that which is specific to a particular network
 *
 *	The network specific part is contained in this file. This file
 *	defines space needed and contains the open code for the module.
 */


#include "sys/types.h"
#include "sys/file.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include <sys/mmu.h>
#include <sys/seg.h>
#include <sys/time.h>
#include <sys/uio.h>

#include "sys/ioctl.h"
#include "sys/conf.h"
#include "sys/vnode.h"
#include "sys/var.h"

#include <sys/user.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <appletalk.h>
#include <at_ddp.h>
#ifdef FEP
#include <fwd.h>
#endif FEP

#ifndef NULL
#define NULL		0
#endif NULL
#define NSOCKETS	255		/* number of ddp sockets */

static int              ddp_open();
extern int              ddp_close();
extern int              ddp_open();
extern int              ddp_close();
extern int              ddp_rsrvc();
extern int              ddp_wputq();
extern int              ddp_rputq();
extern                  nulldev();

static struct module_info ddp_info =	{93, "at ddp",
					 AT_DDP_X_HDR_SIZE, AT_DDP_DATAGRAM_SIZE,
					 4096, 256, NULL};
static struct qinit    	ddprdata =	{ddp_rputq, ddp_rsrvc, ddp_open, ddp_close,
				 	 nulldev, &ddp_info, NULL};
static struct qinit     ddpwdata =	{ddp_wputq, nulldev, nulldev, nulldev,
					 nulldev, &ddp_info, NULL};
struct  streamtab       ddptab = 	{&ddprdata, &ddpwdata, NULL, NULL};

ddp_specifics_t 	ddp_specifics;

ddp_socket_t ddp_socket[NSOCKETS];

static short ddp_index = 0x80;

static int
ddp_open(q, dev, flag, sflag, err, devp
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
 	ddp_socket_t		*ddpp;
	int			s;
	mblk_t			*m;
	struct stroptions	*stroptp;
	extern			wakeup();

	if (ddp_specifics.downstreamq == NULL ||
	    !ddp_specifics.cfg.network_up)
	{
		*err = ENETDOWN;
		return(OPENFAIL);
	}
	if (sflag == CLONEOPEN) {
		for (;;) {
			dev = ddp_index++;
			if (ddp_index >= NSOCKETS)
				ddp_index = 0x80;
			if (ddp_socket[(short)dev].upstreamq == NULL) {
				ddpp = &ddp_socket[(short)dev];
				ddpp->upstreamq = q;
				break;
			}
		}
	} else {
        	dev = minor(dev);
        	if (dev >= NSOCKETS)
		{
            		*err = ENXIO;
            		return(OPENFAIL);
		}
		if (dev < AT_DDP_SOCKET_1st_DYNAMIC &&
#ifdef FEP
		    acktags->cred.cr_uid)
#else
		    u.u_uid)
#endif FEP
		{
            		*err = EACCES;
            		return(OPENFAIL);
		}
		if (q->q_ptr)
			return(0);
		ddpp = &ddp_socket[(short)dev];
		ddpp->upstreamq = q;
	}
	
	/* initialize local data structures for the r&w q's */

	q->q_ptr = OTHERQ(q)->q_ptr = (caddr_t) ddpp;
	ddpp->close = 0;
	ddpp->number = dev;
	ddpp->ddp = &ddp_specifics;

	/*
	 * must set the stream head write offset option, because we must tack on
	 * the lap header information.
	 */
	for (;;) {
		m = allocb(sizeof(struct stroptions), BPRI_LO);
		if (m) {
			m->b_datap->db_type = M_SETOPTS;
			m->b_wptr += sizeof(struct stroptions); 
			stroptp = (struct stroptions *) m->b_rptr;
			stroptp->so_flags = SO_WROFF | SO_READOPT;
			stroptp->so_wroff = AT_LAP_HDR_SIZE;
			stroptp->so_readopt = RMSGD;
			putnext(q, m);
			break;
		}
		s = splclk();
		timeout(wakeup, q, HZ/8);
		if (sleep(q, PZERO)) {
			untimeout(wakeup, q);
			splx(s);
			*err = EINTR;
			return(OPENFAIL);
		}
		splx(s);
	}

	/* initialize those things which have probably already have been
	 * initialized, but auto initialization is not possible since
	 * the sturcture is external and information loaded is static
	 */
	ddp_specifics.nsockets = NSOCKETS;
	ddp_specifics.socket0p = ddp_socket;

	return(dev);
}
