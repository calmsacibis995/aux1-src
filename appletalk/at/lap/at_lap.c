#ifndef lint	/* .../appletalk/at/lap/at_lap.c */
#define _AC_NAME at_lap_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:56:01}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_lap.c on 87/11/11 20:56:01";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)lap.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 *	The lap module is multiplexing which means, there is one connection
 *	downstream (the network) and multiple connections upstream. The
 *	conections upstream are called demultiplexing queues and are viewed from
 *	the user interface as a "cloneable" file in the lap directory for the network
 *	with the name "circuits". There is no relationship between a connection and
 *	the type that is registered on it.
 *
 *	If the network connection is active, then the control file must be
 *	open, since that is the multiplexing queue.
 *
 *	There are two parts to the lap module code. That which is common to
 *	all lap modules on the unix machine, no matter which network it is 
 *	connected to. And, that which is specific to a particular network
 *      This file contains the common routines that all lap modules execute.
 *	As in other multiply occuring streams, the only
 *	difference is durring the open. The open allocates space and must
 *	be seperately compiled, and thus is in a seperate file, at_lapopen.c.
 *
 *	Note, lap does not impose any flow control on an individual stream.
 *	Higher level protocals, ATP and PAP are assumed to handle flow control.
 *	Of course, as an aggregate, the network will only take packets as fast
 *	as it can.
 *
 *	This file is organized into the following sections:
 *		defines and storage
 *		timer interrupts
 *		scc interrupts
 *		lap stream module
 */



#include <sys/types.h>
#include <errno.h>
#include <sys/reg.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/debug.h>
#include <via.h>
#include <appletalk.h>
#include <scc.h>
#include <at_ddp.h>
#define	 DEFINE		/* as in c's define vs declare */
#include <at_lap.h>
#ifdef	FEP
#include <fwdicp.h>
#endif	FEP

#ifndef NULL
#define NULL 0
#endif

#define	NO_RCV		0
#define	RCV_TOO_LARGE	1
#define	RCV_OVERRUN	2
#define	RCV_CRC		3
#define	RCV_OK		4
#define	RCV_TIMEOUT	5

#define NLAP_SPECIFICS	4	/* one for each scc channel */

static	void		access_net();
static	void		adjust_backoff();
lap_specifics_t		lap_specifics[NLAP_SPECIFICS];
extern ddp_specifics_t 	ddp_specifics;
extern ddp_socket_t	ddp_socket[];
rwp_t			at_rwp[NLAP_SPECIFICS/2];
extern	void 		schedule();
extern	int 		T_lap;
extern	int 		T_lap_backoff;
extern	int 		T_lap_errors;
extern	int 		T_lap_interrupt;
extern	int 		T_lap_states;


	/*
	 * We have an inline macro for timeout since C and calls take so long,
	 * 56us or about two character times! This takes ????us
	 * Note that this does not set the timer mode because timer 2 is reset
	 * to count down with 2theta clock as source
	 */
#define	at_timeout(TIME, TIMER)						\
	{								\
	register via_chip_t	*viap = vias[AT_TIMER];			\
	/* reset any pending interrupt */				\
	viap->t2loc;							\
	viap->t2loc = TIME & 0x00ff;					\
	/* this resets the interrupt flag, & starts the counting */	\
	viap->t2hoc = (TIME >> 8) & 0x00ff;				\
	/* set up timer for one shot operation */			\
	viap->ier = VIA_IER_SET | VIA_IER_TIMER2;			\
	}

/*----------------------------------------------------------------------*/
/*	RECEIVE macros							*/
/*	assumes that status is defined and valid			*/
/*----------------------------------------------------------------------*/

#define	    CHECK_ABORT							\
	    if  (status == RCV_OVERRUN)					\
	    {								\
		TRACE(T_lap_errors, ("CHECK_ABORT\n"));			\
		lapp->cfg.stats.abort_errors++;				\
		return;							\
	    }


#define	    CHECK_CRC							\
	    if  (status == RCV_OVERRUN)					\
	    {								\
		TRACE(T_lap_errors, ("CHECK_CRC\n"));			\
		lapp->cfg.stats.crc_errors++;				\
		return;							\
	    }


#define	CHECK_OVERRUN							\
	    if  (status == RCV_OVERRUN)					\
	    {								\
		TRACE(T_lap_errors, ("CHECK_OVERRUN\n"));		\
		lapp->cfg.stats.overrun_errors++;			\
		return;							\
	    }


#define	CHECK_SIZE							\
	if  (status == RCV_TOO_LARGE)					\
	{								\
	    /* packet too large, throw packet away, reset */		\
	    TRACE(T_lap_errors, ("CHECK_SIZE: ptr %x end %x\n", lapp->rwp->ptr, lapp->rwp->end));\
	    lapp->cfg.stats.too_long_errors++;				\
	    return;							\
	}

/*----------------------------------------------------------------------*/
/*	XMIT macros							*/
/*	assumes that lapp and rr0					*/
/*----------------------------------------------------------------------*/

#define	CHECK_UNDERRUN							\
	if  (*scc_ctl_read & SCC_RR0_TXU)				\
	    {								\
	    printf("CHECK_UNDERRUN: RR0=%x\n", rr0_byte);		\
	    lapp->cfg.stats.underrun_errors++;				\
	    schedule(lapp);						\
	    return;							\
	    }


/*
 *	this sees if there is more data to send. The extern rwp structure
 *	is used because interrupts might be used to send the bytes
 */
#define	MORE_DATA							\
	rwpp->ptr < rwpp->end


/*	this sends a lap frame. start of the frame and 1 beyond the end */
/*	of the frame. Lapp is assumed.					*/

#define	XMIT_DATA_FRAME(BEGIN, END)					\
	scc_cmd(at_scc_rvd_xmit_on, lapp->scc);				\
	lapp->rwp->ptr = BEGIN;						\
	lapp->rwp->end = END;						\
	NEWSTATE(XMIT_IN_FRAME);					\
	xmit_frame(lapp);


/*	this sends a control packet. lapp is assumed to be defined	*/
/*	note, fill in the destination first, since might be using same	*/
/*	structure							*/

#define	XMIT_CTL(TYPE, DESTINATION)					\
	scc_cmd(at_scc_rvd_xmit_on, lapp->scc);				\
	lapp->ctl.destination = DESTINATION;				\
	lapp->ctl.source = lapp->cfg.node;				\
	lapp->ctl.type = TYPE;						\
	lapp->rwp->ptr = (caddr_t) &lapp->ctl;				\
	lapp->rwp->end = ((caddr_t) &lapp->ctl) + AT_LAP_HDR_SIZE;	\
	xmit_frame(lapp);

/*	this sends a control packet, preceded by a LAP pulse for RTS	*/
/*	and ENQ openning frames. lapp is assumed to be defined		*/
/*	note, fill in the destination first, since might be using same	*/
/*	structure							*/

#define	XMIT_CTL_P(TYPE, DESTINATION)					\
	scc_cmd(at_scc_rvd_pulse, lapp->scc);				\
	lapp->ctl.destination = DESTINATION;				\
	lapp->ctl.source = lapp->cfg.node;				\
	lapp->ctl.type = TYPE;						\
	lapp->rwp->ptr = (caddr_t) &lapp->ctl;				\
	lapp->rwp->end = ((caddr_t) &lapp->ctl) + AT_LAP_HDR_SIZE;	\
	xmit_frame(lapp);


/*----------------------------------------------------------------------*/
/*	MISCELANEOUS functions						*/
/*----------------------------------------------------------------------*/

/*
 *	return the M_IOCTL on the read stream. Note that the same message
 *	block is used as the vehicle, and that if there is an error return,
 *	then linked blocks are lopped off. BEWARE of multiple references.
 *	Used by other appletalk modules, so it is not static!
 *	The q passed is the WRITE queue.
 */

ioc_ack(errno,m,q)
int		errno;
struct msgb	*m;
struct queue	*q;

{
	struct iocblk	*iocbp = (struct iocblk *) m->b_rptr;

	if  (iocbp->ioc_error = errno)
	{
	    /* errno != 0, then there is an error, get rid of linked blocks! */
	    m->b_datap->db_type = M_IOCNAK;
	    iocbp->ioc_count = 0;	/* only make zero length if error */
	    if (m->b_cont)
		freemsg(unlinkb(m));
	}
	else
	{
	    m->b_datap->db_type = M_IOCACK;
	}
	if (RD(q)->q_next) {
		qreply(q, m);
	} else {
		(*RD(q)->q_qinfo->qi_putp)(RD(q),m);
	}
}


/*
 *	return an M_ERROR on the read stream. Note that the same message
 *	block that caused the problem is used as the vehicle. So chained 
 *	blocks to this message will be free'd; i.e. ASSUMES no multiple
 *	references.
 *	Used by other appletalk modules, so it is not static!
 *	The q passed is the WRITE queue.
 */

data_error(errno,mp,q)
int		errno;
struct msgb	*mp;
struct queue	*q;
{
	mp->b_datap->db_type = M_ERROR;
	mp->b_wptr = mp->b_rptr = mp->b_datap->db_base;
	*mp->b_wptr++ = errno;
	if (mp->b_cont)
	    freemsg(unlinkb(mp->b_cont));
	if (RD(q)->q_next) {
		qreply(q, mp);
	} else {
		(*RD(q)->q_qinfo->qi_putp)(RD(q),mp);
	}
}



/*----------------------------------------------------------------------*/
/*	get_upstreamq returns the pointer to the stream that has the 	*/
/*	registered type. NULL is returned if it cannot find it		*/
/*----------------------------------------------------------------------*/
static queue_t *
get_upstreamq(type, lapp)
u8		type;
lap_specifics_t *lapp;

{
	register u8		circuit;
	register u8		ncircuits;
	register lap_circuit_t	*circuitp;

	/* get the circuit number for this type of data packet */
	for (   circuit = 0, ncircuits = lapp->ncircuits, circuitp = lapp->circuit0p;
		(circuit < ncircuits) && (type != circuitp->type);
		circuit++, circuitp++
	    )
	    ;
	if (circuit >= ncircuits)
	{
	    TRACE(  T_lap, ("get_upstream: no upstream for type %d\n", type));
	    lapp->cfg.stats.type_unregistered++;
	    return(NULL);
	}
	else
	{
	    return(circuitp->upstreamq);
	}
}

/************************************************************************/
/*									*/
/*									*/
/*			timer interrupt code				*/
/*									*/
/*									*/
/************************************************************************/

void at_timer_interrupt()

{
	register  lap_specifics_t	*lapp = &lap_specifics[AT_CHANNEL];
	register  u8	  		*scc_ctl_read;
	register  u8          		*scc_ctl_write;
	register mblk_t       		*m;
	register queue_t       		*upstreamq;
	register  u8          		circuit;

	TRACE(T_lap_interrupt, ("at_timer_interrupt: state %d\n", lapp->state));
	switch (lapp->state)
	{
	case ENQ_WAIT_LINE_FREE:
	case XMIT_WAIT_LINE_FREE:
	    SCC_CTL_WRITE(SCC_WR1, SCC_WR1_RIA); /* turn off BAI interrupt */
	    TRACE(T_lap_errors, ("at_timer: missing sync interrupt\n"));
	    lapp->cfg.stats.missing_sync_irupt++;
	    NEWSTATE(   lapp->state == ENQ_WAIT_LINE_FREE ?
			ENQ_WAIT_TdTr : XMIT_WAIT_TdTr
		    );
	    access_net(lapp);
	    break;
	case XMIT_WAIT_TdTr:
	case ENQ_WAIT_TdTr:
	    scc_ctl_read = scc_ctl_write = lapp->scc->scc_ctl_write;
	    if  (   (SCC_CTL_READ(SCC_RR10) & (SCC_RR10_1CM | SCC_RR10_2CM))
	         || !(*scc_ctl_read & SCC_RR0_SYH)
		)
	    {
		/* well the line is busy, wait for the break/abort interrupt */
	        SCC_CTL_WRITE(SCC_WR14, SCC_WR14_RMC | SCC_WR14_BGE | SCC_WR14_BGS);
		TRACE(T_lap, ("at_timer: defer \n"));
		SCC_CTL_WRITE(SCC_WR1, SCC_WR1_EIN | SCC_WR1_RIA);
		at_timeout(AT_LAP_TIME_MAXFRAME, lapp->timer);
		NEWSTATE(lapp->state == XMIT_WAIT_TdTr ?
				    XMIT_WAIT_LINE_FREE : ENQ_WAIT_LINE_FREE);
	    }
	    else
	    {
		/*
		 * well the line appears to be available, go for it!
		 * But, first send the synchronization pulse, and leave the xmitter on
		 */
		if (lapp->state == XMIT_WAIT_TdTr)
		{
		    XMIT_CTL_P(AT_LAP_TYPE_RTS,
			    ((at_lap_t *) lapp->xmit_msgp->b_rptr)->destination
			    );
		}
		else
		{
		    XMIT_CTL_P(AT_LAP_TYPE_ENQ, lapp->cfg.node);
		}
		INC_STATE();
	    }
	    break;

	/*
	 * Sent the RTS, now rcv the CTS, or if broadcast, wait the interdialog gap.
	 */
	case XMIT_RTS_OFF:
	    scc_cmd(at_scc_rvd_xmit_off, lapp->scc);
	    if  (lapp->ctl.destination == 0xff)
	    {
		/*
		 * this is a broadcast, so we will not expect a CTS.
		 */
		XMIT_DATA_FRAME(lapp->xmit_msgp->b_rptr,lapp->xmit_msgp->b_wptr);
	    }
	    else
	    {
		/* set up for the hoped for CTS */
		lapp->rwp->ptr = (caddr_t) &lapp->ctl.destination;
		lapp->rwp->end = lapp->rwp->ptr + AT_LAP_HDR_SIZE + 2;
	        INC_STATE();
		at_timeout(AT_LAP_TIME_INTERFRAME, lapp->timer);
	    }
	    break;

	/*
	 * Set up to rcv the ack frame which might be coming in if someone is
	 * is at the same address as what we are trying to obtain..
	 */
	case ENQ_XMIT_OFF:
	    scc_cmd(at_scc_rvd_xmit_off, lapp->scc);
	    lapp->rwp->ptr = (caddr_t) &lapp->ctl.destination;
	    lapp->rwp->end = lapp->rwp->ptr + AT_LAP_HDR_SIZE + 2;
	    INC_STATE();
	    at_timeout(AT_LAP_TIME_INTERFRAME, lapp->timer);
	    break;

	/*
	 * Set up to rcv the data frame which should be coming in.
	 */
	case RCV_XMIT_CTS_OFF:
	    scc_cmd(at_scc_rvd_xmit_off, lapp->scc);
	    lapp->rwp->ptr = lapp->rcv_msgp->b_rptr;
	    lapp->rwp->end = lapp->rwp->ptr + AT_LAP_SIZE + 2;
	    INC_STATE();
	    at_timeout(AT_LAP_TIME_INTERFRAME, lapp->timer);
	    break;


	case XMIT_FRAME_OFF:
	    {
		register at_lap_stats_t	*statsp = &lapp->cfg.stats;

		scc_cmd(at_scc_rvd_xmit_off, lapp->scc);
		statsp->xmit_bytes += lapp->xmit_msgp->b_wptr-lapp->xmit_msgp->b_rptr-3;
		statsp->xmit_packets++;
		statsp->collisions += lapp->cfg.rts_attempts - lapp->collision_count;
		statsp->defers += lapp->cfg.rts_attempts - lapp->defer_count;
		freemsg(lapp->xmit_msgp);
		lapp->xmit_msgp = NULL;
	    }
	case ACK_XMIT_OFF:
	    scc_cmd(at_scc_rvd_xmit_off, lapp->scc);
	    schedule(lapp);
	    break;

	case ENQ_READY_FOR_ACK:
	case XMIT_READY_FOR_CTS:
	    /* can only guess there was a collision */
	    if  (--lapp->collision_count <= 0)
	    {
		/* defered once too many, return error */
		m = lapp->xmit_msgp;
#ifdef	DEBUG
		if  (!m)
		{
		    printf("no write message\n");
		    schedule(lapp);
		    break;
		}
#endif
		TRACE(  T_lap,
			("at_timer: collision to %x\n",
			((at_lap_t *) m->b_rptr)->destination
			)
		     );
		if  (lapp->state == ENQ_READY_FOR_ACK)
		{
		    if  (--(((struct iocblk *)m->b_rptr)->ioc_count))
		    {
			/* we still have to send it more times */
			adjust_backoff(lapp);
		    }
		    else
		    {
			/* no one responded to this address, so guess we can use it */
			circuit = LAP_IOCTAGS(m)->circuit;
			if ((upstreamq =
			    lapp->circuit0p[(unsigned short)circuit].upstreamq) != NULL)
			{
			    /* there is an upstream! */
        		    ddp_specifics.cfg.network_up = 1;
        		    ddp_specifics.cfg.this_node = lapp->cfg.node;
			    ioc_ack(0, m, WR(upstreamq));
			}
			else
			{
			    TRACE(  T_lap,
				    ("at_timer: no upstream for ONLINE ack %d\n",
				    circuit)
				 );
			    lapp->cfg.stats.ioc_unregistered++;
			    freemsg(m);
			}
			lapp->xmit_msgp = NULL;
		    }
		}
		else
		{
		    /* data frame, get type so can figure what to do with it */
		    if (upstreamq =
			    get_upstreamq(((at_lap_t *) m->b_rptr)->type, lapp))
		    {
			/* there is an upstream! */
			data_error(EHOSTUNREACH, m, WR(upstreamq));
		    }
		    else
		    {
			freemsg(m);
		    }
		    lapp->xmit_msgp = NULL;
		}
	    }   /* of if  (--lapp->collision_count <= 0)  */
	    lapp->collision_history |= 1;
	    schedule(lapp);
	    break;

	case RCV_XMIT_CTS:
	case RCV_IN_FRAME:
	case XMIT_RTS:
	case XMIT_IN_FRAME:
	case ACK_XMIT:
	    /* well something didnot happen in time, record, and reset */
	    TRACE(T_lap_errors, ("at_timer: timeout in state %x\n", lapp->state));
	    lapp->cfg.stats.timeouts++;
	    schedule(lapp);
	    break;
	case OFFLINE:
	case ONLINE:
	default:
	    /* we should have not gotten an interrupt, timer not active */
	    TRACE(T_lap, ("at_timer: %d: unwanted via interrupts state %d\n",
			lapp-lap_specifics, lapp->state));
	    at_untimeout(lapp->timer);
	}
}

/************************************************************************/
/*									*/
/*									*/
/*			scc interrupt code				*/
/*									*/
/*									*/
/************************************************************************/
/*----------------------------------------------------------------------*/
/*	adjust_backoff adjusts the backoff mask according to the past	*/
/*	defer and collision history as discussed in the appletalk doc 	*/
/*	and code. I agree that the "discussion" in those are contra-	*/
/*	dictory but the code is the defacto.				*/
/*----------------------------------------------------------------------*/

	/* assumes i and count are defined!!!!! */
BIT_COUNT(VALUE)
u8	VALUE;
{
	int	count = 0;
	int	i;

	for (i=8; i-->0; VALUE >>= 1)
	    if  (VALUE & 1)
		count++;
	return(count);
}


static void
adjust_backoff(lapp)
register lap_specifics_t *lapp;

{
	register u8	temp;

	
	if  (BIT_COUNT(lapp->collision_history) > 2)
	{
	    /*
	     * net is getting busy, increase randomness range of backoff
	     * NOTE that the doc says that the result should be at least
	     * 2, but their code doesn't care
	     */
	    lapp->local_backoff = lapp->global_backoff =
				     ((lapp->global_backoff << 1) + 1) & 0x0f;
	    lapp->collision_history = 0;
	}
	else
	{
	    
	    if  (BIT_COUNT(lapp->defer_history) < 2)
	    {
		/* well net not so busy now, decrease randomness range */
		lapp->local_backoff = (lapp->global_backoff >>= 1) & 0xff;
		lapp->defer_history = 0x0ff;
	    }
	}
	lapp->collision_history <<= 1;
	lapp->defer_history <<= 1;
	lapp->collision_count = lapp->defer_count = lapp->cfg.rts_attempts;
	TRACE(  T_lap_backoff, ("adjust_backoff: defer hist %x cnt %d collision hist %x cnt %d\n",
				 lapp->defer_history,
				 lapp->defer_count,
				 lapp->collision_history,
				 lapp->collision_count
			       )
	     );
	TRACE(  T_lap_backoff, (" mask %x\n", lapp->local_backoff));
}


/*----------------------------------------------------------------------*/
/*	access_net handles all the collision and defer backoffs. This 	*/
/*	is confusing so follow closely.					*/
/*									*/
/*	    First we find out if the line is busy. This is not so easy  */
/*	    since there is no carrier detect (/CS). If we start looking */
/*	    for a packet in the middle of a transmission, we will think */
/*	    the line idle and step all over it. The sync/hunt bit 	*/
/*	    according to Apple (not Zilog) reflects in or out of frame	*/
/*									*/
/*	    Next we wait the 400us interdialog gap termed Td and then	*/
/*	    the random component in the range 0 - 1500us termed Tr.	*/
/*	    This could have already expired if this write occurs at the	*/
/*	    queue service level; i.e. we were doing nothing, the net 	*/
/*	    was doing nothing, and now want to transmit.		*/
/*									*/
/*	    If during this time someone else started transmitting, then */
/*	    we listen for the end of the frame, adjust defer history,	*/
/*	    and start over.						*/
/*									*/
/*	    Otherwise the line is free, and we send a rts. We then wait */
/*	    for a CTS or a timeout. If timeout, we assume a collision   */
/*	    adjust the collision history, and start over if under the   */
/*	    maximum retry count.					*/
/*									*/
/*	NOTE: the state variable is setup prior to entry!		*/
/*----------------------------------------------------------------------*/
static void
access_net(lapp)
register lap_specifics_t *lapp;

{
	register unsigned short	temp;
	register unsigned int	time;
	register  u8	  	*scc_ctl_read;
	register  u8          	*scc_ctl_write;
	register mblk_t       	*m;
	register queue_t       	*upstreamq;
	register  u8          	circuit;

	/* see if the receiver is busy */
	scc_ctl_read  = scc_ctl_write = lapp->scc->scc_ctl_write;
	if  (!(*scc_ctl_read & SCC_RR0_SYH))
	{
	    /* well the line is busy, how many times has this occured? */
	    if  (--lapp->defer_count <= 0)
	    {
		/* defered once too many, return error */
		m = lapp->xmit_msgp;
		if  (!m)
		{
		    printf("access_net: no write message\n");
		    schedule(lapp);
		    return;
		}
		if  (lapp->state == ENQ_WAIT_TdTr)
		{
		    /* trying to find our address and can't even find the net */
		    circuit = LAP_IOCTAGS(m)->circuit;
		    if ((upstreamq =
			lapp->circuit0p[(unsigned short) circuit].upstreamq) != NULL)
		    {
			/* there is an upstream! */
			ioc_ack(ENETUNREACH, m, WR(upstreamq));
		    }
		    else
		    {
			freemsg(m);
		    }
		    lapp->xmit_msgp = NULL;
		    SCC_CTL_WRITE(SCC_WR9, lapp->scc->wr9_reset);
		    lapp->rwp->ptr = lapp->rwp->end = (caddr_t) &lapp->ctl.destination;
		    lap_hangup(lapp, 0);	/* do not set state, hangup will */
		    lap_offline(lapp);
		}
		else
		{
		    /* get type of packet so can figure what to do with it */
		    if (upstreamq =
			    get_upstreamq(((at_lap_t *) m->b_rptr)->type, lapp))
		    {
			/* there is an upstream! */
			data_error(ENETUNREACH, m, WR(upstreamq));
		    }
		    else
		    {
			freemsg(m);
		    }
		}
		lapp->xmit_msgp = NULL;
		schedule(lapp);
		return;
	    }
	    /*
	     * well the line is busy, but not too busy,
	     *  wait for the break/abort irupt
	     */
 	    lapp->rwp->ptr = (caddr_t) &lapp->ctl.destination;	/* for RTS */
	    lapp->rwp->end = lapp->rwp->ptr + AT_LAP_HDR_SIZE + 2;
	    NEWSTATE(lapp->state == XMIT_WAIT_TdTr ?
				XMIT_WAIT_LINE_FREE : ENQ_WAIT_LINE_FREE);
	    SCC_CTL_WRITE(SCC_WR1, SCC_WR1_EIN | SCC_WR1_RIA);
	    at_timeout(AT_LAP_TIME_MAXFRAME, lapp->timer);
	}
	else
	{
	    /*
	     * well the line appears to be available, go for it!
	     * reset the missing clock, so we can see if someone else sends
	     * synchronization pulse.
	     */
	    SCC_CTL_WRITE(SCC_WR14, SCC_WR14_RMC | SCC_WR14_BGE | SCC_WR14_BGS);
	    /* calculate Tr the way the mac does */
	    temp = (lapp->random * (unsigned short)733) + 1;
	    lapp->random = (temp << 8 & 0xff00) | (temp >> 8 & 0x00ff);
	    time = AT_LAP_TIME_Td
		   + ((unsigned short)(lapp->random & lapp->local_backoff) *
		      (unsigned short)AT_LAP_TIME_100us);
	    TRACE(  T_lap_backoff, ("access_net: wait TdTr: time %x, random %x\n",
				     time, lapp->random 
				   )
		 );
 	    lapp->rwp->ptr = (caddr_t) &lapp->ctl.destination;	/* for RTS */
	    lapp->rwp->end = lapp->rwp->ptr + AT_LAP_HDR_SIZE + 2;
	    SCC_CTL_WRITE(SCC_WR1, SCC_WR1_RIA); /* turn off BAI */
	    at_timeout(time, lapp->timer);
	}
	return;
}


/*----------------------------------------------------------------------
 *	schedule checks to see what needs to be done next. If there is
 *	something to transmit, i.e. the multiplexer queue is not empty,
 *	then transmission is started, else the receiver is.
/*----------------------------------------------------------------------*/
static void
schedule(lapp)
register lap_specifics_t	*lapp;

{
	register mblk_t		*m;
	register queue_t	*upstreamq;
	register at_lap_t	*lap_packetp;
	u8			circuit;
	mblk_t			*cm;
	u8			destination;
	u8			*scc_data_read = lapp->scc->scc_data_read;

	/*---------------------------------------------------------------*/
	/* There might be an error condition, so empty FIFO completely,  */
	/* could be 3 bytes left, and issue Error REset			 */
	/*---------------------------------------------------------------*/
	circuit  = *scc_data_read;
	circuit  = *scc_data_read;
	circuit  = *scc_data_read;
	*lapp->scc->scc_ctl_write = SCC_WR0_ERE;

	/*---------------------------------------------------------------*/
	/* First we decide if we are going to transmit or receive, in    */
	/* otherwords is there any thing in the transmit q?		 */
	/*---------------------------------------------------------------*/
	if  ((m = lapp->xmit_msgp) == NULL)
	    /*------------------------------------------------------*/
	    /* nothing there immediately, process write queue until */
	    /* the write queue is empty or there is something to    */
	    /* xmit						    */
	    /*------------------------------------------------------*/
	    while (1)
	    {
		if ((m = getqHI(lapp->downstreamq)) == NULL)
		{
		    /*------------------------------------------------------*/
		    /* nothing to transmit, so now go into receive mode	    */
		    /*------------------------------------------------------*/
		    at_untimeout(lapp->timer);
		    lapp->rwp->ptr = (caddr_t) &lapp->ctl.destination;
		    lapp->rwp->end = lapp->rwp->ptr + AT_LAP_HDR_SIZE + 2;
		    NEWSTATE(ONLINE);
		    TRACE(T_lap, ("schedule: idle\n")); 
		    return;
		}
		else
		{
		    /*------------------------------------------------------*/
		    /* something in the write queue, proccess what can	    */
		    /* but make sure only echo to this node once!	    */
		    /*------------------------------------------------------*/
		    if (m->b_datap->db_type == M_DATA) 
		    {
			lap_packetp = (at_lap_t *) m->b_rptr;
			destination = lap_packetp->destination;
			TRACE(T_lap, ("schedule: M_DATA to %x\n",destination)); 
			if (destination == lapp->cfg.node)
			{
			    /* this message is intended for this node, this */
			    /* circuit so send it upstream, and we are done!*/
			    if  (upstreamq =
					get_upstreamq(lap_packetp->type, lapp)
				)
			    {
				(*upstreamq->q_qinfo->qi_putp)(upstreamq,m);
			    }
			    else
			    {
				freemsg(m);
			    }
			}
			else
			{
			    if (destination == 0xff)
			    {
				/* message is intended for this node as well */
				if  (upstreamq =
					get_upstreamq(lap_packetp->type, lapp)
				    )
				{
				    /*
				     * copymsg not dupmsg, since receive side might do
				     * something to the data before it is transmitted
				     */
					
				    if  (cm = copymsg(m))
				    {
					/* no room on the next q next q! */
					(*upstreamq->q_qinfo->qi_putp) (upstreamq,cm);
				    }
				}
			    }
			    lapp->xmit_msgp = m;
			    adjust_backoff(lapp);
			    break;
			}
		    }
		    else
		    {
			/* not data, better be a synchronization control! */
			if (   (m->b_datap->db_type == M_IOCTL) 
			    && (((struct iocblk *)m->b_rptr)->ioc_cmd==AT_SYNC)
			   )
			{
			    TRACE(  T_lap, ("schedule: AT_SYNC\n"));
			    circuit = LAP_IOCTAGS(m)->circuit;
			    if ((upstreamq =
				    lapp->circuit0p[(unsigned short)circuit].upstreamq)
						!= NULL)
				ioc_ack(0,m,WR(upstreamq));
			    else
			    {
				freemsg(m);
				TRACE(T_lap_errors, ("schedule: ioctl unregistered %d\n",
						     circuit
						    )
				     );
				lapp->cfg.stats.ioc_unregistered++;
			    }
			    continue;
			}
			else
			    /* not synchronization, last chance, an M_FLUSH?! */
			    if (   (m->b_datap->db_type == M_FLUSH) 
				&& (*m->b_rptr & FLUSHR)
			       )
			    {
				TRACE(  T_lap, ("schedule: M_FLUSH\n"));
				circuit = LAP_IOCTAGS(m)->circuit;
				if ((upstreamq =
				   lapp->circuit0p[(unsigned short)circuit].upstreamq)
							!= NULL)
				{
				    flushq(upstreamq, FLUSHALL);
				    putnext(upstreamq, m);
				}
				else
				{
				    freemsg(m);
				    TRACE(T_lap_errors, ("schedule: ioctl unregistered %d\n",
							 circuit
							)
					 );
				    lapp->cfg.stats.ioc_unregistered++;
				}
			    }
			    else   /*if ((m->b_datap->db_type == M_FLUSH)... */
			    {
				printf("schedule: unknown message %x\n",
					m->b_datap->db_type
				      );
				lapp->cfg.stats.unknown_mblks++;
				freemsg(m);
			    }
		    }  /* else if (m->b_datap->db_type == M_DATA) */
		} /* elseif ((m = getqHI(lapp->downstreamq)) == NULL) */
	    }   /* while (1) */
	/*-------------------------------------------------------------------*/
	/* the wrt_msgp points to an enq or message to start transmission on */
	/*-------------------------------------------------------------------*/
	NEWSTATE((m->b_datap->db_type == M_IOCTL)
					 ? ENQ_WAIT_TdTr : XMIT_WAIT_TdTr);
	access_net(lapp);
	return;
}

/*----------------------------------------------------------------------*/
/*	This looks at the status flag and accounts for errors, 		*/
/*----------------------------------------------------------------------*/

static void
account_rcverr(lapp, status)
register lap_specifics_t *lapp;

{
	CHECK_OVERRUN;
	CHECK_ABORT;
	CHECK_CRC;
	CHECK_SIZE;
}

/*----------------------------------------------------------------------*/
/*	This interprets the frame just read in. If not a data frame	*/
/*	then it can only be and RTS or ENQ. If data, it is put on the	*/
/*	demultiplexer q's.						*/
/*----------------------------------------------------------------------*/

static void
rcv_data_frame(lapp)
lap_specifics_t *lapp;

{
	register u8		type;
	register mblk_t		*m;
	register queue_t	*upstreamq;
	register at_lap_stats_t	*statsp = &lapp->cfg.stats;

	m = lapp->rcv_msgp;
	/*
	 * The read ptr was set at alloc time, so now set the write ptr.
	 */
	m->b_wptr = lapp->rwp->ptr - 2;		/* remove checksum */
	statsp->rcv_bytes += m->b_wptr - m->b_rptr - AT_LAP_HDR_SIZE;
	statsp->rcv_packets++;
	if (m->b_wptr - m->b_rptr < AT_LAP_HDR_SIZE)
	{
	    TRACE(T_lap_errors, ("rcv_data_frame: too short %x\n",
				  ((at_lap_t *) m->b_rptr)->source
				)
		 );
	    lapp->cfg.stats.too_short_errors++;
	    return;
	}
	/* get type of lap packet so can figure what to do with it */
	type = ((at_lap_t *) m->b_rptr)->type;
	TRACE(T_lap, ("rcv_data_frame: from %x of type %x\n",
		      ((at_lap_t *) m->b_rptr)->source, type
		     )
	     );
	if (type & 0x80)
	{
	    /*
	     * then this is not a data packet, it is a control! If it is
	     * an RTS or ENQ then try to accomodate it.
    	     */
	    printf("protocol violation: expecting data frame and got ctl packet %x\n", type);
	    return;
	}
	/* find if the frame came from who we asked for it from */
	if  ((lapp->ctl.destination == 0xff) ?
		  (lapp->ctl.source != ((at_lap_t *) m->b_rptr)->source)
		: (lapp->ctl.destination != ((at_lap_t *) m->b_rptr)->source)
	    )
	{
	    printf("protocol violation, data frame not from desired node\n");
	    return;
	}
	/* get the circuit number for this type of data packet */
	if (upstreamq = get_upstreamq(type, lapp))
	{
	    /* there is an upstream! */
	    (*upstreamq->q_qinfo->qi_putp) (upstreamq, m);
	    if (   (lapp->rcv_msgp =
		    allocb(AT_LAP_SIZE + (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE), BPRI_MED)
		   )
		 == NULL
		)
	    {
		printf("rcv_data_frame: %x: out of stream buffers\n",lapp-lap_specifics);
		return;
	    }

	    /*
	     *	Leave some space for our DDP friends
	     */

	    lapp->rcv_msgp->b_rptr += (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE);
	}
	return;
}

/*----------------------------------------------------------------------*/
/*	This routine transmits the frame passed to it. NOTE that the 	*/
/*	state variable has to be set up prior to the call, since this	*/
/*	routine does not know what type of frame is being transmitted.	*/
/*	Also the transmitter has already been turned on because it might*/
/*	have the special pulse turn on for RTS or CTS			*/
/*----------------------------------------------------------------------*/

static int
xmit_frame(lapp)
lap_specifics_t	*lapp;

{
	register  u8          *scc_data_write = lapp->scc->scc_data_write;
	register  rwp_t       *rwpp = lapp->rwp;
	u8          	      *scc_ctl_read;
	u8          	      *scc_ctl_write;

	scc_ctl_read = scc_ctl_write = lapp->scc->scc_ctl_write;
	if  (!(*scc_ctl_read & SCC_RR0_TBE))
	{
	    printf("xmit_bytes: not TBE at start of frame, RR0=%x\n", *scc_ctl_read);
	    lapp->cfg.stats.unknown_irupts++;
	    schedule(lapp);
	    return;
	}
	/*
	 * It is assumed that there is more data to transmit! Checks are always
	 * done after the writing of a byte. When a frame is started, only two
	 * bytes of a minimum three are written, leaving at least one for this.
	 */
	*scc_data_write = *(rwpp->ptr++);
	/*
	 * Now that the first character is in there, allow aborts to be sent
	 * after the closing flag. This way we won't have to play with the 
	 * transmitters and drivers like the MAC does.
	 */
	SCC_CTL_WRITE(SCC_WR10, SCC_WR10_CPI | SCC_WR10_FM0 | SCC_WR10_MFI);
	do
	{
	    register  int          time_out;
	    /*
	     * Transmit the characters, till no more to transmit.
	     */
	    time_out = AT_DELAY_B_INTERCHAR;
	    do
	    {
		;     
		if (*scc_ctl_read & SCC_RR0_TBE)
		{
		    *scc_data_write = *(rwpp->ptr++);
		    break;
		}
	    }   while (--time_out);
	    if (time_out == 0)
	    {
		printf("xmit_bytes: intercharacter timeout, RR0=%x\n", *scc_ctl_read);
		return;
	    }
	} while  (MORE_DATA);
	/*
	 * All the data is at least in the fifo if not out the door, so
	 * reset transmit underrun latch so the silly scc will send out
	 * the CRC and ectera.
	 */
	*scc_ctl_write = SCC_WR0_RTU;
	/*
	 * we have loaded all the characters into the transmitter, so
	 * set the timer to bring us back and turn off the transmitter when
	 * that character, the two crc bytes and the flag have gone out,
	 * i.e. 34.72 * 4 = 138.88us. Turn off scc interrupts.
	 */
	lapp->rwp->ptr = (caddr_t) &lapp->ctl.destination;	/* for RTS */
	lapp->rwp->end = lapp->rwp->ptr + AT_LAP_HDR_SIZE + 2;
	at_timeout(AT_LAP_TIME_XMIT_OFF, lapp->timer);
	INC_STATE();
	return;
}

/*----------------------------------------------------------------------*/
/*	This routine transmits a CTS, and sets up the state variable	*/
/*----------------------------------------------------------------------*/
static int
xmit_cts(lapp)
register lap_specifics_t	*lapp;

{
	/* can we respond with a cts, i.e. is there a mblk to receive into */
	if  (lapp->rcv_msgp == NULL)
	{
	    TRACE(T_lap, ("xmit_cts: allocating\n"));
	    if (   (lapp->rcv_msgp =
		    allocb(AT_LAP_SIZE + (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE), BPRI_MED)
		   )
		 == NULL
		)
	    {
		printf("lap: %x: out of stream buffers\n", lapp-lap_specifics); 
		schedule(lapp);
		return;
	    }
	    /*
	     * Well got a new buffer, but we have to give room for ddp to convert what
	     * is possibly a short header into an extended header.
	     */
	    lapp->rcv_msgp->b_rptr += (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE);
	}
	if  (lapp->ctl.destination == 0xff)
	{
	    /* broadcast packet, do not send cts */
	    lapp->rwp->ptr = lapp->rcv_msgp->b_rptr;
	    lapp->rwp->end = lapp->rwp->ptr + AT_LAP_SIZE + 2;
	    NEWSTATE(RCV_IN_FRAME);
	    at_timeout(AT_LAP_TIME_INTERFRAME, lapp->timer);
	    return;
	}
	/* not a broadcast, send cts */
	NEWSTATE(RCV_XMIT_CTS);
	XMIT_CTL( AT_LAP_TYPE_CTS, lapp->ctl.source);
	return;
}


/*----------------------------------------------------------------------*/
/*	This routine transmits a ACK, and sets up the state variable	*/
/*	This can occur in the ONLINE state, or when expecting data	*/
/*----------------------------------------------------------------------*/
static int
xmit_ack(lapp)
lap_specifics_t *lapp;

{
	/* we respond with a ack */
	NEWSTATE(ACK_XMIT);
	XMIT_CTL( AT_LAP_TYPE_ACK, lapp->ctl.source);
}

/*----------------------------------------------------------------------*/
/*	This routine services all scc interrupts.			*/
/*----------------------------------------------------------------------*/
void at_lap_interrupt(dev, status)

{
	register  lap_specifics_t *lapp = &lap_specifics[(unsigned short)dev];
	u8              	  *scc_ctl_write = lapp->scc->scc_ctl_write;

	TRACE(T_lap_interrupt, ("at_lap_interrupt: dev %d state %d status %d\n",
				dev, lapp->state, status
			       )
	     );
	switch (lapp->state)
	{
	case OFFLINE:
	    printf("lap: %x: turned off and still getting interrupts\n",
		    lapp-lap_specifics); 
	    SCC_CTL_WRITE(SCC_WR9, lapp->scc->wr9_reset);
	    break;

	case ONLINE:
	    /* start reading in the control packet */
	    NEWSTATE(RCV_RECEIVING_RTS);
	    /* fall through */
	case RCV_RECEIVING_RTS:
	    /* continue reading in the control packet */
	    if  (status == RCV_OK)
	    {
		/*
		 * if it is an RCV_OK, then we know that rcv_bytes made sure its
		 * a control packet. Lets check the type
		 */
		switch (lapp->ctl.type)
		{
		case AT_LAP_TYPE_RTS:
		    TRACE(T_lap, ("at_scc: RTS from %x\n", lapp->ctl.source));
		    xmit_cts(lapp);
		    break;
		case AT_LAP_TYPE_ENQ:
		    TRACE(T_lap, ("at_scc: ENQ from %x\n", lapp->ctl.source));
		    xmit_ack(lapp);
		    break;
		default:
		    schedule(lapp);
		}
	    }
	    else
	    {
		account_rcverr(lapp, status);
		schedule(lapp);
	    }
	    break;
		/*
		 * should not get here since the xmit_bytes routine handles it
		 * or if interrupt based transmit, the assembly rountine should.
		 */
	case ENQ_XMIT:
	case ACK_XMIT:
	case RCV_XMIT_CTS:
	case XMIT_RTS:
	case XMIT_IN_FRAME:
	    printf("lap: %d scc interrupts when transmitting\n",
		    lapp-lap_specifics
		  ); 

	case ENQ_XMIT_OFF:
	case ACK_XMIT_OFF:
	case RCV_XMIT_CTS_OFF:
	case XMIT_RTS_OFF:
	case XMIT_FRAME_OFF:
	    /*
	     * should not get here in this interrupt routine since the timeout
	     * interrupt will handle it.
	     */
	    printf("lap: %x: receiving scc interrupts when transmitting EOF\n",
		    lapp-lap_specifics
		  ); 
	    lapp->cfg.stats.unknown_irupts++;
	    lapp->rwp->ptr = (caddr_t) &lapp->ctl.destination;	/* for RTS */
	    lapp->rwp->end = lapp->rwp->ptr + AT_LAP_HDR_SIZE + 2;
	    break;

	case RCV_IN_FRAME:
	    if  (status == RCV_OK)
	    {
		rcv_data_frame(lapp);
	    }
	    else
	    {
		account_rcverr(lapp, status);
	    }
	    schedule(lapp);
	    break;

	case ENQ_WAIT_TdTr:
	case XMIT_WAIT_TdTr:
	    /*
	     * should not get here in this interrupt routine since the timeout
	     * interrupt will handle it. But if we do then there is either an
	     * error, or there is a packet coming in, perhaps someone else
	     * got the line. Assume an RTS.
	     */
	    if  (status == RCV_OK)
	    {
		/*
		 * if it is an RCV_OK, then we know that rcv_bytes made sure its
		 * a control packet. Lets check the type
		 */
		switch (lapp->ctl.type)
		{
		case AT_LAP_TYPE_RTS:
		    TRACE(T_lap, ("at_scc: RTS from %x when waiting for line free\n", lapp->ctl.source));
		    xmit_cts(lapp);
		    break;
		case AT_LAP_TYPE_ENQ:
		    TRACE(T_lap, ("at_scc: ENQ from %x when waiting for line free\n", lapp->ctl.source));
		    xmit_ack(lapp);
		    break;
		default:
		    schedule(lapp);
		}
	    }
	    else
	    {
		account_rcverr(lapp, status);
		schedule(lapp);
	    }
	    break;

	case ENQ_WAIT_LINE_FREE:
	    SCC_CTL_WRITE(SCC_WR1, SCC_WR1_RIA); /* turn off BAI */
	    /* do not respond to network, we are not online yet */
	    NEWSTATE(ENQ_WAIT_TdTr);
	    access_net(lapp);
	    break;

	case XMIT_WAIT_LINE_FREE:
	    SCC_CTL_WRITE(SCC_WR1, SCC_WR1_RIA); /* turn off BAI */
	    if  (status == RCV_OK)
	    {
		/*
		 * if it is an RCV_OK, then we know that rcv_bytes made sure its
		 * a control packet. Lets check the type
		 */
		switch (lapp->ctl.type)
		{
		case AT_LAP_TYPE_RTS:
		    TRACE(T_lap, ("at_scc: RTS from %x when waiting for line free\n", lapp->ctl.source));
		    xmit_cts(lapp);
		    break;
		case AT_LAP_TYPE_ENQ:
		    TRACE(T_lap, ("at_scc: ENQ from %x when waiting for line free\n", lapp->ctl.source));
		    xmit_ack(lapp);
		    break;
		default:
		    schedule(lapp);
		}
	    }
	    else
	    {
		NEWSTATE(XMIT_WAIT_TdTr);
		access_net(lapp);
	    }
	    break;

	case XMIT_READY_FOR_CTS:
	    /* continue reading in the control packet */
	    if  (status == RCV_OK)
	    {
		/*
		 * if it is an RCV_OK, then we know that rcv_bytes made sure its
		 * a control packet. Lets check the type
		 */
		switch (lapp->ctl.type)
		{
		case AT_LAP_TYPE_CTS:
		    /* check if node sent to is the one responding */
		    TRACE(T_lap, ("at_scc: CTS from %x\n", lapp->ctl.source));
		    if  (lapp->ctl.source !=
			     ((at_lap_t *) lapp->xmit_msgp->b_rptr)->destination
			)
		    {
			printf("protocol violation, CTS not from desired node\n");
			schedule(lapp);
			break;
		    }
		    XMIT_DATA_FRAME(lapp->xmit_msgp->b_rptr,lapp->xmit_msgp->b_wptr);
		    break;

		case AT_LAP_TYPE_RTS:
		    printf("lap %d got an RTS from %x when it expected a CTS\n",
			    lapp-lap_specifics
			  ); 
		    xmit_cts(lapp);
		    break;
		case AT_LAP_TYPE_ENQ:
		    xmit_ack(lapp);
		    break;
		default:
		    schedule(lapp);
		}
	    }
	    else
	    {
		account_rcverr(lapp, status);
		schedule(lapp);
	    }
	    break;

	case ENQ_READY_FOR_ACK:
	    /* continue reading in the control packet */
	    if  (status == RCV_OK)
	    {
		/*
		 * if it is an RCV_OK, then we know that rcv_bytes made sure its
		 * a control packet. Lets check the type
		 */
		switch (lapp->ctl.type)
		{
		case AT_LAP_TYPE_ACK:
		    TRACE(T_lap, ("at_scc: ACK from %x\n", lapp->ctl.source));
		    /* check if node sent to is the one responding */
		    if  (lapp->ctl.source != lapp->cfg.node)
		    {
			/* the rts we sent to is not the one CTS is from */
			/* protocol violation */
			schedule(lapp);
			break;
		    }
		    /* well we got an ack, someone else is using this node */
		    /* onto the next one */
		    if  (lapp->cfg.node < 128)
			/* keep the address in the same server/client range */
			lapp->cfg.node = (lapp->cfg.node + 1) & 0x7f;
		    else
			lapp->cfg.node = (lapp->cfg.node + 1) | 0x80;
		    if  (lapp->cfg.node == lapp->cfg.initial_node)
		    {
			register  queue_t	*upstreamq;
			register  u8		circuit;

			/* we have gone full circle, no node available */
			circuit = LAP_IOCTAGS(lapp->xmit_msgp)->circuit;
			if ((upstreamq =
				lapp->circuit0p[(unsigned short)circuit].upstreamq)
				!=  NULL
			   )
			{
			    /* there is an upstream! */
			    ioc_ack(EADDRNOTAVAIL, lapp->xmit_msgp, WR(upstreamq));
			}
			else
			{
			    freemsg(lapp->xmit_msgp);
			}
			lapp->xmit_msgp = NULL;
			SCC_CTL_WRITE(SCC_WR9, lapp->scc->wr9_reset);
			lap_hangup(lapp, 0); /* do not set state hangup will */
			lap_offline(lapp);
			break;
		    }
		    /* if this a server node and must try longer or just a lowly wks */
		    ((struct iocblk *) lapp->xmit_msgp->b_rptr)->ioc_count =
						    (lapp->cfg.node > 127) ? 120 : 20;
		    SCC_CTL_WRITE(SCC_WR6, lapp->cfg.node);
		    schedule(lapp);
		    break;

		default:
		    schedule(lapp);
		}
	    }
	    else
	    {
		account_rcverr(lapp, status);
		schedule(lapp);
	    }
	    break;


	default:
	    printf("lap: %x: illegal state %x\n", lapp-lap_specifics, lapp->state); 
	    schedule(lapp);
	    break;
	} /* of switch (lapp->state) */
	SCC_CTL_WRITE(SCC_WR0, SCC_WR0_RHI);
}

/************************************************************************/
/*									*/
/*									*/
/*			module code,					*/
/*				used by other appletalk modules		*/
/*									*/
/*									*/
/************************************************************************/

/*
 *	zero out the stats structure. I know this is the lazy way, but it is
 *	very immune to changes in the structure's definition.
 */

init_stats(lapp)
lap_specifics_t *lapp;
{
	caddr_t		p;
	u32		i;

	for (p = (caddr_t) &lapp->cfg.stats, i = sizeof(at_lap_stats_t);
	     i--;
	     *p++ = 0
	    );
}



/*
 *	Get a new queue and initialize it structures. Return queue pointer is succesful,
 *	else NULL
 */

static queue_t *
new_queue(rd_qinit, wr_qinit)
struct qinit	*rd_qinit;
struct qinit	*wr_qinit;

{
	queue_t		*nq;

	if (!(nq = allocq())) {
	    return(NULL);
	}
	nq->q_qinfo = rd_qinit;
	WR(nq)->q_qinfo = wr_qinit;
	nq->q_minpsz = nq->q_qinfo->qi_minfo->mi_minpsz;
	nq->q_maxpsz = nq->q_qinfo->qi_minfo->mi_maxpsz;
	nq->q_hiwat = nq->q_qinfo->qi_minfo->mi_hiwat;
	nq->q_lowat = nq->q_qinfo->qi_minfo->mi_lowat;
	nq->q_flag |= QWANTR;
	WR(nq)->q_minpsz = WR(nq)->q_qinfo->qi_minfo->mi_minpsz;
	WR(nq)->q_maxpsz = WR(nq)->q_qinfo->qi_minfo->mi_maxpsz;
	WR(nq)->q_hiwat = WR(nq)->q_qinfo->qi_minfo->mi_hiwat;
	WR(nq)->q_lowat = WR(nq)->q_qinfo->qi_minfo->mi_lowat;
	WR(nq)->q_flag |= QWANTR;
	return(nq);
}


lap_close(q, flag)
queue_t *q;
{
#ifdef DEBUG
	printf("lap_close\n");
#endif
	CIRCUIT(q)->upstreamq = NULL;
	CIRCUIT(q)->type = 0;
	CIRCUIT(q)->name[0] = '\0';
}


/*
 *	we are at a demultiplexer queue, and would only get here only if the 
 *	upstream q was blocked when there was data for it, and it is now not.
 *	Normally data is passed on beyond the demultiplexer q's.
 */

lap_rsrvc(q)
queue_t *q;
{
	mblk_t		*mp;

	TRACE(T_lap, ("lap_rsrvc: q %d\n", q-queue));
	while  (canput(q->q_next))
	{
	    if ((mp = getqHI(q)) != NULL) 
		putnext(q, mp);
	    else
		break;
	}   /* while (canput) */
}


lap_wputq(q, m)
queue_t 	*q;
register mblk_t	*m;
{
	register lap_specifics_t *lapp = CIRCUIT(q)->lap;
	register at_lap_t *lap_packetp;
	struct iocblk 	*iocbp;
	int		s;
	u16		circuit;
	u16		size;
	lap_circuit_t	*circuitp;
	u8		type;
	at_lap_entry_t	*entryp;
	at_lap_cfg_t	*cfgp;
	u8    		*scc_ctl_write;
	queue_t 	*lapq;
	queue_t 	*ddpq;
	extern struct streamtab ddptab;

	
	TRACE(T_lap, ("lap_wputq: q %d type %x\n", q-queue, m->b_datap->db_type));
	switch (m->b_datap->db_type) 
	{
	case M_DATA:
	    if  (lapp->state == OFFLINE)
	    {
		data_error(ENETDOWN, m, q);
		break;
	    }
	    size = m->b_wptr - m->b_rptr;
	    if  (m->b_cont)
	    {
		register caddr_t	top;
		register caddr_t	fromp;
		register int		i;
		mblk_t			*tm;
		mblk_t			*nm;

		/*
		 * find out how big all the data blocks are and concatenate
		 * Would be nice if we had pullupmsg. First get buffer big enough to
		 * handle it all, the copy all the data blocks into it and free them.
		 * Don't forget to give ddp some room to grow if dest = this node
		 */
		size += msgdsize(m->b_cont);
		if ((nm = allocb(size + (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE),
				BPRI_MED)) == NULL)
		{
		    data_error(ENOBUFS, m, q);
		    break;
		}
		nm->b_rptr += (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE);
		top = nm->b_rptr;
		do
		{
		    for (i = m->b_wptr - m->b_rptr, fromp = m->b_rptr;
			 i--;
			 *top++ = *fromp++
			)
			;
		    tm = m;
		    m = unlinkb(tm);
		    freemsg(tm);
		}  while (m);
		nm->b_wptr = top;
		m = nm;
	    }
	    if  ((size > AT_LAP_SIZE) || (size < AT_LAP_HDR_SIZE))
	    {
		/* the packet is too large or small, barf */
		break;
	    }
	    lap_packetp = (at_lap_t *) m->b_rptr;
	    if (lap_packetp->destination == 0)
	    {
		/* can be anything but */
		data_error(EMSGSIZE, m, q);
		break;
	    }
	    if  (!lap_packetp->type)
	    {
		/* since the user has cleared this field, put in the type */
		/* registered on this circuit				  */
		lap_packetp->type = CIRCUIT(q)->type;
	    }
	    lap_packetp->source = lapp->cfg.node;
	    putqHI(lapp->downstreamq, m);
	    break;

	case M_IOCTL:
	    iocbp = (struct iocblk *) m->b_rptr;
	    switch (iocbp->ioc_cmd) 
	    {
	    case AT_SYNC:
		if  (lapp->state == OFFLINE)
		{
		    ioc_ack(0, m, q);
		    break;
		}
		if  (m->b_wptr - m->b_datap->db_lim < sizeof(lap_tags_t))
		{
		    /* there is no room to store the multiplexer tag */
		    ioc_ack(ERANGE, m, q);
		    break;
		}
		LAP_IOCTAGS(m)->circuit = CIRCUIT(q)->circuit;
		putqHI(lapp->downstreamq, m);
		break;
	    case AT_LAP_ONLINE:
		if  (iocbp->ioc_uid != 0)
		{
 		    /* not super user! */
		    ioc_ack(EACCES, m, q);
		    break;
		}
		if  (lapp->state != OFFLINE)
		{
 		    /* hey dummy! the network must be up already! */
		    ioc_ack(lapp->state == HANGING_UP ? EAGAIN : EALREADY, m, q);
		    break;
		}

		/*
		 *	Allocate ddp specific circuits
		 *	Get the queues for ddp comunication
		 *	First ddp level
		 */
		if (!(ddpq = new_queue(ddptab.st_rdinit, ddptab.st_wrinit)))
		{
		    printf("lap_wputq: out of queues\n");
		    ioc_ack(EAGAIN, m, q);
		    break;
		}
		/*
		 * Set up the q->q_ptr so if this upstream queue is activated it's
		 * local data structure is like a real ddp module. Also, socket 0
		 * is used because no one else is. NOTE: that some fields in the 
		 * ddp_specifics field will not be set till ddp open time!
		 */
		ddp_socket[0].ddp = &ddp_specifics;
		ddp_specifics.socket0p = ddp_socket;
		ddpq->q_ptr = WR(ddpq)->q_ptr = (caddr_t) &ddp_socket[0];
		/*
		 * Next, the lap level just like this lap queue.
		 * And link the ddp and lap levels together
		 */
		if (!(lapq = new_queue(RD(q)->q_qinfo, q->q_qinfo)))
		{
		    printf("lap_wputq: out of queues\n");
		    lap_hangup(lapp, 0);
		    ioc_ack(EAGAIN, m, q);
		    break;
		}
		lapq->q_next = ddpq;
		WR(ddpq)->q_next = WR(lapq);
	        ddp_specifics.downstreamq = WR(lapq);
		/*
		 * Indicate the lap queue as the upstream for types 1 and 2.
		 */
		type = 1;
	    	for (circuit=1,circuitp=&lapp->circuit0p[1];
		     circuit<lapp->ncircuits;
		     circuit++,circuitp++
		    )
		    if (circuitp->upstreamq == NULL) {
			circuitp->upstreamq = lapq;
			circuitp->lap = lapp;
			circuitp->circuit = circuit;
			circuitp->type = type;
			lapq->q_ptr = WR(lapq)->q_ptr = (caddr_t) circuitp;
			if (type == 1) {
				strncpy(circuitp->name, "DDP short", 10);
				type = 2;
			} else {
				strncpy(circuitp->name, "DDP long", 9);
				break;
			}
		    }
		if (circuit >= lapp->ncircuits)
		{
		    printf("lap_wputq: out of LAP circuits for DDP\n");
		    lap_hangup(lapp, 0);
		    ioc_ack(EAGAIN, m, q);
		    break;
		}

		
		/*
		 * Get the queue for the transmit scheduling. This is done so it is
		 * possible to close the control queue and still keep the net active.
		 * This queue must be removed at OFFLINE time.
		 */
		if (!(lapq = new_queue(RD(q)->q_qinfo, q->q_qinfo)))
		{
		    printf("lap_wputq: out of queues\n");
		    lap_hangup(lapp, 0);
		    ioc_ack(EAGAIN, m, q);
		    break;
		}
		lapq->q_ptr = WR(lapq)->q_ptr = q->q_ptr; /* borrow this one */
		lapp->downstreamq = WR(lapq);
	
		/*
	 	 *	Allocate a receive buffer - if required
		 */

	    	if (lapp->rcv_msgp == NULL &&
	    	    (lapp->rcv_msgp =
		   allocb(AT_LAP_SIZE + (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE), BPRI_MED))
				!= NULL
			)
	    	{

	    		/*
	    		 *	Leave some space for our DDP friends
	    		 */

	    		lapp->rcv_msgp->b_rptr += (AT_DDP_X_HDR_SIZE - AT_DDP_HDR_SIZE);
	    	}
		init_stats(lapp);
		if (!lapp->cfg.initial_node)
		{
		    lapp->cfg.initial_node = AT_LAP_DEFAULT_NODE;
		}
		if  ((lapp->cfg.node = lapp->cfg.initial_node) > 127)
		{
		    /* then it is a server node and must try longer, bunk! */
		    iocbp->ioc_count = 120;
		}
		else
		{
		    /* just a lowly workstation */
		    iocbp->ioc_count = 20;
		}
		if (!lapp->cfg.rts_attempts)
		{
		    lapp->cfg.rts_attempts = AT_LAP_DEFAULT_RETRIES;
		}
		scc_ctl_write  = lapp->scc->scc_ctl_write;
		SCC_CTL_WRITE(SCC_WR9, lapp->scc->wr9_reset);
		LAP_IOCTAGS(m)->circuit = CIRCUIT(q)->circuit;
		s = splat();
		scc_cmd(at_scc_rvd_init, lapp->scc);
		SCC_CTL_WRITE(SCC_WR6, lapp->cfg.node);
		lapp->xmit_msgp = m;
		adjust_backoff(lapp);
		NEWSTATE(ENQ_WAIT_TdTr);
		access_net(lapp);
		splx(s);
		break;
	    case AT_LAP_OFFLINE:
		if  (iocbp->ioc_uid != 0)
		{
 		    /* not super user! */
		    ioc_ack(EACCES, m, q);
		    break;
		}
		if  (lapp->state == OFFLINE)
		{
 		    /* hey dummy! the network must be up already! */
		    ioc_ack(EALREADY, m, q);
		    break;
		}
		scc_ctl_write  =  lapp->scc->scc_ctl_write;
		s = splat();
		/* reset the channel, and turn the sucker off */
		SCC_CTL_WRITE(SCC_WR9, lapp->scc->wr9_reset);
		splx(s);
		lap_hangup(lapp, 0); /* do not set state hangup will */
		lap_offline(lapp);
		ioc_ack(0, m, q);
		break;
	    case AT_LAP_REGISTER:
		if  (   (iocbp->ioc_count != sizeof(at_lap_entry_t))
		     && (m->b_cont != NULL)
		     && (m->b_cont->b_wptr - m->b_cont->b_rptr
						    != sizeof(at_lap_entry_t))
		    )
		{
		    /* the structure is the wrong size, barf */
		    ioc_ack(EMSGSIZE, m, q);
		    break;
		}
		entryp = (at_lap_entry_t *) m->b_cont->b_rptr;
		type = entryp->type;
		/* first make sure that this type is not registered already! */
		if  (get_upstreamq(type, lapp))
		{
		    /* already registered by someone else */
		    ioc_ack(EEXIST, m, q);
		    break;
		}
		circuitp = CIRCUIT(q);
		circuitp->type = type;
		strncpy(circuitp->name, entryp->name, sizeof(entryp->name));
		entryp->circuitx = circuitp->circuit;
		ioc_ack(0, m, q);
		break;
	    case AT_LAP_LOOKUP:
		if  (m->b_cont != NULL)
		{
		    /* what gives! there is continuation data? */
		    data_error(EMSGSIZE, m, q);
		    break;
		}
		size = sizeof(at_lap_entry_t) * lapp->ncircuits;
		if ((m->b_cont = allocb(size, BPRI_MED)) == NULL)
		{
		    ioc_ack(ENOBUFS, m, q);
		    break;
		}
		for (circuit = 0,
		     circuitp = lapp->circuit0p,
		     entryp = (at_lap_entry_t *) m->b_cont->b_rptr;
		     circuit < lapp->ncircuits;
		     circuit++, circuitp++, entryp++
		    )
		{
		    entryp->type = circuitp->type;
		    strncpy(entryp->name, circuitp->name, sizeof(entryp->name));
		    entryp->circuitx = circuit;
		}
		iocbp->ioc_count = size;
		m->b_cont->b_wptr += size;
		ioc_ack(0, m, q);
		break;
	    case  AT_LAP_GET_CFG:
		if  (m->b_cont != NULL)
		{
		    /* what gives! there is continuation data? */
		    ioc_ack(EMSGSIZE, m, q);
		    break;
		}
		if ((m->b_cont = allocb(sizeof(at_lap_cfg_t), BPRI_MED))
									== NULL)
		{
		    ioc_ack(ENOBUFS, m, q);
		    break;
		}
		/*
		 * if they haven't set the configuration and the network has
		 * not been online, lets initialize these values.
		 */
		if (lapp->state == OFFLINE && !lapp->cfg.rts_attempts)
		{
		    lapp->cfg.rts_attempts = AT_LAP_DEFAULT_RETRIES;
		}
		if (lapp->state == OFFLINE && !lapp->cfg.initial_node)
		{
		    lapp->cfg.initial_node = AT_LAP_DEFAULT_NODE;
		}
		cfgp = ((at_lap_cfg_t *) m->b_cont->b_rptr);
		*cfgp = lapp->cfg;
		cfgp->network_up = lapp->state;
		m->b_cont->b_wptr += sizeof(at_lap_cfg_t);
		iocbp->ioc_count = sizeof(at_lap_cfg_t);
		ioc_ack(0, m, q);
		break;
	    case  AT_LAP_SET_CFG:
		if  (iocbp->ioc_uid != 0)
		{
 		    /* not super user! */
		    ioc_ack(EACCES, m, q);
		    break;
		}
		if  (   (iocbp->ioc_count != sizeof(at_lap_cfg_t))
		     || (m->b_cont == NULL)
		     || (m->b_cont->b_wptr - m->b_cont->b_rptr != sizeof(at_lap_cfg_t))
		    )
		{
		    /* the structure is the wrong size, barf */
		    ioc_ack(EMSGSIZE, m, q);
		    break;
		}
		cfgp = (at_lap_cfg_t *) m->b_cont->b_rptr;
		if  (cfgp->initial_node)
		    if  (cfgp->initial_node < 0xff)
		    {
			lapp->cfg.initial_node = cfgp->initial_node;
		    }
		    else
		    {
			ioc_ack(EINVAL, m, q);
			break;
		    }
		if  (cfgp->rts_attempts)
			lapp->cfg.rts_attempts = cfgp->rts_attempts;
		ioc_ack(0,m,q);
		break;
	    default:
		ioc_ack(ENOTTY,m,q);
		TRACE(T_lap_errors, ("lap_wputq: unknown ioctl %x\n", iocbp->ioc_cmd));
		CIRCUIT(q)->lap->cfg.stats.unknown_mblks++;
		break;
	    }
	    break;
	case M_FLUSH:
	    /*
	     * if flush is for write queue then flush it.  The first
	     * byte of the message contains the flush flags.
	     */
	    if (*m->b_rptr & FLUSHW)
	    {
		flushq(q, FLUSHALL);
		*m->b_rptr &= ~FLUSHW;
	    }
	    if (*m->b_rptr & FLUSHR)
	    {
		if  (lapp->state == OFFLINE)
		{
		    flushq(RD(q), FLUSHALL);
		    qreply(q, m);
		}
		else
		{
		    /* pass the flush down just like a sync */
		    if  (m->b_wptr - m->b_datap->db_lim < sizeof(lap_tags_t))
		    {
			/* there is no room to store the multiplexer tag */
			data_error(ERANGE, m, q);
			break;
		    }
		    LAP_IOCTAGS(m)->circuit = CIRCUIT(q)->circuit;
		    putqHI(lapp->downstreamq, m);
		}
	    }
	    else
	    {
		freemsg(m);
	    }
	    break;
	default:
	    TRACE(T_lap_errors, ("lap_wputq: unknown stream message %x\n",
				  m->b_datap->db_type
				)
		 );
	    CIRCUIT(q)->lap->cfg.stats.unknown_mblks++;
	    freemsg(m);
	    break;
	} /* of switch (m->b_datap->db_type) */
}

/*
 *	The write service routine should only be active on the multiplexed 
 *	queue. The demultiplexed write queues should be empty since the lap_put
 *	procedure places items directly onto the multiplexed queue.
 */
lap_wsrvc(q)
queue_t *q;
{
	register lap_specifics_t *lapp = CIRCUIT(q)->lap;
	int		s;

	TRACE(T_lap, ("lap_wsrvc: q %d\n", q-queue));
#ifdef	DEBUG
	/*
	 * if this is a demultiplexed queue, then there is something wrong since
	 * it should never be serviced because it should be empty!
	 */
	if (q != lapp->downstreamq)
	{
	    printf("lap_wsrvc: should not be active on circuit other than 0\n");
	    flushq(q, FLUSHALL);
	    return;
	}
#endif	DEBUG
	s = splat();
	if ( lapp->state == ONLINE && lapp->xmit_msgp == NULL)
	{
	    /* interrupt level not active, so prime it */
	    schedule(lapp);
	}
	splx(s);
	return;
}

/*
 *	bring the network down, free xmit and rcv msgs, and dealloc the schedule queue
 */

lap_offline(lapp)
register lap_specifics_t *lapp;

{
	if  (lapp->xmit_msgp != NULL)
	{
	    freemsg(lapp->xmit_msgp);
	    lapp->xmit_msgp = NULL;
	}
	if  (lapp->rcv_msgp != NULL)
	{
	    freemsg(lapp->rcv_msgp);
	    lapp->rcv_msgp = NULL;
	}
	flushq(lapp->downstreamq, FLUSHALL);
	flushq(RD(lapp->downstreamq), FLUSHALL);  /* just in case */
	freeq(RD(lapp->downstreamq));
	lapp->downstreamq = NULL;
}



/*
 * well lap has brought the network down, dup and send it upstream to 
 * everyone who is open.
 */

lap_hangup(lapp, time)
register lap_specifics_t *lapp;
{

	register u8		circuit;
	register u8		ncircuits;
	register lap_circuit_t	*circuitp;
	mblk_t			*m;
	mblk_t			*m2;
	queue_t			*upstreamq;

	NEWSTATE(HANGING_UP);
	ddp_specifics.downstreamq = NULL; /* let the ddp layer know */
        ddp_specifics.cfg.network_up = 0;
	if (!(m = allocb(0, BPRI_MED)))
	{
	    /* we are out of buffers, wait for more */
	    timeout(lap_hangup, lapp, HZ/4);
	    return;
	}
	m->b_datap->db_type = M_HANGUP;
	/* send hangups on all the circuits except the control (0) */
	for (circuit = 1, circuitp = &lapp->circuit0p[1], ncircuits = lapp->ncircuits;
	     circuit < ncircuits;
	     circuit++, circuitp++
	    )
	{
	    if (upstreamq = circuitp->upstreamq)
	    {
		/* there is an upstream! */
		if (!(m2 = dupb(m)))
		{
		    /* we are out of buffers, wait for more */
		    timeout(lap_hangup, lapp, HZ/4);
		    break;
		}
		if (circuitp->type == 1) {
			if (upstreamq->q_next)
			{
			    putnext(upstreamq, m2);
			    upstreamq->q_next = NULL;
			}
			circuitp->upstreamq = NULL;
			circuitp->type = 0;
			flushq(upstreamq, FLUSHALL);
			freeq(upstreamq);	/* get rid of lap queue for ddp */
		} else
		if (circuitp->type == 2) {
			circuitp->upstreamq = NULL;
			circuitp->type = 0;
			freemsg(m2);		/* already sent it to ddp 1 */
		} else
		    (*upstreamq->q_qinfo->qi_putp) (upstreamq, m);
		
	    }
	}
	freemsg(m);
	NEWSTATE(OFFLINE);
	return;
}
