#ifndef lint	/* .../appletalk/fwd/fwdicp/fwdicplib.c */
#define _AC_NAME fwdicplib_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:09:21}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of fwdicplib.c on 87/11/11 21:09:21";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#ifndef	FEP		/* UNIX side */
#define VERIFY				/* some 1current hardware fails unless */
#endif	FEP		/* UNIX side */
/*	@(#)fwdicplib.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *      Forwarder driver for communication with a Front End Processor.
 *	This forwarder is for communications with a peer to peer, or otherwise
 *	master to master relationship BUT with no read modified write (TAS).
 *      This is the unix side, there is an identical pair for the
 *      FEP side. Each side is divided up into two parts, a generic which is
 *      common to all FEPs and a component which is specific to a particular
 *      type of FEP. The specific component described here is for an AST
 *	4 port serial card and is further devided into two parts; that 
 *	which can talk to any ACP in the system (follows below), and the part
 *	which can onlytalk to a particular ACP. There must be one of the 
 *	latter for every card installed. The routines below can talk to any
 *	card, because they have been written so the board information 
 *	structure, icp_info is passed to these functions via the q local 
 *	pointer q->q_ptr->fep->ginfop.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/reg.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/debug.h>
#ifdef	DEBUG
extern	int	T_ipc;
extern	int	T_fwd;
#endif	DEBUG
#include <sys/signal.h>
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
#include <fwd.h>
#include <via.h>
#include <fwdicp.h>

#ifndef NULL
#define NULL 0
#endif NULL

static fwd_rcv_timer();
static mblk_t *ip_allocmsg();
#ifndef	FEP		/* UNIX side */
void fwdicp_int();
#else	FEP		/* FEP side  */
void fwdicpirupt();
#endif

#ifndef	FEP		/* UNIX side */
extern fep_specifics_t	*icp_boards[];
#else	FEP		/* FEP side  */
extern fep_specifics_t	icp_specifics;
extern	mblk_t		*qopenclose_h;
extern	mblk_t		*qopenclose_t;
#endif	FEP

/************************************************************************/
/*									*/
/*  This section deals with the accessing of the fep making use of the  */
/*  stream structures of the fep. 					*/
/*									*/
/************************************************************************/

/*
 *	other_to_me converts the "ptr" address expressed relative to the fep's
 *	address space to unix's side and returns it.
 */

static caddr_t
other_to_me(ptr, infop)
caddr_t		ptr;
icp_info_t 	*infop;

{
#ifndef FEP	/* UNIX side */
	/*
	 * then we are unix side and want to convert fep's address space
	 * to unix's address space
	 */
	if ((ptr >= (caddr_t)FEP_RAM_START) && (ptr <= (caddr_t)FEP_RAM_END))
	{
	    return(ptr+(u_int)infop->ramp);
	}
	else
	    /* illegal address */
	    return(NULL);
#else		/* FEP */
	/*
	 * then we are fep side and want to convert unix's address space
	 * to the fep's address space, which includes setting up nbus page
	 * register (A31-A24) and the via register (A23-A22).
	 * ptr | 0x80000000, put a23 24 in via, upper byte inverted in page
	 *
	 * SIDE EFFECTS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 * NOTE: this is a DANGEROUS routine, it sets up registers that might
	 * be changed by another invocation of this routine. i.e. you may get
	 * an address back that is good at that time, but if you call this
	 * routine again, or the interrupt level slips in, the address might
	 * NO LONGER be valid!!!
	 */

	register int	tmp;

	tmp = ((uint) ptr) >> 16;
	vias[0]->ora_ira = (tmp & 0xc0) | 2;	/* donot forget reset* */
	tmp >>= 8;
	* (char *) PAGE_REGISTER = (char) ~tmp;
	/* turn on nbus, off bus lock */
	return((caddr_t) (((uint) ptr & 0x3fffff) | 0x00800000));
#endif  /* FEP */
}


#ifdef  FEP	/* FEP side */
#define	CHECK_PAGE_FAULT(START, LENGTH) \
		(int) ((((int) START & 0x003fffff) + LENGTH) - 0x00400000)
#endif  /* FEP side */


/************************************************************************/
/*									*/
/*			Inter Processor Communications			*/
/*									*/
/************************************************************************/

/*
 * put_ipc puts an inter processor command on the others queue. Note that
 * the code changes depending which side it is.
 */

static int
put_ipc(ipc, infop, where)
ipc_t		*ipc;
icp_info_t 	*infop;

{
	register ipc_ptrs_t	*ipc_qp;
	ipc_t			*keepers_wp;
	ipc_t			*wp;
	int			s;
#ifndef FEP	/* UNIX side */
	switch(where) {
	case IPC_REP:
		ipc_qp = &(infop->accessp->rep2fep);
		break;
	case IPC_CMD_NORM:
		ipc_qp = &(infop->accessp->cmd2fep);
		break;
	case IPC_CMD_EXP:
		ipc_qp = &(infop->accessp->exp2fep);
		break;
	}
#else		/* FEP side */
	switch(where) {
	case IPC_REP:
		ipc_qp = &(infop->accessp->rep2unix);
		break;
	case IPC_CMD_NORM:
		ipc_qp = &(infop->accessp->cmd2unix);
		break;
	case IPC_CMD_EXP:
		ipc_qp = &(infop->accessp->exp2unix);
		break;
	}
#endif
	/* first we see that the queue is not filled. This is indicated by */
	/* the wp pointing to an element whose cmd is not zero (0). 	   */
	/* lock out interrupts to protect us from ourselves!		   */
	s = splipc();
#ifndef FEP	/* UNIX side */
	keepers_wp = (ipc_t *) read_int_by_short(ipc_qp->wp);
	wp = (ipc_t *) other_to_me(keepers_wp, infop);
#else	FEP	/* FEP side  */
	/*
	 * NOTE, do not convert to others address space since the FEP is the 
	 * keeper of the pointers.
	 */
	wp = keepers_wp = (ipc_t *) read_int_by_short(ipc_qp->wp);
#endif	FEP
#ifdef  DEBUG
	if (wp == NULL)
	{
	    splx(s);
	    printf("put_ipc_q: ipc write pointer not converted to our space\n");
	    return(0);
	}
#endif	DEBUG
	if  (wp->cmd != 0)
	{
	    /* filled up! */
	    splx(s);
	    printf("put_ipc_q: cmd != 0, wp = %x, cmd %x, id = %x, arg = %x\n",
						wp, wp->cmd, wp->id, wp->arg);
	    return(0);
	}
	wp->id = ipc->id;
	wp->circuit = ipc->circuit;
	write_int_by_short(wp->arg, ipc->arg);
	wp->cmd = ipc->cmd;		/* NOTE: this must be last */
	if ((++keepers_wp) >= (ipc_t *) read_int_by_short(ipc_qp->end))
	    keepers_wp = (ipc_t *) read_int_by_short(ipc_qp->start);
	write_int_by_short(ipc_qp->wp, keepers_wp);
	splx(s);
	TRACE(T_ipc, ("put_ipc: cmd = 0x%x, circuit = 0x%x, arg = 0x%x\n",
			ipc->cmd, ipc->circuit, ipc->arg));
	return(1);
}

static
fwd_rcv_timer(dev)
{
	struct args a;
#ifndef	FEP		/* UNIX side */
	icp_info_t 	*infop = (icp_info_t *) (icp_boards[dev]->ginfop);
#else	FEP		/* FEP side  */
	icp_info_t 	*infop = (icp_info_t *) (icp_specifics.ginfop);
#endif	FEP

	if (infop->rcvtimer) {
		infop->rcvtimer = 0;
		a.a_dev = dev;
#ifndef	FEP		/* UNIX side */
		fwdicp_int(&a);
#else	FEP		/* FEP side  */
		fwdicpirupt(&a);
#endif
	}
}

/*
 * get_ipc gets an inter processor command on the others queue. Note that
 * the code changes depending which side it is. Called at splipc().
 */

static mblk_t *
get_ipc(ipc, infop, dev)
ipc_t		*ipc;
icp_info_t 	*infop;
{
	register ipc_ptrs_t	*ipc_qp;
	register ipc_t		*rp;
	register ipc_t		*keepers_rp;
	mblk_t			*m;
	int			ind;
	register int 		s;

	ind = 3;
	for (;;) {	
#ifndef FEP	/* UNIX side */
		switch (ind) {
		case 3:
			ipc_qp = &(infop->accessp->rep2unix);
			break;
		case 2:
			ipc_qp = &(infop->accessp->exp2unix);
			break;
		case 1:
			ipc_qp = &(infop->accessp->cmd2unix);
			break;
		case 0:
	    		return(NULL);
		}
#else	FEP	/* FEP side */
		switch (ind) {
		case 3:
			ipc_qp = &(infop->accessp->rep2fep);
			break;
		case 2:
			ipc_qp = &(infop->accessp->exp2fep);
			break;
		case 1:
			ipc_qp = &(infop->accessp->cmd2fep);
			break;
		case 0:
	    		return(NULL);
		}
#endif	FEP

		/* first see if there is an element, then read it, then update the */
		/* read pointer							   */

#ifndef FEP	/* UNIX side */
		keepers_rp = (ipc_t *) read_int_by_short(ipc_qp->rp);
		rp = (ipc_t *) other_to_me(keepers_rp, infop);
#else	FEP	/* FEP side */

		/*
		 * NOTE, do not convert to others address space since the FEP is the 
		 * keeper of the pointers.
		 */

		rp = keepers_rp = (ipc_t *) read_int_by_short(ipc_qp->rp);
#endif	FEP
#ifdef  DEBUG
		if (rp == NULL) {
		    printf("get_ipc_q: ipc read pointer not converted to our space\n");
		    return(NULL);
		}
#endif	DEBUG
		if  (rp->cmd == 0) {
			ind--;
			continue;
		}
		break;
	}
	ipc->arg = read_int_by_short(rp->arg);
	ipc->cmd = rp->cmd;
	switch(ipc->cmd) {
	case IPC_TAKE_MESSAGE:
	case IPC_TAKE_MESS_FLOW:
		if ((m = ip_allocmsg((mblk_t *)(ipc->arg), infop)) == NULL) {
			s = splclk();
			if (infop->rcvtimer == 0) {
				timeout(fwd_rcv_timer, dev, HZ/8);
				infop->rcvtimer = 1;
			}
			splx(s);
			return(NULL);
		}
		break;
	default:
		m = (mblk_t *)1;
	}
	ipc->id = rp->id;
	ipc->circuit = rp->circuit;
	rp->cmd = 0;		/* now we are done with the element */
	if ((++keepers_rp) >= (ipc_t *) read_int_by_short(ipc_qp->end))
	    keepers_rp = (ipc_t *) read_int_by_short(ipc_qp->start);
	write_int_by_short(ipc_qp->rp, keepers_rp);
	TRACE(T_ipc, ("get_ipc: cmd = 0x%x, circuit = 0x%x, arg = 0x%x\n",
			ipc->cmd, ipc->circuit, ipc->arg));
	return(m);
}


/*
 * we only need copy in since each side only copies in one direction so that
 * side can allocate it own storage.
 * NOTE: an EVEN starting address is assumed!
 */

static int
ip_copy_in(from, to, size, infop)
caddr_t		from;
caddr_t		to;
int		size;
icp_info_t 	*infop;

#ifndef FEP	/* UNIX side */
{
	int	i;

	if ((int)from & 1 || size & 1)
	{
	    printf("ip_copy_in: not even from = %x, size = %x\n", from, size);
	    return(0);
	}
	if ((from = other_to_me(from, infop)) == NULL)
	{
	    printf("ip_copy_in: bad from: %x\n", from);
	    return(0);
	}
	for (i = size; i > 0; i-=2, from+=2, to+=2)
	    * (ushort *) to = * (ushort *) from;
}
#else		/* FEP side */
{
	/* ??? might be speeded up by long writes, 68000 converts to shorts */
	int	over;
	int	i;
	int	s;
	caddr_t from1;

	if ((int)from & 1 || size & 1)
	{
	    printf("ip_copy_in: not even from = %x, size = %x\n", from, size);
	    return(0);
	}
	if  ((over = CHECK_PAGE_FAULT(from, size)) > 0)
	{
	    size -= over;
	    from1 = from + size;
	}
	if ((from = other_to_me(from, infop)) == NULL)
	{
	    printf("ip_copy_in: bad from: %x\n", from);
	    return(0);
	}
	for (i = size; i > 0; i-=2, from+=2, to+=2)
		*(ushort *)to = *(ushort *)from;
	if (over > 0)
	{
	    if ((from = other_to_me(from1, infop)) == NULL)
	    {
		printf("ip_copy_in: bad from: %x\n", from);
		return(0);
	    }
	    for (i = over; i > 0; i-=2, from+=2, to+=2)
		* (ushort *) to = * (ushort *) from;
	}
	return(1);
}
#endif


/*
 * Copies data from message block to newly allocated message block and
 * data block.  (dupb(), in contrast, shares the data block.)
 * The new message block pointer is returned if successful, NULL
 * if not.
 */

static int
ip_copyb(bp, infop, nbp)
register mblk_t *bp;
icp_info_t 	*infop;
register mblk_t *nbp;
{
	register mblk_t		*sbp;
	register dblk_t		*dp, *ndp;
	register int		size;
	register caddr_t	base;
	register caddr_t	rptr;
	register caddr_t	wptr;

#ifdef FEP	/* FEP side */
	sbp = bp;
#endif  FEP
	/*
	 *	NOTE: other_to_me does not need protection here since this copy
	 *	only happens at the INTERRUPT level.
	 */
	bp = (mblk_t *) other_to_me(bp, infop);
#ifdef	FEP	/* FEP side */
	if  (CHECK_PAGE_FAULT(bp, sizeof(mblk_t)) > 0)
	{
		printf("ip_copyb: pagefault in mblk_t at %x\n", bp);
		return(0);
	}
#endif  FEP
	dp = (dblk_t *) read_int_by_short(bp->b_datap);
#ifdef	FEP	/* FEP side */
	if  (CHECK_PAGE_FAULT(dp, sizeof(dblk_t)) > 0)
	{
		printf("ip_copyb: pagefault in dblk_t at %x\n", dp);
		return(0);
	}
#endif
	dp = (dblk_t *) other_to_me(dp, infop);
	size = read_int_by_short(dp->db_lim);
	size -= (int) (base = (caddr_t) read_int_by_short(dp->db_base));
	ndp = nbp->b_datap;
	ndp->db_type = dp->db_type;
#ifdef	FEP
	bp = (mblk_t *) other_to_me(sbp, infop);	/* restore registers */
#endif	FEP
	rptr = (caddr_t) read_int_by_short(bp->b_rptr);
	wptr = (caddr_t) read_int_by_short(bp->b_wptr);
	nbp->b_rptr = ndp->db_base + (rptr - base);
	nbp->b_wptr = ndp->db_base + (wptr - base);
	TRACE(T_ipc, ("copyb: nbp=0x%x ndp=0x%x lim = 0x%x base = 0x%x\n", nbp, ndp, ndp->db_lim, ndp->db_base));
	TRACE(T_ipc, ("copyb: rptr = 0x%x wptr = 0x%x base = 0x%x\n", rptr, wptr, base));
#ifdef	DEBUG
	if  (nbp->b_rptr > ndp->db_lim || nbp->b_wptr > ndp->db_lim) {
	    printf("ip_copyb: pointers exceed dblim %x, rptr %x, wptr %x\n",
		    ndp->db_lim, nbp->b_rptr, nbp->b_wptr
		  );
	    return(0);
	}
#endif
	ip_copy_in(base, ndp->db_base, size, infop);
	return(1);
}


/*
 * Copies data from a message to a newly allocated message by 
 * copying each block in the message (uses copyb, not dupb).
 * A pointer to the new message is returned if successful,
 * NULL if not.
 */

static mblk_t
*ip_copymsg(bp, infop, head)
register mblk_t *bp;
icp_info_t 	*infop;
register mblk_t *head;
{
	register mblk_t 	*nbp, *temp;
	register int		s;

	if (ip_copyb(bp, infop, head) == 0) {
		freemsg(head);
		return(NULL);
	}
	nbp = head->b_cont;
	while (nbp) {
		bp= (mblk_t*)read_int_by_short(((mblk_t*)other_to_me(bp,infop))->b_cont);
	    	if (ip_copyb(bp, infop, nbp) == 0) {
			freemsg(head);
			return(NULL);
	    	}
	    	nbp = nbp->b_cont;
	}
	return(head);
}

/*
 * allocs space required for a message
 */

static mblk_t
*ip_allocb(bp, infop)
register mblk_t *bp;
icp_info_t 	*infop;
{
	register dblk_t		*dp;
	register int		size;

	/*
	 *	NOTE: other_to_me does not need protection here since this copy
	 *	only happens at the INTERRUPT level.
	 */
	bp = (mblk_t *) other_to_me(bp, infop);
#ifdef	FEP	/* FEP side */
	if  (CHECK_PAGE_FAULT(bp, sizeof(mblk_t)) > 0)
	{
		printf("ip_allocb: pagefault in mblk_t at %x\n", bp);
		return(NULL);
	}
#endif  FEP
	dp = (dblk_t *) read_int_by_short(bp->b_datap);
#ifdef	FEP	/* FEP side */
	if  (CHECK_PAGE_FAULT(dp, sizeof(dblk_t)) > 0)
	{
		printf("ip_allocb: pagefault in dblk_t at %x\n", dp);
		return(NULL);
	}
#endif
	dp = (dblk_t *) other_to_me(dp, infop);
	size = read_int_by_short(dp->db_lim) - read_int_by_short(dp->db_base);
	return(allocb(size, BPRI_MED));
}


/*
 * Allocs space for a message
 */

static mblk_t
*ip_allocmsg(bp, infop)
register mblk_t *bp;
icp_info_t 	*infop;
{
	register mblk_t		*head = NULL;
	register mblk_t 	*nbp, *temp;

#ifdef DEBUG
	if (bp == NULL)
		panic("ip_allocmsg: null message");
#endif DEBUG
	if ((nbp = head = ip_allocb(bp, infop)) == NULL) {
TRACE(T_fwd, ("ip_allocmsg: fail 1\n"));
		return(NULL);
	}

	while (temp =
	    (mblk_t *)read_int_by_short(((mblk_t *)other_to_me(bp,infop))->b_cont)){
		if ((nbp->b_cont = ip_allocb(temp, infop)) == NULL) {
			freemsg(head);
TRACE(T_fwd, ("ip_allocmsg: fail 2\n"));
			return(NULL);
	    	}
	    	nbp = nbp->b_cont;
	    	bp = temp;
	}
	return(head);
}



#ifndef	FEP		/* UNIX side */
void fwdicp_int(ap)
#else	FEP		/* FEP side  */
void fwdicpirupt(ap)
#endif
struct args *ap;

{
	register int	s;
	ipc_t		ipc;
	int		prod_other = 0;
	via_chip_t	*viap;
	mblk_t		*mp;
#ifndef	FEP		/* UNIX side */
	fep_specifics_t	*fep_specifics = icp_boards[ap->a_dev];
#else	FEP		/* FEP side  */
	fep_specifics_t	*fep_specifics = &icp_specifics;
#endif	FEP
	icp_info_t 	*infop = (icp_info_t *) (fep_specifics->ginfop);
	queue_t		*upstreamq;
	short		circuit;
	fwd_circuit_t	*circuitp;
	int		i;
	

	/*
	 *	Turn off the interrupt (this makes the hardware timing hole smaller
	 *		but doesn't close it)
	 */

#ifndef FEP	/* UNIX side */
	viap = (via_chip_t *)(VIA_ADDRESS+infop->ramp);
	viap->pcr = IRUPT_UNIX_N | (viap->pcr & 0x1f);
#else		/* FEP side */
	viap = vias[0];
	viap->pcr = (viap->pcr & 0xf1) | IRUPT_FEP_N;
#endif

	if (infop->rcvtimer) {
		infop->rcvtimer = 0;
		untimeout(fwd_rcv_timer, ap->a_dev);
	}

	/*
	 * process the command from the other processor
	 */

	for  (;;) {
		if ((mp = get_ipc(&ipc, infop, ap->a_dev)) == NULL) {
TRACE(T_fwd, ("get_ipc failed\n"));
			break;
		}
		circuitp = &fep_specifics->fwd_circuit[ipc.circuit];
		switch (ipc.cmd) {
		case IPC_TAKE_MESSAGE:
		case IPC_TAKE_MESS_FLOW:
			if  ((mp = ip_copymsg((mblk_t *) ipc.arg, infop, mp)) == NULL) {
				printf("nbusirupt: could not take message\n");
				ipc.cmd = (ipc.cmd == IPC_TAKE_MESSAGE ?
					IPC_COULD_NOT_TAKE_MESSAGE :
					IPC_COULD_NOT_TAKE_FLOW);
				if  (!put_ipc(&ipc, infop, IPC_REP)) {
			    		printf("nbusirupt: could not send take error\n");
				} else {
			    		prod_other++;
				}
		    	} else {
	    			circuit = ipc.circuit;
				TRACE(T_fwd, ("icpirupt: mp = 0x%x, class = 0x%x, size = 0x%x,  circuit = 0x%x\n",
				      mp,
				      mp->b_datap->db_lim-mp->b_datap->db_base,
				      msgdsize(mp),
				      circuit ));
				if (((upstreamq = circuitp->upstreamq) == NULL)
#ifdef	FEP		/* FEP side */
			    /* because there is no upstream on the first open */
			    		&&  ( mp->b_datap->db_type != M_FWD_1stOPEN)
			    		&&  ( mp->b_datap->db_type != M_FWD_CLOSE)
			    		&&  ( mp->b_datap->db_type != M_FWD_CANCEL)
#endif	FEP
			   							) {
			 		/*
			    		 * there is not an upstream! either it has gone 
			    		 * away, or the other side is confused
			    		 */
#ifndef	FEP		/* Unix side */
			    		if (circuitp->state != CIRC_CLOSED)
#endif	FEP		/* Unix side */
			    			printf("nbusirupt: no upstream for circuit %d\n",
								    circuit);
			    		freemsg(mp);
			    		ipc.cmd = (ipc.cmd == IPC_TAKE_MESSAGE ?
						   IPC_COULD_NOT_TAKE_MESSAGE :
						   IPC_COULD_NOT_TAKE_FLOW);
			    		if  (!put_ipc(&ipc, infop, IPC_REP)) {
						printf("nbusirupt: could not send not take\n");
			    		}
			    		prod_other++;
			    		break;
				}
				if (ipc.cmd == IPC_TAKE_MESSAGE) {
					ipc.cmd = IPC_GOT_MESSAGE;
					if  (!put_ipc(&ipc, infop, IPC_REP)) {

			    			/*
						 * if cannot respond then do not do cmd
						 */

			    			printf("nbusirupt: could not send got msg\n");
			    			freemsg(mp);
			    			break;
					}
					prod_other++;
				}
				switch (mp->b_datap->db_type) {
#ifndef	FEP	/* UNIX side */
				case M_FWD_ACK:
			    		{
			    			/*
			    			 * wakeup the open/close that is waiting
						 * for an ack from the other end and
						 * return the result of the open.
			    			 */

			    			int id;

			    			id = FWD_ACKTAGS(mp)->id;
			    			circuitp->errno[id] =
								FWD_ACKTAGS(mp)->errno;
			    			circuitp->setpgrp[id] =
								FWD_ACKTAGS(mp)->setpgrp;
			    			circuitp->idstate[id] =
								FWD_ACKTAGS(mp)->sflag;
			    			circuitp->dev[id] = FWD_ACKTAGS(mp)->dev;
						circuitp->minpsz=FWD_ACKTAGS(mp)->minpsz;
						circuitp->maxpsz=FWD_ACKTAGS(mp)->maxpsz;
						if (circuitp->rname[id]) {
							strncpy (circuitp->rname[id],
			   				    FWD_ACKTAGS(mp)->name,
		    				   	    sizeof(FWD_ACKTAGS(mp)->name)
							    );
							circuitp->rname[id] = NULL;
						}
			    			wakeup((caddr_t) &circuitp->idstate[id]);
			    			freemsg(mp);
			    		}
			    		break;

				case M_FWD_PRINTF:
#ifdef FWDPRINTF
			    		fwdprintf(mp);
#endif FWDPRINTF
			    		freemsg(mp);
			    		break;
#else	FEP	/* FEP side  */
				case M_FWD_OPEN:
				case M_FWD_1stOPEN:
				case M_FWD_CLOSE:
				case M_FWD_PUSH:
				case M_FWD_POP:
				case M_FWD_CANCEL:
				case M_FWD_LOOK:
				case M_FWD_FIND:
				case M_FWD_MNAME:
	
					/*
					 * activate the open, at 0 priority, from stream
					 * head
					 */

			    		if  (qopenclose_h) {
						qopenclose_t->b_next = mp;
						qopenclose_t = mp;
			    		} else {
						qopenclose_h = qopenclose_t = mp;
			    		}
			    		mp->b_next = NULL;
			    		break;
#endif	FEP
				case M_FLUSH:

			    		/*
			    		 * if flush is for this (read/write) queue then
			    		 * flush it.  The first byte of the message
					 * contains the flush flags.
			    		 */
#ifndef	FEP	/* UNIX side */
			    		if (*mp->b_rptr & FLUSHR)
#else	FEP	/* FEP side */
			    		if (*mp->b_rptr & FLUSHW)
#endif	FEP
						flushq(upstreamq, FLUSHALL);
			    		putnext(upstreamq, mp);
			    		break;
				default:
			    		/*
			     		 * demultiplex and send to proper q. First find
			    		 * out if it can be put on the next queues next
			    		 * queue. Otherwise, no canput on the next q,
			    		 * it would stop all traffic!
			    		 */

			    		if (upstreamq->q_count == 0 &&
					    canput(upstreamq->q_next)) {
						putnext(upstreamq, mp);
			    		} else {/* no room on the next q next q! */
						putq(upstreamq, mp);
					}
				} /* switch (mp->b_datap->db_type) */
				if (ipc.cmd == IPC_TAKE_MESS_FLOW) {
					if (upstreamq->q_count == 0 &&
					    canput(upstreamq->q_next)) {
						ipc.cmd = IPC_GOT_MESSAGE_RUN;
					} else {
			    	        	CIRCUIT(upstreamq)->wait++;
						ipc.cmd = IPC_GOT_MESSAGE_STOP;
					}
					if  (!put_ipc(&ipc, infop, IPC_REP)) {
			    			printf("nbusirupt: could not send got msg\n");
			    			break;
					}
					prod_other++;
				}
		    	}
		    	break;
		case IPC_GOT_RUN:
			circuitp->halt -= ipc.arg;
			if (circuitp->halt == 0 && circuitp->upstreamq) {

				/*
				 * Note: upstreamq here actually points to the
				 *	 downstream Q
				 */

				upstreamq = OTHERQ(circuitp->upstreamq);
				s = splstr();
				if (!upstreamq->q_count)
					upstreamq->q_flag &= ~QFULL;
				upstreamq->q_hiwat = 1;
				upstreamq->q_lowat = 0;
				splx(s);
				qenable(upstreamq);
		    	}
			ipc.cmd = IPC_GOT_RUN_ACK;
			if  (!put_ipc(&ipc, infop, IPC_REP)) {
		  		printf("nbusirupt: could not send got msg\n");
				break;
			}
			prod_other++;
		    	break;

		case IPC_GOT_MESSAGE_RUN:
			if (--circuitp->halt == 0 && circuitp->upstreamq) {

				/*
				 * Note: upstreamq here actually points to the
				 *	 downstream Q
				 */

				upstreamq = OTHERQ(circuitp->upstreamq);
				s = splstr();
				if (!upstreamq->q_count)
					upstreamq->q_flag &= ~QFULL;
				upstreamq->q_hiwat = 1;
				upstreamq->q_lowat = 0;
				splx(s);
				qenable(upstreamq);
		    	}
		    	goto got1;
		case IPC_COULD_NOT_TAKE_FLOW:
			if (--circuitp->halt == 0 && circuitp->upstreamq) {

				/*
				 * Note: upstreamq here actually points to the
				 *	 downstream Q
				 */

				upstreamq = OTHERQ(circuitp->upstreamq);
				s = splstr();
				if (!upstreamq->q_count)
					upstreamq->q_flag &= ~QFULL;
				upstreamq->q_hiwat = 1;
				upstreamq->q_lowat = 0;
				splx(s);
				qenable(circuitp->upstreamq);
		    	}
		case IPC_COULD_NOT_TAKE_MESSAGE:
		    	printf("nbusirupt: other side could not take message\n");
		    	/* fall through */

		case IPC_GOT_MESSAGE_STOP:
		case IPC_GOT_MESSAGE:
		got1:
		    	freemsg((mblk_t *) ipc.arg);
			/* fall through */

		case IPC_GOT_RUN_ACK:
			if (infop->count++) {
				if (infop->sleep) {
					wakeup(&infop->sleep);
					infop->sleep = 0;
				} else {
					circuitp = &fep_specifics->fwd_circuit[0];
					for (i = 0;
					     i < fep_specifics->ncircuits;
					     circuitp++, i++)
					if (circuitp->up_wait) {
						if (circuitp->upstreamq)
							qenable(circuitp->upstreamq);
						circuitp->up_wait = 0;
					} else 
					if (circuitp->down_wait) {
						if (circuitp->upstreamq)
							qenable(OTHERQ(circuitp->upstreamq));
						circuitp->down_wait = 0;
					} 
				}
			}
		    	break;
		default:
		    	printf("nbusirupt: unknown ipc command\n");
		    	break;
	    	}  /* of switch (ipc.cmd) */
	}

	/*
	 * turn off your interrupt now
	 */

#ifndef FEP	/* UNIX side */
	if  (prod_other) {

	    	/*
		 * and turn on the others interrupt
		 */

	    viap->pcr = IRUPT_UNIX_N | IRUPT_FEP;
	}
#else		/* FEP side */
	viap = vias[0];
	if  (prod_other) {

	    	/*
		 * and turn on the others interrupt
		 */

	    viap->pcr = IRUPT_UNIX | IRUPT_FEP_N;
	}
#endif
}


/************************************************************************/
/*									*/
/* These are the specific routines called from the generic fwd.c.	*/
/*									*/
/************************************************************************/

icpputq(mp, circuitp, flow, sleeptype)
register mblk_t *mp;
fwd_circuit_t	*circuitp;
int		flow;		/* true if want flow control return */
int 		sleeptype;	/* true if we can sleep */
{
 	register icp_info_t	*infop = (icp_info_t *) circuitp->fep->ginfop;
 	register access_t	*accessp;
	via_chip_t		*viap;
	ipc_t			ipc;
	register int		s;

	if ((accessp = infop->accessp) == NULL) {
	    printf("icpputq: attempt to access an unconfigured board\n");
	    return(ENODEV);
	}
#ifndef	FEP	/* unix side */
	if (infop->status != FEP_READY) {
	    printf("icpputq: %x board not OK, status = %x\n",
		    accessp, infop->status);
	    return(EIO);
	}
#endif	FEP
	if (accessp->status != FEP_READY) {
	    printf("icpputq: %x board considers itself not OK, status = %x\n",
		    accessp, accessp->status);
	    return(EIO);
	}
	s = splipc();
	while (infop->count == 0) {
		if (sleeptype == 2) {
			infop->sleep = 1;
			while (infop->sleep) 
			if (sleep(&infop->sleep, STOPRI|PCATCH))
				return(EINTR);
		} else {
			if (sleeptype == 0) {
				circuitp->down_wait = 1;
			} else {
				circuitp->up_wait = 1;
			}
			splx(s);
			return(ENOSPC);
		}
	}
	infop->count--;
	ipc.cmd = (flow ? IPC_TAKE_MESS_FLOW : IPC_TAKE_MESSAGE);
	ipc.id = infop->accessp->next_id++;
	ipc.circuit = circuitp->circuit;
	ipc.arg = (uint) mp;
	if (!put_ipc(&ipc, infop,
		(mp->b_datap->db_type >= QPCTL?IPC_CMD_EXP:IPC_CMD_NORM))) {
	    panic("icpputq: outbound queue is filled\n");
	}
	splx(s);
	/* turn on others interrupt now, and leave yours alone */
#ifndef FEP	/* UNIX side */
	viap = (via_chip_t *) (VIA_ADDRESS+infop->ramp);
	viap->pcr = (viap->pcr & 0xf1) | IRUPT_FEP;
#else		/* FEP side */
	viap = vias[0];
	viap->pcr = IRUPT_UNIX | (viap->pcr & 0x1f);
#endif
	TRACE(T_fwd, ("icpputq:  mp = 0x%x, class = 0x%x, size = 0x%x, circuit = 0x%x\n",
		      mp, mp->b_datap->db_lim - mp->b_datap->db_base,
		      msgdsize(mp), circuitp->circuit ));
	return(0);
}



icpflow(circuitp, sleeptype)
fwd_circuit_t	*circuitp;
int sleeptype;
{
 	register icp_info_t	*infop = (icp_info_t *) circuitp->fep->ginfop;
 	register access_t	*accessp;
	via_chip_t		*viap;
	ipc_t			ipc;
	register int		s;

	if ((accessp = infop->accessp) == NULL) {
	    printf("icpflow: attempt to access an unconfigured board\n");
	    return(ENODEV);
	}
#ifndef	FEP	/* unix side */
	if (infop->status != FEP_READY) {
	    printf("icpflow: %x board not OK, status = %x\n",
		    accessp, infop->status);
	    return(EIO);
	}
#endif	FEP
	if (accessp->status != FEP_READY) {
	    printf("icpflow: %x board considers itself not OK, status = %x\n",
		    accessp, accessp->status);
	    return(EIO);
	}
	s = splipc();
	while (infop->count == 0) {
		if (sleeptype == 2) {
			infop->sleep = 1;
			while (infop->sleep) 
			if (sleep(&infop->sleep, STOPRI|PCATCH))
				return(EINTR);
		} else {
			if (sleeptype == 0) {
				circuitp->down_wait = 1;
			} else {
				circuitp->up_wait = 1;
			}
			splx(s);
			return(ENOSPC);
		}
	}
	infop->count--;
	ipc.arg = circuitp->wait;
	circuitp->wait = 0;
	ipc.cmd = IPC_GOT_RUN;
	ipc.id = infop->accessp->next_id++;
	ipc.circuit = circuitp->circuit;
	if (!put_ipc(&ipc, infop, IPC_CMD_EXP)) {
	    panic("icpflow: outbound queue is filled\n");
	}
	splx(s);
	/* turn on others interrupt now, and leave yours alone */
#ifndef FEP	/* UNIX side */
	viap = (via_chip_t *) (VIA_ADDRESS+infop->ramp);
	viap->pcr = (viap->pcr & 0xf1) | IRUPT_FEP;
#else		/* FEP side */
	viap = vias[0];
	viap->pcr = IRUPT_UNIX | (viap->pcr & 0x1f);
#endif
	TRACE(T_fwd, ("icpflow:  circuit = 0x%x\n", circuitp->circuit));
	return(0);
}

#ifndef	FEP	/* UNIX side */
/************************************************************************/
/*									*/
/* This is the end of the streams related processing, now we have the	*/
/*  board downloading and control functions.				*/
/*									*/
/************************************************************************/

static
icpresetreply(fep_specifics, err)
fep_specifics_t	*fep_specifics;
int err;
{
	fwd_circuit_t *circuitp;
	mblk_t *m;

	if (m = fep_specifics->reset_msg) {
		fep_specifics->reset_msg = NULL;
		circuitp = &fep_specifics->fwd_circuit[0];
		if (circuitp->upstreamq) {
			((struct iocblk *)m->b_rptr)->ioc_error = err;
			((struct iocblk *)m->b_rptr)->ioc_count = 0;
			if (m->b_cont) 
				freemsg(unlinkb(m));
			m->b_datap->db_type = (err ? M_IOCNAK : M_IOCACK);
			putnext(circuitp->upstreamq, m);
		} else {
			freemsg(m);
		}
	}
}

static
icpresetwakeup(fep_specifics)
fep_specifics_t	*fep_specifics;
{
	switch(fep_specifics->state) {
	case FEP_RESET1:
		(void) icpreset(fep_specifics, 1);
		break;

	case FEP_RESET2:
		(void) icpreset(fep_specifics, 3);
		break;
	}
}

icpreset(fep_specifics, state)
fep_specifics_t	*fep_specifics;
int 		state;			/* reset state .... 0 to start */
{
 	register icp_info_t	*infop = (icp_info_t *) fep_specifics->ginfop;
	register int 		i;
	register unsigned char	result;
	ipc_t			*ipcp;

	for (;;) {
		switch(state) {
		case 0:
			infop->status = FEP_NOTREADY;
			ICP_RESET;
	    		if  (((result = ICP_SELF_STATUS) & 0x40) != 0x40) {
				state = 2;
				continue;
			}
			fep_specifics->state = FEP_RESET1;
			fep_specifics->reset_count = 10;
			timeout(icpresetwakeup, fep_specifics, HZ);
			break;

		case 1:
	    		if  (((result = ICP_SELF_STATUS) & 0x40) != 0x40) {
				state = 2;
				continue;
			}
			if (fep_specifics->reset_count--) {
				timeout(icpresetwakeup, fep_specifics, HZ);
				break;
			}
			fep_specifics->state = FEP_DEAD;
			icpresetreply(fep_specifics, EIO);
			break;

		case 2: if  (((result = ICP_SELF_STATUS) & 0x40) == 0x40) {
				state = 4;
				continue;
			}
			fep_specifics->state = FEP_RESET2;
			fep_specifics->reset_count = 60;
			timeout(icpresetwakeup, fep_specifics, HZ);
			break;

		case 3: if  (((result = ICP_SELF_STATUS) & 0x40) == 0x40) {
				state = 4;
				continue;
			}
			if (fep_specifics->reset_count--) {
				timeout(icpresetwakeup, fep_specifics, HZ);
				break;
			}
			fep_specifics->state = FEP_DEAD;
			infop->status = FEP_NOTREADY;
			icpresetreply(fep_specifics, EIO);
	    		printf("icpreset: unable to reset, status = %x\n", result);
			break;

		case 4:
			infop->status = FEP_RESET;
			fep_specifics->state = FEP_CLOSED;
			icpresetreply(fep_specifics, 0);
			break;
		}
		break;
	}
	return(0);
}



icpdownld(from, to, count, ginfop)
caddr_t		from;
caddr_t		to;
int		count;
caddr_t		*ginfop;

{
 	register icp_info_t *infop = (icp_info_t *) ginfop;

	if ((to = other_to_me(to, infop)) == NULL)
	    return(EFAULT);
	while (count--)
	{
	    *to++ = *from++;
	}
	return(0);
}



icpupld(from, to, count, ginfop)
caddr_t		from;
caddr_t		to;
int		count;
caddr_t		*ginfop;

{
 	register icp_info_t *infop = (icp_info_t *) ginfop;

	if ((from = other_to_me(from, infop)) == NULL)
	    return(EFAULT);
	while (count--)
	{
	    *to++ = *from++;
	}
	return(0);
}



static
icpstartwakeup(fep_specifics)
fep_specifics_t	*fep_specifics;
{
	if (fep_specifics->state == FEP_START)
		(void) icpstart(0, fep_specifics, 1);
}

icpstart(addr, fep_specifics, state)
caddr_t		addr;
fep_specifics_t	*fep_specifics;
int 		state;			/* start state .... 0 to start */
{
 	register icp_info_t	*infop = (icp_info_t *) fep_specifics->ginfop;
	int			i;
	fwd_entry_t		*entryp;
	fwd_entry_t		*from_entryp;

	for (;;) {
		switch (state) {
		case 0: if (infop->status != FEP_RESET) {
	    			printf("icpstart: attempt to an unreset board\n");
	    			return(EL2NSYNC);
			}
			if (infop->accessp == NULL) {
	    			printf("icpstart: attempt to access unconfigured board\n");
	    			return(ENODEV);
			}

			/*
			 * first validate start address, then put address in location 4
			 */

			if (other_to_me(addr, infop) == NULL)
	    			return(EFAULT);
#ifdef NOTYET
			*other_to_me((caddr_t) 4, infop) = addr;
#endif
			ICP_INTERRUPT;
	    		if  (infop->accessp->status == FEP_READY) {
				state = 2;
				continue;
			}
			fep_specifics->state = FEP_START;
			fep_specifics->reset_count = 60;
			timeout(icpstartwakeup, fep_specifics, HZ);
			break;

		case 1: if  (infop->accessp->status == FEP_READY) {
				state = 2;
				continue;
			}
			if (fep_specifics->reset_count--) {
				timeout(icpstartwakeup, fep_specifics, HZ);
				break;
			}
	    		printf("icpstart: start failed: status rom: %x, access: %x\n",
				      ICP_SELF_STATUS, infop->accessp->status);
			fep_specifics->state = FEP_DEAD;
	    		infop->status = FEP_READY;
			icpresetreply(fep_specifics, EIO);
			break;

		case 2: infop->count = infop->accessp->fep_q_size;
			infop->status = FEP_READY;
			fep_specifics->state = FEP_RUNNING;
		    	entryp = &fep_specifics->entry_tbl[fep_specifics->entry_index];
			if (fep_specifics->reset_msg) {
		    		from_entryp = (fwd_entry_t *) 
					fep_specifics->reset_msg->b_cont->b_rptr;
		    		entryp->start = from_entryp->start;
		    		strncpy(entryp->name, from_entryp->name,FWD_NAME_LENGTH);
			}

		    	/*
			 * we are done with this entry in the table
			 * now, ++
		    	 * remember the last entry in table is used
			 * for highwater
			 */
			
		    	fep_specifics->entry_index++;
			icpresetreply(fep_specifics, 0);
			break;
		}
		break;
	}
	return(0);
}
#endif		/* UNIX side */
