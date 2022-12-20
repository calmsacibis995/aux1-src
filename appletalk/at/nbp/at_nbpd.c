#ifndef lint	/* .../appletalk/at/nbp/at_nbpd.c */
#define _AC_NAME at_nbpd_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:04:20}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_nbpd.c on 87/11/11 21:04:20";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)at_nbpd.c	UniPlus VVV.2.1.9	*/
/*
 * (C) 1987 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/*
 *	This file contains the two streams modules used for NBP comunication.
 *
 *	NBPD 	- this module is pushed ONLY by the NBPD daemon, it must be root
 *		  and only one daemon may have it pushed at the same time.
 *		  It accepts commands from two places .. incoming network lookups
 *		  and local requests for lookup, registration and deletion etc
 *		 
 *	NBP	- this module understands the request ioctls from user processes
 *		  all they have to do is open a socket and push it, then start
 *		  doing ioctls
 *
 *	Data structures:
 *
 *		User's request ioctl packets are linked into two queues (actually
 *		they are duped and linked into both queues). The waitingq is a
 *		queue of all the ioctls that are waiting for something to happen
 *		this is a doubly linked list. It is searched when lookup reply packets
 *		come in from DDP. We play a bit with fields in the M_IOCTL packets in
 *		this queue. The following fields are used:
 *
 *			ioc_count	The timeout retry count left
 *			ioc_rval	Flags saying what is required for
 *					completion of the ioctl
 *			ioc_error	The queue on which the ioctl was 
 *					made.
 *
 *		These fields are put back correctly before the packet is returned
 *		to the user process. 
 *			Also there are two queues of commands waiting for the NBPD
 *		server. The first is of ioctl packets waiting for their chance to
 *		be serviced. The other is of incoming network lookup requests and
 *		close notifications from DDP. (in actual fact this queue is the
 *		modules input queue, except for the one on the front which is
 *		available)
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/param.h>
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
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/sysmacros.h>
#include <sys/debug.h>
#include <fwd.h>
#include "../appletalk.h"

extern int T_nbp;

#define NBP_RETRIES	5

#define LOCAL_DONE	1
#define	REMOTE_DONE	2

/*
 *	What follows is the NBPD daemon module interface
 *	Note: in this file "nbpd_info" and "nbp_info"
 *	      are the only externally accessable names
 *	      in the whole file.     
 */

static int nbpd_open();
static int nbpd_close();
static int nbpd_rsrvc();
static int nbpd_wsrvc();
static int nbpd_rputq();
static int nbpd_wputq();
extern int nulldev();
extern int putq();

static struct 	module_info nbpd_minfo = { 98, "at_nbpd", 0, 256, 256, 256, NULL };
static struct	qinit nbpdrdata = { nbpd_rputq, nbpd_rsrvc, nbpd_open, nbpd_close,
			nulldev, &nbpd_minfo, NULL};
static struct	qinit nbpdwdata = { nbpd_wputq, nbpd_wsrvc, nbpd_open, nbpd_close, 
			nulldev, &nbpd_minfo, NULL};
struct	streamtab nbpd_info = {&nbpdrdata, &nbpdwdata, NULL, NULL};


static nbp_timeout();
static nbpd_age_bridge();

static queue_t *nbpd_q = NULL;			/* the daemon's read q */
static int nbpd_valid = 0;			/* the daemon is initialised */
static int nbpd_ioctl_waiting = 0;		/* there are ioctls waiting for at_nbpd*/
static int nbpd_more = 0;			/* there are network packets waiting */
static mblk_t *nbpd_waitingq = NULL;		/* the queue of active ioctls */
static mblk_t *nbpd_active = NULL;		/* the current ioctl being serviced */
static mblk_t *nbpd_incoming = NULL;		/* the most current waiting network
						   lookup request or close notification*/
static mblk_t *nbpd_work_head = NULL;		/* the queue containg waiting ioctls
						   to be given to the daemon */
static mblk_t *nbpd_work_tail = NULL;		/* the tail of the above Q for
						   insertion */
static unsigned nbpd_net;			/* our network number */
static unsigned nbpd_node;			/* our node number */
static unsigned nbpd_bridge;			/* our friendly neighbourhood bridge */


/*
 *	daemon open routine:
 *		- check the socket number
 *		- make sure we are not already open
 *		- make sure we are root
 *		- set up the global q pointer
 */

static int
nbpd_open(q, dev, flag, sflag, err, devp
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
	if (minor(dev) != 2) {
		*err = EPROTOTYPE;
		return(OPENFAIL);
	}
	if (nbpd_q != NULL) {
		*err = EBUSY;
		return(OPENFAIL);
	}
#ifdef FEP
	if (acktags->cred.cr_uid) {
		*err = EPERM;
		return(OPENFAIL);
	}
#else FEP
	if (u.u_uid) {
		*err = EPERM;
		return(OPENFAIL);
	}
#endif FEP
	nbpd_q = q;
	nbpd_bridge = 0;
	nbpd_node = 0;
	nbpd_net = 0;
	return(0);
}

/*
 *	On close clean up all the queues ... go through them looking for unfinished
 *		ioctls and send them packing with errors. Free up the work queues.
 */

static
nbpd_close(q, flag)
queue_t *q;
{
	register mblk_t *m, *m2;
	register int s;
	register queue_t *qx;
	register struct iocblk *iocbp;

	nbpd_q = NULL;
	nbpd_valid = 0;
	flushq(q, 1);
	flushq(WR(q), 1);
	untimeout(nbpd_age_bridge, q);
	s = splclk();
	for (m = nbpd_waitingq; m;) {
		m2 = m;
		untimeout(nbp_timeout, m2);
		m = m->b_next;
		if (m2->b_cont) {
			freemsg(m2->b_cont);
			m2->b_cont = NULL;
		}
		iocbp = (struct iocblk *)m2->b_rptr;
		qx = (queue_t *)iocbp->ioc_error;
		iocbp->ioc_error = ENETDOWN;
		iocbp->ioc_count = 0;
		m2->b_datap->db_type = M_IOCNAK;
		qreply(qx, m2);
	}
	nbpd_waitingq = NULL;
	for (m = nbpd_work_head; m;) {
		m2 = m;
		m = m->b_next;
		freemsg(m2);
	}
	nbpd_work_head = NULL;
	if (nbpd_incoming) {
		freemsg(nbpd_incoming);
		nbpd_incoming = NULL;
	}
	nbpd_active = NULL;
	splx(s);
}

/*
 *	The only write message recognised are flushes (do them now) and
 *		ioctls (put them in the Q) toss any others received
 */

static
nbpd_wputq(q, m)
queue_t *q;
register mblk_t *m;
{
	switch(m->b_datap->db_type) {
	case M_IOCTL:
		putq(q, m);
		break;
	case M_FLUSH:
		if (*m->b_rptr&FLUSHW)
			flushq(q, 1);
		if (*m->b_rptr&FLUSHR) {
			flushq(RD(q), 1);
			*m->b_rptr &= ~FLUSHW;
			qreply(q, m);
			break;
		}
		freemsg(m);
		break;
	default:
		freemsg(m);
	}
}

/*
 *	The write service routine processes the ioctls from the daemon
 */

static
nbpd_wsrvc(q)
register queue_t *q;
{
	register mblk_t *m, *m2;
	mblk_t *m3;
	register struct iocblk *iocbp;
	register int s;

	/*
	 *	loop getting messages
	 */

	while (m = getq(q)) {
		switch(m->b_datap->db_type) {
		case M_IOCTL:
			iocbp = (struct iocblk *)m->b_rptr;
			switch(iocbp->ioc_cmd) {
			case AT_NBPD_GET_NEXT:

				/*
				 *	The daemon makes this call at the start of
				 *	the main loop. It expects a command to be
				 *	returned in the rval field.
				 */

				if (m->b_cont) {
					freemsg(m->b_cont);
					m->b_cont = NULL;
				}

				if (nbpd_incoming) {

					/*
				 	 *	If there are incoming requests do them
					 *	first (they are relatively time
					 *	critical). If more are waiting wait up
					 *	the read q so that by the time we come
					 *	back for more it will be waiting. M_DATA
					 *	packets are lookup requests, M_CTL
					 *	packets are close notifications from DDP.
					 */

					m->b_cont = nbpd_incoming;
					nbpd_incoming = NULL;
					if (nbpd_more) {
						nbpd_more = 0;
						qenable(RD(q));
					}
					if (m->b_cont->b_datap->db_type == M_CTL) {
						m->b_cont->b_datap->db_type = M_DATA;
						iocbp->ioc_rval = NBPD_CLOSE | (nbpd_net<<16);
					} else {
						iocbp->ioc_rval = NBPD_REMOTE_LOOKUP | (nbpd_net<<16);
					}
				} else {

					/*
					 *	If no ioctl messages are waiting either
					 *	put the wait ioctl back on the queue
					 *	the queue will be rescheduled when there
					 *	is more data waiting
					 */
					
					s = splclk();
					if ((m2 = nbpd_work_head) == NULL) {
						nbpd_ioctl_waiting = 1;
						splx(s);
						putbq(q, m);
						return;
					}

					/*
					 *	If there is one in the work queue
					 *	remove it for return. Set the NBPD
					 *	command to that stored in the wptr
					 *	of the request packet (insert the
					 *	user's uid into it as well) point
					 *	nbpd_active at the entry in the
					 *	waiting Q so we can get back to
					 *	it when the command is done.
					 */

					nbpd_ioctl_waiting = 0;
					nbpd_work_head = m2->b_next;
					splx(s);
					iocbp->ioc_rval = ((int)m2->b_wptr&0xff)|
						(((((struct iocblk*)m2->b_prev->b_rptr)->ioc_uid)<<8)&0xff00) |
						(nbpd_net << 16);
					m->b_cont = m2->b_cont;
					nbpd_active = m2->b_prev;
					freeb(m2);
				}
				iocbp->ioc_count = msgdsize(m->b_cont);
				m->b_datap->db_type = M_IOCACK;
				qreply(q, m);
				break;

			case AT_NBPD_REMOTE:

				/*
				 *	This command sais send a packet to the net
				 *	it is used to send lkup reply packets in
				 *	response to lkup requests from the net
			 	 */
				
				if (iocbp->ioc_count == 0) {
					m->b_datap->db_type = M_IOCNAK;
				} else {
					if (!canput(q->q_next)) {
						putbq(q, m);
						return;
					}
					putnext(q, m->b_cont);
					m->b_cont = NULL;
					iocbp->ioc_count = 0;
					m->b_datap->db_type = M_IOCACK;
				}
				qreply(q, m);
				break;

			case AT_NBPD_LOCAL:

				/*
				 *	This is a reply to a local lookup 
				 */

				m2 = nbpd_active;
				if (m2 && iocbp->ioc_count) {
					m->b_cont->b_rptr += (AT_NBP_HDR_SIZE +
							      AT_DDP_X_HDR_SIZE);
					m3 = m;
					while (m3->b_cont)
						m3 = m3->b_cont;
					m3->b_cont = m2->b_cont->b_cont;
					m2->b_cont->b_cont = m->b_cont;
					m->b_cont = NULL;
					iocbp->ioc_count = 0;
					if (((struct iocblk *)m2->b_rptr)->ioc_cmd ==
							AT_NBP_REGISTER) 
					    ((struct iocblk*)m2->b_rptr)->ioc_rval |=
									REMOTE_DONE;
				}
				m->b_datap->db_type = M_IOCACK;
				qreply(q, m);
				break;

			case AT_NBPD_SHUTDOWN_DONE:

				/*
				 *	Tell the requesting process we are shutting down
				 */

				m2 = nbpd_active;
				goto norm_end;

			case AT_NBPD_REGISTER_DONE:
			case AT_NBPD_DELETE_DONE:

				/*
				 *	These requests pass their results back to
				 *	the user. No data means an error.
				 */

				m2 = nbpd_active;
				if (m->b_cont) {
					m2->b_datap->db_type = M_IOCACK;
					if (m2->b_cont) 
						freemsg(m2->b_cont);
					m2->b_cont = m->b_cont;
					m->b_cont = NULL;
				} else {
					m2->b_datap->db_type = M_IOCNAK;
				}
				iocbp->ioc_count = 0;
				goto norm_end;

			case AT_NBPD_LOCAL_DONE:

				/*
				 *	local_done returns no data, just wakes up
				 *	the ioctl if it is waiting
				 */

				m2 = nbpd_active;

				/*
				 *	Generic complete finds the ioctl that is waiting
				 *	if it is done waiting for remote access then
				 *	complete it, otherwise mark it as local done
				 *	so that when the remote end finishes it will
				 *	complete
				 */

			norm_end:
				if (m2) {	
					s = splclk();
					if (((struct iocblk *)m2->b_rptr)->ioc_rval&
									REMOTE_DONE){
						splx(s);
						nbp_complete(m2, 0);
					} else {
						((struct iocblk*)m2->b_rptr)->ioc_rval |=
									LOCAL_DONE;
						splx(s);
					}
					nbpd_active = NULL;
				}
				m->b_datap->db_type = M_IOCACK;
				qreply(q, m);
				break;

			default:

				/*
				 *	Otherwise pass the ioctl on to DDP
				 */

				putnext(q, m);
			}
			break;
		default:
			freemsg(m);
			break;
		}
	}
}

/*
 *	Our read putq routine does the following things with packets:
 *
 *		- error etc pass them on
 *		- iocack, if a get config save the net/node numbers and mark the
 *		  daemon as up, pass it on
 *		- iocnak pass it on
 *		- ctl, a close notification from DDP, put it on the read  Q
 *		- data, decode it, if not a NBP packet drop it, otherwise:
 *		  	- lookup request, send it to the daemon
 *			- lookup reply, try and find the ioctl requesting it
 *				do duplicate filtering
 */

static
nbpd_rputq(q, m)
queue_t *q;
register mblk_t *m;
{
	mblk_t *m2, *m3;
	char *cp, *ci, *co;
	register int s;
        register at_nbp_tuple_hdr *tpn, *tpo;
	int i;
	at_ddp_t *ddp;
	register at_nbp *nbp, *nbp2;
	struct iocblk *iocbp;

	switch(m->b_datap->db_type) {
	case M_IOCACK:

		/*
		 *	Catch the DDP parameters
		 */

		iocbp = (struct iocblk *)m->b_rptr;
		if (iocbp->ioc_cmd == AT_DDP_GET_CFG) {
			at_ddp_cfg_t *cfgp;

			cfgp = (at_ddp_cfg_t *)m->b_cont->b_rptr;
			nbpd_net = cfgp->this_net;
			nbpd_node = cfgp->this_node;
			nbpd_bridge = cfgp->a_bridge;
			nbpd_valid = 1;
		}
		/* fall thru */

	case M_HANGUP:
	case M_IOCNAK:
		putnext(q, m);
		break;

	case M_CTL:
		putq(q, m);
		break;

	case M_DATA:

		/*
		 *	Check out and decode the DDP header. Ignore non NBP messages
		 *	and broadcast messages from ourselves .....
		 */

TRACE(T_nbp, ("nbp_rputq: 1\n"));
		ddp = NBP_DDP(m->b_rptr);
		if ((ddp->type != AT_DDP_TYPE_NBP &&
		     ddp->type != AT_DDP_TYPE_RTMP) ||
		    (ddp->src_node == nbpd_node &&
		     (R16(ddp->src_net) == 0 || R16(ddp->src_net) == nbpd_net))) {
TRACE(T_nbp, ("nbp_rputq: 2\n"));
			freemsg(m);
			break;
		}
		if (ddp->type == AT_DDP_TYPE_RTMP) {
        		at_rtmp *rp;
		
TRACE(T_nbp, ("nbp_rputq: 2a\n"));
			untimeout(nbpd_age_bridge, q);
			timeout(nbpd_age_bridge, q, 90*HZ);
			rp = (at_rtmp *)(((at_ddp_t *)(m->b_rptr))->data);
			if (rp->at_rtmp_id_length >= 1) {
				nbpd_net = R16(rp->at_rtmp_this_net);
				nbpd_bridge = rp->at_rtmp_id[0];
TRACE(T_nbp, ("nbp_rputq: 2b\n", nbpd_bridge));
			}
			freemsg(m);
			break;
		}
		nbp = NBP_NBP(m->b_rptr);
		switch(nbp->at_nbp_ctl) {
		case AT_NBP_BRRQ:

			/*
			 *	Ignore broadcast request ... not a bridge
			 */

TRACE(T_nbp, ("nbp_rputq: 3\n"));
			freemsg(m);
			break;
		case AT_NBP_LKUP:

			/*
			 *	lookup request pass to the daemon
			 */

TRACE(T_nbp, ("nbp_rputq: 4\n"));
			putq(q, m);
			break;
		case AT_NBP_LKUP_REPLY:

			/*
			 *	lookup reply ... look for a pending ioctl that
			 *	is waiting for responses
			 */

TRACE(T_nbp, ("nbp_rputq: 5\n"));
			i = nbp->at_nbp_id;
			s = splclk();
			m2 = nbpd_waitingq;
			while (m2) {
TRACE(T_nbp, ("nbp_rputq: 6\n"));
				if (m2->b_cont) {
					nbp2 = NBP_NBP(&m2->b_cont->
						b_rptr[sizeof(struct nbp_param)]);
					if (nbp2->at_nbp_id == i)
						break;
				}
				m2 = m2->b_next;
			}	

			/*
			 *	if none waiting ignore
			 */

			if (m2 == NULL) {
				splx(s);
				freemsg(m);
TRACE(T_nbp, ("nbp_rputq: 7\n"));
				break;
			}

TRACE(T_nbp, ("nbp_rputq: 8\n"));
			/*
			 *	now look at the ioctl that is waiting, some need one
			 *	reply to fail ... others are ignored
			 */

			iocbp = (struct iocblk*)m2->b_rptr;
			if (iocbp->ioc_cmd == AT_NBP_REGISTER_DONE){
				splx(s);
TRACE(T_nbp, ("nbp_rputq: 9\n"));
				freemsg(m);
				break;
			} else
			if (iocbp->ioc_cmd == AT_NBP_REGISTER) {
TRACE(T_nbp, ("nbp_rputq: 10\n"));
				m->b_cont = m2->b_cont->b_cont;
				m2->b_cont->b_cont = m;
				if (iocbp->ioc_rval&LOCAL_DONE) {
					splx(s);
					nbp_complete(m2, 0);
				} else {
					iocbp->ioc_rval |= REMOTE_DONE;
					splx(s);
				}
				break;
			} 

			/*
			 *	Set the count negative, this is a signal to the
			 *	timeout code to indicate that the ioctl packet
			 *	is being used and should not be completed just
			 *	yet
			 */

			iocbp->ioc_count = -iocbp->ioc_count;
			splx(s);

TRACE(T_nbp, ("nbp_rputq: 11\n"));

			/*
			 *	remove the DDP/NBP header
			 */

			m->b_rptr += (AT_NBP_HDR_SIZE + AT_DDP_X_HDR_SIZE);

			/*
			 *	start looking for duplicates ... if there is nothing it's
			 *	easy
			 */

			m3 = m2->b_cont->b_cont;
			if (m3 == NULL) {

				/*
				 *	If nothing just append, and return
				 */

				m2->b_cont->b_cont = m;
TRACE(T_nbp, ("nbp_rputq: 12\n"));
				iocbp->ioc_count = -iocbp->ioc_count;
				break;
			}

			/*
			 *	now search for duplicates hanging off the ioctl message,
			 *	remove any from the received message, if the message
			 *	ever becomes empty discard it
			 */

			while (m3) {

				/*
				 *	get a pointer the tuples
				 */

TRACE(T_nbp, ("nbp_rputq: 13\n"));
				cp = m3->b_rptr;
				while (cp < m3->b_wptr) {
					tpo = (at_nbp_tuple_hdr *)cp;

					/*
					 *	skip the pointer to the end of the 
					 *	tuple
					 */

					cp = (char *)tpo->at_nbp_tuple_data;
					cp += *cp + 1;
					cp += *cp + 1;
					cp += *cp + 1;

					/*
					 *	now do the same for the incoming message
					 */

					ci = m->b_rptr;
					while (ci < m->b_wptr) {
						tpn = (at_nbp_tuple_hdr *)ci;
						ci = (char *)tpn->at_nbp_tuple_data;
						ci += *ci + 1;
						ci += *ci + 1;
						ci += *ci + 1;

						/*
						 *	if the tuples are the same
						 *	remove the tuple from the 
						 *	incoming packet
						 */

						if ((tpn->at_nbp_tuple_node ==
						     tpo->at_nbp_tuple_node) &&
						    (tpn->at_nbp_tuple_socket ==
						     tpo->at_nbp_tuple_socket) &&
						    (R16(tpn->at_nbp_tuple_net) ==
						     R16(tpo->at_nbp_tuple_net)) &&
						    (tpn->at_nbp_tuple_enumerator ==
						     tpo->at_nbp_tuple_enumerator)) {
							if (m->b_rptr >= (char *)tpn) {
								m->b_rptr = ci;
							} else 
							if (m->b_wptr <= ci) {
								m->b_wptr = (char *)
									tpn;
							} else {
								co = (char *)tpn;
								while (ci < m->b_wptr)
									*co++ = *ci++;
								m->b_wptr = co;
							}
							if (m->b_rptr >= m->b_wptr) {
								freemsg(m);
								m = NULL;
							}
							break;
						}
					}	
					if (m == NULL)
						break;
				}
				if (m == NULL)
					break;
				m3 = m3->b_cont;
			}

TRACE(T_nbp, ("nbp_rputq: 14\n"));
			/*
			 *	If the message is not all duplicates add it to the
			 *	ioctl's list
			 */

			if (m) {
TRACE(T_nbp, ("nbp_rputq: 15\n"));
				m->b_cont = m2->b_cont->b_cont;
				m2->b_cont->b_cont = m;
			}

			/*
			 *	Unlock the ioctl
			 */

			iocbp->ioc_count = -iocbp->ioc_count;
			break;

		default:
TRACE(T_nbp, ("nbp_rputq: 16\n"));
			freemsg(m);
			break;
		}
		break;
	default:
		freemsg(m);
	}
}

/*
 *	The read service routine just keeps nbp_incoming supplied with
 *	waiting messages
 */

static
nbpd_rsrvc(q)
register queue_t *q;
{
	register mblk_t *m;

	while (m = getq(q)) {
		switch(m->b_datap->db_type) {
		case M_DATA:
		case M_CTL:
			if (nbpd_incoming) {
				nbpd_more = 1;
				putbq(q, m);
				return;
			}
			nbpd_incoming = m;
			qenable(WR(q));
			break;
		default:
			freemsg(m);
			break;
		}
	}
}

/*
 *	The NBP module is pushed onto any socket by processes wishing to make
 *	NBP calls. 
 */

static int nbp_open();
static int nbp_close();
static int nbp_wsrvc();
static int nbp_rputq();

static struct 	module_info nbp_minfo = { 98, "at_nbp", 0, 256, 256, 256, NULL };
static struct	qinit nbprdata = { nbp_rputq, NULL, nbp_open, nbp_close,
			nulldev, &nbp_minfo, NULL};
static struct	qinit nbpwdata = { putq, nbp_wsrvc, nbp_open, nbp_close, 
			nulldev, &nbp_minfo, NULL};
struct	streamtab nbp_info = {&nbprdata, &nbpwdata, NULL, NULL};


/*
 *	Open fails if the NBP daemon is not around
 */

static int
nbp_open(q, dev, flag, sflag, err, devp
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
	if (nbpd_q == NULL || nbpd_valid == 0) {
		*err = ENETDOWN;
		return(OPENFAIL);
	}
	return(0);
}

/*
 *	Close the module, if there are any outstanding ioctls toss them
 */

static
nbp_close(q)
register queue_t *q;
{
	register mblk_t *m, *m2;
	register struct iocblk *iocbp;
	register int s;

	m2 = NULL;
	s = splclk();
	for (m = nbpd_work_head; m;) {
		if ((queue_t *)(((struct iocblk *)m->b_prev->b_rptr)->ioc_error)==WR(q)){
			if (nbpd_active == m)
				nbpd_active = NULL;
			if (m2) {
				m2->b_next = m->b_next;
				freemsg(m);
				m = m2->b_next;
			} else {
				nbpd_work_head = m->b_next;
				freemsg(m);
				m = nbpd_work_head;
			}
		} else {
			m2 = m;
			m = m->b_next;
		}
	}
	for (m = nbpd_waitingq; m;) {
		if ((queue_t *)(((struct iocblk *)m->b_rptr)->ioc_error) == WR(q)) {
			m2 = m;
			untimeout(nbp_timeout, m);
			if (m->b_next)
				m->b_next->b_prev = m->b_prev;
			if (m->b_prev) {
				m = m->b_prev->b_next = m->b_next;
			} else {
				m = nbpd_waitingq = m->b_next;
			}
			freemsg(m2);
			break;
		}
		m = m->b_next;
	}
	splx(s);
}

/*
 *	This is used to wakeup queues that are looking for space
 */

static
nbp_wtime(q)
register queue_t *q;
{
	qenable(q);
	q->q_ptr = NULL;
}

/*
 *	The timeout routine is used to repeatedly send lkup requests to the net
 *	for pending searches. It honours the locking protocol used by nbp_rputq.
 *	If no space is available to make the message to be sent it waits a little
 *	while. When the count hits 0 it does a complete.
 */

static
nbp_timeout(m)
register mblk_t *m;
{
	register struct iocblk *iocbp;
	register at_nbp_tuple_hdr *tp;
	register mblk_t *m2;
	register int tm;

	iocbp = (struct iocblk *)m->b_rptr;
	if (iocbp->ioc_count > 0) {
		if (canput(WR(nbpd_q)->q_next)) {
			m2 = copyb(m->b_cont);
			if (m2) {
				m2->b_rptr += sizeof(struct nbp_param);
				tp = (at_nbp_tuple_hdr *)
					     (NBP_NBP(m2->b_rptr)->at_nbp_tuples);
        			W16(tp->at_nbp_tuple_net, nbpd_net);
        			tp->at_nbp_tuple_node = nbpd_node;
        			tp->at_nbp_tuple_socket = 2;
				putnext(WR(nbpd_q), m2);
				iocbp->ioc_count--;
				tm = ((struct nbp_param*)m->b_cont->b_rptr)->nbp_secs*HZ;
				if (tm <= 0)
					tm = HZ;
			} else {
				tm = HZ/8;
			}
		} else {
			tm = HZ/8;
		}
		timeout(nbp_timeout, m, tm);
	} else
	if (iocbp->ioc_count < 0) {		/* locked out .... */
		timeout(nbp_timeout, m, 2);
	} else
	if (iocbp->ioc_rval&LOCAL_DONE) {
		nbp_complete(m, 0);
	} else {
		iocbp->ioc_rval |= REMOTE_DONE;
	}
}

/*
 *	read put routine just passes things on
 */

static
nbp_rputq(q, m)
register queue_t *q;
mblk_t *m;
{
	putnext(q, m);
}

/*
 *	The write service handles only ioctls from peocesses
 */

static
nbp_wsrvc(q)
register queue_t *q;
{
	register mblk_t *m;
	register at_ddp_t *ddp;
	register at_nbp *nbp;
	register int s;
	mblk_t *m2;
	register struct iocblk *iocbp;
	at_nbp_tuple_hdr *tp;
	int i;

	/*
	 *	loop getting messages
	 */

	while (m = getq(q)) {
		switch(m->b_datap->db_type) {
		case M_IOCTL:

			/*
			 *	if the NBPD went down return an error
			 */

			iocbp = (struct iocblk *)m->b_rptr;
			if (nbpd_q == NULL) {
				m->b_datap->db_type = M_IOCNAK;
				iocbp->ioc_error = ENETDOWN;
				qreply(q, m);
				break;
			}
			switch(iocbp->ioc_cmd) {
			case AT_NBP_CONFIRM:

				/*
				 *	For confirm, set up the DDP header from
				 *	the tuple being requested. First verify
				 *	the packet is correct.
				 */

				if (nbp_verify(m->b_cont, 1))
					goto iocnak;

				/*
				 *	If the confirm is to the local node
				 *	send the message to the local daemon
				 *	this is reliable so will be fast
				 */

				nbp = NBP_NBP(&m->b_cont->
					b_rptr[sizeof(struct nbp_param)]);
				tp = (at_nbp_tuple_hdr *) (nbp->at_nbp_tuples);
				if (nbpd_node == tp->at_nbp_tuple_node &&
				    (nbpd_net == R16(tp->at_nbp_tuple_net) ||
				     tp->at_nbp_tuple_node  == 0))
					goto to_nbpd;

				/*
				 *	If it is remote then set us up to do
				 *	retransmission and do the DDP header
				 *	put a unique id in the NBP header
				 */

				iocbp->ioc_rval = LOCAL_DONE;
				iocbp->ioc_count =
				    ((struct nbp_param *)m->b_cont->b_rptr)->nbp_retries;
				if (iocbp->ioc_count <= 0)
					iocbp->ioc_count = NBP_RETRIES;
				iocbp->ioc_error = (int) q;
				ddp = NBP_DDP(&m->b_cont->
						b_rptr[sizeof(struct nbp_param)]);
				C16(ddp->dst_net, tp->at_nbp_tuple_net);
				ddp->dst_node = tp->at_nbp_tuple_node;
				ddp->dst_socket = 2;
				ddp->type = AT_DDP_TYPE_NBP;
        			nbp->at_nbp_ctl = AT_NBP_LKUP;
				nbp->at_nbp_id = nbpd_newid();

				/*
				 *	now put the packet into the waiting queue
				 */

				m->b_prev = NULL;
				s = splclk();
				if (nbpd_waitingq)
					nbpd_waitingq->b_prev = m;
				m->b_next = nbpd_waitingq;
				nbpd_waitingq = m;
				splx(s);

				/*
				 *	Next try and send the message, queue the
				 *	timeout so it will be repeated
				 */

				if (canput(WR(nbpd_q)->q_next)) {
					m2 = copyb(m->b_cont);
					if (m2) {
						m2->b_rptr += sizeof(struct nbp_param);
						tp = (at_nbp_tuple_hdr *)
						    (NBP_NBP(m2->b_rptr)->at_nbp_tuples);
        					W16(tp->at_nbp_tuple_net, nbpd_net);
        					tp->at_nbp_tuple_node = nbpd_node;
        					tp->at_nbp_tuple_socket = 2;
						putnext(WR(nbpd_q), m2);
						i=((struct nbp_param *)m->b_cont->b_rptr)
								->nbp_secs*HZ;
						if (i <= 0)
							i = HZ;
					} else {
						i = HZ/8;
					}
				} else {
					i = HZ/8;
				}
				timeout(nbp_timeout, m, i);
				break;

			case AT_NBP_SHUTDOWN:

				/*
				 *	Shutdown can only be done by root. Send
				 *	the message to the daemon
				 */

				if (iocbp->ioc_uid != 0) {
					iocbp->ioc_error = EPERM;
					m->b_datap->db_type = M_IOCNAK;
					qreply(q, m);
					break;
				}

				/*
				 *	generic NBPD send code
				 */

			to_nbpd:

				/*
				 *	allocate a packet to queue 
				 */

				m2 = dupmsg(m);
				if (m2 == NULL) {
					putbq(q, m);
					if (q->q_ptr == NULL) {
						q->q_ptr = (char *)1;
						timeout(nbp_wtime, q, HZ/8);
					}
					return;
				}

				/*
				 *	set up the context in the packet's header
				 *	put the NBPD command in the B_wptr
				 */

				iocbp->ioc_rval = REMOTE_DONE;
				m2->b_prev = m;
				switch (iocbp->ioc_cmd) {
				case AT_NBP_DELETE:
					m2->b_wptr = (char *)NBPD_DELETE;
					break;

				case AT_NBP_DELETE_NAME:
					m2->b_wptr = (char *)NBPD_DELETE_NAME;
					m2->b_cont->b_rptr += sizeof(struct nbp_param);
					break;
				
				case AT_NBP_CONFIRM:
					m2->b_wptr = (char *)NBPD_LOCAL_LOOKUP;
					m2->b_cont->b_rptr += sizeof(struct nbp_param);
					break;

				case AT_NBP_SHUTDOWN:
					m2->b_wptr = (char *)NBPD_SHUTDOWN;
					break;
				}
				iocbp->ioc_error = (int) q;

				/*
				 *	Put the packets in the waiting and work queues
				 */

				m2->b_next = NULL;
				m->b_prev = NULL;
				s = splclk();
				if (nbpd_work_head) {
					nbpd_work_tail->b_next = m2;
				} else {
					nbpd_work_head = m2;
				}
				nbpd_work_tail = m2;
				if (nbpd_waitingq)
					nbpd_waitingq->b_prev = m;
				m->b_next = nbpd_waitingq;
				nbpd_waitingq = m;
				splx(s);

				/*
				 *	Wake the NBPD ioctl code
				 */

				qenable(WR(nbpd_q));
				break;

			case AT_NBP_DELETE:

				/*
				 *	Verify the header and send to the daemon
				 */

				if (iocbp->ioc_count != sizeof(int))
					goto iocnak;
				goto to_nbpd;

			case AT_NBP_DELETE_NAME:

				/*
				 *	Verify the header and send to the daemon
				 */

				if (nbp_verify(m->b_cont, 1))
					goto iocnak;
				goto to_nbpd;

			case AT_NBP_LOOK_LOCAL:
			case AT_NBP_LOOKUP:
			case AT_NBP_REGISTER:

				/*
				 *	All of these start by doing lookups
				 *	LOOK_LOCAL just sends to the local daemon
				 */

				if ((i = nbp_verify(m->b_cont, nbpd_bridge)) != 0) {

					/* 
					 *	If they requested a zone and we can't reach a bridge
				       	 *	then return a 'zone unreachable' error
					 */

					if (i == 2)
						iocbp->ioc_error = ENETUNREACH;
					goto iocnak;
				}
				m2 = dupmsg(m);
				if (m2 == NULL) {
					putbq(q, m);
					if (q->q_ptr == NULL) {
						q->q_ptr = (char *)1;
						timeout(nbp_wtime, q, HZ/8);
					}
					return;
				}
				if (iocbp->ioc_cmd == AT_NBP_LOOK_LOCAL) {
					iocbp->ioc_rval = REMOTE_DONE;
				} else {
					iocbp->ioc_rval = 0;
				}
				iocbp->ioc_count =
				    ((struct nbp_param *)m->b_cont->b_rptr)->nbp_retries;
				if (iocbp->ioc_count <= 0)
					iocbp->ioc_count = NBP_RETRIES;
				m2->b_prev = m;
				m2->b_cont->b_rptr += sizeof(struct nbp_param);
				m2->b_wptr = (char *)NBPD_LOCAL_LOOKUP;
				iocbp->ioc_error = (int) q;
				m2->b_next = NULL;
				s = splclk();
				if (nbpd_work_head) {
					nbpd_work_tail->b_next = m2;
				} else {
					nbpd_work_head = m2;
				}
				nbpd_work_tail = m2;
				splx(s);
				qenable(WR(nbpd_q));
				m->b_prev = NULL;
				s = splclk();
				if (nbpd_waitingq)
					nbpd_waitingq->b_prev = m;
				m->b_next = nbpd_waitingq;
				nbpd_waitingq = m;
				if (iocbp->ioc_cmd == AT_NBP_LOOK_LOCAL) {
					iocbp->ioc_cmd = AT_NBP_LOOKUP;
					splx(s);
					break;
				}
				nbp = NBP_NBP(&m->b_cont->
						b_rptr[sizeof(struct nbp_param)]);
				nbp->at_nbp_id = nbpd_newid();
				splx(s);
				ddp = NBP_DDP(&m->b_cont->
						b_rptr[sizeof(struct nbp_param)]);
				W16(ddp->dst_net, 0);
				if (nbpd_bridge ) {
        				nbp->at_nbp_ctl = AT_NBP_BRRQ;
					ddp->dst_node = nbpd_bridge;
				} else {
        				nbp->at_nbp_ctl = AT_NBP_LKUP;
					ddp->dst_node = 0xff;
				}
				ddp->dst_socket = 2;
				ddp->type = AT_DDP_TYPE_NBP;
				if (canput(WR(nbpd_q)->q_next)) {
					m2 = copyb(m->b_cont);
					if (m2) {
						m2->b_rptr += sizeof(struct nbp_param);
						tp = (at_nbp_tuple_hdr *)
						    (NBP_NBP(m2->b_rptr)->at_nbp_tuples);
        					W16(tp->at_nbp_tuple_net, nbpd_net);
        					tp->at_nbp_tuple_node = nbpd_node;
        					tp->at_nbp_tuple_socket = 2;
						putnext(WR(nbpd_q), m2);
						i=((struct nbp_param *)m->b_cont->b_rptr)
								->nbp_secs*HZ;
						if (i <= 0)
							i = HZ;
					} else {
						i = HZ/8;
					}
				} else {
					i = HZ/8;
				}
				timeout(nbp_timeout, m, i);
				break;

			default:
			iocnak:
				m->b_datap->db_type = M_IOCNAK;
				if (iocbp->ioc_error == 0)
					iocbp->ioc_error = EINVAL;
				qreply(q, m);
				break;
			}
			break;
		default:
			freemsg(m);
			break;
		}
	}
}

/*
 *	verify a 1 tuple NBP packet
 */

static
nbp_verify(m, zone)
register mblk_t *m;
unsigned zone;
{
	register at_nbp *nbp;
	register int len;
	register int i;
	register char *cp;

	if (m == NULL)
		return(1);
	len = m->b_wptr - m->b_rptr;
	if (len < (NBP_HDR_SIZE+6+sizeof(struct nbp_param)))
		return(1);
	nbp = NBP_NBP(&m->b_rptr[sizeof(struct nbp_param)]);
	if (nbp->at_nbp_tuple_count != 1) {
		return(1);
	}
	cp = (char *)((at_nbp_tuple_hdr *)(nbp->at_nbp_tuples))->at_nbp_tuple_data;
	len -= sizeof(struct nbp_param) + NBP_HDR_SIZE;
	i = *cp++;
	if (i < 0 || i > 32) {
		return(1);
	}
	len -= i+1;
	cp += i;
	if (len <= 0) {
		return(1);
	}
	i = *cp++;
	if (i < 0 || i > 32) {
		return(1);
	}
	len -= i+1;
	cp += i;
	if (len <= 0) {
		return(1);
	}
	i = *cp++;
	if (i < 0 || i > 32) {
		return(1);
	}
	if (zone == 0 && (i != 1 || *cp != '*'))
		return(2);
	len--;
	if (len != i) {
		return(1);
	}
	return(0);
}

/*
 *	This routine is used as a timeout to complete a REGISTER when no messages
 *	are available
 */

static
nbp_fin_register(m)
register mblk_t *m;
{
	register mblk_t *m2;

	m2 = dupmsg(m);
	if (m2 == NULL) {
		timeout(nbp_fin_register, m, HZ/8);
		return;
	}
	m2->b_cont->b_rptr += sizeof(struct nbp_param);
	m2->b_prev = m;
	m2->b_wptr = (char *)NBPD_REGISTER;
	m2->b_next = NULL;
	if (nbpd_work_head) {
		nbpd_work_tail->b_next = m2;
	} else {
		nbpd_work_head = m2;
	}
	nbpd_work_tail = m2;
	qenable(WR(nbpd_q));
}

/*
 *	The complete routine tidies up the ioctl messages and returns them to
 *	the process with the correct information. In the case of when the first
 *	part of a registration has completed (the name space search) a register
 *	packet is sent to the local daemon.
 */

static
nbp_complete(m, err)
register mblk_t *m;
int err;
{
	register at_nbp *nbp = NULL;
	register struct iocblk *iocbp;
	register int s;
	register queue_t *q;
	register mblk_t *m2;
	register int type;

	untimeout(nbp_timeout, m);
	iocbp = (struct iocblk *)m->b_rptr;
	q = (queue_t *)(iocbp->ioc_error);
	switch(iocbp->ioc_cmd) {
	case AT_NBP_REGISTER:
		if (m->b_cont->b_cont) {
			freemsg(m->b_cont);
			m->b_cont = NULL;
			type = M_IOCNAK;
		} else {
			iocbp->ioc_cmd = AT_NBP_REGISTER_DONE;
			iocbp->ioc_rval = REMOTE_DONE;
			m2 = dupmsg(m);
			if (m2 == NULL) {
				timeout(nbp_fin_register, m, HZ/8);
				return;
			}
			m2->b_cont->b_rptr += sizeof(struct nbp_param);
			m2->b_wptr = (char *)NBPD_REGISTER;
			m2->b_prev = m;
			m2->b_next = NULL;
			if (nbpd_work_head) {
				nbpd_work_tail->b_next = m2;
			} else {
				nbpd_work_head = m2;
			}
			nbpd_work_tail = m2;
			qenable(WR(nbpd_q));
			return;
		}
		goto finish;

	case AT_NBP_DELETE:
	case AT_NBP_DELETE_NAME:
	case AT_NBP_REGISTER_DONE:
		type = m->b_datap->db_type;
		goto finish;

	case AT_NBP_LOOKUP:
	case AT_NBP_CONFIRM:
		if (m->b_cont->b_cont) {
			m->b_cont->b_rptr += sizeof(struct nbp_param);
			m->b_cont->b_wptr = m->b_cont->b_rptr +
					(AT_NBP_HDR_SIZE + AT_DDP_X_HDR_SIZE);
			type = M_IOCACK;
			nbp = NBP_NBP(m->b_cont->b_rptr);
		} else {
			freeb(m->b_cont);
			m->b_cont = NULL;
			type = M_IOCNAK;
		}
		goto finish;

	case AT_NBP_SHUTDOWN:
		type = M_IOCACK;
		goto finish;

	finish:
		s = splclk();
		if (m->b_prev) {
			m->b_prev->b_next = m->b_next;
		} else {
			nbpd_waitingq = m->b_next;
		}
		if (m->b_next)
			m->b_next->b_prev = m->b_prev;
		splx(s);
		iocbp->ioc_error = err;
		if (m->b_cont) {
			iocbp->ioc_count = msgdsize(m->b_cont);
			if (iocbp->ioc_count > NBP_NAME_MAX)
				iocbp->ioc_count = NBP_NAME_MAX;
			if (nbp) {
				((unsigned char *)nbp)[0] = iocbp->ioc_count>>8;
				((unsigned char *)nbp)[1] = iocbp->ioc_count&0xff;
			}
		} else {
			iocbp->ioc_count = 0;
		}
		iocbp->ioc_rval = 0;
		m->b_datap->db_type = type;
		qreply(q, m);
		break;
	}
}

/*
 *	This allocates a unique id for each pending request
 */

static int
nbpd_newid()
{
	static int count = 0;
	register int i, s;
	register mblk_t *m;
	register at_nbp *nbp;

	for (;;) {
		i = count++;
		s = splclk();
		m = nbpd_waitingq;
		while (m) {
			nbp = NBP_NBP(&m->b_cont->b_rptr[sizeof(struct nbp_param)]);
			if (nbp->at_nbp_id == i)
				break;
			m = m->b_next;
		}	
		splx(s);
		if (m) 
			continue;
		return(i);
	}
}

static
nbpd_age_bridge(q)
queue_t *q;
{
	nbpd_bridge = 0;
}
