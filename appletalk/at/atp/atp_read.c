#ifndef lint	/* .../appletalk/at/atp/atp_read.c */
#define _AC_NAME atp_read_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:55:25}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_read.c on 87/11/11 20:55:25";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "./atp.inc.h"

/*
 *	Decide what to do about received messages
 */

static
atp_rputq(q, m)
queue_t *q;
mblk_t *m;
{
	TRACE(T_atp, ("atp_rputq(0x%x) type = 0x%x\n", q->q_ptr, m->b_datap->db_type));
	switch(m->b_datap->db_type) {
	case M_DATA:
	case M_EXDATA:
		putq(q, m);
		break;

	case M_HANGUP:
	case M_IOCACK:
	case M_IOCNAK:
	case M_BREAK:
	case M_SIG:
	case M_CTL:
	case M_SETOPTS:
	case M_ADMIN:
	case M_EXSIG:
	case M_PCSIG:
		putnext(q, m);
		break;
	default:
		freemsg(m);
	}
}

/* 
 *	The receive service routine processes received data messages
 *		each is assumed to be a ddp packet with an atp header
 */

static
atp_rsrvc(q)
queue_t *q;
{
	register struct atp_state *atp;
	register struct atp_rcb *rcbp;
	register struct atp_trans *trp;
	struct atp_rcb *newrcbp;
	register mblk_t *m;
	mblk_t *m2;
	at_atp *athp;
	register int s;
	int i;
	at_ddp_t *ddp;

	/*
	 *	get the state structure from the queue
	 */

TRACE(T_atp, ("atp_rsrvc 1\n"));
	atp = (struct atp_state *)q->q_ptr;	

	/*
	 *	loop processing incoming messages
	 */

	while (m = getq(q)) {
		TRACE(T_atp, ("atp_rsrvc(0x%x) type = 0x%x\n", atp, m->b_datap->db_type));
		switch (m->b_datap->db_type) {
		case M_DATA:

			/*
			 *	Decode the message, make sure it is an atp
			 *		message
			 */

TRACE(T_atp, ("atp_rsrvc 2\n"));
			ddp = AT_DDP_HDR(m);
			if (ddp->type != AT_DDP_TYPE_ATP) {
TRACE(T_atp, ("atp_rsrvc 3\n"));
				freemsg(m);
				break;
			}
			athp = AT_ATP_HDR(m);
			TRACE(T_atp_req, ("atp_rsrvc(0x%x) command = %d\n", atp, athp->at_atp_cmd));
			switch (athp->at_atp_cmd) {
			case AT_ATP_CMD_TREQ:

				/*
				 *	If it is a request message, first check to see
				 *		if matches something in our active
				 *		request queue
				 */

				TRACE(T_atp_req, ("atp_rsrvc(0x%x) request tid 0x%x\n", atp, R16(athp->at_atp_transaction_id)));
				s = splclk();
				for (rcbp = atp->atp_rcb.head; rcbp;
				     rcbp = rcbp->rc_list.next)
				if (atp_same_socket(&rcbp->rc_socket, AT_DDP_HDR(m)) &&
				    rcbp->rc_tid == R16(athp->at_atp_transaction_id))
					break;

TRACE(T_atp_req, ("atp_rsrvc 20\n"));
				/*
				 *	If this is a new req then do something with it
				 */

				if (rcbp == NULL) {
					TRACE(T_atp_req, ("no active RCB\n"));
					for (rcbp = atp->atp_unattached.head; rcbp;
					     rcbp = rcbp->rc_list.next)
					if (rcbp->rc_state == RCB_IOCTL)
						break;

					/*
					 *	if no-one is waiting then see if anyone
					 *	has declared their interest
					 */

					if (rcbp == NULL) {
						newrcbp = NULL;
						for (rcbp = atp->atp_unattached.head;
							rcbp; rcbp = rcbp->rc_list.next)
						if (rcbp->rc_state == RCB_IOCTL_NW) {
							if (newrcbp == NULL)
								newrcbp = rcbp;
						} else
						if (atp_same_socket(&rcbp->rc_socket,
								AT_DDP_HDR(m)) &&
				    		  rcbp->rc_tid ==
						     R16(athp->at_atp_transaction_id))
							break;

TRACE(T_atp_req, ("atp_rsrvc 21\n"));
						/*
						 *	If no one is interested or we
						 *	already have this one then drop
						 *	the packet
						 */

						if (rcbp || newrcbp == NULL) {

							/*
							 * if we already have it, ok
							 *	it's validity timer
							 */

TRACE(T_atp_req, ("atp_rsrvc 22\n"));
							if  (rcbp) {
								untimeout(atp_clear,
									rcbp);
								timeout(atp_clear, rcbp,
									HZ);
TRACE(T_atp_req, ("atp_rsrvc 23\n"));
								if (rcbp->rc_note) {
TRACE(T_atp, ("atp_rsrvc 23a\n"));
								    m2 = allocb(1,
									        BPRI_LO);
								    if (m2) {
								        rcbp->rc_note=0;
									*m2->b_wptr++=0;
									putnext(q, m2);
								    }
								}
							}
							TRACE(T_atp_req,
								("no waiting RCB\n"));
							freemsg(m);
							break;
						}

						/*
						 *	Send the note if required
						 */

						if (rcbp->rc_note) {
TRACE(T_atp, ("atp_rsrvc 23b\n"));
							m2 = allocb(1, BPRI_LO);
							if (m2) {
								rcbp->rc_note = 0;
								*m2->b_wptr++ = 0;
								putnext(q, m2);
							}
						}

						/*
					 	 *	Otherwise mark the slot as full
						 */

TRACE(T_atp_req, ("atp_rsrvc 24\n"));
						rcbp = newrcbp;
						rcbp->rc_state = RCB_IOCTL_FULL;
						timeout(atp_clear, rcbp, HZ);
					}

					splx(s);

					/*
				  	 *	If it is a synchronous ioctl then
					 *		complete it
					 *	Otherwise, if async then send it as
					 *		data
					 */

					rcbp->rc_socket.socket = ddp->src_socket;
					rcbp->rc_socket.node = ddp->src_node;
					rcbp->rc_socket.net = R16(ddp->src_net);
				        rcbp->rc_tid =
						R16(athp->at_atp_transaction_id);
					rcbp->rc_bitmap = athp->at_atp_bitmap_seqno;
					rcbp->rc_not_sent_bitmap =
						athp->at_atp_bitmap_seqno;
				        rcbp->rc_xo = athp->at_atp_xo;
TRACE(T_atp_req, ("atp_rsrvc 25\n"));
					if (rcbp->rc_state == RCB_IOCTL) {
						m2 = rcbp->rc_ioctl;
						rcbp->rc_ioctl = NULL;
						rcbp->rc_state = RCB_RESPONDING;
						m2->b_cont = m;
						TRACE(T_atp_req, ("acking GET req\n"));
						atp_iocack(atp, m2);
						s = splclk();
						ATP_Q_REMOVE(atp->atp_unattached, rcbp,
								rc_list);
						ATP_Q_APPEND(atp->atp_rcb, rcbp,
								rc_list);
						splx(s);
					} else {
						rcbp->rc_ioctl = m;
TRACE(T_atp_req, ("atp_rsrvc 26\n"));
					}
TRACE(T_atp_req, ("atp_rsrvc 27\n"));
					break;
				} else 

				/*
				 *	Otherwise we have found a matching request
				 *		look for what to do
				 */

				switch (rcbp->rc_state) {
				case RCB_RESPONDING:
				case RCB_RESPONSE_FULL:

					/*
					 *	If it is one we have in progress 
					 *		(either have all the responses
					 *		or are waiting for them)
					 *		update the bitmap and resend
					 *		the replies
					 */

TRACE(T_atp, ("atp_rsrvc 5\n"));
					rcbp->rc_bitmap = athp->at_atp_bitmap_seqno;
					freemsg(m);
					untimeout(atp_rcb_timer, rcbp);
					splx(s);
					atp_reply(rcbp);
					break;

				default:
				case RCB_RELEASED:

					/*
					 *	If we have a release ignore the
					 *	request
					 */

TRACE(T_atp, ("atp_rsrvc 6\n"));
					splx(s);
					freemsg(m);
					break;
				}
				
				break;

			case AT_ATP_CMD_TRESP:

				/*
				 *	If we just got a response, find the trans
				 *		record
				 */

TRACE(T_atp, ("atp_rsrvc 7 seq = %d\n", athp->at_atp_bitmap_seqno));
				s = splclk();
				for (trp = atp->atp_trans_wait.head; trp;
					   trp = trp->tr_list.next)
				if (atp_same_socket(&trp->tr_socket, AT_DDP_HDR(m)) &&
				    trp->tr_tid == R16(athp->at_atp_transaction_id))
					break;

				/*
				 *	If we can't find one then ignore the message
				 */

				if (trp == NULL) {
TRACE(T_atp, ("atp_rsrvc 8\n"));
					splx(s);
					freemsg(m);
					break;
				}

				/*
				 *	If we have already received it ignore it
				 */

				if (!(trp->tr_bitmap&atp_mask[athp->at_atp_bitmap_seqno])
					|| trp->tr_rcv[athp->at_atp_bitmap_seqno]) {
TRACE(T_atp, ("atp_rsrvc 9\n"));
					splx(s);
					freemsg(m);
					break;
				}

				/*
				 *	Update the received packet bitmap
				 */

				if (athp->at_atp_eom) {
					trp->tr_bitmap &=
						atp_lomask[athp->at_atp_bitmap_seqno];
TRACE(T_atp, ("atp_rsrvc 9a = 0x%x\n",trp->tr_bitmap));
				} else {
					trp->tr_bitmap &=
						~atp_mask[athp->at_atp_bitmap_seqno];
TRACE(T_atp, ("atp_rsrvc 9b = 0x%x\n",trp->tr_bitmap));
				}

				/*
				 *	Save the message in the trans record
				 */

				trp->tr_rcv[athp->at_atp_bitmap_seqno] = m;

				/*
			 	 *	If it isn't the first message then
				 *		can the header
				 */

				if (athp->at_atp_bitmap_seqno)
					m->b_rptr += AT_DDP_X_HDR_SIZE;

				/*
				 *	If we now have all the responses then return
				 *		the message to the user
				 */

				if (trp->tr_bitmap == 0) {

TRACE(T_atp, ("atp_rsrvc 10\n"));
					/*
					 *	Cancel the request timer and any
					 *		pending transmits
					 */

					untimeout(atp_req_timeout, trp);
					if (trp->tr_state == TRANS_REQUEST) {
						ATP_Q_REMOVE(atp->atp_trans_snd,
							     trp, tr_snd_wait);
						trp->tr_state = TRANS_TIMEOUT;
					}
					splx(s);

					/*
					 *	Send the results back to the user
					 */

					atp_x_done(trp);
				} else
				if (athp->at_atp_sts) {
TRACE(T_atp, ("atp_rsrvc 12\n"));

					/*
					 *	If they want acks send them
					 */

					untimeout(atp_req_timeout, trp);
					splx(s);
					atp_send(trp);
				} else {
TRACE(T_atp, ("atp_rsrvc 13\n"));
					splx(s);
				}
				break;

			case AT_ATP_CMD_TREL:

				/*
			 	 *	Search for a matching transaction
				 */

TRACE(T_atp, ("atp_rsrvc 14\n"));
				s = splclk();
				for (rcbp = atp->atp_rcb.head;rcbp;rcbp=rcbp->rc_list.next)
				if (atp_same_socket(&rcbp->rc_socket, AT_DDP_HDR(m)) &&
				    rcbp->rc_tid == R16(athp->at_atp_transaction_id))
					break;

				/*
				 *	If none found then ignore the packet
				 */

				if (rcbp == NULL) {
					splx(s);
					freemsg(m);
TRACE(T_atp, ("atp_rsrvc 15\n"));
					break;
				}

				/*
				 *	Otherwise mark the rcb released (if XO
				 *		free the buffers)
				 */

				freemsg(m);
				rcbp->rc_state = RCB_RELEASED;
				atp_rcb_free(rcbp, 0);
				splx(s);
				break;
			default:
				freemsg(m);
			}
			break;
		default:
			freemsg(m);
		}
	}
}

static
atp_x_done(trp)
struct atp_trans *trp;
{
	int i, offset;
	mblk_t *m, *m2;
	struct atp_result *rp;

	untimeout(atp_x_done, trp);

	/*
	 *	Alocate space for the buffer map
	 */

TRACE(T_atp, ("atp_x_done 1\n"));
	m2 = allocb(sizeof(struct atp_result), BPRI_LO);
	if (m2 == NULL) {
TRACE(T_atp, ("atp_x_done 2\n"));
	 	trp->tr_state = TRANS_BUILD;
		timeout(atp_x_done, trp, HZ/8);
		return;
	}
	m2->b_wptr += sizeof(struct atp_result);
	rp = (struct atp_result *)m2->b_rptr;
	rp->count = 0;

	/*
	 *	Tack all the messages on the
	 *		end of the requesting ioctl
	 */

	if (trp->tr_xmt->b_cont) {
		freemsg(trp->tr_xmt->b_cont);
		trp->tr_xmt->b_cont == NULL;
	}
	m = trp->tr_xmt;
	m->b_cont = m2;
	m = m2;
	rp->hdr = offset = sizeof(struct atp_result);
	offset += AT_DDP_X_HDR_SIZE;
	for (i = 0; i < 8; i++) {
		if (trp->tr_rcv[i] == NULL)
			break;
		rp->offset[i] = offset;
		rp->len[i] = msgdsize(trp->tr_rcv[i]);
		if (i == 0)
			rp->len[0] -= AT_DDP_X_HDR_SIZE;
		rp->count++;
		offset += rp->len[i];
		m->b_cont = trp->tr_rcv[i];
		trp->tr_rcv[i] = NULL;
		while (m->b_cont)
			m = m->b_cont;
	}

	/*
	 *	If execute once send a release
	 */

	if (trp->tr_xo) {
TRACE(T_atp, ("atp_x_done 3\n"));
		if (trp->tr_state == TRANS_TIMEOUT) {
			ATP_Q_APPEND(trp->tr_queue->atp_trans_snd, trp, tr_snd_wait);
		}
		trp->tr_state = TRANS_RELEASE;
		qenable(WR(trp->tr_queue->atp_q));
		return;
	}

	/*
	 *	send the reply back to the requester
	 *		and free the transaction
	 */

	i = ((struct iocblk *)(trp->tr_xmt->b_rptr))-> ioc_cmd;
	switch (i) {
	case AT_ATP_ISSUE_REQUEST:
TRACE(T_atp, ("atp_x_done 4\n"));
		atp_iocack(trp->tr_queue, trp->tr_xmt);
		trp->tr_xmt = NULL;
		atp_free(trp);
		break;

	case AT_ATP_ISSUE_REQUEST_NOTE:
TRACE(T_atp, ("atp_x_done 5\n"));
		m = trp->tr_xmt;
		trp->tr_xmt = m->b_cont;
		m->b_cont = NULL;
		m->b_wptr = m->b_rptr+1;
		*m->b_rptr = 1;
		m->b_datap->db_type = M_DATA;
		putnext(trp->tr_queue->atp_q, m);
	 	trp->tr_state = TRANS_DONE;
		break;

	case AT_ATP_ISSUE_REQUEST_NW:
TRACE(T_atp, ("atp_x_done 6\n"));
		m = trp->tr_xmt;
		trp->tr_xmt = m->b_cont;
		m->b_cont = NULL;
		freemsg(m);
		trp->tr_state = TRANS_DONE;
		break;
	}
}
