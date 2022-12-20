#ifndef lint	/* .../appletalk/at/atp/atp_open.c */
#define _AC_NAME atp_open_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:55:22}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of atp_open.c on 87/11/11 20:55:22";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#define ATP_DECLARE
#include "./atp.inc.h"

/*
 *	The init routine creates all the free lists
 */

static atp_inited = 0;
static atp_free_wait = 0;

atp_init()
{
	register int i;

	for (i = 0; i < NATP_TRANS; i++) {
		atp_trans_data[i].tr_list.next = atp_trans_free_list;
		atp_trans_free_list = &atp_trans_data[i];
	}
	for (i = 0; i < NATP_RCB; i++) {
		atp_rcb_data[i].rc_list.next = atp_rcb_free_list;
		atp_rcb_free_list = &atp_rcb_data[i];
	}
	for (i = 0; i < NATP_STATE; i++) {
		atp_state_data[i].atp_trans_waiting = atp_free_list;
		atp_free_list = &atp_state_data[i];
	}
}

/*
 *	The open routine allocates a state structure
 */

static int
atp_open(q, dev, flag, sflag, err, devp)
register queue_t *q;
dev_t dev, *devp;
int *err, flag, sflag;
{
	register struct atp_state *atp;

	if (!atp_inited) {
		atp_inited = 1;
		atp_init();
	}

	/*
	 *	If already open just return
	 */

	if (q->q_ptr)
		return(0);

	/*
	 *	If no atp structure available return failure
	 */

	while ((atp = atp_free_list) == NULL) {
		atp_free_wait = 1;
		if (sleep(&atp_free_list, STOPRI|PCATCH)) {
			*err = EINTR;
			return(OPENFAIL);
		}
	}

	/*
	 *	Update free list
	 */

	atp_free_list = atp->atp_trans_waiting;

	/*
	 *	Initialise the data structure
	 */

	atp->atp_trans_wait.head = NULL;
	atp->atp_trans_snd.head = NULL;
	atp->atp_trans_waiting = NULL;
	atp->atp_trans_wait_flag = 0;
	atp->atp_q = q;
	atp->atp_retry = 10;
	atp->atp_timeout = HZ/8;
	atp->atp_rcb_waiting = NULL;
	atp->atp_rcb_wait_flag = 0;
	atp->atp_rcb.head = NULL;	
	atp->atp_rep_wait.head = NULL;
	atp->atp_unattached.head = NULL;
	atp->atp_finish = NULL;
	atp->atp_mwait = 0;
	atp->atp_lasttid = 0;
	atp->atp_nwait = 0;

	/*
	 *	set up the q pointers
	 */

	WR(q)->q_ptr = q->q_ptr = (char *)atp;

	/*
	 *	Return success
	 */

	TRACE(T_atp, ("atp_open(dev=0x%x, atp=0x%x)\n", dev, atp));
	return(0);
}

/*
 *	The close routine frees all the data structures
 */

static
atp_close(q, flag)
queue_t *q;
{

	register struct atp_state *atp;
	register struct atp_trans *trp;
	register struct atp_rcb *rcbp;
	register int i;
	register int s;
	struct atp_state *xatp;
	struct atp_state *oldatp;

	atp = (struct atp_state *)q->q_ptr;

	TRACE(T_atp, ("atp_close(atp=0x%x)\n", atp));
	if (atp->atp_mwait)
		untimeout(atp_wwake, atp);
	s = splclk();

	/*
	 *	Release pending transactions
	 */

	while (trp = atp->atp_trans_wait.head) {
		untimeout(atp_req_timeout, trp);
		atp_free(trp);
	}


	/*
	 *	Release pending rcbs
	 */

	while (rcbp = atp->atp_rcb.head) {
		atp_rcb_free(rcbp, 0);
	}
	while (rcbp = atp->atp_unattached.head) {
		atp_rcb_free(rcbp, 0);
	}

	/*
	 *	get us off resource wait lists
	 */

	if (atp->atp_trans_wait_flag) {
		oldatp = NULL;
		xatp = atp_state_trans_waiting;
		for(;xatp;) {
			if (atp == xatp)
				break;
			oldatp = xatp;
			xatp = xatp->atp_trans_waiting;
		}
		if (xatp) {
			if (oldatp) {
				oldatp->atp_trans_waiting = atp->atp_trans_waiting;
			} else {
				atp_state_trans_waiting = atp->atp_trans_waiting;
			}
		}
	}
	if (atp->atp_rcb_wait_flag) {
		oldatp = NULL;
		xatp = atp_state_rcb_waiting;
		for(;xatp;) {
			if (atp == xatp)
				break;
			oldatp = xatp;
			xatp = xatp->atp_rcb_waiting;
		}
		if (xatp) {
			if (oldatp) {
				oldatp->atp_rcb_waiting = atp->atp_rcb_waiting;
			} else {
				atp_state_rcb_waiting = atp->atp_rcb_waiting;
			}
		}
	}
	splx(s);
	if (atp->atp_finish)
		freemsg(atp->atp_finish);

	/*
	 *	free the state variable
	 */

	atp->atp_trans_waiting = atp_free_list;
	atp_free_list = atp;
	if (atp_free_wait) {
		atp_free_wait = 0;
		wakeup(&atp_free_list);
	}

	/*
	 *	flush the queues and return
	 */

	flushq(q, 1);
	flushq(WR(q), 1);
}
