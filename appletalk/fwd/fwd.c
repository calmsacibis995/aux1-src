#ifndef lint	/* .../appletalk/fwd/fwd.c */
#define _AC_NAME fwd_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:10:05}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of fwd.c on 87/11/11 21:10:05";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)fwd.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems			*/
/*	All Rights Reserved					*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	Forwarder driver for communication with a Front End Processor.
 *	This is the unix side, there is an almost identical pair for the 
 *	FEP side.
 *	Each side is divided up into two parts, a generic which is
 *	common to all FEPs (which follows below), and a component which is 
 *	specific to a particular type of FEP.
 *	NOTE: that there is only one copy of this code in the kernel and is
 *	shared by all forwarders regardless of the fep!!!
 */




#include <sys/types.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/stream.h>
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
#include <sys/sysmacros.h>
#include <sys/debug.h>
#include <fwd.h>

#ifndef NULL
#define NULL 0
#endif NULL

extern int T_fwd, T_ipc;


#ifndef	FEP	/* UNIX side */
/*
 *	return the M_IOCTL on the read stream. Note that the same message
 *	block is used as the vehicle, and that if there is an error return,
 *	then linked blocks are lopped off. BEWARE of multiple references.
 */

static void
ioc_ack(errno,mp,q)
int		errno;
struct msgb	*mp;
struct queue	*q;
{
	struct iocblk *	iocbp;

	iocbp = (struct iocblk *) mp->b_rptr;
	if  (iocbp->ioc_error = errno)
	{
	    /* errno != 0, then this is an error */
	    mp->b_datap->db_type = M_IOCNAK;
	    iocbp->ioc_count = 0;
	    if (mp->b_cont)
		/* get rid of linked blocks! */
		freemsg(unlinkb(mp));
	}
	else
	    mp->b_datap->db_type = M_IOCACK;
	qreply(q, mp);
}
#endif  /* n FEP */

/*
 *	return an M_ERROR on the read stream. Note that the same message
 *	block that caused the problem is used as the vehicle. So chained 
 *	blocks to this message will be free'd; i.e. ASSUMES no multiple
 *	references.
 */

static void
data_error(errno,mp,q)
int		errno;
struct msgb	*mp;
struct queue	*q;
{
	mp->b_datap->db_type = M_ERROR;
	mp->b_wptr = mp->b_rptr = mp->b_datap->db_base;
	*mp->b_wptr++ = errno;
	if (mp->b_cont)
	    freemsg(unlinkb(mp));
	qreply(q, mp);
}


#ifndef	FEP	/* UNIX side */
/* 	This routine sends a request to the fep's forwarder, and sleep for an
 *	ack. Waiting is very important, since the fep might take longer then
 * 	Unix side since it has real work to do, and Unix might issue another
 *	command. Sleeping is ok since only executed during opens and closes.
 */
static
send_nwait(m_type, id, q, dev, flag, sflag, name)
char		m_type;
queue_t		*q;
dev_t dev;
{
	int		s;
	int		errno;
	int		size;
	fwd_circuit_t	*circuitp = CIRCUIT(q);
	mblk_t		*mp;

	size = sizeof(fwd_acktags_t);
	while (!(mp = allocb(size, BPRI_MED)))
	    if (sleep(&rbsize[getclass(size)], STOPRI|PCATCH))
		return(EINTR);
	mp->b_wptr += sizeof(fwd_acktags_t);
	mp->b_datap->db_type = m_type;
	FWD_ACKTAGS(mp)->flag = flag;
	circuitp->errno[id] = 0;
	FWD_ACKTAGS(mp)->errno = 0;
        FWD_ACKTAGS(mp)->circuit = circuitp->circuit;
        FWD_ACKTAGS(mp)->id = id;
	switch (m_type) {
	case M_FWD_1stOPEN:
	case M_FWD_OPEN:
	case M_FWD_PUSH:
	case M_FWD_FIND:
		strncpy (   FWD_ACKTAGS(mp)->name,
		    	name,
		    	sizeof(FWD_ACKTAGS(mp)->name)
			);
		FWD_ACKTAGS(mp)->dev = dev;
		FWD_ACKTAGS(mp)->sflag = sflag;
		FWD_ACKTAGS(mp)->cred = *(u.u_cred);
		break;
	}
	s = splstr();
	circuitp->idstate[id] = TRANS_WAITING;
	if (errno = EX(q, putq) (mp, circuitp, 0, 2)) {
	    splx(s);
	    return(errno);
	}
	while (circuitp->idstate[id] == TRANS_WAITING) {
		if (sleep((caddr_t)&circuitp->idstate[id], STOPRI|PCATCH)) {
			while ((mp = allocb(size, BPRI_MED)) == NULL) {
	    			(void) sleep(&rbsize[getclass(size)], PZERO);
			}
			mp->b_wptr += sizeof(fwd_acktags_t);
			mp->b_datap->db_type = M_FWD_CANCEL;
			FWD_ACKTAGS(mp)->flag = flag;
			circuitp->errno[id] = 0;
			FWD_ACKTAGS(mp)->errno = 0;
        		FWD_ACKTAGS(mp)->circuit = circuitp->circuit;
        		FWD_ACKTAGS(mp)->id = id;
			for (;;) {
				errno = EX(q, putq)(mp, circuitp, 0, 2);
				if (errno == 0)
					break;
				if (errno != EINTR) {
	    				splx(s);
	    				return(errno);
				}
			}
			circuitp->idstate[id] = TRANS_SLEEP;
			break;
		}
	}
	while (circuitp->idstate[id] != TRANS_DONE) {
		(void)sleep(&circuitp->idstate[id], PZERO);
	}
	splx(s);
	return(circuitp->errno[id]);
}

/*
 *	This routine allocates a transaction id for an open/close type action
 *	across a circuit. This means that multiple such actions can be outstanding
 *	at any such time 
 */

static
fep_trans_alloc(circuitp)
register fwd_circuit_t	*circuitp;
{
	register int s, id;

	s = splstr();
	for (;;) {
		for (id = 0; id < CIRC_MAXID; id++)
		if (circuitp->idstate[id] == TRANS_FREE)
			break;
		if (id < CIRC_MAXID) {
			circuitp->idstate[id] = TRANS_ALLOC;
			break;
		}
		circuitp->idwanted = 1;
		if (sleep(&circuitp->idwanted, STOPRI|PCATCH)) {
			id = -1;
			break;
		}
	}
	splx(s);
	circuitp->rname[id] = NULL;
	return(id);
}

/*
 *	This routine frees a transaction id for an open/close type action
 *	across a circuit. 
 */

static
fep_trans_free(circuitp, id)
register fwd_circuit_t	*circuitp;
int id;
{
	register int s;

	s = splstr();
	circuitp->idstate[id] = TRANS_FREE;
	if (circuitp->idwanted) {
		circuitp->idwanted = 0;
		wakeup(&circuitp->idwanted);
	}
	splx(s);
	return(id);
}

#endif		/* UNIX side */



/*
 * This is the open routine for all applications, except direct board control.
 * The major difference between this and a board open, is that this searches
 * for a free virtual circuit and then invokes an open of the module on the
 * other side, and waits for a response. The board open, uses circuit zero, an
 * no open of the other side is required.
 */

#ifndef	FEP		/* UNIX side */
fwd_open(q, dev, flag, sflag, err, devp, fep_specificsp)
queue_t		*q;
int		*err;
fep_specifics_t *fep_specificsp;
dev_t	dev, *devp;
int	flag, sflag;
{
	ushort			circuit;
	register ushort		temp;
	fwd_circuit_t		*fwd_circuitp;
        char   			m_type;
	register int		s;
	register int		id;
	struct stroptions	*stroptp;
	struct msgb		*mp;
	struct proc		*pp;

	if (fep_specificsp->state != FEP_RUNNING) {
		*err = EIO;
		return(OPENFAIL);
	}
	switch (sflag) {
	case FWDPUSH:
	        fwd_circuitp = (fwd_circuit_t *) q->q_ptr;
		if ((id = fep_trans_alloc(fwd_circuitp)) < 0) {
			*err = EINTR;
			return(OPENFAIL);
		}
		fwd_circuitp->setpgrp[id] = 0;
		fwd_circuitp->state = CIRC_PUSHING;
		if (*err = send_nwait(M_FWD_PUSH, id, q, dev, 0, sflag, (char *)flag)) {
			fwd_circuitp->state = CIRC_OPEN;
			fep_trans_free(fwd_circuitp, id);
			return(OPENFAIL);
		}
		fwd_circuitp->state = CIRC_OPEN;
		if (fwd_circuitp->setpgrp[id]) {
	    		fwd_circuitp->setpgrp[id] = 0;
	    		pp = u.u_procp;
	    		while (q->q_next)
				q = q->q_next;
	    		if ((pp->p_pid == pp->p_pgrp) &&
	        		(u.u_ttyp == NULL) &&
	        		(((struct stdata *)q->q_ptr)->sd_pgrp == 0)) {
		    		u.u_ttyp = &((struct stdata *)q->q_ptr)->sd_pgrp;
	    		}
		}
		fep_trans_free(fwd_circuitp, id);
		return(0);
	
	case FWDPOP:
	        fwd_circuitp = (fwd_circuit_t *) q->q_ptr;
		if ((id = fep_trans_alloc(fwd_circuitp)) < 0) {
			*err = EINTR;
			return(OPENFAIL);
		}
		fwd_circuitp->state = CIRC_POPPING;
	        s = send_nwait(M_FWD_POP, id, q, 0, flag, 0, NULL);
		fep_trans_free(fwd_circuitp, id);
		fwd_circuitp->state = CIRC_OPEN;
		if (s) 
			return(OPENFAIL);
		return(0);
	case FWDLOOK:
	        fwd_circuitp = (fwd_circuit_t *) q->q_ptr;
		if ((id = fep_trans_alloc(fwd_circuitp)) < 0) {
			*err = EINTR;
			return(OPENFAIL);
		}
		fwd_circuitp->rname[id] = (char *)flag;
		if (*err = send_nwait(M_FWD_LOOK, id, q, dev, 0, sflag, NULL)) {
			fep_trans_free(fwd_circuitp, id);
			return(OPENFAIL);
		}
		fep_trans_free(fwd_circuitp, id);
		return(0);

	case FWDFIND:
	        fwd_circuitp = (fwd_circuit_t *) q->q_ptr;
		if ((id = fep_trans_alloc(fwd_circuitp)) < 0) {
			*err = EINTR;
			return(OPENFAIL);
		}
		if (*err = send_nwait(M_FWD_FIND, id, q, dev, 0, sflag, (char *)flag)) {
			fep_trans_free(fwd_circuitp, id);
			return(OPENFAIL);
		}
		fep_trans_free(fwd_circuitp, id);
		return(0);

	case FWDMNAME:
	        fwd_circuitp = (fwd_circuit_t *) q->q_ptr;
		if ((id = fep_trans_alloc(fwd_circuitp)) < 0) {
			*err = EINTR;
			return(OPENFAIL);
		}
		fwd_circuitp->rname[id] = (char *)flag;
		if (*err = send_nwait(M_FWD_MNAME, id, q, dev,
				      *(char *)flag, sflag, NULL)) {
			fep_trans_free(fwd_circuitp, id);
			return(OPENFAIL);
		}
		fep_trans_free(fwd_circuitp, id);
		return(0);

	default:
		break;
	}
	if  (q->q_ptr == NULL)
	{
	    /*
	     * this dev was closed, so this is the first open, allocate a
	     * virtual circuit and set up the circuit parameters.
	     */
	    temp = fep_specificsp->ncircuits;
	    s = splhi();
	    for ( fwd_circuitp = &(fep_specificsp->fwd_circuit[1]), circuit = 1;
		  (circuit < temp) && (fwd_circuitp->upstreamq != NULL);
		  fwd_circuitp++, circuit++
		)
		;
	    if  (circuit >= fep_specificsp->ncircuits)
	    {
		splx(s);
		*err = EAGAIN;
		return(OPENFAIL);
	    }
	    fwd_circuitp->upstreamq = q;
	    fwd_circuitp->wait = 0;
	    fwd_circuitp->halt = 0;
	    splx(s);
	    if ((id = fep_trans_alloc(fwd_circuitp)) < 0) {
		fwd_circuitp->open = 0;
                fwd_circuitp->upstreamq = NULL;
		fwd_circuitp->state = CIRC_CLOSED;
		*err = EINTR;
		return(OPENFAIL);
	    }
	    /* initialize local data structures for the r&w q's */
	    q->q_ptr = OTHERQ(q)->q_ptr = (caddr_t) fwd_circuitp;
	    fwd_circuitp->circuit = circuit;
	    fwd_circuitp->open = 1;
	    m_type = M_FWD_1stOPEN;
	    fwd_circuitp->fep = fep_specificsp;
	    fep_specificsp->open++;
	}
	else  /* if  (q->q_ptr == NULL) */
	{
	    /* everything already set up, just bump */
	    fwd_circuitp = (fwd_circuit_t *) q->q_ptr;
	    if ((id = fep_trans_alloc(fwd_circuitp)) < 0) {
		*err = EINTR;
		return(OPENFAIL);
	    }
	    fwd_circuitp->open++;
	    m_type = M_FWD_OPEN;
	}
	fwd_circuitp->setpgrp[id] = 0;
	fwd_circuitp->state = CIRC_OPENING;
	/* send an open to the other side and wait for the response */
	*err = send_nwait(m_type, id, q, dev, flag, sflag, q->q_qinfo->qi_minfo->mi_idname);
        if (*err) {   /* the fep's open module resulted in error, undo open */
            if  (--fwd_circuitp->open<=0) {/*no others have it open,close up other side*/
		fwd_circuitp->state = CIRC_CLOSED;
	        fep_specificsp->open--;
		fwd_circuitp->open = 0;
                fwd_circuitp->upstreamq = NULL;
	    } else {
		fwd_circuitp->state = CIRC_OPEN;
	    }
	    fep_trans_free(fwd_circuitp, id);
            return(OPENFAIL);
	}
	if (m_type == M_FWD_1stOPEN) {
		WR(q)->q_minpsz = fwd_circuitp->minpsz;
		WR(q)->q_maxpsz = fwd_circuitp->maxpsz;
	}
	fwd_circuitp->state = CIRC_OPEN;
	if (fwd_circuitp->setpgrp[id]) {
	    fwd_circuitp->setpgrp[id] = 0;
	    pp = u.u_procp;
	    while (q->q_next)
		q = q->q_next;
	    if ((pp->p_pid == pp->p_pgrp) &&
	        (u.u_ttyp == NULL) &&
	        (((struct stdata *)q->q_ptr)->sd_pgrp == 0)) {
		    u.u_ttyp = &((struct stdata *)q->q_ptr)->sd_pgrp;
	    }
	}
	dev = fwd_circuitp->dev[id];
	fep_trans_free(fwd_circuitp, id);
	if (sflag == CLONEOPEN)
		return(minor(dev));
	return(0);
}
#endif	FEP
 

int
fwd_close(q, flag)
queue_t *q;
{
	register fwd_circuit_t		*fwd_circuitp;
	register int id;

	flushq(WR(q), FLUSHALL);
#ifndef	FEP	/* UNIX side */
	fwd_circuitp = CIRCUIT(q);
	fwd_circuitp->state = CIRC_CLOSING;
	if (fwd_circuitp->circuit > 0)
	{
	    if ((id = fep_trans_alloc(fwd_circuitp)) >= 0) {
	    	send_nwait(M_FWD_CLOSE, id, q, NULL, flag, 0, NULL);
	    	fep_trans_free(fwd_circuitp, id);
	    }
	} else {
		id = splclk();
		if (fwd_circuitp->fep->reset_msg) {
			freemsg(fwd_circuitp->fep->reset_msg);
			fwd_circuitp->fep->reset_msg = NULL;
		}
		splx(id);
	}
	fwd_circuitp->fep->open--;
#endif
	flushq(q, FLUSHALL);
	flushq(WR(q), FLUSHALL);
	fwd_circuitp->open = 0;
	fwd_circuitp->upstreamq = NULL;
	fwd_circuitp->state = CIRC_CLOSED;
}


/*
 *	Pick up what was left there by the interrupt routine if this is the
 *	control/multiplexed queue; else upstream must have been freed up.
 */
int
#ifndef	FEP	/* UNIX side */
fwd_rsrvc(q)
#else		/* FEP side */
fwd_wsrvc(q)
#endif
queue_t *q;
{
	mblk_t		*mp;

#ifdef	FEP	/* FEP side */
	if (((fwd_circuit_t *)q->q_ptr)->sd_flag&WSLEEP) {
		wakeup(q);
		((fwd_circuit_t *)q->q_ptr)->sd_flag &= ~WSLEEP;
	}
#endif	FEP	/* FEP side */
	while ((mp = getq(q)) != NULL) {
#ifdef NOTYET
	    printf("fwd_rsrvc: message = 0x%x\n", mp->b_datap->db_type);
#endif
	    /*
	     * we are at a demultiplexer queue, and 
	     * would only get here only if the upstream q was blocked when
	     * there was data for it, and it is now not. Normally
	     * data is passed on beyond the demultiplexer q's.
	     */
	     if  (canput(q->q_next)) {
		 putnext(q, mp);
	     } else {
		 putbq(q, mp);
		 return;
	     }
	} /* while ((mp = getq(q)) != NULL) */
	if (CIRCUIT(q)->wait > 0 && canput(q->q_next)) {
TRACE(T_fwd, ("rsrvc: sending flow\n"));
		(void) EX(q, flow) (CIRCUIT(q), 1);
	}
}


fwd_wputq(q, m)
queue_t *q;
mblk_t *m;
{
	register int s;
	register fwd_circuit_t *circuitp;

	circuitp = CIRCUIT(q);
	putq(q, m);
	s = splstr();
	if (circuitp->halt == 0 && q->q_hiwat) {
		q->q_hiwat = 0;
		q->q_flag |= QFULL;
		q->q_lowat = -1;
	}
	splx(s);
}

int
#ifndef	FEP	/* UNIX side */
fwd_wsrvc(q)
#else		/* FEP side */
fwd_rsrvc(q)
#endif
queue_t *q;
{
	mblk_t		*blkptr;
	mblk_t		*mp;
	struct iocblk 	*iocbp;
	caddr_t		from;
	fwd_circuit_t 	*circuitp;
	fep_specifics_t *fepp;
	fwd_entry_t	*entryp;
	fwd_entry_t	*from_entryp;
	fwd_record_t	*fwd_record;	/* this is only here so we can sizeof */
	int		size;
	int		offset;		/* the error for the begin in fwd_rec */
	int		errno;
	int		i;
	register int	s;
	caddr_t		to;
	int		count;
	short		send_it_on;

	circuitp = CIRCUIT(q);
	for (;;) {
		while ((mp = getq(q)) != NULL) {
#ifdef NOTYET
	    		printf("fwd_wsrvc: message = 0x%x\n", mp->b_datap->db_type);
#endif NOTYET
	    		send_it_on = 0;
	    		switch (mp->b_datap->db_type) {
#ifndef FEP	/* UNIX side */
	    		case M_IOCTL:
				iocbp = (struct iocblk *) mp->b_rptr;
				fepp = circuitp->fep;
				switch (iocbp->ioc_cmd) {
				case I_FWD_RESET:
		    	    		if (fepp->open > 1) {
						ioc_ack(EBUSY,mp,q);
						break;
		    			}
		    			/* zero out the entry_tbl and index */
		    			fepp->entry_index = 0;
		    			for (i=0, entryp = fepp->entry_tbl;
			 			i < FWD_NENTRIES;
			 			i++, entryp++) {
						entryp->start = 0;
						entryp->begin = fepp->begin_ram;
						strncpy(entryp->name, "",
							FWD_NAME_LENGTH);
		    			}
					s = splclk();
		    			if (CIRCUIT(q)->fep->reset_msg)
		    				freemsg(CIRCUIT(q)->fep->reset_msg);
		    			CIRCUIT(q)->fep->reset_msg = mp;
					splx(s);
		    			if (errno = EX(q,reset) (CIRCUIT(q)->fep, 0)) {
		    				CIRCUIT(q)->fep->reset_msg = NULL;
						ioc_ack(errno,mp,q);
		    				fepp->state = FEP_DEAD;
		    			}
		    			break;
				case I_FWD_DOWNLD:
		    			if (fepp->open > 1 ||
					    fepp->state != FEP_CLOSED) {
						ioc_ack(EBUSY,mp,q);
						break;
		    			}
		    			/* if we are in the last entry in table,
					   no ops allowed */
		    			if  (fepp->entry_index >= FWD_NENTRIES-2) {
						ioc_ack(EADDRNOTAVAIL,mp,q);
						break;
		    			}
		    			count = iocbp->ioc_count -
						sizeof(fwd_record->begin);
		    			blkptr = mp->b_cont;
		    			if ((count <= 0) || !blkptr) {
						ioc_ack(EMSGSIZE, mp, q);
						break;
		    			}
		    			fwd_record = (fwd_record_t *) mp->b_cont->b_rptr;
		    			to = fwd_record->begin;
		    			offset = sizeof(fwd_record->begin);
							/* ~left flush in 1st */
		    			/* update the highwater mark */
		    			if (fepp->entry_tbl[fepp->entry_index+1].begin <
						(to+count))
						fepp->entry_tbl[fepp->entry_index+1].begin = to+count;
		    			errno = 0;
		    			while (blkptr != NULL && count > 0) {
						from = blkptr->b_rptr + offset;
						size = min(count,
							   blkptr->b_wptr -
							   blkptr->b_rptr - offset);
						if (errno = EX(q, downld)
							(from, to, size, GINFOP(q))) {
			    				break;
						}
						count =- size;
						to += size;
						blkptr = blkptr->b_cont;
						offset = 0;
						    /* left flush in remaining buffers */
		    			}
		    			iocbp->ioc_count = 0;	/* no data to return */
		    			ioc_ack(errno,mp,q);
		    			break;
		    		case I_FWD_UPLD:
		        		if (fepp->open > 1 ||
						fepp->state != FEP_CLOSED) {
						ioc_ack(EBUSY,mp,q);
						break;
		    			}
		    			fwd_record = (fwd_record_t *) mp->b_cont->b_rptr;
		    			if ((iocbp->ioc_count !=
					     sizeof(fwd_record->begin) +
					     sizeof(fwd_record->opt.ld_length)) ||
					     !fwd_record) {
					     ioc_ack(EMSGSIZE, mp, q);
						break;
		    			}
		    			from = fwd_record->begin;
		    			count = fwd_record->opt.ld_length;

		    			/* save count now since variable will change */

		    			iocbp->ioc_count = count +
							   sizeof(fwd_record->begin);

		    			/*
		     			 * now we have the request, we get rid of the
					 * mblk, so we can allocate the ones needed,
					 * and copy in the data
		    			 */

		    			freemsg(mp->b_cont);
		    			blkptr = mp;

		    			/*
		    			 * offset is a pain! It comes from the
					 * packetization of the message. The first block
					 * has the address, which makes offset get
					 * involved in all the calculations
		    			 */

		    			offset = sizeof(fwd_record->begin);
		    			errno = 0;
		    			while (count > 0) {
						size = min(count, MAXIOCBSZ - offset);
						if ((blkptr->b_cont =
						     allocb(size+offset, BPRI_MED)) ==
								NULL) {
			    				errno = ENOBUFS;
			    				break;
						}
						blkptr = blkptr->b_cont;
						blkptr->b_wptr = blkptr->b_rptr + size +
								 offset;
						to = blkptr->b_rptr + offset;
						if (errno = EX(q, upld)
							(from, to, size, GINFOP(q))) {
			    				break;
						}
						count -= size;
						from += size;
						offset = 0;
						/* now write to the beginning of buf */
		    			}
		    			ioc_ack(errno, mp, q);
		    			break;
				case I_FWD_START:
		    			if (fepp->open > 1 ||
					    fepp->state != FEP_CLOSED) {
						ioc_ack(EBUSY,mp,q);
						break;
		    			}

		    			/*
					 * if we are in the last entry in table,
					 *  no ops allowed
					 */

		    			if  (fepp->entry_index >= FWD_NENTRIES-2) {
						ioc_ack(EADDRNOTAVAIL,mp,q);
						break;
		    			}
		    			if  (mp->b_cont == NULL ||
			 	      	     iocbp->ioc_count != sizeof(fwd_entry_t)) {
					     	/* what gives! size is wrong? */
						ioc_ack(EMSGSIZE, mp, q);
						break;
		    			}
		    			from_entryp = (fwd_entry_t *) mp->b_cont->b_rptr;
					s = splclk();
		    			if (CIRCUIT(q)->fep->reset_msg)
		    				freemsg(CIRCUIT(q)->fep->reset_msg);
		    			CIRCUIT(q)->fep->reset_msg = mp;
					splx(s);
		    			if (errno = EX(q, start)
					    (from_entryp->start, CIRCUIT(q)->fep, 0)) {
		    				CIRCUIT(q)->fep->reset_msg = NULL;
						ioc_ack(errno,mp,q);
						break;
		    			}
		    			break;

				case I_FWD_LOOKUP:
		    			if  (mp->b_cont != NULL) {

						/*
						 * what gives! there is continuation
						 * data?
						 */

						ioc_ack(EMSGSIZE, mp, q);
						break;
		    			}
		    			size = sizeof(fepp->entry_tbl);
		    			if ((mp->b_cont = allocb(size, BPRI_MED)) ==
									NULL) {
						ioc_ack(ENOBUFS, mp, q);
						break;
		    			}
		    			/* copy the table to the buffer to be returned */
		    			for (i=0,entryp=(fwd_entry_t*)mp->b_cont->b_wptr,
			 		     from_entryp = fepp->entry_tbl;
			 		     i < fepp->entry_index;
			 		     i++, entryp++, from_entryp++) {
						*entryp = *from_entryp;
		    			}

		    			/*
					 * the last entry needs to be cleared up
					 */

		    			entryp->start = 0;
		    			strncpy(entryp->name, "AVAIL", FWD_NAME_LENGTH);

		    			/*
					 * create the last entry so the end of ram is
					 * shown
					 */

		   			 entryp++;
		    			entryp->begin = fepp->end_ram;
		    			strncpy(entryp->name, "END", FWD_NAME_LENGTH);
		    			mp->b_cont->b_wptr += (iocbp->ioc_count =
						sizeof(fwd_entry_t) * (i + 2));
		    			ioc_ack(0, mp, q);
		    			break;
		  		default:
		    			send_it_on++;
				} /* switch (((struct iocblk *)mp->b_rptr)->ioc_cmd) */
				break;
#endif		/* UNIX side */

	    		case M_FLUSH:

				/*
				 * if flush is for write queue then flush it.  The first
				 * byte of the message contains the flush flags.
				 */

				if (*mp->b_rptr & FLUSHW)
		    			flushq(q, FLUSHALL);
				send_it_on++;
				break;

	    		default:
				send_it_on++;
	    		} /* switch (mp->b_datap->db_type) */
	    		if (send_it_on) {
		
				s = splstr();	
				if (mp->b_datap->db_type < QPCTL &&
				    circuitp->halt > 0) {
					splx(s);
					putbq(q, mp);
					break;
				}	
				circuitp->halt++;
				splx(s);

				/*
				 * multiplex the packet and put it on the remote's q
				 */

				if (errno = EX(q, putq) (mp, circuitp, 1, 0)) {

					if (errno == ENOSPC) {
						putbq(q, mp);
						return;
					}
		    			/*
					 * uh oh!  an error occured on write, pass
					 * back M_ERROR
					 */

		    			if (mp->b_datap->db_type == M_FLUSH) {

						/*
						 * special case, process upstream
						 * read queue?
						 */

						if (*mp->b_rptr & FLUSHR) {
			    				*mp->b_rptr &= ~FLUSHW;
			    				flushq(OTHERQ(q), FLUSHALL);
			    				qreply(q, mp);
						} else {
			    				freemsg(mp);
						}
		    			} else {
						printf("rmt_putq: failed\n");
#ifndef FEP	/* UNIX side */
						data_error(errno,mp,q);
#else		/* FEP side */
						freemsg(mp);
#endif
						break;
		    			} /* else if (mp->b_datap->db_type == M_FLUSH) */
				}
	    		}
		} /* while ((mp = getq(q)) != NULL) */
		s = splstr();
		if (q->q_count == 0 && circuitp->halt == 0 && q->q_hiwat == 0) {
			q->q_flag &= ~QFULL;
	    		q->q_hiwat = 1;
			q->q_lowat = 0;
			splx(s);
			continue;
		}
		splx(s);
		break;
	}
}

#ifdef FWDPRINTF
#ifndef FEP
fwdprintf(m)
mblk_t *m;
{
	register char *cp;
	
	cp = m->b_rptr;
	printf(cp[4*sizeof(int)],
		((int *)cp)[0],
		((int *)cp)[1],
		((int *)cp)[2],
		((int *)cp)[3]);
}
#else
extern fep_specifics_t icp_specifics;

fwdprintf(s, a, b, c, d)
char *s;
int a, b, c, d;
{
	register mblk_t *m;
	register int i, *ip;
	register fwd_circuit_t *cp;

	i = strlen(s)+1+4*sizeof(int);
	m = allocb(i, BPRI_HI);
	m->b_wptr += i;
	ip = (int *)m->b_rptr;
	*ip++ = a;
	*ip++ = b;
	*ip++ = c;
	*ip = d;
	strcpy(s, m->b_rptr+4*sizeof(int));
	cp = icp_specifics.fwd_circuit;
	m->b_datap->db_type = M_FWD_PRINTF;
	if((*cp->fep->putq) (m, cp, 0, 3))
		freemsg(m);
}
#endif FEP
#endif FWDPRINTF
