#ifndef lint	/* .../appletalk/fwd/streams/streamio.c */
#define _AC_NAME streamio_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:05:38}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of streamio.c on 87/11/11 21:05:38";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)streamio.c	UniPlus 2.1.3	*/

/*   Copyright (c) 1984 AT&T	and UniSoft Systems */
/*     All Rights Reserved  	*/

/*   THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T and UniSoft Systems */
/*   The copyright notice above does not evidence any   	*/
/*   actual or intended publication of such source code.	*/

#include "sys/types.h"
#include "sys/file.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/conf.h"
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
#include "fwd.h"
#include "via.h"
#include "fwdicp.h"
#include "sys/debug.h"
#include <sys/process.h>

extern		fep_specifics_t	icp_specifics;
/*
 *	Added by UniSoft
 */

#ifndef ASSERT
#define ASSERT(x)
#endif ASSERT
#ifndef min
#define min(x,y) (x < y ? x : y)
#endif min
#define sigbit(a) (1<<(a-1))


extern char	dblkflag;
mblk_t 		*qopenclose_h;		/* head and tail to open/close queue */
mblk_t 		*qopenclose_t;
extern struct streamtab fwd_info;
extern struct fmodsw fepdevsw[];
extern struct fmodsw fepmodsw[];
extern int	fepdevcnt;
extern int	fepmodcnt;

/*
 *	Global structures allocated on the fly
 */

dblk_t *dblock;
mblk_t *mblock;
queue_t *queue;
struct stdata *streams;
int nmblock;

/*
 *	System streams initialization routine
 */

strinit(ubase, length)
int ubase;
int length;
{
	register long i, ndblock, bsize, size, base;
	register caddr_t addr, addr2;

	base = ubase;
	ndblock = v.v_nblk4096 + v.v_nblk2048 + v.v_nblk1024 + v.v_nblk512 +
	    v.v_nblk256 + v.v_nblk128 + v.v_nblk64 + v.v_nblk16 +
	    v.v_nblk4;
	nmblock = (ndblock * 5) >> 2;
	bsize =	v.v_nblk4096*4096 + v.v_nblk2048*2048 + v.v_nblk1024*1024 + 
		v.v_nblk512*512 + v.v_nblk256*256 + v.v_nblk128*128 + 
		v.v_nblk64*64 + v.v_nblk16*16 + v.v_nblk4*4;
	size = nmblock*sizeof(mblk_t) +
		ndblock*sizeof(dblk_t) +
		v.v_nqueue*sizeof(queue_t) +
		v.v_nstream*sizeof(struct stdata) +
		bsize;

	/*
	 *	check to see if there is enough buffer space allocated
	 */

	base += size;
	if (base >= ubase + length) {
		printf("Streams space not available");
		dblkflag++;
		return;
	}

	/*
	 *	add extra 4 byte blocks to use up any extra space
	 */

	i = (ubase + length) - base;
#ifdef	DEBUG
	printf("extra stream space = %x\n", i);
#endif	DEBUG
	i /= sizeof(dblk_t) + sizeof(mblk_t) + 4;
	v.v_nblk4 += i;
	nmblock += i;
	ndblock += i;
	bsize += 4*i;
	size += i * (sizeof(dblk_t) + sizeof(mblk_t) + 4);
	
	/*
	 *	Allocate the space
	 */
 
	addr2 = addr = (caddr_t)(ubase);

	/*
	 *	parcel it out to the various data structures (the first, page
	 *	(who cares) alligned, bits become buffers)
	 */	

	addr += bsize;
	mblock = (mblk_t *)addr;
	addr += nmblock*sizeof(mblk_t);
	dblock = (dblk_t *)addr;
	addr += ndblock*sizeof(dblk_t);
	queue = (queue_t *)addr;
	addr += v.v_nqueue*sizeof(queue_t);
	streams = (struct stdata *)addr;
	qinit(addr2);
}
/*
 * open a stream device
 */

stropen(mp)
register mblk_t	*mp;
{
	register queue_t *qp;
	register	nok;
	struct fmodsw	*fepdevp;
	char		*name_wanted;
	register fwd_circuit_t	*circuitp;
	queue_t		*q;
	int		temp;
	int		err;
	fwd_acktags_t	*acktags;
	fep_specifics_t *fep_specifics = &icp_specifics;

	acktags = FWD_ACKTAGS(mp);
#ifdef	DEBUG
	if  (   (mp->b_datap->db_type != M_FWD_1stOPEN)
	     && (mp->b_datap->db_type != M_FWD_OPEN)
	    )
	{
	    printf("stropen, qopenclose is corrupted\n");
	    acktags->errno = EIO;
	    goto quit;
	}
#endif	DEBUG
	circuitp = &fep_specifics->fwd_circuit[acktags->circuit];
	circuitp->proc[acktags->id] = pr_current;
	if (qp = circuitp->upstreamq) {
		/*
		 * Already open!
		 * Open all modules and devices down stream to notify
		 * that another user is streaming.
		 * For drivers pass down minor device number.
		 */
#ifdef	DEBUG
		if  (mp->b_datap->db_type != M_FWD_OPEN) {
		    printf("stropen, M_FWD_1stOPEN on existing stream\n");
		}
#endif	DEBUG
		q = qp;
		while (q = q->q_next) {
			if ((nok = (*RD(q)->q_qinfo->qi_qopen)(RD(q),
					acktags->dev,
					q->q_next? 0: acktags->flag,
					q->q_next? MODOPEN: DEVOPEN,
					&acktags->errno,
					&acktags->dev,
					acktags)) == OPENFAIL) {
				if (!acktags->errno)
					acktags->errno = ENXIO;
				goto quit;
			}
		}

	} else {
		/* 
		 * Not already streaming so create a stream to driver.
		 */
#ifdef	DEBUG
		if  (mp->b_datap->db_type != M_FWD_1stOPEN)
		{
		    printf("stropen, M_FWD_OPEN on non-existing stream\n");
		}
#endif	DEBUG

		/*
		 * Get queue pair for stream head, i.e. fwd demultiplexing q's
		 */

		if (!(qp = allocq())) {
			printf("stropen:out of queues\n");
			acktags->errno = ENOSR;
			goto quit;
		}

		qp->q_qinfo = fwd_info.st_rdinit;
		qp->q_hiwat = fwd_info.st_rdinit->qi_minfo->mi_hiwat;
		qp->q_lowat = fwd_info.st_rdinit->qi_minfo->mi_lowat;
		qp->q_minpsz = fwd_info.st_rdinit->qi_minfo->mi_minpsz;
		qp->q_maxpsz = fwd_info.st_rdinit->qi_minfo->mi_maxpsz;
		qp->q_flag |= QREADR|QWANTR;
		WR(qp)->q_qinfo = fwd_info.st_wrinit;
		WR(qp)->q_hiwat = fwd_info.st_wrinit->qi_minfo->mi_hiwat;
		WR(qp)->q_lowat = fwd_info.st_wrinit->qi_minfo->mi_lowat;
		WR(qp)->q_minpsz = fwd_info.st_wrinit->qi_minfo->mi_minpsz;
		WR(qp)->q_maxpsz = fwd_info.st_wrinit->qi_minfo->mi_maxpsz;
		WR(qp)->q_flag |= QWANTR;
		circuitp->upstreamq = WR(qp);
		circuitp->wait = 0;
		circuitp->halt = 0;
		circuitp->circuit = acktags->circuit;
		circuitp->fep = fep_specifics;
		qp->q_ptr = WR(qp)->q_ptr = (caddr_t)circuitp;

		/*
		 * Now we find if the module indicated by the open, is over
		 * here. Compare the name field, to that in the fepdevsw.
		 */

		name_wanted = acktags->name;
		for  (fepdevp = fepdevsw, temp = fepdevcnt; temp; fepdevp++, temp--)
		    if  ( strncmp(name_wanted,
				  fepdevp->f_name,
				  sizeof(fepdevp->f_name)
				 )
			  == 0
			)  
			break;
		if  (temp == 0) {
		    	acktags->errno = ENODEV;
			goto quit;
		}

		nok = qattach(fepdevp->f_str,
			      qp,
			      &acktags->dev,
			      acktags->flag,
			      mp,
			      &acktags->errno 
			     );

		if (nok == OPENFAIL) {
			/* free queue pair */
			freeq(qp);
			/* NULL out stream pointer in vnode */
			circuitp->upstreamq = NULL;
		} else {
			acktags->minpsz = WR(qp)->q_next->q_minpsz;
			acktags->maxpsz = WR(qp)->q_next->q_maxpsz;
		}

	/* Unisoft signals:  There is one process group variable for
	 * the stream.  It's maintained in the stream head.  Things down
	 * stream use pointers to it.  However, things down stream
	 * determine if controlling tty stuff should be set.  On first open
	 * u_ttyp is overloaded and carries back the location of the 
	 * module's pointer.  We notice this and set things right.
	 */
	}

quit:
	circuitp->proc[acktags->id] = NULL;
	acktags->sflag = TRANS_DONE;
	mp->b_datap->db_type = M_FWD_ACK;
	(void) EX(qp, putq) (mp, circuitp, 0, 2);
}




/*
 * push a stream module
 */

strpush(mp)
register mblk_t	*mp;
{
	register queue_t *qp;
	register	nok;
	struct fmodsw	*fepmodp;
	char		*name_wanted;
	fwd_circuit_t	*circuitp;
	int		temp;
	fwd_acktags_t	*acktags;
	fep_specifics_t *fep_specifics = &icp_specifics;

	acktags = FWD_ACKTAGS(mp);
#ifdef	DEBUG
	if  (   (mp->b_datap->db_type != M_FWD_PUSH)) {
print_message(mp);
	    printf("strpush, qopenclose is corrupted\n");
	    acktags->errno = EIO;
	    goto quit;
	}
#endif	DEBUG
	circuitp = &fep_specifics->fwd_circuit[acktags->circuit];
	circuitp->proc[acktags->id] = pr_current;

	qp = circuitp->upstreamq;

	/*
	 * Now we find if the module indicated by the open, is over
	 * here. Compare the name field, to that in the fepdevsw.
	 */
	name_wanted = acktags->name;
	for  (fepmodp = fepmodsw, temp = fepmodcnt; temp; fepmodp++, temp--)
	    if  ( strncmp(name_wanted,
			  fepmodp->f_name,
			  sizeof(fepmodp->f_name)
			 )
		  == 0
		)  
		break;
	if  (temp == 0) {
	    acktags->errno = EINVAL;
	    goto quit;
	}
	nok = qattach(fepmodp->f_str,
		      RD(qp),
		      &acktags->dev,
		      acktags->flag,
		      mp,
		      &acktags->errno
		     );

	/* Unisoft signals:  There is one process group variable for
	 * the stream.  It's maintained in the stream head.  Things down
	 * stream use pointers to it.  However, things down stream
	 * determine if controlling tty stuff should be set.  On first open
	 * u_ttyp is overloaded and carries back the location of the 
	 * module's pointer.  We notice this and set things right.
	 */

quit:
	circuitp->proc[acktags->id] = NULL;
	acktags->sflag = TRANS_DONE;
	mp->b_datap->db_type = M_FWD_ACK;
	(void) EX(qp, putq) (mp, circuitp, 0, 2);
}



strtime(circuitp)
register fwd_circuit_t *circuitp;
{
	if (circuitp->sd_flag & STRTIME) {
		wakeup(circuitp->upstreamq);
		circuitp->sd_flag &= ~STRTIME;
	}
}

/*
 * Shut down a stream
 *  -- pop all line disciplines
 *  -- shut down the driver
 */

strclose(mp)
register mblk_t	*mp;
{
	register queue_t *qp;
	register s;
	fwd_acktags_t	*acktags;
	fwd_circuit_t	*circuitp;
	fep_specifics_t *fep_specifics = &icp_specifics;

	acktags = FWD_ACKTAGS(mp);
#ifdef	DEBUG
	if  (mp->b_datap->db_type != M_FWD_CLOSE) {
	    print_message(mp);
	    printf("strclose, qopenclose is corrupted\n");
	    acktags->errno = EIO;
	    goto quit;
	}
#endif	DEBUG
	circuitp = &fep_specifics->fwd_circuit[acktags->circuit];
	circuitp->proc[acktags->id] = pr_current;
	qp = circuitp->upstreamq;
	if  (qp == NULL) {			/* this can happen on a open fail */
	    acktags->errno = EIO;
	    goto quit;
	}

	/* 
	 * Pop all modules and close driver. (via qdetach)
	 * Wait STRTIMOUT seconds for write queue to empty.
	 * If not wake up and close via qdetach anyway. 
	 * qdetach is called with (1) as the 2nd arg to indicate
	 * that the device close should be called.
	 */

	while (qp->q_next) {
		if (!(acktags->flag&FNDELAY)) {
			s = splstr();
			circuitp->sd_flag |= (STRTIME | WSLEEP);
			timeout(strtime,circuitp,STRTIMOUT*HZ);

			/*
			 * sleep until awakened by strwsrv() or strtime()
			 */

			while((circuitp->sd_flag &STRTIME) && qp->q_next->q_count) {

/*
 *	UniSoft 
 */

				qp->q_next->q_flag |= QWANTW;
				circuitp->sd_flag |= WSLEEP;
/*
 */
				if (sleep((caddr_t)qp, STIPRI|PCATCH))
					circuitp->sd_flag &= ~(STRTIME | WSLEEP);
			}
			untimeout(strtime, circuitp);
			circuitp->sd_flag &= ~(STRTIME | WSLEEP);
			splx(s);
		}

		/* qdetach pops off module, altering qp->q_next.
		 * The 2nd arg (1) indicates the detached module or
		 * driver was open and must be closed.
		 */

		qdetach(RD(qp->q_next), 1, acktags->flag);
	}
	circuitp->upstreamq = NULL;
	/* free stream head queue pair */
	s = splstr();
	flushq(RD(qp),1);
	flushq(qp,1);
	unschedq(RD(qp));
	unschedq(qp);
	freeq(RD(qp));
	splx(s);
	acktags->errno = 0;
quit:
	circuitp->proc[acktags->id] = NULL;
	acktags->sflag = TRANS_DONE;
	mp->b_datap->db_type = M_FWD_ACK;
	(void) EX(qp, putq) (mp, circuitp, 0, 2);
}


strpop(mp)
register mblk_t	*mp;
{
	register queue_t *qp;
	fwd_circuit_t	*circuitp;
	fwd_acktags_t	*acktags;
	fep_specifics_t *fep_specifics = &icp_specifics;

	acktags = FWD_ACKTAGS(mp);
#ifdef	DEBUG
	if  (mp->b_datap->db_type != M_FWD_POP) {
print_message(mp);
	    printf("strpop, qopenclose is corrupted\n");
	    acktags->errno = EIO;
	    goto quit;
	}
#endif	DEBUG
	circuitp = &fep_specifics->fwd_circuit[acktags->circuit];
	circuitp->proc[acktags->id] = pr_current;
	qp = circuitp->upstreamq;
#ifdef	DEBUG
	if  (qp == NULL) {
	    printf("strpop, attempt to close nonexistent downstream\n");
	    acktags->errno = EIO;
	    goto quit;
	}
#endif	DEBUG

	/* 
	 * Pop all modules and close driver. (via qdetach)
	 * Wait STRTIMOUT seconds for write queue to empty.
	 * If not wake up and close via qdetach anyway. 
	 * qdetach is called with (1) as the 2nd arg to indicate
	 * that the device close should be called.
	 */
	if (qp->q_next->q_next == NULL) {
	    acktags->errno = EINVAL;
	    goto quit;
	}

	/* qdetach pops off module, altering qp->q_next.
	 * The 2nd arg (1) indicates the detached module or
	 * driver was open and must be closed.
	 */
	qdetach(RD(qp->q_next), 1, acktags->flag);
quit:
	circuitp->proc[acktags->id] = NULL;
	acktags->sflag = TRANS_DONE;
	mp->b_datap->db_type = M_FWD_ACK;
	(void) EX(qp, putq) (mp, circuitp, 0, 2);
}




/*
 * attach a stream device or line discipline
 *   qp is a read queue; the new queue goes in so its next
 *   read ptr is the argument, and the write queue corresponding
 *   to the argument points to this queue.  1 is returned if
 *   successful, 0 if not.
 */

qattach(qinfo, qp, dev, flag, mp, err)
register struct streamtab *qinfo;
register queue_t *qp;
dev_t *dev;
mblk_t *mp;
int *err;
{
	register queue_t *nq;
	register s;
	int i;
	extern putq();

	if (!(nq = allocq())) {
		printf("qattach: out of queues\n");
		*err = ENOSR;
		return(OPENFAIL);
	}
	*err = 0;
	s = splstr();
	nq->q_next = qp;
	WR(nq)->q_next = WR(qp)->q_next;
	if (WR(qp)->q_next)
		OTHERQ(WR(qp)->q_next)->q_next = nq;
	WR(qp)->q_next = WR(nq);
	nq->q_qinfo = qinfo->st_rdinit;
	nq->q_minpsz = nq->q_qinfo->qi_minfo->mi_minpsz;
	nq->q_maxpsz = nq->q_qinfo->qi_minfo->mi_maxpsz;
	nq->q_hiwat = nq->q_qinfo->qi_minfo->mi_hiwat;
	nq->q_lowat = nq->q_qinfo->qi_minfo->mi_lowat;
	nq->q_flag |= QREADR|QWANTR;
	
	WR(nq)->q_qinfo = qinfo->st_wrinit;
	WR(nq)->q_minpsz = WR(nq)->q_qinfo->qi_minfo->mi_minpsz;
	WR(nq)->q_maxpsz = WR(nq)->q_qinfo->qi_minfo->mi_maxpsz;
	WR(nq)->q_hiwat = WR(nq)->q_qinfo->qi_minfo->mi_hiwat;
	WR(nq)->q_lowat = WR(nq)->q_qinfo->qi_minfo->mi_lowat;
	WR(nq)->q_flag |= QWANTR;
	if ((i = (*nq->q_qinfo->qi_qopen)(nq,
				     *dev,
				     flag,
				     (WR(nq)->q_next ? MODOPEN :
				      	(FWD_ACKTAGS(mp)->sflag == CLONEOPEN ?
						CLONEOPEN : DEVOPEN)),
				     err,
				     dev,
				     FWD_ACKTAGS(mp))) == OPENFAIL){
		qdetach(nq, 0, 0);
		splx(s);
		if (*err == 0)
			*err = ENXIO;
		return(OPENFAIL);
	}
	splx(s);
	if (FWD_ACKTAGS(mp)->sflag == CLONEOPEN)
		*dev = i;
	return(0);
}

/*
 * Detach a stream device or line discipline.
 * if clmode==1, then the device close routine should be
 * called.  If clmode==0, the detach is a result of a
 * failed open and so the device close routine must not
 * be called.
 */
qdetach(qp, clmode, flag)
register queue_t *qp;
{
	register s;
	register i;
	register queue_t *q, *prev = NULL;

	s = splstr();
	if (clmode) {
		if (qready()) runqueues();
		(*qp->q_qinfo->qi_qclose)(qp,flag);
		/*
		 * Remove the service functions from the run list.
		 * Note that qp points to the read queue.
		 */
		for (i=0; (qp->q_flag|WR(qp)->q_flag)&QENAB; i++) {
			runqueues();
			if (i>10) {
				for (q = qhead; q; q = q->q_link)  {
					if ((q == qp) || (q == WR(qp))) {
						if (prev)
							prev->q_link = q->q_link;
						else
							qhead = q->q_link;
						if (q == qtail)
							qtail = prev;
					}
					prev = q;
				}
				break;
			}
		}
		flushq(qp, 1);
		flushq(WR(qp), 1);
	}
	if (WR(qp)->q_next)
		backq(qp)->q_next = qp->q_next;
	if (qp->q_next)
		backq(WR(qp))->q_next = WR(qp)->q_next;
	unschedq(qp);
	unschedq(WR(qp));
	freeq(qp);
	splx(s);
}

unschedq(q)
register queue_t *q;
{
	register queue_t *qq;

	if (q->q_flag&QENAB) {
		q->q_flag &= ~QENAB;
		if (q == qhead) {
			if (!(qhead = q->q_link)) qtail = NULL;
		} else {
			qq = qhead;
			while (qq) {
				if (q == qq->q_link) {
					if (!(qq->q_link = q->q_link)) qtail = qq;
					break;
				}
				qq = qq->q_link;
			}
		}
	}
}

/*
 * find a module on a stream module
 */

strfind(mp)
register mblk_t	*mp;
{
	register queue_t *q;
	struct fmodsw	*fepmodp;
	char		*name_wanted;
	fwd_circuit_t	*circuitp;
	int		temp;
	fwd_acktags_t	*acktags;
	fep_specifics_t *fep_specifics = &icp_specifics;
	register int s;

	acktags = FWD_ACKTAGS(mp);
	circuitp = &fep_specifics->fwd_circuit[acktags->circuit];
	circuitp->proc[acktags->id] = pr_current;
	q = circuitp->upstreamq;

	name_wanted = acktags->name;
	for (fepmodp = fepmodsw, temp = fepmodcnt; temp; fepmodp++, temp--)
	if (strncmp(name_wanted, fepmodp->f_name, sizeof(fepmodp->f_name)) == 0)  
		break;
	if  (temp == 0) {
	    acktags->errno = EINVAL;
	    goto quit;
	}
	acktags->errno = ENODEV;
	s = splstr();
	for (q = q->q_next; q; q = q->q_next) 
	if (q->q_qinfo == fepmodp->f_str->st_wrinit) {
	    	acktags->errno = 0;
		break;
	} 
	splx(s);
quit:
	circuitp->proc[acktags->id] = NULL;
	acktags->sflag = TRANS_DONE;
	mp->b_datap->db_type = M_FWD_ACK;
	(void) EX(q, putq) (mp, circuitp, 0, 2);
}

strlook()
{
}

strmname()
{
}
