#ifndef lint	/* .../sys/PAGING/os/fault.c */
#define _AC_NAME fault_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:24:24}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of fault.c on 87/11/11 21:24:24";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)fault.c	UniPlus VVV.2.1.6	*/

#ifdef HOWFAR
extern int T_fault;
extern int T_pfault;
#endif HOWFAR
#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/param.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vnode.h"
#include "sys/var.h"
#include "sys/buf.h"
#include "sys/utsname.h"
#include "sys/sysinfo.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/swap.h"
#include "sys/getpages.h"
#include "sys/vfs.h"
#include "sys/debug.h"
#endif lint

extern int	freemem;
pte_t	temppte;
extern struct vnode *devtovp();


/*	Protection fault handler
 */

pfault(vaddr, pt)
caddr_t vaddr;
register pte_t	*pt;	/* Physical address of faulting pte.	*/
{
	register dbd_t	*dbd;
	register proc_t	*p;
	register pfd_t	*pfd;
	register reg_t	*rp;
#ifndef AWS
	pte_t		*phystosvir();
#endif AWS
	register pte_t *tp = &temppte;

	/*	Convert the pte address from physical to virtual.
	 */
	
#ifndef AWS
/*	for the AWS we store the virtual addr in the segment table */
	pt = phystosvir(pt);
#endif AWS

	/*	Get a pointer to the region which the faulting
	 *	virtual address is in.
	 */

	p = u.u_procp;
	rp = findreg(p, vaddr);

TRACE(T_pfault, ("pfault(vaddr=0x%x pt=0x%x): *pt=0x%x proc[%d] rp=0x%x\n",
			vaddr, pt, *((int *)pt), p-proc, rp));

	/*	Check to see that the pte hasn't been modified
	 *	while waiting for the lock
	 */

	if(!pt->pgm[0].pg_v){
		regrele(rp);
		return(1);
	}

	/*	Now check for a real protection error as opposed
	 *	to a copy on write.
	 */

	minfo.pfault++;
 	pteget(pt);
 	if(!(npteget(pt))->pgm[0].pg_cw){
		regrele(rp);
		return(0);
	}

	ASSERT(rp->r_type == RT_PRIVATE);

	dbd = dbdget(pt);

#ifdef NEW_PMMU
	pfd = pftopfd(pt->pgm[0].pg_pfn);
#else
	pfd = pfdat + pftopfi((int)(pt->pgm[0].pg_pfn));
#endif

	/*	Copy on write
	 *	If use is 1, and page is not from a file,
	 *	steal it, otherwise copy it
	 */

	memlock();

	if(pfd->pf_use > 1  ||  dbd->dbd_type == DBD_FILE
	   ||  dbd->dbd_type == DBD_LSTFILE) {
		minfo.cw++;
		pg_zero(tp);

		/*	We are locking the page we faulted on
		**	before calling ptmemall because
		**	ptmemall may unlock the region.  If
		**	he does, then the page could be stolen
		**	and we would be copying incorrect
		**	data into our new page.
		*/

		pg_setlock(pt);
#ifdef NEW_PMMU
		{
			struct pfdat *pf2 = pftopfd(pt->pgm[0].pg_pfn);
			pf2->pf_rawcnt++;
		}
#else
		pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt++;
#endif
#ifdef NEW_PMMU
/*		if(ptmemall(rp, tp, 1)) {
/*printf("pfault: ptmemall(0x%x, tp, 1) returned non-zero\n", rp);
/*			memunlock();
/*			regrele(rp);
/*			return(0);
/*		}
/* */
		while (ptmemall(rp, tp, 1)) {
TRACE(T_pfault, ("pfault: ptmemall(0x%x, tp, 1) returned non-zero\n", rp));
		}
#else NEW_PMMU
/*		if(ptmemall(rp, tp, 1, 1)) {
/*printf("pfault: ptmemall(0x%x, tp, 1, 1) returned non-zero\n", rp);
/*			memunlock();
/*			regrele(rp);
/*			return(0);
/*		}
/* */
		while (ptmemall(rp, tp, 1, 1)) {
TRACE(T_pfault, ("pfault: ptmemall(0x%x, tp, 1, 1) returned non-zero\n", rp));
		}
#endif NEW_PMMU

		/*	Its O.K. to unlock the page now since
		**	ptmemall has locked the region again.
		*/

		ASSERT(rp->r_lock);
		ASSERT((npteget(pt))->pgm[0].pg_lock);
#ifdef NEW_PMMU
		{
			pfd_t *pfd2 = pftopfd(pt->pgm[0].pg_pfn);
			ASSERT(pfd2->pf_rawcnt > 0);
	 
			if(--pfd2->pf_rawcnt == 0)
				pg_clrlock(pt);
		}
#else
		ASSERT(pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt > 0);
 
		if(--pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt == 0)
			pg_clrlock(pt);
#endif
		memunlock();
		copypage((int)pt->pgm[0].pg_pfn, (int)tp->pgm[0].pg_pfn);
		memlock();
#ifdef NEW_PMMU
		pfree(rp, pt, dbd);
#else
		pfree(rp, pt, dbd, 1);
#endif
		*pt = *tp;
	} else {

		/*	We are modifiying the page so
		 *	break the disk association to swap.
		 */
		
		if(pfd->pf_flags & P_HASH)
			(void)premove(pfd);

		if(dbd->dbd_type == DBD_SWAP)
			(void)swfree1(dbd);
		dbd->dbd_type = DBD_NONE;
		minfo.steal++;
	}

	memunlock();

	/*	Set the modify bit here before the region is unlocked
	 *	so that getpages will write the page to swap if necessary.
	 */

	pg_setmod(pt);
	pg_clrcw(pt);
	pg_clrprot(pt);
	pg_setprot(pt, PG_RW);
	invsatb(USRATB, vaddr, 1);
	regrele(rp);

	/*	Recalculate priority for return to user mode.
	 */

	curpri = p->p_pri = calcppri(p);


	return(1);
}

/*	Translation fault handler
 */

vfault(vaddr, pt)
caddr_t vaddr;
register pte_t	*pt;	/* Physical address of faulting pte.	*/
{
	register proc_t	*p;
	register dbd_t	*dbd;
	register pfd_t	*pfd;
	register reg_t	*rp;
	register pte_t *tp = &temppte;

	ASSERT(u.u_procp->p_flag & SLOAD);

	/*	Convert the pte address from physical to virtual.
	 */

#ifndef AWS
/*	for the AWS we store the virtual addr in the segment table */
	pt = phystosvir(pt);
#endif AWS
	dbd = dbdget(pt);

	/*	Lock the region containing the page that faulted.
	 */

	p = u.u_procp;
	rp = findreg(p, vaddr);

TRACE(T_fault, ("vfault(vaddr=0x%x pt=0x%x): *pt=0x%x proc[%d] rp=0x%x\n",
			vaddr, pt, *((int *)pt), p-proc, rp));

	/*	Check for an unassigned page.  This is a real
	 *	error.
	 */

	if(dbd->dbd_type == DBD_NONE) {
		regrele(rp);
		return(0);
	}

	/*	Check that the page has not been read in by
	 *	another process while we were waiting for
	 *	it on the reglock above.
	 */

	if(pt->pgm[0].pg_v) {
		regrele(rp);
		return(1);
	}
	minfo.vfault++;

	/*	Allocate a page in case we need it.  We must
	 *	do it now because it is not convenient to
	 *	wait later if no memory is available.  If
	 *	ptmemall does a wait and some other process
	 *	allocates the page first, then we have
	 *	nothing to do.
	 */
	
	memlock();
#ifdef NEW_PMMU
	if(ptmemall(rp, pt, 0)){
		memunlock();
		regrele(rp);
		return(1);
	}
#else
	if(ptmemall(rp, pt, 1, 0)){
		memunlock();
		regrele(rp);
		return(1);
	}
#endif

	/*	See what state the page is in.
	 */

	switch(dbd->dbd_type){
	case DBD_DFILL:
	case DBD_DZERO:{

		/* 	Demand zero or demand fill page.
		 */

		minfo.demand++;
		memunlock();
		if(dbd->dbd_type == DBD_DZERO)
			clearpage((int)pt->pgm[0].pg_pfn);
		dbd->dbd_type = DBD_NONE;
		break;
	}
	case DBD_SWAP:
	case DBD_FILE:
	case DBD_LSTFILE:{

		/*	Page is on swap or in a file.  See if a
		 *	copy is in the hash table.
		 */

		if(pfd = pagefind(rp, dbd)){

			/*	Page is in cache.
			 *	If it is also on the free list,
			 *	remove it.
			 */

			ASSERT(mem_lock);

			minfo.cache++;
			if (pfd->pf_flags&P_QUEUE) {
				ASSERT(pfd->pf_use == 0);
				ASSERT(freemem > 0);
				freemem--;
				pfd->pf_flags &= ~P_QUEUE;
				pfd->pf_prev->pf_next = pfd->pf_next;
				pfd->pf_next->pf_prev = pfd->pf_prev;
				pfd->pf_next = NULL;
				pfd->pf_prev = NULL;
			}

			/*	Free the page we allocated above
			 *	since we don't need it.
			 */

			*tp = *pt;
			pg_setvalid(tp);
#ifdef NEW_PMMU
			pfree(rp, tp, (dbd_t *)NULL);
#else
			pfree(rp, tp, (dbd_t *)NULL, 1);
#endif
			rp->r_nvalid++;
			pfd->pf_use++;
#ifdef MMB
			{ register i;
#ifdef NEW_PMMU
			  register ppfn = pfdtopf(pfd);
#else
			  register ppfn = (pfitopf(pfd - pfdat));
#endif
			for (i = 0; i < (1 << PPTOPSHFT); i++)
				pt->pgm[i].pg_pfn = ppfn++;
			}
#else MMB
#ifdef NEW_PMMU
			pt->pgm[0].pg_pfn = pfdtopf(pfd);
#else
			pt->pgm[0].pg_pfn = pfitopf(pfd - pfdat);
#endif
#endif MMB

			/*	If the page has not yet been read
			 *	in from swap or file, then wait for
			 *	the I/O to complete.
			 */

			if(pfd->pf_flags & P_DONE) {
				memunlock();
			} else {
				pfd->pf_waitcnt++;
				memunlock();
				(void)sleep(pfd, PZERO);
			}
		} else {

			/*	Must read from swap or a file.
			 *	Get the pfdat for the newly allocated
			 *	page and insert it in the hash table.
			 *	Note that it cannot already be there
			 *	because the pfind above failed and
			 *	mem_lock is still locked.
			 */
			
#ifdef NEW_PMMU
			pfd = pftopfd(pt->pgm[0].pg_pfn);
#else
			pfd = &pfdat[pftopfi(pt->pgm[0].pg_pfn)];
#endif
			ASSERT((pfd->pf_flags & P_HASH) == 0);

			/*	Don't insert in hash table if this
			 *	block is from a swap file we are
			 *	trying to delete.
			 */

			if(dbd->dbd_type == DBD_SWAP){
				register int	swapdel;
				pglst_t		pglist;

				swapdel = swaptab[dbd->dbd_swpi].st_flags
						& ST_INDEL;
				if(swapdel == 0)
					pinsert(rp, dbd, pfd);
				memunlock();

				/*	Read from swap.
				 */

				minfo.swap++;
				pglist.gp_ptptr = pt;
				p->p_flag |= SPAGEIN;
				swap(&pglist, 1, B_READ, (pte_t *)0);
				p->p_flag &= ~SPAGEIN;
				if(swapdel){
					(void)swfree1(dbd);
					dbd->dbd_type = DBD_NONE;
				}
			} else {
				/*	Read from file
				 */
				minfo.file++;
				pinsert(rp, dbd, pfd);
				memunlock();
				if(readpg(rp, pt, dbd) < 0) {
					regrele(rp);
					return(-1);
				}
			}

			/*	Mark the I/O done in the pfdat and
			 *	awaken anyone who is waiting for it.
			 */

			memlock();
			pfd->pf_flags |= P_DONE;
			if (pfd->pf_waitcnt) {
				pfd->pf_waitcnt = 0;
				wakeup(pfd);
			}
			memunlock();
		}
		break;
	}
	default:
		panic("vfault - bad dbd_type");
	}

	pg_setvalid(pt);
	pg_clrmod(pt);
	regrele(rp);
	invsatb(USRATB, vaddr, 1);

	/*	Recalculate priority for return to user mode
	 */

	curpri = p->p_pri = calcppri(p);


	return(1);
}

/*	Read page from a file
 *
 *	return 0  - when no error occurs
 *	return -1 - when read error occurs
 */
readpg(rp, pt, dbd)
reg_t	*rp;
pte_t	*pt;
dbd_t	*dbd;
{
	register struct buf	*bp;
	register int	*bnptr;
	register int	i;
	int		nblks;
	int		bsize;
	struct vnode	*vp;
	int		vaddr;
	int		lastpage;

	/*	Get the number of blocks to read and
	 *	a pointer to the block number list.
	 */

	vp = rp->r_vptr;
	bsize = vp->v_vfsp->vfs_bsize;
	nblks = NBPP/bsize;
	bnptr = &vp->v_map[dbd->dbd_blkno];
	lastpage = dbd->dbd_type == DBD_LSTFILE? 1 : 0;

	/*	We must read the page into a kernel virtual
	 *	window because the pte must be valid to do
	 *	the bcopy/bzero but we can't mark the actual pte
	 *	valid since the page is shared and another process
	 *	may try to use it.
	 */

#ifdef NEW_PMMU
	vaddr = ptob(pptop((u_int) pt->pgm[0].pg_pfn));
#else
	if((vaddr = sptalloc(1, PG_V|PG_RW,
			pptop((int)pt->pgm[0].pg_pfn))) == NULL)
	{
		printf("vfault - no virtual space");
		killpage(rp, pt);
		return(-1);
	}
#endif

	for(i = 0  ;  i < nblks  ; i++){
#ifdef NOTDEFASA
		if (*bnptr == 0)
#else NOTDEFASA
		if (*bnptr == -1)
#endif NOTDEFASA
			break;
		if (!lastpage && *(bnptr + 1)) {
			bp = breada(vp->v_mappedvp, (daddr_t)*bnptr, bsize,
				(daddr_t)*(bnptr + 1), bsize);
		}
		else
			bp = bread(vp->v_mappedvp, (daddr_t)*bnptr, bsize);
		bnptr++;
		if (bp->b_flags&B_ERROR) {
			prdev("page read error", vp->v_mappedvp->v_rdev);
			brelse(bp);
			killpage(rp, pt);
#ifndef NEW_PMMU
			sptfree(vaddr, 1, 0);
#endif
			return(-1);
		}
		bcopy(bp->b_un.b_addr, (caddr_t)(vaddr + (i*bsize)), bsize);
		brelse(bp);
	}

	/*	If last page of a region.  Clear tail
	 *	end past mapped portion if it is not on
	 *	a page boundary.
	 */

	if(lastpage) {
		if(i = poff(rp->r_filesz))
			bzeroba((caddr_t)(vaddr + i), NBPP - i);
	}

#ifndef NEW_PMMU
	sptfree(vaddr, 1, 0);
#endif
	return(0);
}

/*
 * Clean up after a read error during vfault processing.
 * This code frees the previously allocated page, and marks
 * the pfdat as bad.  It leaves the pte, and dbd in their original
 * state.  It assumes the pte is presently invalid.
 */
killpage(rp, pt)
reg_t *rp;
register pte_t *pt;
{
	register pfd_t *pfd;
	extern pte_t temppte;
	register pte_t *tp = &temppte;

	*tp = *pt;
	pg_setvalid(pt);
	memlock();

#ifdef NEW_PMMU
	pfd = pftopfd(pt->pgm[0].pg_pfn);
#else
	pfd = &pfdat[pftopfi(pt->pgm[0].pg_pfn)];
#endif
	pfd->pf_flags |= P_BAD|P_DONE;
	while (pfd->pf_waitcnt) {
		pfd->pf_waitcnt = 0;
		wakeup(pfd);
	}
#ifdef NEW_PMMU
	pfree(rp, pt, (dbd_t *)NULL);
#else
	pfree(rp, pt, (dbd_t *)NULL, 1);
#endif
	memunlock();
	*pt = *tp;
}

/* <@(#)fault.c	1.4> */
