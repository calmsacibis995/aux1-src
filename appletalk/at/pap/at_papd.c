#ifndef lint	/* .../appletalk/at/pap/at_papd.c */
#define _AC_NAME at_papd_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:03:56}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_papd.c on 87/11/11 21:03:56";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)at_papd.c	UniPlus VVV.2.1.9	*/
/*
 * (C) 1987 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
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

extern T_papd;

static int papd_open();
static int papd_close();
static int papd_rsrvc();
static int papd_wsrvc();
static int papd_rputq();
static int papd_wputq();
extern int nulldev();
extern int putq();

static struct 	module_info papd_minfo = { 98, "at_papd", 0, 256, 256, 256, NULL };
static struct	qinit papdrdata = { papd_rputq, papd_rsrvc, papd_open, papd_close,
			nulldev, &papd_minfo, NULL};
static struct	qinit papdwdata = { papd_wputq, papd_wsrvc, papd_open, papd_close, 
			nulldev, &papd_minfo, NULL};
struct	streamtab papd_info = {&papdrdata, &papdwdata, NULL, NULL};

/*
 *	Client state
 */

struct papd_state {
	int		papd_state;	/* current state ... see below */
	mblk_t		*papd_status;	/* current status */
	mblk_t		*papd_get_next;	/* waiting ioctl */
	mblk_t		*papd_requests;	/* unanswered requests */
	unsigned char	papd_outstanding;/* number of outstanding requests */
	unsigned char	papd_timing;	/* timeout flag */
	unsigned char	papd_outtimer;	/* timeout flag */
};


static struct papd_state papd[NPAPSERVERS];

#define NOUTSTANDING 	3	/* number of outstanding requests */

extern int wakeup();
static papd_arbitrate();
static papd_wake();
static papd_outwake();
static papd_block();


/*
 *	Server arbitration states
 */

#define	PAPD_UNUSED	0	/* structure is free */
#define	PAPD_BLOCKED	1	/* no get next entries are available */
#define	PAPD_WAITING	2	/* get next entries are waiting ... no requests yet */
#define	PAPD_ARBITRATE	3	/* got one of each, arbitrating for longest waiting
				   requests */
#define	PAPD_UNBLOCKED	4	/* got an ioctl no one waiting */
#define	PAPD_CLOSING	5	/* closing, discard incoming ioctl acks etc */

/*
 *	The following state transitions occur .....
 *
 *	Event		State			Next State	Action
 *	=====		=====			==========	======
 *
 *	papd_open	UNUSED		->	BLOCKED		allocate state var
 *
 *	get_next_ioctl	BLOCKED		->	WAITING		queue ioctl
 *
 *	get_next_ioctl	WAITING		->	WAITING		queue ioctl
 *
 *	get_next_ioctl	ARBITRATE	->	ARBITRATE	queue ioctl
 *
 *	get_next_ioctl	UNBLOCKED	->	UNBLOCKED	queue ioctl
 *								if open waiting complete
 *								cancel papd_block timer
 *
 *	open_received	BLOCKED		->	BLOCKED		NAK open request
 *
 *	open received	WAITING		->	ARBITRATE	queue message
 *								set papd_arbitate timer
 *
 *	open_received	ARBITRATE	->	ARBITRATE	queue message
 *
 *	open_received	UNBLOCKED	->	UNBLOCKED	queue message
 *								if ioctl waiting complete
 *
 *	papd_arbitrate	ARBITRATE	->	UNBLOCKED	complete
 *	timer expires
 *
 *	ioctl complete	UNBLOCKED	->	UNBLOCKED	set papd_blopck timer	
 *
 *	papd_block 	UNBLOCKED	->	BLOCKED		NAK all pending open
 *	timer expires						requests
 *
 */

static int
papd_open(q, dev, flag, sflag, err, devp
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
	struct papd_state *	papdp;
	int			i;
	register int 		s;
	struct iocblk 		*iocbp;
	mblk_t			*m;

TRACE(T_papd, ("papd_open 1\n"));
	/*
	 *	Search for a free server context, when one is found clear
	 *	it to a known state
	 */

	for (i = 0; i < NPAPSERVERS; i++) 
	if (papd[i].papd_state == PAPD_UNUSED) {
		papdp = &papd[i];
		papdp->papd_state = PAPD_BLOCKED;
		papdp->papd_get_next = NULL;
		papdp->papd_status = NULL;
		papdp->papd_requests = NULL;
		papdp->papd_outstanding = 0;
		break;
	}

	/*
	 *	If we couldn't find one return an error
	 */

	if (i >= NPAPSERVERS) {
		*err = EAGAIN;
		return(OPENFAIL);
	}

	/*
	 *	Set up the queues pointers to point to the server's context
	 */

	WR(q)->q_ptr = q->q_ptr = (char *)papdp;

	/*
	 *	Now queue some ATP requests for incoming transactions
	 */

	while (papdp->papd_outstanding < NOUTSTANDING) {
TRACE(T_papd, ("papd_open 2\n"));
		for (;;) {
			m = allocb(sizeof(struct iocblk), BPRI_LO);
			if (m)
				break;
TRACE(T_papd, ("papd_open 3\n"));
			timeout(wakeup, q, HZ>>8);
			s = splclk();
			if (sleep(q, STOPRI|PCATCH)) {
				untimeout(wakeup, q);
TRACE(T_papd, ("papd_open 4\n"));
				splx(s);
				*err = EINTR;
				return(OPENFAIL);
			}
			splx(s);
		}
TRACE(T_papd, ("papd_open 5\n"));
		m->b_wptr += sizeof(struct iocblk);
		iocbp = (struct iocblk *)m->b_rptr;
		iocbp->ioc_cmd = AT_ATP_GET_REQUEST;
		iocbp->ioc_error = 0;
		iocbp->ioc_count = 0;
		putnext(WR(q), m);
		papdp->papd_outstanding++;
	}
TRACE(T_papd, ("papd_open 6\n"));
	return(0);
}

/*
 *	The close routine releases resources and exits
 */

static 
papd_close(q)
queue_t *q;
{
	struct papd_state *papdp;
	mblk_t *m, *m2;
	struct iocblk *iocbp;

	untimeout(papd_outwake, q);
	untimeout(papd_wake, q);
	untimeout(papd_wake, WR(q));
	untimeout(papd_block, WR(q));
	untimeout(papd_arbitrate, WR(q));
	papdp = (struct papd_state *)q->q_ptr;
TRACE(T_papd, ("papd_close 1\n"));
	papdp->papd_state = PAPD_CLOSING;
	if (papdp->papd_status)
		freemsg(papdp->papd_status);
	if (papdp->papd_get_next)
		freemsg(papdp->papd_get_next);
	while (m = papdp->papd_requests) {
TRACE(T_papd, ("papd_close 2\n"));
		papdp->papd_requests = m->b_next;
		for (;;) {
TRACE(T_papd, ("papd_close 3\n"));
			m2 = allocb(sizeof(struct iocblk), BPRI_LO);
			if (m2)
				break;
			timeout(wakeup, q, HZ/8);
			(void) sleep(q, PZERO);
		}
		m2->b_cont = m;
		m2->b_wptr += sizeof(struct iocblk);
		iocbp = (struct iocblk *)m2->b_rptr;
		iocbp->ioc_cmd = AT_ATP_RELEASE_RESPONSE;
		iocbp->ioc_error = 0;
		iocbp->ioc_count = m->b_wptr - m->b_rptr;
		qreply(q, m2);
	}
TRACE(T_papd, ("papd_close 4\n"));
	papdp->papd_state = PAPD_UNUSED;
}

/*
 *	The read put routine recognises incoming transaction requests and
 *	passes them to the read Q. It also frees ACK packets from ATP completes.
 *	It also attempts to allocate more request packets if possible
 */

static
papd_rputq(q, m)
queue_t *q;
mblk_t *m;
{
	struct papd_state *papdp;
	struct iocblk *iocbp;
	at_atp *atp;

TRACE(T_papd, ("papd_rputp 1\n"));
	papdp = (struct papd_state *)q->q_ptr;
	switch(m->b_datap->db_type) {
	case M_HANGUP:
	case M_IOCNAK:
	case M_IOCACK:
TRACE(T_papd, ("papd_rputp 2\n"));
		putnext(q, m);
		break;

	case M_DATA:
TRACE(T_papd, ("papd_rputp 3\n"));
		if ((m->b_wptr - m->b_rptr) != sizeof(struct iocblk) ||
		    papdp->papd_state == PAPD_CLOSING) {
TRACE(T_papd, ("papd_rputp 4\n"));
			freemsg(m);
			break;
		}
		iocbp = (struct iocblk *)m->b_rptr;
		switch (iocbp->ioc_cmd) {
		case AT_ATP_GET_REQUEST:
			papdp->papd_outstanding--;
TRACE(T_papd, ("papd_rputp err=%d, count = %d\n",iocbp->ioc_error,iocbp->ioc_count));
			if (iocbp->ioc_error || iocbp->ioc_count < ATP_HDR_SIZE) {
				freemsg(m);
				break;
			}
			atp = ATP_ATP_HDR(m->b_cont->b_rptr);
			switch(atp->at_atp_user_bytes[1]) {
			case AT_PAP_TYPE_OPEN_CONN:
			case AT_PAP_TYPE_SEND_STATUS:
TRACE(T_papd, ("papd_rputp 7\n"));
				putq(q, m);
				break;
			default:
			release:
TRACE(T_papd, ("papd_rputp 8\n"));
				iocbp->ioc_cmd = AT_ATP_RELEASE_RESPONSE;
				iocbp->ioc_error = 0;
				qreply(q, m);
			}
TRACE(T_papd, ("papd_rputp 9\n"));
			break;
			
		default:
TRACE(T_papd, ("papd_rputp 10\n"));
			freemsg(m);
			break;
		}
		break;
	default:
		freemsg(m);
	}
TRACE(T_papd, ("papd_rputp 11\n"));
	while (papdp->papd_outstanding < NOUTSTANDING) {
TRACE(T_papd, ("papd_rputp 13\n"));
		m = allocb(sizeof(struct iocblk), BPRI_LO);
		if (m == NULL) {
TRACE(T_papd, ("papd_rputp 12\n"));
			if (papdp->papd_outstanding || papd->papd_outtimer)
				break;
			papd->papd_outtimer = 1;
			timeout(papd_outwake, q, HZ/8);
			break;
		}
		m->b_wptr += sizeof(struct iocblk);
		iocbp = (struct iocblk *)m->b_rptr;
		iocbp->ioc_count = 0;
		iocbp->ioc_error = 0;
		iocbp->ioc_cmd = AT_ATP_GET_REQUEST;
		putnext(WR(q), m);
		papdp->papd_outstanding++;
	}
TRACE(T_papd, ("papd_rputp 14\n"));
}

static
papd_rsrvc(q)
queue_t *q;
{
	struct iocblk *iocbp;
	mblk_t *m, *m2, *m3;
	struct papd_state *papdp;
	at_atp *atp;
	at_ddp_t *ddp;

TRACE(T_papd, ("papd_rsrvc 1\n"));
	papdp = (struct papd_state *)q->q_ptr;

	/*
	 *	Remove the timeout for lack of resources
	 */

	untimeout(papd_wake, q);
	while (m = getq(q)) {
TRACE(T_papd, ("papd_rsrvc 2\n"));
		switch (m->b_datap->db_type) {
		case M_DATA:
TRACE(T_papd, ("papd_rsrvc 3\n"));

			/*
		 	 *	Check out which sort of pap request this is
			 */

			atp = ATP_ATP_HDR(m->b_cont->b_rptr);
			switch(atp->at_atp_user_bytes[1]) {
			case AT_PAP_TYPE_SEND_STATUS:

				/*
				 *	The status return is also used by
				 *	the OPEN failure response
				 */

				atp->at_atp_user_bytes[0] = 0;
				atp->at_atp_user_bytes[1] = AT_PAP_TYPE_SEND_STS_REPLY;
			status:
TRACE(T_papd, ("papd_rsrvc 4\n"));
				m2 = NULL;

				/*
				 *	If there is a status string duplicate it
				 *	if no space is available wait a while and
				 *	retry
				 */

				if (papdp->papd_status) {
TRACE(T_papd, ("papd_rsrvc 5\n"));
					m2 = dupmsg(papdp->papd_status);
					if (m2 == NULL) {
TRACE(T_papd, ("papd_rsrvc 6\n"));
						timeout(papd_wake, q, HZ/8);
						putbq(q, m);
						return;
					}
				}
TRACE(T_papd, ("papd_rsrvc 7\n"));

				/*
				 *	If it is a status reply then pack out the first
				 *	4 bytes of data
				 */

				if (atp->at_atp_user_bytes[1] ==
							AT_PAP_TYPE_SEND_STS_REPLY)
					m->b_cont->b_wptr += 4;

				/*
				 *	Turn the packet around to its sender
				 */

				ddp = ATP_DDP_HDR(m->b_cont->b_rptr); 
				C16(ddp->dst_net, ddp->src_net);
				ddp->dst_node = 	ddp->src_node;
				ddp->dst_socket = 	ddp->src_socket;
				atp->at_atp_eom = 	1;
				atp->at_atp_bitmap_seqno = 0;

				/*
				 *	tack the status message on the end
				 */

				m3 = m->b_cont;
				while (m3->b_cont)
					m3 = m3->b_cont;
				m3->b_cont = m2;

				/*
				 *	Send it back to ATP
				 */

				iocbp = (struct iocblk *)m->b_rptr;
				iocbp->ioc_cmd = AT_ATP_SEND_RESPONSE;
				iocbp->ioc_error = 0;
				iocbp->ioc_count = msgdsize(m->b_cont);
TRACE(T_papd, ("papd_rsrvc 8\n"));
				qreply(q, m);
				break;

			case AT_PAP_TYPE_OPEN_CONN:
TRACE(T_papd, ("papd_rsrvc 9\n"));
				m2 = NULL;

				/*
				 *	Received an open request
				 */

				switch(papdp->papd_state) {
				case PAPD_BLOCKED:
TRACE(T_papd, ("papd_rsrvc 10\n"));
					/*
					 *	If blocked then return status to
					 *	requester
					 */

					atp->at_atp_user_bytes[1] =
						AT_PAP_TYPE_OPEN_CONN_REPLY;
					atp->at_atp_data[2] = 0xff;
					atp->at_atp_data[3] = 0xff;
					goto status;

				case PAPD_WAITING:
TRACE(T_papd, ("papd_rsrvc 11\n"));
					/*
					 *	If waiting then start an arbitration
					 *	period and queue the request for
					 *	resending
					 */

					papdp->papd_state = PAPD_ARBITRATE;
					timeout(papd_arbitrate, WR(q), HZ*5);
					papd_insertq(papdp, m);
					break;

				case PAPD_ARBITRATE:
TRACE(T_papd, ("papd_rsrvc 15\n"));

					/*
					 *	If arbitrating then just queue and wait
					 */

					papd_insertq(papdp, m);
					break;

				case PAPD_UNBLOCKED:
TRACE(T_papd, ("papd_rsrvc 18\n"));

					/*
					 *	If unblocked then queue and complete 
					 *	the request
					 */

					papd_insertq(papdp, m);
					if (papdp->papd_get_next)
						papd_complete(WR(q));
TRACE(T_papd, ("papd_rsrvc 19\n"));
					break;
				default:
TRACE(T_papd, ("papd_rsrvc 20\n"));
					freemsg(m);
				}
				break;
			default:
				freemsg(m);
			}
TRACE(T_papd, ("papd_rsrvc 24\n"));
			break;
		default:
TRACE(T_papd, ("papd_rsrvc 25\n"));
			freemsg(m);
		}
	}
TRACE(T_papd, ("papd_rsrvc 26\n"));

	/*
	 *	If we still don't have enough transaction receive requests outstanding
	 *	then try and get some more. If not able to then queue a qenable for
	 *	a later poll
	 */

	while (papdp->papd_outstanding < NOUTSTANDING) {
TRACE(T_papd, ("papd_rsrvc 27\n"));
		m = allocb(sizeof(struct iocblk), BPRI_LO);
		if (m == NULL) {
TRACE(T_papd, ("papd_rsrvc 28\n"));
			if (papdp->papd_outstanding || papd->papd_outtimer)
				break;
			papd->papd_outtimer = 1;
			timeout(papd_outwake, q, HZ/8);
			break;
		}
		m->b_wptr += sizeof(struct iocblk);
		iocbp = (struct iocblk *)m->b_rptr;
		iocbp->ioc_count = 0;
		iocbp->ioc_error = 0;
		iocbp->ioc_cmd = AT_ATP_GET_REQUEST;
		putnext(WR(q), m);
		papdp->papd_outstanding++;
	}
TRACE(T_papd, ("papd_rsrvc 29\n"));
}


/*
 *	Put an incoming open request in the request q in order with the ones that have
 *		been waiting the longest at the front
 */

static
papd_insertq(papdp, m)
struct papd_state *papdp;
mblk_t *m;
{
	at_atp *atp;
	int i;
	mblk_t **mpp;

TRACE(T_papd, ("papd_insertq 1\n"));
	atp = ATP_ATP_HDR(m->b_cont->b_rptr);
	i = R16(&atp->at_atp_user_bytes[2]);
	mpp = &papdp->papd_requests;
	for (;;) {
		if (*mpp == NULL) {
			*mpp = m;
			m->b_next = NULL;
			break;
		}
		atp = ATP_ATP_HDR((*mpp)->b_cont->b_rptr);
		if (i >= R16(&atp->at_atp_user_bytes[2])) {
			m->b_next = *mpp;	
			*mpp = m;
			break;
		}
		mpp = &(*mpp)->b_next;
	}
}

static
papd_wputq(q, m)
queue_t *q;
mblk_t *m;
{
TRACE(T_papd, ("papd_wputq 1\n"));
	switch(m->b_datap->db_type) {
	case M_IOCTL:
		putq(q, m);
		break;

	default:
		freemsg(m);
	}
}

static 
papd_wsrvc(q)
queue_t *q;
{
	mblk_t *m;
	struct papd_state *papdp;
	struct iocblk *iocbp;

TRACE(T_papd, ("papd_wsrvc 1\n"));
	papdp = (struct papd_state *)q->q_ptr;

	/*
	 *	Loop processing IOCTLs
	 */

	while (m = getq(q)) {
TRACE(T_papd, ("papd_wsrvc 2\n"));
		switch(m->b_datap->db_type) {
		case M_IOCTL:
TRACE(T_papd, ("papd_wsrvc 3\n"));
			iocbp = (struct iocblk *)m->b_rptr;
			switch(iocbp->ioc_cmd) {
			case AT_PAPD_SET_STATUS:
TRACE(T_papd, ("papd_wsrvc 4\n"));

				/*
				 *	SET_STATUS just sets up the
				 *	status message in the queue head
				 */

				if (papdp->papd_status)
					freemsg(papdp->papd_status);
				papdp->papd_status = m->b_cont;
				m->b_cont = NULL;
				iocbp->ioc_count = 0;
				m->b_datap->db_type = M_IOCACK;
TRACE(T_papd, ("papd_wsrvc 5\n"));
				qreply(q, m);
				break;
		
			case AT_PAPD_GET_NEXT_JOB:
TRACE(T_papd, ("papd_wsrvc 6\n"));

				/*
				 *	GET_NEXT_JOB queues the IOCTL packet and
				 *	jabs the arbitation state machine
				 */

				switch (papdp->papd_state) {
				case PAPD_BLOCKED:
TRACE(T_papd, ("papd_wsrvc 7\n"));
					papdp->papd_state = PAPD_WAITING;
					/* fall thru */

				case PAPD_WAITING:
				case PAPD_ARBITRATE:
TRACE(T_papd, ("papd_wsrvc 8\n"));
					if (papdp->papd_get_next)
						freemsg(papdp->papd_get_next);
TRACE(T_papd, ("papd_wsrvc 9\n"));
					papdp->papd_get_next = m;
					break;

				case PAPD_UNBLOCKED:
TRACE(T_papd, ("papd_wsrvc 10\n"));
					/*
					 *	We move from the UNBLOCKED to the
					 *	BLOCKED state when no IOCTL is received
					 *	within 1/2 second. Untimeout this
					 *	transition when we get one
					 */

					untimeout(papd_block, q);
					if (papdp->papd_get_next)
						freemsg(papdp->papd_get_next);
TRACE(T_papd, ("papd_wsrvc 11\n"));
					papdp->papd_get_next = m;
					if (papdp->papd_requests) 
						papd_complete(q);
TRACE(T_papd, ("papd_wsrvc 12\n"));
					break;
				default:
TRACE(T_papd, ("papd_wsrvc 13\n"));
					goto iocnak;
				}
				break;
			default:
			iocnak:
TRACE(T_papd, ("papd_wsrvc 14\n"));
				m->b_datap->db_type = M_IOCNAK;
				if (m->b_cont) {
					freemsg(m->b_cont);
					m->b_cont = NULL;
				}
				iocbp->ioc_count = NULL;
				qreply(q, m);
				break;
			}
			break;
		default:
TRACE(T_papd, ("papd_wsrvc 15\n"));
			freemsg(m);
			break;
		}
	}
TRACE(T_papd, ("papd_wsrvc 16\n"));
}

/*
 *	The arbitration timeout routine is called to move us from ARBITRATE to
 *	UNBLOCKED via a timeout when the arbitration timeout has terminated.
 *	It calls papd_complete() to wake up the current waiting ioctl and the
 *	highest priority message
 */

static
papd_arbitrate(q)
queue_t *q;
{
	struct papd_state *papdp;

TRACE(T_papd, ("papd_arbitrate 1\n"));
	papdp = (struct papd_state *)q->q_ptr;
	papdp->papd_state = PAPD_UNBLOCKED;
	papd_complete(q);
}

/*
 *	Papd_complete() is called when an open/get_next_job is oked. It replies
 *	to the open request and returns a DDP/ATP header for use by the get_next_job
 */

static
papd_complete(q)
queue_t *q;
{
	struct papd_state *papdp;
	register int s;
	mblk_t *m1, *m2;
	at_ddp_t *ddp1, *ddp2;
	at_atp *atp1, *atp2;
	struct iocblk *iocbp;
	int i;
	

TRACE(T_papd, ("papd_complete 1\n"));
	papdp = (struct papd_state *)q->q_ptr;

	/*
	 *	Untimeout any pending state transitions
	 */

	s = splclk();
	if (papdp->papd_timing) {
TRACE(T_papd, ("papd_complete 2\n"));
		papdp->papd_timing = 0;
		untimeout(papd_block, q);
	}
	splx(s);
	if (papdp->papd_state != PAPD_UNBLOCKED ||
	    papdp->papd_requests == NULL) {
TRACE(T_papd, ("papd_complete 3\n"));
		return;
	}

	/*
	 *	If there is an ioctl waiting complete it
	 */

	if (papdp->papd_get_next) {
TRACE(T_papd, ("papd_complete 4\n"));

		/*
		 *	get the ioctl packet
		 */

		m1 = papdp->papd_get_next;
		papdp->papd_get_next = NULL;

		/*
		 *	get the connection request
		 */

		s = splstr();
		m2 = papdp->papd_requests;
		papdp->papd_requests = m2->b_next;
		splx(s);

		/*
		 *	get pointers to the ATP/DDP headers
		 */

		ddp1 = ATP_DDP_HDR(&m1->b_cont->b_rptr[sizeof(struct atp_set_default)]);
		ddp2 = ATP_DDP_HDR(m2->b_cont->b_rptr);
		atp1 = ATP_ATP_HDR(&m1->b_cont->b_rptr[sizeof(struct atp_set_default)]);
		atp2 = ATP_ATP_HDR(m2->b_cont->b_rptr);

		/*
		 *	Set up the ddp headers
		 */

		ddp1->dst_node = ddp2->src_node;
		ddp2->dst_node = ddp2->src_node;
		C16(ddp1->dst_net, ddp2->src_net);
		C16(ddp2->dst_net, ddp2->src_net);
		ddp1->dst_socket = atp2->at_atp_data[0];
		ddp2->dst_socket = ddp2->src_socket;

		/*
		 *	Return the user's connection id to the server
		 */

		atp1->at_atp_user_bytes[0] = atp2->at_atp_user_bytes[0];

		/*
		 *	next set up the commands
		 */

		atp1->at_atp_user_bytes[1] = AT_PAP_TYPE_SEND_DATA;
		atp2->at_atp_user_bytes[1] = AT_PAP_TYPE_OPEN_CONN_REPLY;

		/*
	 	 *	now return the socket number
		 */

		atp1->at_atp_data[0] = atp2->at_atp_data[0];
		atp2->at_atp_data[0] = ddp1->src_socket;

		/*
		 *	And find the flow quantum
		 */

		i = atp2->at_atp_data[1];
		if (i < 1) {
			i = 1;
		} else 
		if (i > 8) {
			i = 8;
		}
		atp2->at_atp_data[1] = i;
		atp1->at_atp_bitmap_seqno = (1<<i)-1;

		/*
		 *	finally return the open connection complete
		 */

		atp2->at_atp_data[2] = 0;
		atp2->at_atp_data[3] = 0;
		atp2->at_atp_eom = 1;
		atp2->at_atp_bitmap_seqno = 0;
		iocbp = (struct iocblk *)m2->b_rptr;
		iocbp->ioc_cmd = AT_ATP_SEND_RESPONSE;
		iocbp->ioc_error = 0;
		iocbp->ioc_count = msgdsize(m2->b_cont);
TRACE(T_papd, ("papd_complete 5\n"));
		putnext(q, m2);

		/*
		 *	and wake up the process
		 */

		iocbp = (struct iocblk *)m1->b_rptr;
		iocbp->ioc_count = msgdsize(m1->b_cont);
		m1->b_datap->db_type = M_IOCACK;
TRACE(T_papd, ("papd_complete 6\n"));
		qreply(q, m1);
	}
TRACE(T_papd, ("papd_complete 7\n"));

	/*
	 *	finally run a timeout untill there are no more ioctls
	 */

	papdp->papd_timing = 1;
	timeout(papd_block, q, HZ/2);
}

/*
 *	papd_block is called when no ioctls have been received in the UNBLOCKED
 *	state for a period of time (1/2 second). In this case we respond to the
 *	unacknowledged open requests with open failures.
 */

static
papd_block(q)
queue_t *q;
{
	struct iocblk *iocbp;
	mblk_t *m, *m2, *m3;
	at_atp *atp;
	at_ddp_t *ddp;
	struct papd_state *papdp;

TRACE(T_papd, ("papd_block 1\n"));
	papdp = (struct papd_state *)q->q_ptr;
	if (papdp->papd_state != PAPD_UNBLOCKED)
		return;
	while (papdp->papd_requests) {
TRACE(T_papd, ("papd_block 3\n"));
		m2 = NULL;
		if (papdp->papd_status) {
			m2 = dupmsg(papdp->papd_status);
			if (m2 == NULL) {
TRACE(T_papd, ("papd_block 4\n"));
				timeout(papd_block, q, HZ/8);
				return;
			}
		}
		m = papdp->papd_requests;
		papdp->papd_requests = m->b_next;
		atp = ATP_ATP_HDR(m->b_cont->b_rptr); 
		atp->at_atp_user_bytes[1] = AT_PAP_TYPE_OPEN_CONN_REPLY;
		atp->at_atp_data[2] = 0xff;
		atp->at_atp_data[3] = 0xff;
		ddp = ATP_DDP_HDR(m->b_cont->b_rptr); 
		ddp->dst_node = 	ddp->src_node;
		C16(ddp->dst_net, ddp->src_net);
		ddp->dst_socket = 	ddp->src_socket;
		atp->at_atp_eom = 	1;
		atp->at_atp_bitmap_seqno = 0;
		atp->at_atp_user_bytes[0] = 0;
		m3 = m->b_cont;
		while (m3->b_cont)
			m3 = m3->b_cont;
		m3->b_cont = m2;
		iocbp = (struct iocblk *)m->b_rptr;
		iocbp->ioc_cmd = AT_ATP_SEND_RESPONSE;
		iocbp->ioc_error = 0;
		iocbp->ioc_count = msgdsize(m->b_cont);
TRACE(T_papd, ("papd_block 5\n"));
		putnext(q, m);
	}
	papdp->papd_state = PAPD_BLOCKED;
}

/*
 *	This routine reschedules a queue to poll for a resource condition
 */

static 
papd_wake(q)
queue_t *q;
{
TRACE(T_papd, ("papd_wake 1\n"));
	qenable(q);
}

/*
 *	This also polls a queue ... with checks to make sure there is only ever one
 *	outstanding
 */

static
papd_outwake(q)
queue_t *q;
{
	struct papd_state *papdp;

TRACE(T_papd, ("papd_outwake 1\n"));
	papdp = (struct papd_state *)q->q_ptr;
	papd->papd_outtimer = 0;
	qenable(q);
}
