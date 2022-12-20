#ifndef lint	/* .../appletalk/at/atp/atp_write.c */
#define _AC_NAME atp_write_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/11 20:55:30}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of atp_write.c on 87/11/11 20:55:30";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "./atp.inc.h"

/*
 *	write queue put routine .... filter out other than IOCTLs
 */

static
atp_wputq(q, m)
queue_t *q;
mblk_t *m;
{
	struct iocblk *iocbp;
	
	TRACE(T_atp, ("atp_wputq(0x%x)\n", q->q_ptr));
	switch(m->b_datap->db_type) {
	case M_DATA:
	case M_IOCTL:
		putq(q, m);
		break;
	default:
		freemsg(m);
		break;
	}
}


/*
 *	The service routine does two main things:
 *
 *	If upstream is flow control blocked then we just return, we will be 
 *		backenabled when space ia available
 *
 *	If no message space is available then we schedule a timeout for
 *		sometime soon and reenable the queue then (via atp_wwake)
 *
 *	The main loop tries to send pending transaction replies if possible.
 *
 *	Next it sends queued responses if possible.
 *
 *	Next pending user ioctl's are processed
 *
 */

static
atp_wsrvc(q)
queue_t *q;
{
	register struct atp_state *atp;
	register struct iocblk *iocbp;
	register int s;
	int i;
	int old;
	register struct atp_trans *trp;
	register struct atp_rcb *rcbp;
	struct atp_rcb *newrcbp;
	mblk_t *m, *m2;
	register at_atp *athp;
	at_ddp_t *ddp;
	unsigned int timer, retry;

	/*
 	 *	Get the state structure pointer from the queue
	 */

	atp = (struct atp_state *)q->q_ptr;	
	TRACE(T_atp, ("atp_wsrvc(0x%x)\n", atp));

	/*
	 *	For every entry in the list of transacions waiting for space
	 *		to send messages, try and send it
	 */

	while (trp = atp->atp_trans_snd.head) {

		TRACE(T_atp_req, ("sending(0x%x)\n", trp));
		/*
	 	 *	check to see if we can send downstream, if not just
		 *	return, when there is space we will be backenabled
		 */

		if (!canput(q->q_next)) {
			return;
		}

		/*
		 *	See which sort of message to send
		 */

		if (trp->tr_state == TRANS_RELEASE) {

			/*
			 *	Now try and allocate enough space to send the message
			 *		if none is available the schedule a timeout
			 *		so we can retry for more space soon
			 */

			if ((m = allocb(AT_ATP_HDR_SIZE + AT_DDP_X_HDR_SIZE,
					 BPRI_HI)) == NULL) {
				s = splclk();
				if (!atp->atp_mwait) {
					timeout(atp_wwake, atp, 4);
					atp->atp_mwait = 1;
				}
				splx(s);
				return;
			}
	
			/*
			 *	if the space is available format and send the message
			 */

			m->b_wptr += AT_ATP_HDR_SIZE + AT_DDP_X_HDR_SIZE;
			ddp = AT_DDP_HDR(m);
			ddp->type = AT_DDP_TYPE_ATP;
			athp = AT_ATP_HDR(m);
			ddp->dst_socket = trp->tr_socket.socket;
			ddp->dst_node = trp->tr_socket.node;
			W16(ddp->dst_net, trp->tr_socket.net);
			athp->at_atp_cmd = AT_ATP_CMD_TREL;
			athp->at_atp_eom = 0;
			athp->at_atp_sts = 0;
			athp->at_atp_unused = 0;
			W16(athp->at_atp_transaction_id, trp->tr_tid);
			putnext(q, m);
	
			/*
			 *	Now remove the transaction from the queue 
			 */

			s = splclk();
			ATP_Q_REMOVE(atp->atp_trans_snd, trp, tr_snd_wait);
			splx(s);

			/*
			 *	Now send back the transaction reply to the process
			 *		or notify the process if required
			 */

			switch(((struct iocblk *)(trp->tr_xmt->b_rptr))->ioc_cmd) {
			case AT_ATP_ISSUE_REQUEST:
				atp_iocack(atp, trp->tr_xmt);
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
				putnext(RD(q), m);
			 	trp->tr_state = TRANS_DONE;
				break;

			case AT_ATP_ISSUE_REQUEST_NW:
				m = trp->tr_xmt;
				trp->tr_xmt = m->b_cont;
				m->b_cont = NULL;
				freemsg(m);
			 	trp->tr_state = TRANS_DONE;
				break;
			}
		} else {

			/*
			 *	trp->tr_state = TRANS_REQUEST
			 *		transmit a message, first check for resources
			 */

			if ((m = copymsg(trp->tr_xmt->b_cont)) == NULL) {
				s = splclk();
				if (!atp->atp_mwait) {
					timeout(atp_wwake, atp, 4);
					atp->atp_mwait = 1;
				}
				splx(s);
				return;
			}
		
			/*
			 *	Now send the message and set the timer	
			 */

			trp->tr_state = TRANS_TIMEOUT;
			timeout(atp_req_timeout, trp, trp->tr_timeout);
			putnext(q, m);

			/*
			 *	Finally remove the transaction from the send queue
			 */

			s = splclk();
			ATP_Q_REMOVE(atp->atp_trans_snd, trp, tr_snd_wait);
			splx(s);
		}
	}

	/*
	 *	Next do the same thing with pending replies (or reply retransmissions)
	 */

	while (rcbp = atp->atp_rep_wait.head) {
		TRACE(T_atp_rep, ("replying(0x%x)\n", rcbp));

		/*
		 *	Do this for each message that hasn't been sent
		 */

		for (i = 0; i < 8; i++) 
		if (rcbp->rc_snd[i]) {

			/*
			 *	If no downstream space wait for the backenable
			 */

			if (!canput(q->q_next)) {
				return;
			}

			/*
			 *	If no space for the reply schedule a timeout
			 *		to look for more
			 */

			if ((m = copymsg(rcbp->rc_xmt[i])) == NULL) {
				s = splclk();
				if (!atp->atp_mwait) {
					timeout(atp_wwake, atp, 4);
					atp->atp_mwait = 1;
				}
				splx(s);
				return;
			}

			/*
			 *	Mark the message as sent and send it
			 */

			rcbp->rc_snd[i] = 0;
			rcbp->rc_not_sent_bitmap &= ~atp_mask[i];
TRACE(T_atp_rep, ("replying, size=%d\n",msgdsize(m)));
			putnext(q, m);
		}

		/*
		 *	If all replies from this reply block have been send then 
		 *		remove it from the queue and mark it so
		 */

		rcbp->rc_rep_waiting = 0;
		s = splclk();
		ATP_Q_REMOVE(atp->atp_rep_wait, rcbp, rc_rep_wait);
		splx(s);

		/*
		 *	If we are doing execute once set the reply timeout
		 *		otherwise, if we have sent all of a response then
		 *		free the resources
		 */

		if (rcbp->rc_not_sent_bitmap == 0) {
			if (rcbp->rc_xo) {
				timeout(atp_rcb_timer, rcbp, rcbp->rc_timer);
			} else {
				atp_rcb_free(rcbp, 1);
			}
		}
	}

	/*
	 *	Next we loop processing IOCTLs from processes
	 */

	for (;;) {

TRACE(T_atp, ("atp_wsrvc 1\n"));
		/*
		 *	Get a message from the queue
		 *		if none are available then return
		 */

		if ((m = getq(q)) == NULL)
			break;
		TRACE(T_atp, ("wsrvc type=0x%x\n", m->b_datap->db_type));

		/*
		 *	Checkout its type (should be only IOCTLs)
		 */

		switch(m->b_datap->db_type) {
		case M_DATA:

			/*
			 *	The M_DATA interface is identical to that used by
			 *		ioctls, it is intended for up stream module's
			 *		use (so they can tell these from normal ioctls)
			 */

			iocbp = (struct iocblk *)m->b_rptr;
			if ((m->b_wptr-m->b_rptr) != sizeof(struct iocblk) ||
			    (iocbp->ioc_count && m->b_cont == NULL)) {
				freemsg(m);
				break;
			}
			TRACE(T_atp, ("wsrvc cmd->ioctl\n"));
			/* fall thru */
			
		case M_IOCTL:

			/*
			 *	Now check out the ioctl type
			 */

			iocbp = (struct iocblk *)m->b_rptr;
			TRACE(T_atp, ("wsrvc ioctl=0x%x\n", iocbp->ioc_cmd));
			m2 = m->b_cont;
			switch (iocbp->ioc_cmd) {
			case AT_ATP_SET_DEFAULT:
				if (iocbp->ioc_count != sizeof(struct atp_set_default) ||
				    ((struct atp_set_default *)m->b_cont->b_rptr)->
								      def_retries <= 0) {
					atp_iocnak(atp, m, EINVAL);
					break;
				}
				i = ((struct atp_set_default *)m->b_cont->b_rptr)->
								      def_rate;
				i = (i * HZ) / 100;
				if (i <= 0)
					i = 1;
				atp->atp_timeout = i;
				atp->atp_retry = 
				    ((struct atp_set_default *)m->b_cont->b_rptr)->
								      def_retries;
				atp_iocack(atp, m);
				break;

			case AT_ATP_POLL_REQUEST:
				if (iocbp->ioc_count != sizeof(int)) {
					atp_iocnak(atp, m, EINVAL);
					break;
				}
				i = *(int *)m->b_cont->b_rptr;
				freemsg(m->b_cont);
				m->b_cont = NULL;
				s = splclk();
				for (trp = atp->atp_trans_wait.head; trp;
					   trp = trp->tr_list.next)
				if (trp->tr_tid == i)
					break;
				splx(s);
				if (trp == NULL || 
				    (trp->tr_state != TRANS_DONE &&
				     trp->tr_state != TRANS_FAILED)) {
					atp_iocnak(atp, m, EAGAIN);
					break;
				}
				if (trp->tr_state == TRANS_FAILED) {
					freemsg(trp->tr_xmt);
					trp->tr_xmt = NULL;
					atp_iocnak(atp, m, ETIMEDOUT);
				} else {
					m->b_cont = trp->tr_xmt;
					trp->tr_xmt = NULL;
					atp_iocack(atp, m);
				}
				atp_free(trp);
				break;
				
			case AT_ATP_ISSUE_REQUEST_DEF:
			case AT_ATP_ISSUE_REQUEST_DEF_NW:
			case AT_ATP_ISSUE_REQUEST_DEF_NOTE:
				switch(old = iocbp->ioc_cmd) {
				case AT_ATP_ISSUE_REQUEST_DEF:
					iocbp->ioc_cmd = AT_ATP_ISSUE_REQUEST;
					break;
				case AT_ATP_ISSUE_REQUEST_DEF_NW:
					iocbp->ioc_cmd = AT_ATP_ISSUE_REQUEST_NW;
					break;
				case AT_ATP_ISSUE_REQUEST_DEF_NOTE:
					iocbp->ioc_cmd = AT_ATP_ISSUE_REQUEST_NOTE;
					break;
				}
				if (iocbp->ioc_count < sizeof(struct atp_set_default)) {
					atp_iocnak(atp, m, EINVAL);
					break;
				}
				timer = ((struct atp_set_default *)m->b_cont->b_rptr)->
									def_rate;
				timer = (timer * HZ) / 100;
				if (timer <= 0)
					timer = 1;
				retry = ((struct atp_set_default *)m->b_cont->b_rptr)->
									def_retries;
				iocbp->ioc_count -= sizeof(struct atp_set_default);
				m->b_cont->b_rptr += sizeof(struct atp_set_default);
				goto issue_request;

			case AT_ATP_ISSUE_REQUEST:
			case AT_ATP_ISSUE_REQUEST_NW:
			case AT_ATP_ISSUE_REQUEST_NOTE:
				timer = atp->atp_timeout;
				retry = atp->atp_retry;
				old = 0;

			issue_request:

				/*
				 *	These ioctls start a new request, first check
				 *		to see if there is space downstream,
				 *		if not then wait for the backenable
				 */

				if (!canput(q->q_next)) {
TRACE(T_atp, ("atp_wsrvc 2\n"));
					if (old) {
						m->b_cont->b_rptr -=
							sizeof(struct atp_set_default);
						iocbp->ioc_cmd = old;
					}
					putbq(q, m);
					return;
				}


				/*
				 *	check out the request packet's size, if not
				 *		complain
				 */

				if (msgdsize(m2) > (AT_ATP_DATA_SIZE + AT_ATP_HDR_SIZE +
						    AT_DDP_X_HDR_SIZE)){
TRACE(T_atp, ("atp_wsrvc 3\n"));
					atp_iocnak(atp, m, EINVAL);
					break;
				}

				/*
				 *	allocate a transaction control block. If none
				 *		are available just wait we will be
				 *		enabled when one is free
				 */

				if ((trp = atp_trans_alloc(atp)) == NULL) {
TRACE(T_atp, ("atp_wsrvc 4\n"));
					if (old) {
						m->b_cont->b_rptr -=
							sizeof(struct atp_set_default);
						iocbp->ioc_cmd = old;
					}
					putbq(q, m);
					return;
				}
				trp->tr_retry = retry;
				trp->tr_timeout = timer;

				/*
				 *	If this is a non blocking request ack it
				 *		(we know by now it will 'succeed')
				 *		if no space wait for some
				 */

				if (iocbp->ioc_cmd == AT_ATP_ISSUE_REQUEST_NW ||
				    iocbp->ioc_cmd == AT_ATP_ISSUE_REQUEST_NOTE) {
TRACE(T_atp, ("atp_wsrvc 5\n"));
					m2 = dupb(m);
					if (m2 == NULL) {
						if (old) {
							m->b_cont->b_rptr -=
							  sizeof(struct atp_set_default);
							iocbp->ioc_cmd = old;
						}
						putbq(q, m);
						atp_trans_free(trp);
						s = splclk();
						if (!atp->atp_mwait) {
							timeout(atp_wwake, atp, 4);
							atp->atp_mwait = 1;
						}
						splx(s);
						return;
					}
					m2->b_cont = NULL;
					trp->tr_tid = atp_tid(atp);
					((struct iocblk *)(m2->b_rptr))->ioc_rval =
								trp->tr_tid;
					atp_iocack(atp, m2);
					m2 = m->b_cont;
				} else {
					trp->tr_tid = atp_tid(atp);
				}

				/*
				 *	remember the IOCTL packet so we can ack it
				 *		later
				 */

				trp->tr_xmt = m;

				/*
				 *	Now fill in the header (and remember the bits
				 *		we need to know)
				 */

				athp = AT_ATP_HDR(m2);
				athp->at_atp_cmd = AT_ATP_CMD_TREQ;

				/*
				 *	If the user didn't specify a mask give them the
				 *		minimum
				 */

				if (athp->at_atp_bitmap_seqno == 0) 
					athp->at_atp_bitmap_seqno = 0x01;
				W16(athp->at_atp_transaction_id, trp->tr_tid);
				athp->at_atp_eom = 0;
				athp->at_atp_sts = 0;
				athp->at_atp_unused = 0;
				trp->tr_xo = athp->at_atp_xo;
				trp->tr_bitmap = athp->at_atp_bitmap_seqno;
				ddp = AT_DDP_HDR(m2);
				ddp->type = AT_DDP_TYPE_ATP;
				trp->tr_socket.socket = ddp->dst_socket;
				trp->tr_socket.node = ddp->dst_node;
				trp->tr_socket.net = R16(ddp->dst_net);

				/*
				 *	Put us in the transaction waiting queue
				 */

				s = splclk();
				ATP_Q_APPEND(atp->atp_trans_wait, trp, tr_list);
				splx(s);

				/*
				 *	Send the transaction
				 */

TRACE(T_atp, ("atp_wsrvc 6\n"));
				atp_send(trp);
				break;

			case AT_ATP_CANCEL_REQUEST:

				/*
				 *	Cancel a pending request
				 */

				if (iocbp->ioc_count != sizeof(int)) {
					atp_iocnak(atp, m, EINVAL);
					break;
				}
				i = *(int *)m->b_cont->b_rptr;
				freemsg(m->b_cont);
				m->b_cont = NULL;
				s = splclk();
				for (trp = atp->atp_trans_wait.head; trp;
					   trp = trp->tr_list.next)
				if (trp->tr_tid == i)
					break;
				splx(s);
				if (trp == NULL) {
					atp_iocnak(atp, m, ENOENT);
					break;
				}
				atp_free(trp);
				atp_iocack(atp, m);
				break;

			case AT_ATP_RELEASE_RESPONSE:
TRACE(T_atp, ("atp_wsrvc 6a\n"));

				/*
				 * 	cancel a response to a transaction
				 *		first check it out
				 */

				if (iocbp->ioc_count < (AT_ATP_HDR_SIZE +
						AT_DDP_X_HDR_SIZE)){
					atp_iocnak(atp, m, EINVAL);
					break;
				}

TRACE(T_atp, ("atp_wsrvc 6b\n"));
				/*
				 *	remove the response from the message
				 */

				m2 = m->b_cont;
				iocbp->ioc_count = 0;
				m->b_cont = NULL;
				athp = AT_ATP_HDR(m2);
				ddp = AT_DDP_HDR(m2);

				/*
				 *	search for the corresponding rcb
				 */

				s = splclk();
				for (rcbp = atp->atp_rcb.head;rcbp;rcbp=rcbp->rc_list.next)
				if (rcbp->rc_tid ==
						R16(athp->at_atp_transaction_id) &&
				    rcbp->rc_socket.node == ddp->src_node &&
				    rcbp->rc_socket.net == R16(ddp->src_net) &&
				    rcbp->rc_socket.socket == ddp->src_socket)
					break;

TRACE(T_atp, ("atp_wsrvc 6c\n"));
				/*
				 *	If it is not available, or it has already
				 *		been sent then return an error
				 */

				i = athp->at_atp_bitmap_seqno;
				if (rcbp == NULL) {
					splx(s);
TRACE(T_atp, ("atp_wsrvc 6d\n"));
					atp_iocnak(atp, m, ENOENT);
					freemsg(m2);
					break;
				}


				/*
				 *	Free the response and return
				 */

				atp_rcb_free(rcbp, 0);
				splx(s);
TRACE(T_atp, ("atp_wsrvc 6e\n"));
				atp_iocack(atp, m);
				freemsg(m2);
				break;

			case AT_ATP_GET_POLL:

				/*
				 *	clean up the request
				 */

				if (m->b_cont) {
					freemsg(m->b_cont);
					m->b_cont = NULL;
					iocbp->ioc_count = 0;
				}

				/*
				 *	search for a waiting request
				 */

				s = splclk();
				for (rcbp = atp->atp_unattached.head;rcbp;
				     rcbp=rcbp->rc_list.next)
				if (rcbp->rc_state == RCB_IOCTL_FULL) 
					break;
				if (rcbp) {

					/*
					 *	Got one, move it to the active 
					 *		response Q
					 */

					untimeout(atp_clear, rcbp);
					rcbp->rc_state = RCB_RESPONDING;
					TRACE(T_atp_req, ("acking FULL req\n"));
					atp->atp_nwait--;
					ATP_Q_REMOVE(atp->atp_unattached, rcbp, rc_list);
					ATP_Q_APPEND(atp->atp_rcb, rcbp, rc_list);
					splx(s);

					/*
					 *	Now respond
					 */

					m->b_cont = rcbp->rc_ioctl;
					rcbp->rc_ioctl = NULL;
					atp_iocack(atp, m);
					break;
				}
				splx(s);

				/*
				 *	None available - can out
				 */

				atp_iocnak(atp, m, EAGAIN);
				break;

			case AT_ATP_GET_REQUEST:
			case AT_ATP_GET_REQUEST_NW:
			case AT_ATP_GET_REQUEST_NOTE:

TRACE(T_atp, ("atp_wsrvc 7\n"));
				/*
				 *	allocate a request control block, if
				 *		none are available then
				 *		wait for some ..... we will
				 *		be scheduled when some are
				 *		available
				 */

				if ((newrcbp = atp_rcb_alloc(atp)) == NULL) {
TRACE(T_atp, ("atp_wsrvc 8\n"));
					putbq(q, m);
					return;
				}

				/*
				 *	clean up the request
				 */

				if (m->b_cont) {
					freemsg(m->b_cont);
					m->b_cont = NULL;
					iocbp->ioc_count = 0;
				}

				/*
				 *	If this is a non-blocking request then
				 *		ack it 
				 */

				if (iocbp->ioc_cmd == AT_ATP_GET_REQUEST_NW ||
				    iocbp->ioc_cmd == AT_ATP_GET_REQUEST_NOTE) {
TRACE(T_atp, ("atp_wsrvc 9\n"));
					if (atp->atp_nwait >= ATP_MAXWAIT) {
						atp_iocnak(atp, m, EAGAIN);
						break;
					}
					if (iocbp->ioc_cmd == AT_ATP_GET_REQUEST_NOTE) {
TRACE(T_atp, ("atp_wsrvc 9a\n"));
						newrcbp->rc_note = 1;
					} else {
						newrcbp->rc_note = 0;
					}
					atp_iocack(atp, m);
					newrcbp->rc_state = RCB_IOCTL_NW;
					newrcbp->rc_ioctl = NULL;
					atp->atp_nwait++;
					s = splclk();
					ATP_Q_APPEND(atp->atp_unattached, newrcbp,
							rc_list);
					splx(s);
					break;
				}

				/*
				 *	Pick up any actually pending requests ....
				 */

				s = splclk();
				for (rcbp = atp->atp_unattached.head;rcbp;
				     rcbp=rcbp->rc_list.next)
				if (rcbp->rc_state == RCB_IOCTL_FULL) 
					break;
				if (rcbp) {
TRACE(T_atp, ("atp_wsrvc 10a\n"));
					untimeout(atp_clear, rcbp);
					newrcbp->rc_state = RCB_IOCTL_NW;
					newrcbp->rc_ioctl = NULL;
					ATP_Q_APPEND(atp->atp_unattached, newrcbp,
							rc_list);
					rcbp->rc_state = RCB_RESPONDING;
					TRACE(T_atp_req, ("acking FULL req\n"));
					atp->atp_nwait--;
					ATP_Q_REMOVE(atp->atp_unattached, rcbp, rc_list);
					ATP_Q_APPEND(atp->atp_rcb, rcbp, rc_list);
					splx(s);
					m->b_cont = rcbp->rc_ioctl;
					rcbp->rc_ioctl = NULL;
					atp_iocack(atp, m);
					break;
				}
				splx(s);

				/*
				 *	remember the message so we can ack it
				 */

				rcbp = newrcbp;
				rcbp->rc_ioctl = m;

				/*
				 *	set up our rcb and queue it for any incoming
				 *		requests
				 */

TRACE(T_atp, ("atp_wsrvc 10\n"));
				s = splclk();
				rcbp->rc_state = RCB_IOCTL;
				ATP_Q_APPEND(atp->atp_unattached, rcbp, rc_list);
				splx(s);
				break;

			case AT_ATP_SEND_RESPONSE:
			case AT_ATP_SEND_RESPONSE_EOF:

				/*
				 * 	send a response to a transaction
				 *		first check it out
				 */

TRACE(T_atp, ("atp_wsrvc 11 count = %d\n",iocbp->ioc_count));
				if (iocbp->ioc_count < (AT_ATP_HDR_SIZE +
						AT_DDP_X_HDR_SIZE)){
TRACE(T_atp, ("atp_wsrvc 12\n"));
					atp_iocnak(atp, m, EINVAL);
					break;
				}

				/*
				 *	remove the response from the message
				 */

				m2 = m->b_cont;
				iocbp->ioc_count = 0;
				m->b_cont = NULL;
				ddp = AT_DDP_HDR(m2);
				athp = AT_ATP_HDR(m2);

				/*
				 *	search for the corresponding rcb
				 */

				s = splclk();
				for (rcbp = atp->atp_rcb.head;
				     rcbp;
				     rcbp=rcbp->rc_list.next)
				if (rcbp->rc_tid ==
						R16(athp->at_atp_transaction_id) &&
				    rcbp->rc_socket.node == ddp->dst_node &&
				    rcbp->rc_socket.net == R16(ddp->dst_net) &&
				    rcbp->rc_socket.socket == ddp->dst_socket)
					break;

				/*
				 *	If it is not available, or it has already
				 *		been sent then return an error
				 */

				i = athp->at_atp_bitmap_seqno;
				if (rcbp == NULL ||
				    i >= 8 ||
				    rcbp->rc_state != RCB_RESPONDING ||
				    !(rcbp->rc_bitmap&atp_mask[i])) {
					splx(s);
TRACE(T_atp, ("atp_wsrvc 13\n"));
					atp_iocnak(atp, m, ENOENT);
					freemsg(m2);
					break;
				}

				/*
				 *	set up the message header
				 */

				if (iocbp->ioc_cmd == AT_ATP_SEND_RESPONSE_EOF ||
				    i == 7) {
					rcbp->rc_state = RCB_RESPONSE_FULL;
					athp->at_atp_eom = 1;
					rcbp->rc_not_sent_bitmap &= atp_lomask[i+1];
				} else {
					athp->at_atp_eom = 0;
				}
				athp->at_atp_cmd = AT_ATP_CMD_TRESP;
				athp->at_atp_sts = 0;
				athp->at_atp_unused = 0;
				athp->at_atp_xo = 0;
				ddp = AT_DDP_HDR(m2);
				ddp->type = AT_DDP_TYPE_ATP;

				/*
				 *	send the ack back to the responder
				 */

				atp_iocack(atp, m);

				/*
				 *	set the message up to be sent
				 */

				rcbp->rc_xmt[i] = m2;
				rcbp->rc_snd[i] = 1;
	
				/*
				 *	Schedule the reply if required and return
				 */

				if (rcbp->rc_rep_waiting == 0) {
TRACE(T_atp, ("atp_wsrvc 14\n"));
					rcbp->rc_rep_waiting = 1;
					ATP_Q_APPEND(atp->atp_rep_wait,rcbp,rc_rep_wait);
					qenable(WR(atp->atp_q));
				}
TRACE(T_atp, ("atp_wsrvc 14a\n"));

				splx(s);
				break;

			case AT_ATP_SET_FINISH:
				if (atp->atp_finish) {
					freemsg(atp->atp_finish);
					atp->atp_finish = NULL;
				}
				s = splclk();
TRACE(T_atp, ("atp_wsrvc 14b 0x%x 0x%x\n", atp->atp_trans_wait.head, atp->atp_rcb.head));
				if (atp->atp_trans_wait.head || atp->atp_rcb.head) {
					atp->atp_finish = m;
				} else {
					atp_iocack(atp, m);
				}
				splx(s);
				break;

			default:

TRACE(T_atp, ("atp_wsrvc 15\n"));
				/*
				 *	Otherwise pass it on, if possible
				 */

				if (!canput(q->q_next)) {
					putbq(q, m);
					return;
				}
				putnext(q, m);
				break;
			}
			break;

		default:

TRACE(T_atp, ("atp_wsrvc 16\n"));
			/*
			 *	If it is unknown ... ignore it
			 */

			freemsg(m);
			break;
		}
	}
}


/*
 *	This routine is run as a timeout to wake up an atp module to poll for
 *		the ending of a resource condition
 */

static
atp_wwake(atp)
struct atp_state *atp;
{
	qenable(WR(atp->atp_q));
	atp->atp_mwait = 0;
}

