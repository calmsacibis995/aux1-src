#ifndef lint	/* .../appletalk/fwd/streams/streamHI.c */
#define _AC_NAME streamHI_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:05:32}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of streamHI.c on 87/11/11 21:05:32";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)stream.c	UniPlus 2.1.5	*/
/*   Copyright (c) 1984 AT&T	and UniSoft Systems */
/*     All Rights Reserved  	*/

/*   THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T and UniSoft Systems */
/*   The copyright notice above does not evidence any   	*/
/*   actual or intended publication of such source code.	*/

#ifdef	GET_PUT_HI
#include "sys/types.h"
#include "sys/param.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/conf.h"
#ifdef HOWFAR
#include "sys/debug.h"
extern	int	T_stream;
#endif HOWFAR

extern unsigned short	bsize[];

#ifndef ASSERT
#define ASSERT(x)
#endif ASSERT


/*
 * Get a message off head of queue.
 *
 * This runs at splhi, and is intended for communications to the interrupt level which
 * is running higher than splstr (as in the case of appletalk). Unlike, the generic
 * routines, the queues are strictly FIFO with no priorities since at the interrupt
 * level we tend to be interrested in an ordered approach.
 *
 * If the queue is empty then set the QWANTR flag to indicate
 * that it is wanted by a reader (the queue's service procedure).
 * If the queue is not empty remove the first message,
 * turn off the QWANTR flag, substract the weighted size
 * of the message from the queue count, and turn off the QFULL
 * flag if the count is less than the high water mark.
 *
 * In all cases, if the QWANTW flag is set (the upstream
 * module wants to write to the queue) AND the count is
 * below the low water mark, enable the upstream queue and
 * turn off the QWANTW flag.
 *
 * A pointer to the first message on the queue (if any) is
 * returned, or NULL (if not).
 */

mblk_t *
getqHI(q)
register queue_t *q;
{
	register mblk_t *bp;
	register mblk_t *tmp;
	register s;

	ASSERT(q);

	s = splhi();
	if (!(bp = q->q_first)) q->q_flag |= QWANTR;
	else {
		if (!(q->q_first = bp->b_next))	q->q_last = NULL;
		else q->q_first->b_prev = NULL;
		bp->b_next = NULL;
		for (tmp = bp; tmp; tmp = tmp->b_cont)
			q->q_count -= bsize[tmp->b_datap->db_class];
		if (q->q_count < q->q_hiwat)
			q->q_flag &= ~QFULL;
		q->q_flag &= ~QWANTR;
	}

	if (q->q_count<=q->q_lowat && q->q_flag&QWANTW) {
		q->q_flag &= ~QWANTW;
		/* find nearest back queue with service proc */
		for (q = backq(q); q && !q->q_qinfo->qi_srvp; q = backq(q));
		if (q) qenable(q);
	}
	splx(s);
#ifdef HOWFAR 
#ifdef STRDEBUG
	if (bp) {
		register mblk_t *m;

		m = bp;
		while (m) {
			ASSERT((m->b_debug&MM_FREE) == 0);
			m->b_debug &= ~MM_Q;
			m = m->b_cont;
		}
	}
#endif
#endif
	return(bp);
}


/*
 * Put a message on a queue.  
 *
 * Messages are enqueued on a priority basis.  
 *
 * Add appropriate weighted data block sizes to queue count.
 * If queue hits high water mark then set QFULL flag.
 *
 * If QNOENAB is not set (putq is allowed to enable the queue),
 * enable the queue only if the message is PRIORITY,
 * or the QWANTR flag is set (indicating that the service procedure 
 * is ready to read the queue.
 */

putqHI(q, bp)
register queue_t *q;
register mblk_t *bp;
{
	register s;
	register mblk_t *tmp;
	register mcls = queclass(bp);

	ASSERT(q && bp);

#ifdef HOWFAR 
#ifdef STRDEBUG
	tmp = bp;
	while (tmp) {
		ASSERT((tmp->b_debug&MM_FREE) == 0);
		ASSERT((tmp->b_debug&MM_Q) == 0);
		tmp->b_debug |= MM_Q;
		tmp = tmp->b_cont;
	}
#endif
#endif
	s = splhi();

	/* 
	 * tack message on to the end.
	 */
	if (q->q_first) {
		q->q_last->b_next = bp;
		bp->b_prev = q->q_last;
	} else {
		q->q_first = bp;
		bp->b_prev = NULL;
	}
	bp->b_next = NULL;
	q->q_last = bp;

#ifdef NOTDEF
	q->q_flag &= ~QWANTW;
#endif NOTDEF

	for (tmp = bp; tmp; tmp = tmp->b_cont)
		q->q_count += bsize[tmp->b_datap->db_class];
	if (q->q_count >= q->q_hiwat) q->q_flag |= QFULL;

	if (  !(q->q_flag & QNOENB) && 
	      ( (mcls > QNORM) || q->q_flag & QWANTR) )
		qenable(q);

	splx(s);
}
#endif	GET_PUT_HI
