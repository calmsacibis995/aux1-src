#ifndef lint	/* .../appletalk/at/atp/atp_misc.c */
#define _AC_NAME atp_misc_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:55:18}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_misc.c on 87/11/11 20:55:18";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "./atp.inc.h"

/*
 *	The request timer retries a request, if all retries are used up
 *		it returns a NAK
 */

static
atp_req_timeout(trp)
register struct atp_trans *trp;
{
	at_atp *athp;
	register mblk_t *m;

	TRACE(T_atp_req, ("atp_req_timeout(0x%x) retry=%d\n", trp, trp->tr_retry));
	if (trp->tr_retry == 0) {
		switch(((struct iocblk *)(trp->tr_xmt->b_rptr))->ioc_cmd) {
		case AT_ATP_ISSUE_REQUEST:
			atp_iocnak(trp->tr_queue, trp->tr_xmt, ETIMEDOUT);
			trp->tr_xmt = NULL;
			atp_free(trp);
			break;

		case AT_ATP_ISSUE_REQUEST_NOTE:
			m = trp->tr_xmt;
			trp->tr_xmt = m->b_cont;
			m->b_cont = NULL;
			m->b_wptr = m->b_rptr+1;
			*m->b_rptr = 1;
			m->b_datap->db_type = M_DATA;
			putnext(trp->tr_queue->atp_q, m);
		 	trp->tr_state = TRANS_FAILED;
			break;

		case AT_ATP_ISSUE_REQUEST_NW:
			m = trp->tr_xmt;
			trp->tr_xmt = m->b_cont;
			m->b_cont = NULL;
			freemsg(m);
		 	trp->tr_state = TRANS_FAILED;
			break;
		}
	} else {
		athp = AT_ATP_HDR(trp->tr_xmt->b_cont);
		athp->at_atp_bitmap_seqno = trp->tr_bitmap;
		if (trp->tr_retry != ATP_INFINITE_RETRIES)
			trp->tr_retry--;
		atp_send(trp);
	}
}

/*
 *	atp_free frees up a request, cleaning up the queues and freeing
 *		the request packet
 */

static
atp_free(trp)
register struct atp_trans *trp;
{
	register struct atp_state *atp;
	register int i;
	
	TRACE(T_atp_req, ("atp_free(0x%x)\n", trp));
	untimeout(atp_req_timeout, trp);
	if (trp->tr_state == TRANS_BUILD)
		untimeout(atp_x_done, trp);
	atp = trp->tr_queue;
	ATP_Q_REMOVE(atp->atp_trans_wait, trp, tr_list);
	if (trp->tr_xmt) {
		freemsg(trp->tr_xmt);
		trp->tr_xmt = NULL;
	}
	for (i = 0; i < 8; i++)
	if (trp->tr_rcv[i]) {
		freemsg(trp->tr_rcv[i]);
		trp->tr_rcv[i] = NULL;
	}
	if (atp->atp_finish &&
	    atp->atp_trans_wait.head == NULL &&
	    atp->atp_rcb.head == NULL) {
		atp_iocack(atp, atp->atp_finish);
		atp->atp_finish = NULL;
	}
	atp_trans_free(trp);
}

/*
 *	atp_send transmits a request packet by queuing it (if it isn't already) and
 *		scheduling the queue
 */

static
atp_send(trp)
register struct atp_trans *trp;
{
	register struct atp_state *atp;

	TRACE(T_atp_req, ("atp_send(0x%x)\n", trp));
	if (trp->tr_state == TRANS_TIMEOUT) {
		atp = trp->tr_queue;
		ATP_Q_APPEND(atp->atp_trans_snd, trp, tr_snd_wait);
		trp->tr_state = TRANS_REQUEST;
		qenable(WR(atp->atp_q));
	}
}

/*
 *	atp_reply sends all the available messages in the bitmap again
 *		by queueing us to the write service routine
 */

static
atp_reply(rcbp)
register struct atp_rcb *rcbp;
{
	register struct atp_state *atp;
	register int i;

	TRACE(T_atp_rep, ("atp_reply(0x%x)\n", rcbp));
	for (i = 0; i < 8; i++) {
		if (rcbp->rc_bitmap&atp_mask[i]) {
			if (rcbp->rc_xmt[i]) {
				rcbp->rc_snd[i] = 1;
				if (rcbp->rc_rep_waiting == 0) {
					rcbp->rc_rep_waiting = 1;
					atp = rcbp->rc_queue;
					ATP_Q_APPEND(atp->atp_rep_wait, rcbp,
							rc_rep_wait);
					qenable(WR(atp->atp_q));
				}
			}
		} else 
		if (rcbp->rc_xmt[i]) {
			freemsg(rcbp->rc_xmt[i]);
			rcbp->rc_xmt[i] = NULL;
		}
	}
}

/*
 *	The rcb timer just frees the rcb, this happens when we missed a release for XO
 */

static
atp_rcb_timer(rcbp)
struct atp_rcb *rcbp;
{
	TRACE(T_atp_rep, ("atp_rcb_timer(0x%x)\n", rcbp));
	atp_rcb_free(rcbp, 0);
}

/*
 *	This routine compares rcb's sockets for received messages
 */

static
atp_same_socket(socket, ddp) 
register struct atp_socket *socket;
register at_ddp_t *ddp;
{
	if (socket->socket != ddp->src_socket ||
	    socket->node != ddp->src_node ||
	    socket->net != R16(ddp->src_net))
		return(0);
	return(1);
}


/*
 *	These routine do IOCTL replies
 */

static
atp_iocack(atp, m)
struct atp_state *atp;
register mblk_t *m;
{
	TRACE(T_atp, ("atp_iocack(0x%x)\n", atp));
	if (m->b_datap->db_type == M_IOCTL)
		m->b_datap->db_type = M_IOCACK;
	if (m->b_cont) {
		((struct iocblk *)m->b_rptr)->ioc_count = msgdsize(m->b_cont);;
	} else {
		((struct iocblk *)m->b_rptr)->ioc_count = 0;
	}
	qreply(WR(atp->atp_q), m);
}

static
atp_iocnak(atp, m, err)
struct atp_state *atp;
register mblk_t *m;
int err;
{
	TRACE(T_atp, ("atp_iocnak(0x%x) err = %d\n", atp, err));
	if (m->b_datap->db_type == M_IOCTL)
		m->b_datap->db_type = M_IOCNAK;
	((struct iocblk *)m->b_rptr)->ioc_count = 0;
	if (err == 0)
		err = ENXIO;
	((struct iocblk *)m->b_rptr)->ioc_error = err;
	if (m->b_cont) {
		freemsg(m->b_cont);
		m->b_cont = NULL;
	}
	qreply(WR(atp->atp_q), m);
}


/*
 *	Generate a transaction id for a socket
 */

static
atp_tid(atp)
register struct atp_state *atp;
{
	register int i;
	register struct atp_trans *trp;

	for (i = atp->atp_lasttid;;) {
		i = (i+1)&0xffff;
		for (trp = atp->atp_trans_wait.head;trp;trp = trp->tr_list.next)
		if (trp->tr_tid == i)
			break;
		if (trp == NULL) {
			atp->atp_lasttid = i;
			TRACE(T_atp_req, ("atp_tid(0x%x) = 0x%x\n", atp, i));
			return(i);
		}
	}
}

static
atp_clear(rcbp)
register struct atp_rcb *rcbp;
{
	if (rcbp->rc_state == RCB_IOCTL_FULL) {
		rcbp->rc_state = RCB_IOCTL_NW;
		freemsg(rcbp->rc_ioctl);
		rcbp->rc_ioctl = NULL;
	}
}
