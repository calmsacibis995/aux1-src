#ifndef lint	/* .../sys/PAGING/os/grow.c */
#define _AC_NAME grow_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/11 21:24:51}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of grow.c on 87/11/11 21:24:51";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)grow.c	UniPlus VVV.2.1.7	*/

#ifdef HOWFAR
extern int T_grow;
#ifdef NEW_PMMU
extern int T_availmem;
#endif NEW_PMMU
#endif HOWFAR
#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/bitmasks.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/pmmu.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/var.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/tuneable.h"

#include "sys/buserr.h"

#include "sys/debug.h"
#endif lint

/* brk and sbrk system calls
 */

sbreak()
{
	struct a {
		int nva;
	};
	register preg_t	*prp;
	register reg_t	*rp;
	register struct user *up;
	register int	nva;
	register int	change;

	/*	Find the processes data region.
	 */

	up = &u;
	prp = findpreg(up->u_procp, PT_DATA);
	if(prp == NULL)
		goto sbrk_err;
	rp = prp->p_reg;
	reglock(rp);

	nva = ((struct a *)up->u_ap)->nva;
	if(((unsigned) nva) >= USRSTACK) {
		regrele(rp);
		goto sbrk_err;
	}
	if(nva < up->u_exdata.ux_datorg)
		nva = up->u_exdata.ux_datorg;

	change = btop((int)nva) - btotp(prp->p_regva) - rp->r_pgsz;
	if(change > 0  &&  chkpgrowth(change) < 0){
		regrele(rp);
		goto sbrk_err;
	}

	if(growreg(prp, change, DBD_DZERO) < 0){
		regrele(rp);
		goto sbrk_err;
	}
	regrele(rp);

	if(change <= 0){
		register int	n;

		/* clear released part of last page */
		n = ptob(1) - poff(nva);
		if(n < ptob(1))
			uclear(nva, n);
		if(change < 0)
			clratb(USRATB);
	}

	return;

sbrk_err:
	up->u_error = ENOMEM;
	return;
}

/* grow the stack to include the SP
 * true return if successful.
 */

grow(sp)
unsigned sp;
{
	register preg_t	*prp;
	register reg_t	*rp;
	register	si;

	/*	Find the processes stack region.
	 */

	prp = findpreg(u.u_procp, PT_STACK);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;

	/*	The 64 byte offset in the following test is
	 *	to catch a page fault occurring on a save
	 *	instruction.  This instruction does not
	 *	increment the stack pointer if it page faults
	 *	so we could be one save frame (64 bytes) 
	 *	before the top of the stack.
	 */

	reglock(rp);
/* BOBJ: do I need to check sp < USRSTACK ??? */
	if ((sp > (USRSTACK - ptob(rp->r_pgsz))) && (sp < USRSTACK)) {
		regrele(rp);
		return(0);
	}

	si = btop(USRSTACK - sp) - rp->r_pgsz + SINCR;
	if (si <= 0){
		regrele(rp);
		return(0);
	}

	if(chkpgrowth(si) < 0){
		regrele(rp);
		return(0);
	}

	if(growreg(prp, si, DBD_DZERO) < 0){
		regrele(rp);
		return(0);
	}

	regrele(rp);
	return(1);
}

/*	Check that a process is not trying to expand
**	beyond the maximum allowed virtual address
**	space size.
*/

chkpgrowth(size)
register int	size;	/* Increment being added (in pages).	*/
{
	register preg_t	*prp;
	register reg_t	*rp;

	prp = u.u_procp->p_region;

	while(rp = prp->p_reg){
		size += rp->r_pgsz;
		prp++;
	}

	if(size > tune.t_maxumem)
		return(-1);
	
	return(0);
}

/* Allocate page tables.  Typically called to get
 * page tables for user process but may be called
 * for phys IO or to get DMA windows.  Returned address
 * is properly aligned for DMA.  The allocated page tables
 * are contiguous in kernel virtual address space but the
 * physical address is not equal to the virtual address.
 *
 *  npgtbls = number of (PTSIZE byte) page tables to allocate.
 */

pte_t *
uptalloc(npgtbls)
{
	register int	nbytes;
	register int	pt;
	register pfd_t	*pf;
	register int	physaddr;
	register int	i;
	register int	j;

	if (npgtbls == 0)
		return(NULL);

	nbytes = npgtbls << PTSZSHFT;

	/*	If we are trying to allocate less than a full
	 *	page of page tables, then check the list of
	 *	pages which currently are being used for page
	 *	tables.
	 */
	
TRACE(T_grow, ("uptalloc: nbytes=0x%x npgtbls=0x%x\n", nbytes, npgtbls));
	if(npgtbls < NPTPP){

		/*	Get mask with low npgtbls bits on.
		 */

		i = setmask(npgtbls);

		for(pf = ptfree.pf_next ; pf != &ptfree ;
		    pf = pf->pf_next){

			/*	Try all positions of the mask to
			 *	find npgtbls contiguous page tables
			 *	in a page.
			 */

			for(j = 0  ;  j <= NPTPP - npgtbls  ;  j++){
				if((pf->pf_use & (i << j)) == (i << j)){

					/* We have found the page
					 * tables.  Turn off their bits.
					 * If no page tables are left in
					 * the page, then remove page 
					 * from the page table list.
					 */

					pf->pf_use &= ~(i << j);
					if(pf->pf_use == 0){
						pf->pf_prev->pf_next =
							pf->pf_next;
						pf->pf_next->pf_prev =
							pf->pf_prev;
						pf->pf_next = 0;
						pf->pf_prev = 0;
					}

					/* Get address of page tables we
					 * have allocated.  Update the
					 * free page table count and
					 * clear the page tables.
					 */

#ifdef NEW_PMMU
					pt = ptob(pfdtopf(pf));
#else
					physaddr = ptob(pfitop(pf - pfdat));
					pt = (int)uptvaddrs[phystoupti(physaddr)];
TRACE(T_grow, ("uptalloc: found page at physaddr 0x%x pt(0x%x)+=j(0x%x)=0x%x\n",
physaddr, pt, j, pt + (j<<PTSZSHFT)));
#endif NEW_PMMU
					pt += j << PTSZSHFT;
					nptfree -= npgtbls;
					bzero((caddr_t)pt, nbytes);
					return((pte_t *)pt);
				}
			}
		}
	}

	/*	We could not allocate the required number of 
	 *	contiguous page tables from a single page 
	 *	of page tables on the free list.
	 *	Allocate some more kernel virtual address
	 *	space and physical memory for page tables.
	 *	Allocate an integral number of pages
	 *	to use for page tables.
	 */

	i = pttopgs(npgtbls);

#ifdef NEW_PMMU
	availrmem -= i;
	availsmem -= i;

	if (availrmem < tune.t_minarmem  || availsmem < tune.t_minasmem) {
		printf("uptalloc - can't get %d pages\n", i);
		availrmem += i;
		availsmem += i;
		return((pte_t *)-1);
	}
	TRACE(T_availmem,("uptalloc: taking %d avail[RS]mem pages\n", i));

	pt = sptalloc(i, PG_ALL, -1);
TRACE(T_grow, ("uptalloc: pt = sptalloc(0x%x, PG_ALL, -1) = 0x%x\n", i, pt));
#else NEW_PMMU
	pt = sptalloc(i, PG_ALL, 0);
TRACE(T_grow, ("uptalloc: pt = sptalloc(0x%x, PG_ALL, 0) = 0x%x\n", i, pt));
#endif NEW_PMMU

	if(pt == -1) {
#ifdef NEW_PMMU
		availrmem += i;
		availsmem += i;
#endif NEW_PMMU
		return((pte_t *)-1);
	}

	ASSERT((pt & POFFMASK) == 0);
	nptalloced += i << NPTPPSHFT;
	nptfree += i << NPTPPSHFT;

	/*	Clear the free page table bit masks for all
	 *	of the pages which we have just allocated.
	 *	Also update the physical to system virtual
	 *	address map used in pfault and vfault.
	 */
	
	for(j = 0  ;  j < i  ;  j++){
#ifdef  NEW_PMMU
		pf = svtopfd(pt + ptob(j));
#else
		pf = &pfdat[svtopfi(pt + ptob(j))];
#endif
		ASSERT((pf->pf_flags & (P_QUEUE | P_HASH)) == 0);
		pf->pf_use = 0;
#ifndef NEW_PMMU
		physaddr = (int) svirtophys(pt + ptob(j));
		ASSERT((physaddr & POFFMASK) == 0);
TRACE(T_grow, ("uptalloc: uptvaddrs[btotp(0x%x -0x%x)] = 0x%x\n",
physaddr, (int)uptbase, pt + ptob(j)));
		uptvaddrs[phystoupti(physaddr)] = (caddr_t)(pt + ptob(j));
#endif
	}

	/*	Add any unused page tables at the end of the
	 *	last page to the free list.
	 */
	

	j = npgtbls % NPTPP;

	if(j > 0){
#ifdef NEW_PMMU
		pf = svtopfd(pt + ptob(i - 1));
#else
		pf = &pfdat[svtopfi(pt + ptob(i - 1))];
#endif
		ASSERT((pf->pf_flags & (P_QUEUE | P_HASH)) == 0);
		pf->pf_use = setmask(NPTPP - j) << j;
		pf->pf_next = ptfree.pf_next;
		pf->pf_prev = &ptfree;
		ptfree.pf_next->pf_prev = pf;
		ptfree.pf_next = pf;
	}

	/*	Update the count of the number of free page tables
	 *	and zero out the page tables we have just allocated.
	 */

	nptfree -= npgtbls;
	bzero((caddr_t)pt, nbytes);

	return((pte_t *)pt);
}

/*
 * Free previously allocated page tables
 */

uptfree(pt, npgtbls)
register int	pt;
register int	npgtbls;
{
	register int	nfree;
	register int	ndx;
	register pfd_t	*pf;
	register char	*physaddr;
#ifndef NEW_PMMU
	extern pte_t	*ekptbl;
#endif

TRACE(T_grow, ("uptfree(0x%x, 0x%x)\n", pt, npgtbls));
	if (npgtbls == 0)
		return;

	/*	Free the page tables and update the count of
	 *	free page tables.
	 */

	while(npgtbls > 0){

		/* Get a pointer to the pfdat for the page in which
		 * we are freeing page tables.  Compute the index
		 * into the page of the first page table being freed.
		 */

#ifdef NEW_PMMU
		pf = svtopfd(pt);
#else
		pf = &pfdat[svtopfi(pt)];
#endif
		ndx = (pt - ptob(btotp(pt))) >> PTSZSHFT;

		ASSERT((pf->pf_flags & (P_QUEUE | P_HASH)) == 0);

		/* Compute the number of page tables in this page
		 * which we are freeing.
		 */
		
		nfree = min(npgtbls, NPTPP);
		if(((ptob(btop(pt + 1)) - pt) >> PTSZSHFT) < nfree)
			nfree = (ptob(btop(pt)) - pt) >> PTSZSHFT;

		/*	If the current page has no free page tables
		 *	and we are not going to free the entire
		 *	page, then put the pfdat on the free page
		 *	table list.
		 */
		
		if(pf->pf_use == 0  &&  nfree < NPTPP){
			pf->pf_next = ptfree.pf_next;
			pf->pf_prev = &ptfree;
			ptfree.pf_next->pf_prev = pf;
			ptfree.pf_next = pf;
		}

		/*	Set the appropriate bits in the pfdat.  If
		 *	we free the entire page, then return it to
		 *	the system free space list and update the
		 *	physical to system virtual address map to
		 *	show that the page is no longer being used
		 *	for page tables.
		 */
		
		ASSERT((pf->pf_use & (setmask(nfree) << ndx)) == 0);
		pf->pf_use |= setmask(nfree) << ndx;
		if(pf->pf_use == setmask(NPTPP)){

			/* Remove pfdat from page table allocation
			 * list and reset use count.
			 */

			if(nfree != NPTPP) {
				pf->pf_prev->pf_next = pf->pf_next;
				pf->pf_next->pf_prev = pf->pf_prev;
			}
			pf->pf_use = 1;

			/* Clear the entry in the physical to system
			 * virtual address translation table.
			 */

#ifndef NEW_PMMU
			physaddr = (caddr_t)svirtophys(pt & ~POFFMASK);
TRACE(T_grow, ("uptfree: uptvaddrs[btotp(0x%x -0x%x)] = 0\n", physaddr, (int)uptbase));
			uptvaddrs[phystoupti(physaddr)] = 0;
#endif

			/* Free the pages and the system virtual
			 * address space.  Update the count of allocated
			 * and free pages.
			 */

			sptfree(pt & ~POFFMASK, 1, 1);
#ifdef NEW_PMMU
			TRACE(T_availmem,("uptfree: returning %d avail[RS]mem pages\n", 1));
			availrmem++;
			availsmem++;
#endif NEW_PMMU
			nptalloced -= NPTPP;
			nptfree -= NPTPP - nfree;
		} else
			nptfree += nfree;

		npgtbls -= nfree;
		pt += nfree << PTSZSHFT;
	}
}


/*	Get the virtual address of a page table from a segment table
 *	entry (which contains the physical address.)
 */

union ptbl  *
segvptbl(ste)
register ste_t	*ste;
{
	register caddr_t	phaddr;
	register caddr_t	vaddr;

	/*	Get the physical address of the page table.
	 */

	phaddr = (caddr_t)segpptbl(*ste);
#if defined(NEW_PMMU) || defined(AWS)
	return((union ptbl *)phaddr);
#else

	/*	If this is a kernel page table, then physical ==
	 *	virtual.  For a user page table, we must translate
	 *	the physical address to a virtual address using
	 *	the table maintained by uptalloc.
	 */

	if(phaddr >= uptbase){
		vaddr = uptvaddrs[phystoupti(phaddr)];
		ASSERT(vaddr != 0);
		vaddr += poff(phaddr);
	} else {
		vaddr = phaddr;
	}
#ifndef lint
	return((union ptbl *)vaddr);
#else lint	/* "possible pointer alignment problem" */
	phaddr = vaddr;
	return((union ptbl *)0);
#endif lint
#endif NEW_PMMU
}

#ifndef AWS
/*	Convert the physical address of a pte to the kernel
 *	virtual address.  If it is a kernel page table, then
 *	the physical and virtual addresses are the same.
 *	For user page tables, the uptvaddrs table gives the
 *	translated value.
 */

pte_t	*
phystosvir(ppt)
register char	*ppt;
{
	register char	*vpt;

#ifdef NEW_PMMU
	return((pte_t *)ppt);
#else
	if(ppt < uptbase)
		vpt = ppt;
	else {
		vpt = uptvaddrs[phystoupti(ppt)];
		ASSERT(vpt != (char *)0);
		vpt = vpt + poff(ppt);
	}

#ifndef lint
	return((pte_t *)vpt);
#else lint	/* "possible pointer alignment problem" complaint */
	return((pte_t *)0);
#endif lint
#endif NEW_PMMU
}
#endif AWS

/* <@(#)grow.c	1.2> */
