#ifndef lint	/* .../appletalk/at/pap/at_pap.c */
#define _AC_NAME at_pap_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:03:40}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_pap.c on 87/11/11 21:03:40";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)at_pap.c	UniPlus VVV.2.1.9	*/

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
#include <fcntl.h>
#include "../appletalk.h"

#define NPAPREQUESTS 2

extern T_pap;

static int pap_open();
static int pap_close();
static int pap_rsrvc();
static int pap_rputq();
static int pap_wsrvc();
static int pap_wputq();
static int pap_timeout();
extern int nulldev();
extern int wakeup();
extern int putq();

static struct 	module_info pap_minfo = { 98, "at_pap", 0, 256, 256, 256, NULL };
static struct	qinit paprdata = { pap_rputq, pap_rsrvc, pap_open, pap_close,
			nulldev, &pap_minfo, NULL};
static struct	qinit papwdata = { pap_wputq, pap_wsrvc, pap_open, pap_close, 
			nulldev, &pap_minfo, NULL};
struct	streamtab pap_info = {&paprdata, &papwdata, NULL, NULL};


struct pap_state {
	unsigned char	pap_inuse;		/* true if this one is allocated */
	unsigned char	pap_tickle;		/* true if we are tickling the other
						   end */
	unsigned char	pap_request;		/* bitmap from a received request */
	unsigned char	pap_read;		/* true if a read ioctl is waiting */
	unsigned char	pap_eof;		/* true if we have received an EOF */
	unsigned char	pap_eof_sent;		/* true if we have sent an EOF */
	unsigned char	pap_sent;		/* true if we have sent anything (and
						   therefore may have to send an eof
						   on close) */
	unsigned char	pap_error;		/* error message from read request */
	unsigned char	pap_timer;		/* a timeout is pending */
	unsigned char	pap_closing;		/* the link is closing and/or closed */
	unsigned char	pap_request_count;	/* number of outstanding requests */
	unsigned char	pap_req_timer;		/* the request timer is running */
	unsigned char	pap_ending;		/* we are waiting for atp to flush */
	unsigned char	pap_read_ignore;	/* we are in 'read with ignore' mode */
	at_socket	pap_req_socket;		/* the request timer is running */
	unsigned short	pap_send_count;		/* the sequence number to send on the
					 	   next send data request */
	unsigned short	pap_rcv_count;		/* the sequence number expected to
						   receive on the next request */
	unsigned short	pap_tid;		/* ATP transaction ID for responses */
	unsigned short	pap_reqid;		/* our request ID */
	int		pap_tickle_id;		/* the transaction ID for tickles */
	mblk_t		*pap_hdr;		/* pointer to basic message header for
						   all transactions */
	mblk_t 		*pap_rcv;		/* a received message from a read */
};

static pap_read_ignore();
static struct pap_state paps[NPAPSESSIONS];
static int pap_wanted;

static int
pap_open(q, dev, flag, sflag, err, devp
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
	int i;
	struct pap_state *papp;
	mblk_t *m;
	struct iocblk *iocbp;
	register int s;

	TRACE(T_pap, ("pap_open 1\n"));

	/*
	 *	loop looking for a free session state variable, if none are
	 *	found honour the locking protocol and wait until one is available.
	 *	When one is found initialise it and allocate some ATP packets for
	 *	notification of incoming requests
	 */

	for (;;) {
		papp = paps;
		for (i = 0; i < NPAPSESSIONS; i++, papp++)
		if (papp->pap_inuse == 0) {
			papp->pap_inuse = 1;
			papp->pap_tickle = 0;
			papp->pap_send_count = 1;
			papp->pap_rcv_count = 1;
			papp->pap_hdr = NULL;
			papp->pap_rcv = NULL;
			papp->pap_read = 0;
			papp->pap_request = 0;
			papp->pap_error = 0;
			papp->pap_eof = 0;
			papp->pap_eof_sent = 0;
			papp->pap_read_ignore = 0;
			papp->pap_closing = 0;
			papp->pap_reqid = 0xffff;
			papp->pap_timer = 0;
			papp->pap_ending = 0;
			papp->pap_request_count = 0;
			papp->pap_req_timer = 0;
			WR(q)->q_ptr = q->q_ptr = (char *)papp;
			while (papp->pap_request_count < NPAPREQUESTS) {
				m = allocb(sizeof(struct iocblk), BPRI_LO);
				if (m) {
					m->b_wptr += sizeof(struct iocblk);
					iocbp = (struct iocblk *)m->b_rptr;
					iocbp->ioc_cmd = AT_ATP_GET_REQUEST;
					iocbp->ioc_count = 0;
					iocbp->ioc_error = 0;
					putnext(WR(q), m);
					papp->pap_request_count++;
					continue;
				}
				s = splclk();
				timeout(wakeup, q, HZ/8);
				if (sleep(q, STOPRI|PCATCH)) {
					untimeout(wakeup, q);
					splx(s);
					*err = EINTR;
					papp->pap_inuse = 0;
					if (pap_wanted) {
						pap_wanted = 0;
						wakeup(&pap_wanted);
					}
					return(OPENFAIL);
				}
				splx(s);
			}
			return(0);
		}
		if (flag&O_NDELAY) {
			*err = EAGAIN;
			return(OPENFAIL);
		}
		pap_wanted = 1;
		if (sleep(&pap_wanted, STOPRI|PCATCH)) {
			*err = EINTR;
			return(OPENFAIL);
		}
	}
}

/*
 *	Close tries to send an eof if we have written. It then tries to exchange
 *	a close handshake ... if one has not already been received
 */

static 
pap_close(q)
queue_t *q;
{
	struct pap_state *papp;
	mblk_t *m;
	int i, s;
	struct iocblk *iocbp;
	at_atp *atp;
	at_ddp_t *ddp;

	papp = (struct pap_state *)q->q_ptr;
	papp->pap_timer = 0;
	untimeout(pap_timeout, q);
	untimeout(pap_read_ignore, q);
	flushq(q, 1);
	flushq(WR(q), 1);
TRACE(T_pap, ("pap_close 1\n"));
	if (papp->pap_sent && !papp->pap_eof_sent && papp->pap_request && papp->pap_hdr){

		/*
		 *	If we have not sent an EOF and there are outstanding requests
		 *	send an ATP EOF message. This flushes out any pending data
		 *	packets and tells the other end "EOF".
		 */

TRACE(T_pap, ("pap_close 2\n"));
		for (;;) {
TRACE(T_pap, ("pap_close 2a\n"));
			m = copymsg(papp->pap_hdr);
			if (m)
				break;
			s = splclk();
			timeout(wakeup, q, HZ/8);
			(void)sleep(q, PZERO);
			splx(s);
		}
		m->b_cont->b_rptr += sizeof(struct atp_set_default);
TRACE(T_pap, ("pap_close 2b\n"));
		iocbp = (struct iocblk *)m->b_rptr;
		iocbp->ioc_cmd = AT_ATP_SEND_RESPONSE_EOF;
		iocbp->ioc_count = msgdsize(m->b_cont);
		iocbp->ioc_error = 0;
		atp = ATP_ATP_HDR(m->b_cont->b_rptr);
		for (i = 0; i < 8; i++)
		if ((1<<i)&papp->pap_request)
			break;
TRACE(T_pap, ("pap_close 3 i = %d\n",i));
		atp->at_atp_bitmap_seqno = i;
		atp->at_atp_user_bytes[1] = AT_PAP_TYPE_DATA;
		atp->at_atp_user_bytes[3] = 0;
		W16(&atp->at_atp_transaction_id[0], papp->pap_tid);
		atp->at_atp_user_bytes[2] = 1;
TRACE(T_pap, ("pap_close 4\n"));
		putnext(WR(q), m);
	}
TRACE(T_pap, ("pap_close 5\n"));

	/*
	 *	Cancel the tickles
	 */

	for (;;) {
TRACE(T_pap, ("pap_close 5c\n"));
		m = allocb(sizeof(struct iocblk), BPRI_LO);
		if (m)
			break;
		s = splclk();
		timeout(wakeup, q, HZ/8);
		(void)sleep(q, PZERO);
		splx(s);
	}
	for (;;) {
TRACE(T_pap, ("pap_close 5d\n"));
		m->b_cont = allocb(sizeof(int), BPRI_LO);
		if (m->b_cont)
			break;
		s = splclk();
		timeout(wakeup, q, HZ/8);
		(void)sleep(q, PZERO);
		splx(s);
	}
	m->b_wptr += sizeof(struct iocblk);
	m->b_cont->b_wptr += sizeof(int);
	*(int *)(m->b_cont->b_rptr) = papp->pap_tickle_id;
TRACE(T_pap, ("pap_close 5e\n"));
	iocbp = (struct iocblk *)m->b_rptr;
	iocbp->ioc_cmd = AT_ATP_CANCEL_REQUEST;
	iocbp->ioc_count = sizeof(int);
	iocbp->ioc_error = 0;
	putnext(WR(q), m);

	if (papp->pap_hdr && !papp->pap_closing) {

		/*
		 *	If we are completely open and haven't been closed from the
		 *	other end
		 */

		for (;;) {
TRACE(T_pap, ("pap_close 5a\n"));
			m = allocb(sizeof(struct iocblk), BPRI_LO);
			if (m)
				break;
			s = splclk();
			timeout(wakeup, q, HZ/8);
			(void)sleep(q, PZERO);
			splx(s);
		}
		m->b_wptr += sizeof(struct iocblk);
TRACE(T_pap, ("pap_close 5b\n"));
		iocbp = (struct iocblk *)m->b_rptr;
		iocbp->ioc_cmd = AT_ATP_SET_FINISH;
		iocbp->ioc_count = msgdsize(m->b_cont);
		iocbp->ioc_error = 0;
		papp->pap_ending = 1;
		putnext(WR(q), m);
TRACE(T_pap, ("pap_close 5c\n"));
		s = splclk();
		if (papp->pap_ending) {
			timeout(wakeup, q, HZ*60*4);
			(void)sleep(q, PZERO);
			untimeout(wakeup, q);
		}
		splx(s);

TRACE(T_pap, ("pap_close 6\n"));
		ddp = ATP_DDP_HDR(&papp->pap_hdr->b_cont->b_rptr[
						sizeof(struct atp_set_default)]);
		if (!papp->pap_closing && ddp->dst_socket) {

			/*
			 *	Build a pap message from the header that we used to
			 *	clone from ... and send a close request to the other end
			 */

			papp->pap_closing = 1;
			m = papp->pap_hdr;
			papp->pap_hdr = NULL;
			atp = ATP_ATP_HDR(&m->b_cont->b_rptr[
					sizeof(struct atp_set_default)]);
			atp->at_atp_bitmap_seqno = 1;
			atp->at_atp_user_bytes[1] = AT_PAP_TYPE_CLOSE_CONN;
			iocbp = (struct iocblk *)m->b_rptr;
			iocbp->ioc_cmd = AT_ATP_ISSUE_REQUEST_DEF;
			iocbp->ioc_error = 0;
			iocbp->ioc_count = msgdsize(m->b_cont);
			s = splclk();
			putnext(WR(q), m);
			timeout(wakeup, &papp->pap_closing, HZ*2);
			(void) sleep(&papp->pap_closing, PZERO);
			untimeout(wakeup, &papp->pap_closing);
			splx(s);
		}
	}

	/*
	 *	Now get rid of any unused resources
	 */

TRACE(T_pap, ("pap_close 7\n"));
	papp->pap_inuse = 0;
	if (papp->pap_hdr)
		freemsg(papp->pap_hdr);
	if (papp->pap_rcv)
		freemsg(papp->pap_rcv);
	if (pap_wanted) {
		pap_wanted = 0;
		wakeup(&pap_wanted);
	}
}

/*
 *	Read putq handles incoming packets 
 */

static
pap_rputq(q, m)
queue_t *q;
mblk_t *m;
{
	at_atp *atp1;
	at_ddp_t *ddp1;
	mblk_t *m2;
	struct pap_state *papp;
	struct iocblk *iocbp;

TRACE(T_pap, ("pap_rputp 1\n"));
	papp = (struct pap_state *)q->q_ptr;
	switch(m->b_datap->db_type) {
	case M_HANGUP:
		papp->pap_closing = 1;
	case M_IOCNAK:
	case M_IOCACK:

		/*
		 *	Pass these down the line
		 */

		TRACE(T_pap, ("pap_rputp 2\n"));
		putnext(q, m);
		break;

	case M_DATA:
TRACE(T_pap, ("pap_rputp 2\n"));

		/*
		 *	If it is a data message then it is REALLY a data packet
	 	 *	pretending to be an IOCTL packet. And as such is really a
		 *	reply to one we sent up earlier
		 */

		iocbp = (struct iocblk *)m->b_rptr;
		switch(iocbp->ioc_cmd) {
		case AT_ATP_GET_REQUEST:


			/*
			 *	Get request means an incoming ATP/PAP request ... these
			 *		are of 3 types:
			 *			
			 *		SEND_DATA
			 *		TICKLE
			 *		CLOSE
			 */

TRACE(T_pap, ("pap_rputp 3\n"));
			if (papp->pap_timer) {
				untimeout(pap_timeout, q);
				timeout(pap_timeout, q, 130*HZ);
			}
			ASSERT(iocbp->ioc_error == 0);
			papp->pap_request_count--;
			ddp1 = ATP_DDP_HDR(m->b_cont->b_rptr);
			atp1 = ATP_ATP_HDR(m->b_cont->b_rptr);
			if ((papp->pap_ending || papp->pap_closing) &&
			    atp1->at_atp_user_bytes[1] != AT_PAP_TYPE_CLOSE_CONN) {
TRACE(T_pap, ("pap_rputp 4\n"));
				iocbp->ioc_cmd = AT_ATP_RELEASE_RESPONSE;
				qreply(q, m);
				m = NULL;
				break;
			}
			switch (atp1->at_atp_user_bytes[1]) {
			case AT_PAP_TYPE_TICKLE:

				/*
				 *	If it is a tickle then release the incoming 
				 *	ATP request and re prime the timer
				 */

TRACE(T_pap, ("pap_rputp 6\n"));
				iocbp->ioc_cmd = AT_ATP_RELEASE_RESPONSE;
				qreply(q, m);
				m = NULL;
				break;

			case AT_PAP_TYPE_SEND_DATA:

				/*
				 *	If it is data then put it in the RCV q for later
				 *	processing
				 */

TRACE(T_pap, ("pap_rputp 7 = %d\n", R16(&atp1->at_atp_user_bytes[2])));
				if (papp->pap_request == 0 &&
				    (R16(&atp1->at_atp_user_bytes[2]) == 0 ||
				     R16(&atp1->at_atp_user_bytes[2]) ==
						papp->pap_rcv_count) && m->b_cont) {
					putq(q, m);
					m = NULL;
				}
				break;


			case AT_PAP_TYPE_CLOSE_CONN:

				/*
				 *	If it is a valid close request then mark us
				 *	as closing and send a response
				 */

TRACE(T_pap, ("pap_rputp 7c\n"));
				if (papp->pap_reqid == atp1->at_atp_user_bytes[0]) {
TRACE(T_pap, ("pap_rputp 7d\n"));
					if (papp->pap_ending) {
						wakeup(q);
						papp->pap_ending = 0;
					}
					papp->pap_error = ETIMEDOUT;
					wakeup(&papp->pap_closing);
					C16(ddp1->dst_net, ddp1->src_net);
					ddp1->dst_node = ddp1->src_node;
					ddp1->dst_socket = ddp1->src_socket;
					atp1->at_atp_eom = 1;
					atp1->at_atp_bitmap_seqno = 0;
					atp1->at_atp_user_bytes[1] =
						AT_PAP_TYPE_CLOSE_CONN_REPLY;
					iocbp->ioc_cmd = AT_ATP_SEND_RESPONSE_EOF;
					iocbp->ioc_error = 0;
					putnext(WR(q), m);
					m = NULL;
					qenable(WR(q));
					papp->pap_closing = 1;
				}
				break;
			}
TRACE(T_pap, ("pap_rputp 8\n"));
			break;

		case AT_ATP_ISSUE_REQUEST_DEF_NW:
		case AT_ATP_ISSUE_REQUEST_NW:

			/*
			 *	This is the reply from sending the TICKLE packet
			 */

			papp->pap_tickle_id = iocbp->ioc_rval;
TRACE(T_pap, ("pap_rputp 8a 0x%x\n", iocbp->ioc_rval));
			break;

		case AT_ATP_ISSUE_REQUEST:
		case AT_ATP_ISSUE_REQUEST_DEF:

			/*
			 *	An issue request is the reply from a request they are:
			 *
			 *		DATA
			 *		CLOSE ACK
			 */

TRACE(T_pap, ("pap_rputp 9\n"));
			if (papp->pap_timer) {
				untimeout(pap_timeout, q);
				timeout(pap_timeout, q, 130*HZ);
			}
			atp1 = ATP_ATP_HDR(m->b_cont->b_cont->b_rptr);
			switch(atp1->at_atp_user_bytes[1]) {
			case AT_PAP_TYPE_CLOSE_CONN_REPLY:

				/*
				 *	If it is a clse ack then wake up the closing
				 *	process and let it complete its close
				 */

TRACE(T_pap, ("pap_rputp 7a\n"));
				wakeup(&papp->pap_closing);
				break;

			case AT_PAP_TYPE_DATA:

				/*
				 *	check to see if we are in 'read with ignore' mode
				 */

				if (papp->pap_read_ignore) {
					freemsg(m);
					m = NULL;
					timeout(pap_read_ignore, WR(q), HZ);
					break;
				}

				/*
				 *	Check to see if there is a read here already
				 */

				if (papp->pap_read == 0)
					break;

				/*
				 *	If there was an error save it and wake the other
				 *	end
				 */

				if (iocbp->ioc_error) {
TRACE(T_pap, ("pap_rputp 10\n"));
					papp->pap_error = iocbp->ioc_error;
					qenable(WR(q));
					papp->pap_read = 0;
					break;
				}
TRACE(T_pap, ("pap_rputp 11\n"));

				/*
				 *	Otherwise put the packet where the ioctl can
				 *		get it and wake the other end
				 */

				papp->pap_rcv = m->b_cont;
				papp->pap_rcv->b_cont->b_rptr += AT_DDP_X_HDR_SIZE;
				papp->pap_read = 0;
				papp->pap_error = 0;
				qenable(WR(q));
				m->b_cont = NULL;
				break;
			}
			break;
		case AT_ATP_SET_FINISH:
TRACE(T_pap, ("pap_rputp 93\n"));
			if (papp->pap_ending) {
				wakeup(q);
				papp->pap_ending = 0;
			}
			break;
		}
		if (m)
			freemsg(m);
		break;
	default:
		freemsg(m);
	}
TRACE(T_pap, ("pap_rputp 12\n"));

	/*
	 *	loop sending request packets if there are not enough outstanding
	 */

	while (papp->pap_request_count < NPAPREQUESTS) {
TRACE(T_pap, ("pap_rputp 13\n"));
		m = allocb(sizeof(struct iocblk), BPRI_LO);
		if (m == NULL) {
TRACE(T_pap, ("pap_rputp 14\n"));
			if (papp->pap_request_count == 0 && papp->pap_req_timer == 0) {
TRACE(T_pap, ("pap_rputp 15\n"));
				papp->pap_req_timer = 1;
				timeout(qenable, q, HZ/8);
			}
			break;
		}
		papp->pap_request_count++;
		m->b_wptr += sizeof(struct iocblk);
		iocbp = (struct iocblk *)m->b_rptr;
		iocbp->ioc_cmd = AT_ATP_GET_REQUEST;
		iocbp->ioc_error = 0;
		iocbp->ioc_count = 0;
		putnext(WR(q), m);
	}
TRACE(T_pap, ("pap_rputp 16\n"));
}

/*
 *	The read service routine handles incoming requests, it makes sure they are
 *		valid (it also may have to wait until an open is completed)
 */

static
pap_rsrvc(q)
queue_t *q;
{
	mblk_t *m;
	at_atp *atp1, *atp2;
	at_ddp_t *ddp1, *ddp2;
	struct pap_state *papp;
	struct iocblk *iocbp;

	papp = (struct pap_state *)q->q_ptr;
TRACE(T_pap, ("pap_rsrvc 1\n"));

	/*
	 *	untimeout the request timer (we may free messages and we will run
	 *	one later anyway)
	 */

	if (papp->pap_req_timer) {
		papp->pap_req_timer = 0;
		untimeout(qenable, q);
	}

	/*
	 *	loop getting messages
	 */

	while (m = getq(q)) {
TRACE(T_pap, ("pap_rsrvc 2\n"));
		switch(m->b_datap->db_type) {
		case M_DATA:
TRACE(T_pap, ("pap_rsrvc 3\n"));

			/*
			 *	If there is already a message being handled or we are
			 *	not completely open yet wait a while
		 	 */

			if (papp->pap_request ||
			    papp->pap_hdr == NULL) {
TRACE(T_pap, ("pap_rsrvc 4\n"));
				putbq(q, m);
				goto xit;
			}

			/*
			 *	make another check to see if we are completely open
			 */

			ddp2 = ATP_DDP_HDR(&papp->pap_hdr->b_cont->b_rptr[
							sizeof(struct atp_set_default)]);
			if (ddp2->dst_socket == 0) {
TRACE(T_pap, ("pap_rsrvc 5\n"));
				putbq(q, m);
				goto xit;
			}

			/*
			 *	Now make sure this really is the next incoming request 
			 *	and not a bogus one (or one that has been wandering
			 *	the internet a while)
			 */

			ddp1 = ATP_DDP_HDR(m->b_cont->b_rptr);
			atp1 = ATP_ATP_HDR(m->b_cont->b_rptr);
			atp2 = ATP_ATP_HDR(&papp->pap_hdr->b_cont->b_rptr[
							sizeof(struct atp_set_default)]);
			if (ddp1->src_node != ddp2->dst_node ||
			    R16(ddp1->src_net) != R16(ddp2->dst_net) ||
			    atp1->at_atp_user_bytes[0] != papp->pap_reqid ||
			    (R16(&atp1->at_atp_user_bytes[2]) != 0 &&
			     R16(&atp1->at_atp_user_bytes[2]) != papp->pap_rcv_count)) {
TRACE(T_pap, ("pap_rsrvc 6\n"));
				iocbp = (struct iocblk *)m->b_rptr;
				iocbp->ioc_cmd = AT_ATP_RELEASE_RESPONSE;
				qreply(q, m);
				break;
			}

			/*
			 *	bump the received sequence number and save enough info 
			 *	to make the responses. Finally wake the other side to
			 *	make the replies
			 */

			papp->pap_rcv_count++;
			if (papp->pap_rcv_count == 0)
				papp->pap_rcv_count = 1;
			papp->pap_request = atp1->at_atp_bitmap_seqno;
			papp->pap_req_socket = ddp1->src_socket;
			papp->pap_tid = R16(&atp1->at_atp_transaction_id[0]);
			qenable(WR(q));
TRACE(T_pap, ("pap_rsrvc 7\n"));
			/* fall thru */
		default:
TRACE(T_pap, ("pap_rsrvc 8\n"));
			freemsg(m);
		}
	}
xit:

	/*
	 *	Again loop getting more packets to wait for requests
	 */

	while (papp->pap_request_count < NPAPREQUESTS) {
TRACE(T_pap, ("pap_rsrvc 13\n"));
		m = allocb(sizeof(struct iocblk), BPRI_LO);
		if (m == NULL) {
TRACE(T_pap, ("pap_rsrvc 14\n"));
			if (papp->pap_request_count == 0 && papp->pap_req_timer == 0) {
TRACE(T_pap, ("pap_rsrvc 15\n"));
				papp->pap_req_timer = 1;
				timeout(qenable, q, HZ/8);
			}
			break;
		}
		papp->pap_request_count++;
		m->b_wptr += sizeof(struct iocblk);
		iocbp = (struct iocblk *)m->b_rptr;
		iocbp->ioc_cmd = AT_ATP_GET_REQUEST;
		iocbp->ioc_error = 0;
		iocbp->ioc_count = 0;
		putnext(WR(q), m);
	}
TRACE(T_pap, ("pap_rsrvc 16\n"));
}

static
pap_wputq(q, m)
queue_t *q;
mblk_t *m;
{
	switch(m->b_datap->db_type) {
	case M_IOCTL:
		putq(q, m);
		break;

	default:
		freemsg(m);
	}
}

static 
pap_wsrvc(q)
queue_t *q;
{
	struct atp_result *pp;
	int i,l;
	register mblk_t *m;
	mblk_t *m2, *m3;
	register struct pap_state *papp;
	struct iocblk *iocbp, *iocbp2;
	register at_atp *atp;
	register at_ddp_t *ddp;
	char *cp1, *cp2;

TRACE(T_pap, ("pap_wsrvc 1\n"));
	papp = (struct pap_state *)q->q_ptr;

	/*
	 *	Loop getting requests
	 */

	while (m = getq(q)) {
TRACE(T_pap, ("pap_wsrvc 2\n"));

		/*
		 *	Get the commands
		 */

		switch(m->b_datap->db_type) {
		case M_IOCTL:
TRACE(T_pap, ("pap_wsrvc 3\n"));
			iocbp = (struct iocblk *)m->b_rptr;
			switch(iocbp->ioc_cmd) {
			case AT_PAP_SETHDR:

				/*
				 *	validate the sethdr
				 */

TRACE(T_pap, ("pap_wsrvc 3a\n"));
				if (iocbp->ioc_count != (AT_PAP_HDR_SIZE +
							 sizeof(struct atp_set_default)))
					goto iocnak;

				/*
				 *	Allocate a dummy ioctl packet
				 */

				m3 = allocb(sizeof(struct iocblk), BPRI_LO);
				if (m3 == NULL) {
TRACE(T_pap, ("pap_wsrvc 4\n"));
					putbq(q, m);
					timeout(qenable, q, HZ/8);
					return;
				}

				/*
				 *	If we are finally open (we finally know the
				 *	other end's socket number) then start tickling
				 */

				ddp = ATP_DDP_HDR(&m->b_cont->b_rptr[
						sizeof(struct atp_set_default)]);
				if (ddp->dst_socket && !papp->pap_tickle) {
					qenable(RD(q));
TRACE(T_pap, ("pap_wsrvc 5\n"));

					/*
					 *	honour flow control
					 */

					if (!canput(q->q_next)) {
TRACE(T_pap, ("pap_wsrvc 6\n"));
						freeb(m3);
						putbq(q, m);
						return;
					}

					/*
					 *	get space for the tickle request
					 */

					m2 = allocb(sizeof(struct iocblk), BPRI_LO);
					if (m2 == NULL) {
TRACE(T_pap, ("pap_wsrvc 7\n"));
						freeb(m3);
						putbq(q, m);
						timeout(qenable, q, HZ/8);
						return;
					}
					m2->b_cont = copyb(m->b_cont);
					if (m2->b_cont == NULL) {
TRACE(T_pap, ("pap_wsrvc 8\n"));
						freeb(m3);
						freeb(m2);
						putbq(q, m);
						timeout(qenable, q, HZ/8);
						return;
					}

					/*
					 *	build the ATP packet for the request
					 */

					m2->b_wptr += sizeof(struct iocblk);
					((struct atp_set_default *)m2->b_cont->b_rptr)->
						def_retries = ATP_INFINITE_RETRIES;
					((struct atp_set_default *)m2->b_cont->b_rptr)->
						def_rate = 100*20;
					iocbp2 = (struct iocblk *)m2->b_rptr;
					iocbp2->ioc_cmd = AT_ATP_ISSUE_REQUEST_DEF_NW;
					iocbp2->ioc_error = 0;
					iocbp2->ioc_count = AT_PAP_HDR_SIZE +
							sizeof(struct atp_set_default);
					atp = ATP_ATP_HDR(&m2->b_cont->b_rptr[
							sizeof(struct atp_set_default)]);
					atp->at_atp_xo = 0;
					atp->at_atp_bitmap_seqno = 0;
					atp->at_atp_user_bytes[1] = AT_PAP_TYPE_TICKLE;
					atp->at_atp_user_bytes[2] = 0;
					atp->at_atp_user_bytes[3] = 0;
TRACE(T_pap, ("pap_wsrvc 9\n"));

					/*
					 *	send it
					 */

					putnext(q, m2);
					papp->pap_tickle = 1;

					/*
					 *	start the connection timeout
					 */

					if (!papp->pap_timer) {
TRACE(T_pap, ("pap_wsrvc 10\n"));
						timeout(pap_timeout, RD(q), 180*HZ);
						papp->pap_timer = 1;
					}
TRACE(T_pap, ("pap_wsrvc 11\n"));
				}
TRACE(T_pap, ("pap_wsrvc 12\n"));

				/*
				 *	set up the packet header and put it in the
				 *	local state
				 */

				if (papp->pap_hdr)
					freemsg(papp->pap_hdr);
				m3->b_cont = m->b_cont;
				m3->b_wptr += sizeof(struct iocblk);
				atp = ATP_ATP_HDR(&m3->b_cont->b_rptr[
							sizeof(struct atp_set_default)]);
				if (atp->at_atp_bitmap_seqno == 0)
					atp->at_atp_bitmap_seqno = 1;
				iocbp2 = (struct iocblk *)m3->b_rptr;
				iocbp2->ioc_cmd = AT_ATP_ISSUE_REQUEST_DEF;
				iocbp2->ioc_error = 0;
				iocbp2->ioc_count = AT_PAP_HDR_SIZE +
					sizeof(struct atp_set_default);
				papp->pap_hdr = m3;
				papp->pap_reqid = atp->at_atp_user_bytes[0];

				/*
				 *	ack the ioctl
				 */

				m->b_cont = NULL;
				iocbp->ioc_count = 0;
			iocack:
TRACE(T_pap, ("pap_wsrvc 13\n"));
				m->b_datap->db_type = M_IOCACK;
				qreply(q, m);
				break;

			case AT_PAP_READ_IGNORE:
TRACE(T_pap, ("pap_wsrvc X14\n"));

				/*
				 *	can't read if not properly open ....
				 */

				if (papp->pap_hdr == NULL)
					goto iocnak;

				/*
				 *	if received an eof return 0
				 */

				if (papp->pap_eof || papp->pap_read_ignore ||
				    papp->pap_closing) {
TRACE(T_pap, ("pap_wsrvc X15\n"));
					iocbp->ioc_count = 0;
					m->b_datap->db_type = M_IOCACK;
					qreply(q, m);
					break;
				}


				pap_read_ignore(q);


				if (m->b_cont) {
					freemsg(m->b_cont);
					m->b_cont = NULL;
				}

				papp->pap_read_ignore = 1;
				m->b_datap->db_type = M_IOCACK;
				qreply(q, m);
				break;

			case AT_PAP_READ:
TRACE(T_pap, ("pap_wsrvc 14\n"));

				/*
				 *	can't read if not properly open ....
				 */

				if (papp->pap_hdr == NULL)
					goto iocnak;

				/*
				 *	if received an eof return 0
				 */

				if (papp->pap_eof) {
TRACE(T_pap, ("pap_wsrvc 15\n"));
					iocbp->ioc_count = 0;
					m->b_datap->db_type = M_IOCACK;
					qreply(q, m);
					break;
				}

				/*
				 *	if already reading wait for a response
				 */

				if (papp->pap_read) {
TRACE(T_pap, ("pap_wsrvc 16\n"));
					putbq(q, m);
					return;
				}

				/*
				 *	if we got back an error return it
				 */

				if (papp->pap_error) {
TRACE(T_pap, ("pap_wsrvc 17\n"));
					iocbp->ioc_error = papp->pap_error;
					papp->pap_error = 0;
					m->b_datap->db_type = M_IOCNAK;
					qreply(q, m);
				} else
				if (m2 = papp->pap_rcv) {

					/*
					 *	If we got a packet then get the next
					 *	part of it to return to the reader
					 */

TRACE(T_pap, ("pap_wsrvc 18\n"));
					pp = (struct atp_result *)m2->b_rptr;
					for (i = 0; i < 8; i++) 
					if (pp->len[i]) 
						break;
					if (i == 8) {
TRACE(T_pap, ("pap_wsrvc 19\n"));
						freemsg(m2);
						papp->pap_rcv = NULL;
						goto next;
					}

					/*
					 *	get the size and offset
					 */

					l = pp->len[i];
					pp->len[i] = 0;
TRACE(T_pap, ("pap_wsrvc 18 i=%d, l=%d\n",i,l));
					m2 = m2->b_cont;

					/*
					 *	remove the ATP header
					 */

					atp = (at_atp *)m2->b_rptr;
					m2->b_rptr += AT_ATP_HDR_SIZE;
					l -= AT_ATP_HDR_SIZE;

					/*
					 *	if eof flag it
					 */

					if (atp->at_atp_user_bytes[2]) {
						papp->pap_eof = 1;
					}

					/*
					 *	remove empty packets
					 */

					if (m2->b_wptr <= m2->b_rptr) {
TRACE(T_pap, ("pap_wsrvc 20\n"));
						m3 = m2;
						m2 = m2->b_cont;
						freeb(m3);
					}
					if (m->b_cont) {
						freemsg(m->b_cont);
						m->b_cont = NULL;
					}

					/*
					 *	move the data from the incoming reply
					 *	to the ioctl (note we assume that the
					 *	message structure from ATP puts seperate
					 *	packets in different streams messages)
					 */

					if (l) {
						ASSERT(m2);
						m->b_cont = m2;
						while (l) {
TRACE(T_pap, ("pap_wsrvc 21\n"));
							l -= m2->b_wptr - m2->b_rptr;
							if (l <= 0)
								break;
							m2 = m2->b_cont;
						} 
						ASSERT(m2);
						papp->pap_rcv->b_cont = m2->b_cont;
						m2->b_cont = NULL;
					} else {
TRACE(T_pap, ("pap_wsrvc 21a\n"));
						m->b_cont = NULL;
						papp->pap_rcv->b_cont = m2;
					}

					/*
					 *	update the count
					 */

					if (m->b_cont) {
TRACE(T_pap, ("pap_wsrvc 22a\n"));
						iocbp->ioc_count = msgdsize(m->b_cont);
						if (iocbp->ioc_count > 512)
							iocbp->ioc_count = 512;
					} else {
TRACE(T_pap, ("pap_wsrvc 22b\n"));
						iocbp->ioc_count = 0;
					}

					/*
					 *	send the response to the process
					 */

					m->b_datap->db_type = M_IOCACK;
					qreply(q, m);

					/*
					 *	if we are done with the headers
					 *	throw them away
					 */

					if (papp->pap_rcv->b_cont == NULL ||
					    papp->pap_eof) {
TRACE(T_pap, ("pap_wsrvc 23\n"));
						freemsg(papp->pap_rcv);
						papp->pap_rcv = NULL;
					}
				} else {
			next:

					/*
					 *	If we don't have an outstanding request
					 *	or data that has not been returned
					 *	then proceed. First check that we have
					 *	not received a close request
					 */

TRACE(T_pap, ("pap_wsrvc 24\n"));
					if (papp->pap_closing) {
TRACE(T_pap, ("pap_wsrvc 24a\n"));
						iocbp->ioc_count = 0;
						m->b_datap->db_type = M_IOCACK;
						qreply(q, m);
						break;
					}

					/*
				 	 *	next honour flow control
					 */

					if (!canput(q->q_next)) {
TRACE(T_pap, ("pap_wsrvc 16a\n"));
						putbq(q, m);
						return;
					}

					/*
					 *	If we can't build a header then
					 *	wait 'till there is space so we can
					 */

					m2 = copymsg(papp->pap_hdr);
					if (m2 == NULL) {
TRACE(T_pap, ("pap_wsrvc 25\n"));
						putbq(q, m);
						timeout(qenable, q, HZ/8);
						return;
					}

					/*
					 *	clean up our own packet
					 */

					if (m->b_cont) {
						freemsg(m->b_cont);
						m->b_cont = NULL;
					}

					/*
					 * 	build an ATP request
					 */

					atp = ATP_ATP_HDR(&m2->b_cont->b_rptr[
							sizeof(struct atp_set_default)]);
					atp->at_atp_user_bytes[1] =
								AT_PAP_TYPE_SEND_DATA;
					atp->at_atp_eom = 0;
					atp->at_atp_xo = 1;
TRACE(T_pap, ("pap_wsrvc 26 send_count = %d\n",papp->pap_send_count));
					W16(&atp->at_atp_user_bytes[2],
						papp->pap_send_count);
					papp->pap_send_count++;
					if (papp->pap_send_count == 0)
						papp->pap_send_count = 1;

					/*
					 *	send it
					 */

					putnext(q, m2);
					papp->pap_read = 1;
TRACE(T_pap, ("pap_wsrvc 27\n"));
					putbq(q, m);
				}
				break;

			case AT_PAP_WRITE_FLUSH:
			case AT_PAP_WRITE_EOF:
			case AT_PAP_WRITE:
TRACE(T_pap, ("pap_wsrvc 28\n"));

				/*
				 *	Check to see if we are allowed to send a
				 *	write (either not open yet, or closing, or
				 *	we have sent EOF)
				 */

				if (papp->pap_hdr == NULL ||
				    papp->pap_closing ||
				    papp->pap_eof_sent)
					goto iocnak;

				/*
				 *	Next check to see if we are able to send
				 *	packets (either we have to wait for flow
				 *	control or we have not received an incoming
				 *	request). If not we do a qenable to syncronise
				 *	with the other side.
				 */

				if (!canput(q->q_next) ||
				    papp->pap_request == 0) {
TRACE(T_pap, ("pap_wsrvc 29\n"));
					qenable(RD(q));
					putbq(q, m);
					return;
				}

				/*
				 *	Now duplicate the header to use with the reply
				 *	if no space available set up a poll for it
				 */

				m2 = copymsg(papp->pap_hdr);
				if (m2 == NULL) {
TRACE(T_pap, ("pap_wsrvc 30\n"));
					putbq(q, m);
					timeout(qenable, q, HZ/8);
					return;
				}

				/*
				 *	Now fill in the ATP/PAP headers
				 */

				m2->b_cont->b_rptr += sizeof(struct atp_set_default);
				iocbp2 = (struct iocblk *)m2->b_rptr;
				ddp = ATP_DDP_HDR(m2->b_cont->b_rptr);
				ddp->dst_socket = papp->pap_req_socket;
				atp = ATP_ATP_HDR(m2->b_cont->b_rptr);
				for (i = 0; i < 8; i++)
				if ((1<<i)&papp->pap_request)
					break;
TRACE(T_pap, ("pap_wsrvc 31 i = %d\n",i));
				papp->pap_request &= ~(1<<i);
				atp->at_atp_bitmap_seqno = i;
				atp->at_atp_user_bytes[1] = AT_PAP_TYPE_DATA;
				atp->at_atp_user_bytes[3] = 0;
				W16(&atp->at_atp_transaction_id[0], papp->pap_tid);
				if (iocbp->ioc_cmd != AT_PAP_WRITE) {
					if (iocbp->ioc_cmd == AT_PAP_WRITE_EOF) {
						papp->pap_error = ESHUTDOWN; /* in case of another write */
						papp->pap_eof_sent = 1;
						atp->at_atp_user_bytes[2] = 1;
					} else {
						atp->at_atp_user_bytes[2] = 0;
					}
					atp->at_atp_user_bytes[3] = 0;
					papp->pap_request = 0;
					iocbp2->ioc_cmd = AT_ATP_SEND_RESPONSE_EOF;
				} else {
					iocbp2->ioc_cmd = AT_ATP_SEND_RESPONSE;
					atp->at_atp_eom = (papp->pap_request == 0?1:0);
					atp->at_atp_user_bytes[2] = 0;
					atp->at_atp_user_bytes[3] = 0;
				}
				m2->b_cont->b_cont = m->b_cont;
				iocbp2->ioc_count = msgdsize(m2->b_cont);
				iocbp2->ioc_error = 0;
TRACE(T_pap, ("pap_wsrvc 32\n"));
				papp->pap_sent = 1;

				/*
				 *	finally send the packet
				 */

				putnext(q, m2);

				/*
				 *	and return success to the user
				 */

				m->b_cont = NULL;
				iocbp->ioc_count = 0;
				m->b_datap->db_type = M_IOCACK;
TRACE(T_pap, ("pap_wsrvc 33\n"));
				qreply(q, m);
				break;
				
			default:
			iocnak:
TRACE(T_pap, ("pap_wsrvc 34\n"));

				/*
				 *	generic code to retrun an error
				 */

				iocbp->ioc_error = papp->pap_error;
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
TRACE(T_pap, ("pap_wsrvc 35\n"));
			freemsg(m);
			break;
		}
	}
TRACE(T_pap, ("pap_wsrvc 36\n"));
}

/*
 *	Timeout is called when we have not received a TICKLE for 180 secs
 */

static 
pap_timeout(q)
queue_t *q;
{
	mblk_t *m;
	struct pap_state *papp;

	papp = (struct pap_state *)q->q_ptr;

	/*
	 *	Allocate enough space for the hangup message, if none is available 
	 *	then wait a while and try again
	 */

	m = allocb(0, BPRI_LO);
	if (m == NULL) {
		timeout(pap_timeout, q, HZ/8);
		return;
	}

	/*
	 *	If we are closing let it complete
	 */

	if (papp->pap_closing)
		wakeup(&papp->pap_closing);
	papp->pap_closing = 1;

	/*
	 *	send a timeout to the stream head
	 */

	m->b_datap->db_type = M_HANGUP;
	putnext(q, m);
TRACE(T_pap, ("pap_timeout:\n"));
}

static
pap_read_ignore(q)
register queue_t *q;
{
	register struct pap_state *papp;
	register mblk_t *m;
	register at_atp *atp;

	/*
	 *	honour flow control
	 */

	papp = (struct pap_state *)q->q_ptr;
	if (papp->pap_hdr == NULL || !canput(q->q_next)) {
		timeout(pap_read_ignore, q, HZ);
		return;
	}

	/*
	 *	If we can't build a header then
	 *	wait 'till there is space so we can
	 */

	m = copymsg(papp->pap_hdr);
	if (m == NULL) {
TRACE(T_pap, ("pap_send_ignore X25\n"));
		timeout(pap_read_ignore, q, HZ/8);
		return;
	}


	/*
	 * 	build an ATP request
	 */

	atp = ATP_ATP_HDR(&m->b_cont->b_rptr[
			sizeof(struct atp_set_default)]);
	atp->at_atp_user_bytes[1] =
				AT_PAP_TYPE_SEND_DATA;
	atp->at_atp_eom = 0;
	atp->at_atp_xo = 1;
TRACE(T_pap, ("pap_send_ignore 26 send_count = %d\n",papp->pap_send_count));
	W16(&atp->at_atp_user_bytes[2], papp->pap_send_count);
	papp->pap_send_count++;
	if (papp->pap_send_count == 0)
		papp->pap_send_count = 1;

	/*
	 *	send it
	 */

	papp->pap_read_ignore = 1;
	putnext(q, m);
}
