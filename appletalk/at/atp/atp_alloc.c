#ifndef lint	/* .../appletalk/at/atp/atp_alloc.c */
#define _AC_NAME atp_alloc_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:55:12}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_alloc.c on 87/11/11 20:55:12";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "./atp.inc.h"

/*
 *	tcb (transaction) allocation routine. If no transaction data structure
 *		is available then put the module on a queue of modules waiting
 *		for transaction structures. When a tcb is available it will be
 *		removed from this list and its write queue will be scheduled.
 */

static
struct atp_trans *
atp_trans_alloc(atp)
struct atp_state *atp;
{
	struct atp_trans *trp;
	register int s, i;

	s = splclk();
	if (trp = atp_trans_free_list) {
		atp_trans_free_list = trp->tr_list.next;
		splx(s);

		/*
		 *	got one - initialise it
		 */

		for (i = 0; i < 8; i++)
			trp->tr_rcv[i] = NULL;
		trp->tr_queue = atp;
		trp->tr_state = TRANS_TIMEOUT;
		TRACE(T_atp_alloc, ("atp_trans_alloc(0x%x) succeeded = 0x%x\n", atp, trp));
	} else {
		if (atp->atp_trans_wait_flag == 0) {
			atp->atp_trans_wait_flag = 1;
			atp->atp_trans_waiting = atp_state_trans_waiting;
			atp_state_trans_waiting = atp;
		}
		splx(s);
		TRACE(T_atp_alloc, ("atp_trans_alloc(0x%x) failed\n", atp));
	}
	return(trp);
}

/*
 *	tcb free routine - if modules are waiting schedule them
 */

static
atp_trans_free(trp)
struct atp_trans *trp;
{
	register int s;

	TRACE(T_atp_alloc, ("atp_trans_free(0x%x)\n", trp));
	s = splclk();
	trp->tr_list.next = atp_trans_free_list;
	atp_trans_free_list = trp;
	while (atp_state_trans_waiting) {
		atp_state_trans_waiting->atp_trans_wait_flag = 0;
		qenable(WR(atp_state_trans_waiting->atp_q));
		atp_state_trans_waiting = atp_state_trans_waiting->atp_trans_waiting;
	}
	splx(s);
}

/*
 *	This routine allocates a rcb, if none are available it makes sure the
 *		the write service routine will be called when one is
 */

static
struct atp_rcb *
atp_rcb_alloc(atp)
struct atp_state *atp;
{
	struct atp_rcb *rcbp;
	register int s;
	int i;

	s = splclk();
	if (rcbp = atp_rcb_free_list) {
		atp_rcb_free_list = rcbp->rc_list.next;
		splx(s);
		for (i = 0; i < 8; i++)
			rcbp->rc_xmt[i] = NULL;
		rcbp->rc_queue = atp;
		rcbp->rc_timer = 30*HZ;
		TRACE(T_atp_alloc, ("atp_rcb_alloc(0x%x) succeeded = 0x%x\n", atp, rcbp));
	} else {
		if (atp->atp_rcb_wait_flag == 0) {
			atp->atp_rcb_wait_flag = 1;
			atp->atp_rcb_waiting = atp_state_rcb_waiting;
			atp_state_rcb_waiting = atp;
		}
		splx(s);
		TRACE(T_atp_alloc, ("atp_rcb_alloc(0x%x) failed\n", atp));
	}
	return(rcbp);
}

/*
 *	Here we free rcbs, if required reschedule other people waiting for them
 */

static
atp_rcb_free(rcbp, resp)
struct atp_rcb *rcbp;
{
	register int s, i;
	struct atp_state *atp;

	TRACE(T_atp_alloc, ("atp_rcb_free(0x%x, %d)\n", rcbp, resp));
	atp = rcbp->rc_queue;
	if (rcbp->rc_rep_waiting)  {
		rcbp->rc_rep_waiting = 0;
		ATP_Q_REMOVE(atp->atp_rep_wait, rcbp, rc_rep_wait);
	}
	for (i = 0; i < 8; i++) 
	if (rcbp->rc_xmt[i]) {
		freemsg(rcbp->rc_xmt[i]);
		rcbp->rc_xmt[i] = NULL;
		rcbp->rc_snd[i] = 0;
	}
	if (resp && rcbp->rc_xo)
		return;
	if (rcbp->rc_state == RCB_IOCTL || rcbp->rc_state == RCB_IOCTL_NW ||
	    rcbp->rc_state == RCB_IOCTL_FULL) {
	    	if (rcbp->rc_state == RCB_IOCTL_FULL)
			untimeout(atp_clear, rcbp);
		ATP_Q_REMOVE(atp->atp_unattached, rcbp, rc_list);
	} else {
		untimeout(atp_rcb_timer, rcbp);
		ATP_Q_REMOVE(atp->atp_rcb, rcbp, rc_list);
	}
	TRACE(T_atp, ("atp_rcb_free: \n"));
	if (atp->atp_finish &&
	    atp->atp_trans_wait.head == NULL &&
	    atp->atp_rcb.head == NULL) {
		TRACE(T_atp, ("atp_rcb_free: sending finish\n"));
		atp_iocack(atp, atp->atp_finish);
		atp->atp_finish = NULL;
	}
	if (rcbp->rc_ioctl) {
		freemsg(rcbp->rc_ioctl);
		rcbp->rc_ioctl = NULL;
	}
	rcbp->rc_list.next = atp_rcb_free_list;
	atp_rcb_free_list = rcbp;
	while (atp_state_rcb_waiting) {
		atp_state_rcb_waiting->atp_rcb_wait_flag = 0;
		qenable(WR(atp_state_rcb_waiting->atp_q));
		atp_state_rcb_waiting = atp_state_rcb_waiting->atp_rcb_waiting;
	}
}

