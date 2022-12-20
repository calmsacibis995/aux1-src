#ifndef lint	/* .../sys/PAGING/os/physio.c */
#define _AC_NAME physio_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.4 87/11/11 21:25:50}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.4 of physio.c on 87/11/11 21:25:50";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

/*	@(#)physio.c	UniPlus VVV.2.1.2	*/

#ifdef HOWFAR
extern int T_swap;
extern int T_dophys;
#endif HOWFAR

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/dir.h"
#include "sys/map.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/var.h"
#include "sys/vnode.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/swap.h"
#include "sys/getpages.h"
#include "sys/uio.h"
#include "sys/debug.h"
#endif lint


/* Max. number of pages to swap per I/O */

extern caddr_t physiobuf;
extern short physiosize;
#define NPAGE 		((physiosize > 0)? physiosize : 1)
#define usephysiobuf 	physiosize;		/* don't use if no pages */

int  usingphysiobuf;	/* used to lock the dedicated physio buffer */

/*
 * swap I/O
 */

swap(pglptr, npage, rw, ptbl)
register pglst_t	*pglptr;
pte_t			*ptbl;
{
	register struct buf	*bp;
	register int	blkno;
	register dev_t	dev;
	register int	i;
	dbd_t		*dbd;
	int		ospl;

	syswait.swap++;
TRACE(T_swap,("SWAP:pglptr 0x%x npage %d rw 0x%x ptbl 0x%x\n",pglptr,npage,rw,ptbl));
	dbd = dbdget(pglptr->gp_ptptr);
	dev = swaptab[dbd->dbd_swpi].st_dev;
	blkno = dbd->dbd_blkno;

	ospl = splhi();

#ifdef DEBUG
if (T_dophys) {
for (i = 0, bp = pfreelist.av_forw; bp; bp = bp->av_forw, i++) ;
printf("BFREE%d ", i);
}
#endif

	while ((bp = pfreelist.av_forw) == NULL) {
		pfreelist.b_flags |= B_WANTED;
		(void) sleep((caddr_t)&pfreelist, PRIBIO+1);
	}
	pfreelist.av_forw = bp->av_forw;
	/* guaranteed to be not B_BUSY because it was on pfreelist */

	splx(ospl);

	bp->b_proc = u.u_procp;
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
	bp->b_blkno = blkno;
#define DIRECTSWAP
#ifdef DIRECTSWAP
	for(i = 0  ;  i < npage  ;  i++) {
		bp->b_un.b_addr = (caddr_t)
			(ptob(pptop(pglptr++->gp_ptptr->pgm[0].pg_pfn)));
		swapseg(dev, bp, 1, rw);
		bp->b_blkno += ptod(1);
	}
#else DIRECTSWAP
	if (npage > 1 || usephysiobuf) {
		struct proc *p = u.u_procp;

		if ((p == &proc[0] || p == &proc[2]) && usingphysiobuf) {
			if (npage > 0) {
				for(i = 0  ;  i < npage  ;  i++) {
					bp->b_un.b_addr = (caddr_t)
					   (ptob(pglptr++->
					    pptop(gp_ptptr->pgm[0].pg_pfn)));
					swapseg(dev, bp, 1, rw);
					bp->b_blkno += ptod(1);
				}
			}
		} else {
			/* note that the following loop is safe even for
			 * proc[0] and proc[2] because we are guaranteed
			 * by the above check that usingphysiobuf is zero
			 */
			ospl = splhi();
			while (usingphysiobuf) {
				usingphysiobuf |= B_WANTED;
				(void) sleep(&usingphysiobuf, PRIBIO);
			}
			usingphysiobuf |= B_BUSY;
			splx(ospl);

			bp->b_un.b_addr = physiobuf;
			while (npage >= NPAGE) {
				if (!(rw&B_READ))
					for(i = 0  ;  i < NPAGE  ;  i++)
					  copypage((int)
					   (pglptr++->gp_ptptr->pgm[0].pg_pfn),
					   ptopp(btop((int)physiobuf)+i));
				swapseg(dev, bp, NPAGE, rw);
				npage -= NPAGE;
				bp->b_blkno += ptod(NPAGE);
				if ((rw&B_READ))
					for(i = 0  ;  i < NPAGE  ;  i++)
					  copypage(
					   ptopp(btop((int)physiobuf)+i),
					   (int)
					   (pglptr++->gp_ptptr->pgm[0].pg_pfn));
			}
			if (npage > 0) {
				if (!(rw&B_READ))
					for(i = 0  ;  i < npage  ;  i++)
					 copypage((int)
					  (pglptr++->gp_ptptr->pgm[0].pg_pfn),
					  pftopf(btop((int)physiobuf)+i));
				swapseg(dev, bp, npage, rw);
				if ((rw&B_READ))
					for(i = 0  ;  i < npage  ;  i++)
					  copypage(
					   ptopp(btop((int)physiobuf)+i),
					   (int)
					   (pglptr++->gp_ptptr->pgm[0].pg_pfn));
			}
			SPL6();
			if (usingphysiobuf & B_WANTED)
				wakeup(&usingphysiobuf);
			usingphysiobuf = 0;
			SPL0();
		}
	} else {
		bp->b_un.b_addr = 
		(caddr_t)ptob(pptop(pglptr->gp_ptptr->pgm[0].pg_pfn));
		swapseg(dev, bp, npage, rw);
	}
#endif DIRECTSWAP

	ospl = splhi();

	bp->av_forw = pfreelist.av_forw;
	pfreelist.av_forw = bp;
	if (pfreelist.b_flags & B_WANTED) {
		pfreelist.b_flags &= ~B_WANTED;
		wakeup((caddr_t)&pfreelist);
	}
	bp->b_flags &= ~(B_BUSY|B_PHYS);

	splx(ospl);

	syswait.swap--;
}

swapseg(dev, bp, pg, rw)
dev_t		dev;
register struct buf *bp;
register int	pg;
{
	int		ospl;

	u.u_iosw++;
	if (rw) {
		sysinfo.swapin++;
		sysinfo.bswapin += pg;
	} else {
		sysinfo.swapout++;
		sysinfo.bswapout += pg;
	}
	bp->b_bcount = ptob(pg);

	(*bdevsw[(short)major(dev)].d_strategy)(bp);

	ospl = splhi();
	while ((bp->b_flags & B_DONE) == 0)
		(void) sleep((caddr_t)bp, PSWP);
	splx(ospl);

	if ((bp->b_flags & B_ERROR) || bp->b_resid)
		panic("i/o error in swap");
	bp->b_flags &= ~B_DONE;
}


/*
 * Raw I/O. The arguments are
 * The strategy routine for the device
 * A buffer, which is usually NULL, or special buffer
 *   header owned exclusively by the device for this purpose
 * The device number
 * Read/write flag
 */
physio(strat, bp, dev, rw, uio)
register struct buf *bp;
int (*strat)();
dev_t dev;
struct uio *uio;
{
	register struct user *up;
	register struct iovec *iov;
	register unsigned base, count, actcount;
	int	hpf, error;

	up = &u;
nextiov:
	if (uio->uio_iovcnt == 0)
		return (0);

	iov = uio->uio_iov;
	base = (unsigned)iov->iov_base;
	if (rw)
		sysinfo.phread++;
	else
		sysinfo.phwrite++;
	syswait.physio++;
	hpf = (bp == NULL);

	SPL6();

	if (hpf) {
		while ((bp = pfreelist.av_forw) == NULL) {
			pfreelist.b_flags |= B_WANTED;
			(void) sleep((caddr_t)&pfreelist, PRIBIO+1);
		}
		pfreelist.av_forw = bp->av_forw;
	} else while (bp->b_flags & B_BUSY) {
		bp->b_flags |= B_WANTED;
		(void) sleep((caddr_t)bp, PRIBIO+1);
	}

	SPL0();

	bp->b_error = 0;
	bp->b_proc = up->u_procp;
	bp->b_un.b_addr = physiobuf;
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;

	SPL6();
	while (usingphysiobuf) {
		usingphysiobuf |= B_WANTED;
		(void) sleep(&usingphysiobuf, PRIBIO);
	}
	usingphysiobuf |= B_BUSY;
	SPL0();

	while (count = iov->iov_len) {
		if (count > ptob(NPAGE))
			count = ptob(NPAGE);
		if (!useracc(base, count, rw | B_PHYS)) {
			bp->b_flags |= B_ERROR;
			bp->b_error = EFAULT;
			break;
		}
		if (!(rw&B_READ))
			(void) copyin((caddr_t)base, (caddr_t)physiobuf, count);
		bp->b_blkno = uio->uio_offset >> DEV_BSHIFT;
		bp->b_bcount = count;
		(*strat)(bp);
		SPLHI();
		while ((bp->b_flags & B_DONE) == 0)
			(void) sleep((caddr_t)bp, PRIBIO);
		SPL0();
		bp->b_flags &= ~B_DONE;
		if (bp->b_flags & B_ERROR) {
			undma((int)base, (int)count, rw);
			break;
		}
		actcount = count - bp->b_resid;
		
		if ( ! actcount) {
			undma((int)base, (int)count, rw);
			break;
		}
		iov->iov_len -= actcount;
		uio->uio_resid -= actcount;
		uio->uio_offset += actcount;
		if ((rw&B_READ))
			(void) copyout(physiobuf, (caddr_t)base, actcount);
		undma((int)base, (int)count, rw);
		if(bp->b_resid)
			break;
		base += actcount;
	}

	SPL6();
	if (usingphysiobuf & B_WANTED)
		wakeup(&usingphysiobuf);
	usingphysiobuf = 0;
	SPL0();

	bp->b_flags &= ~(B_BUSY|B_PHYS);
	error = geterror(bp);

	SPL6();

	if (hpf) {
		bp->av_forw = pfreelist.av_forw;
		pfreelist.av_forw = bp;
		if (pfreelist.b_flags & B_WANTED) {
			pfreelist.b_flags &= ~B_WANTED;
			wakeup((caddr_t)&pfreelist);
		}
	} else if (bp->b_flags & B_WANTED)
		wakeup((caddr_t)bp);

	SPL0();

	syswait.physio--;
	if (bp->b_resid || error)
		return(error);
	uio->uio_iov++;
	uio->uio_iovcnt--;
	goto nextiov;
}
/* <@(#)physio.c	1.2> */
