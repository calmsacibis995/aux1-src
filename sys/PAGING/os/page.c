#ifndef lint	/* .../sys/PAGING/os/page.c */
#define _AC_NAME page_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.5 87/11/18 10:50:16}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.5 of page.c on 87/11/18 10:50:16";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

/*	@(#)page.c	UniPlus VVV.2.1.17	*/

#ifdef HOWFAR
extern int	T_page;
extern int T_swapalloc;
#ifdef NEW_PMMU
extern int T_availmem;
#endif
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
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "svfs/mount.h"
#include "sys/buf.h"
#include "sys/map.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/swap.h"
#include "sys/tuneable.h"
#ifdef NEW_PMMU
#include "sys/pmmu.h"
#endif
#include "sys/debug.h"
#include "svfs/inode.h"
#include "netinet/in.h"
#include "setjmp.h"
#endif lint

#ifdef NEW_PMMU
extern char k1to1tbl[];
#endif

char	mem_lock;

#ifndef NEW_PMMU
int	firstfree, maxfree;
#endif

int	freemem;
#ifdef NEW_PMMU
int	availrmem;
int	availsmem;
#else
extern int kmemory;
#endif

#ifndef invsatb
extern int	invsatb();
#endif	invsatb
#ifndef clratb
extern int	clratb();
#endif clratb


/*	Lock memory allocation.
 */

#ifndef memlock

memlock()
{
	register int s;

	s = splhi();
	u.u_procp->p_flag |= SLOCK; /* SLOCK NOT USED ON V3 */
	while (mem_lock) {
		mem_lock |= 2;
{/*debug*/
#ifdef DEBUG
printf("ASA:mem_lock=BUSY "); debug();
#endif
		sleep(&mem_lock, PMEM);
	}
/*debug*/}
	mem_lock = 1;
	splx(s);
}

#endif



/*	Unlock memory allocation.
 */

#ifndef memunlock

memunlock()
{
	ASSERT(mem_lock);
	if (mem_lock&2)
		wakeup(&mem_lock);
	mem_lock = 0;
	u.u_procp->p_flag &= ~SLOCK; /* SLOCK NOT USED ON V3 */
}

#endif

/*
 * Allocate pages and fill in page table
 *	rp		-> region pages are being added to.
 *	base		-> address of page table
 *	size		-> # of pages needed
 *	validate	-> Mark pages valid if set.
 * returns:
 *	0	Memory allocated immediately.
 *	1	Had to unlock region and go to sleep before
 *		memory was obtained.  After awakening, the
 *		page was valid or pfree'd so no page was
 *		allocated.
 *
 * Called with mem_lock set and returns the same way.
 */

#ifndef NEW_PMMU
ptmemall(rp, base, size, validate)
reg_t		*rp;
register pte_t	*base;
register int	validate;
{
	register struct pfdat	*pfd;
	register int		i;

	/*	Check for illegal size.
	 */

	ASSERT(size > 0);
	ASSERT(mem_lock);
TRACE(T_page, ("ptmemall(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n", rp, base, size, validate,caller()));
	ASSERT(rp->r_lock);

	if(memreserve(rp, size)  &&  base->pgm[0].pg_v)
	{
		freemem += size;
		return(1);
	}
	ASSERT(mem_lock);
	ASSERT(rp->r_lock);
	/*
	 * Take pages from head of queue
	 */

	pfd = phead.pf_next;
	i = 0;
	while(i < size){
		register pfd_t	*pfd_next;

		ASSERT (pfd != &phead);
		ASSERT(pfd->pf_flags&P_QUEUE);
		ASSERT(pfd->pf_use == 0);


		/* Delink page from free queue and set up pfd
		 */

		pfd_next = pfd->pf_next;
		pfd->pf_prev->pf_next = pfd_next;
		pfd_next->pf_prev = pfd->pf_prev;
		pfd->pf_next = NULL;
		pfd->pf_prev = NULL;
		if (pfd->pf_flags&P_HASH)
			(void)premove(pfd);
		pfd->pf_blkno = BLKNULL;
		pfd->pf_use = 1;
		pfd->pf_flags = 0;
		pfd->pf_rawcnt = 0;
		rp->r_nvalid++;

		/*
		 * Insert in page table
		 */

#ifdef MMB
		{ register n;
#ifdef NEW_PMMU
		register ppfn = pfdtopf(pfd);
#else
		register ppfn = pfitopf(pfd - pfdat);
#endif
		for (n = 0; n < (1 << PPTOPSHFT); n++)
			base->pgm[n].pg_pfn = ppfn++;
		}
#else MMB
#ifdef NEW_PMMU
		base->pgm[0].pg_pfn = pfdtopf(pfd);
#else
		base->pgm[0].pg_pfn = pfitopf(pfd - pfdat);
#endif
#endif MMB
		pg_clrmod(base);

		pg_clrprot(base);
		if(base->pgm[0].pg_cw) {
			pg_setprot(base, PG_RO);
		} else {
			pg_setprot(base, PG_RW);
		}

		if(validate)
			pg_setvalid(base);
		
		i++;
		base++;
		pfd = pfd_next;
	}
	return(0);
}
#endif NEW_PMMU


/*
 * Shred page table and update accounting for swapped
 * and resident pages.
 *	rp	-> ptr to the region structure.
 *	pt	-> ptr to the first pte to free.
 *	dbd	-> ptr to disk block descriptor.
 *	size	-> nbr of pages to free.
 *
 * Called with mem_lock set and returns the same way.
 */

#ifndef NEW_PMMU
pfree(rp, pt, dbd, size)
reg_t		*rp;
register pte_t	*pt;
register dbd_t	*dbd;
int		size;
{
	register struct pfdat	*pfd;
	register int		k;

	ASSERT(mem_lock);

	/* 
	 * Zap page table entries
	 */

	for (k = 0; k < size; k++, pt++) {
		if (pt->pgm[0].pg_v) {
#ifdef NEW_PMMU
			pfd = pftopfd(pt->pgm[0].pg_pfn);
#else
			pfd = &pfdat[pftopfi(pt->pgm[0].pg_pfn)];
#endif

			/* Free pages that aren't being used
			 * by anyone else
			 */
			if(--pfd->pf_use == 0){

				/* Pages that are associated with disk
				 * go to end of queue in hopes that they
				 * will be reused.  All others go to
				 * head of queue so they will be reused
				 * quickly.
				 */

				if(dbd == NULL || dbd->dbd_type == DBD_NONE){
					/*
					 * put at head 
					 */
					pfd->pf_next = phead.pf_next;
					pfd->pf_prev = &phead;
					phead.pf_next = pfd;
					pfd->pf_next->pf_prev = pfd;
				} else {
					/*
					 * put at tail 
					 */
					pfd->pf_prev = phead.pf_prev;
					pfd->pf_next = &phead;
					phead.pf_prev = pfd;
					pfd->pf_prev->pf_next = pfd;
				}
				pfd->pf_flags |= P_QUEUE;
				freemem++;
			}

			rp->r_nvalid--;
		}
		TRACE(T_swapalloc,
			("pfree: size=0x%x pt=0x%x dbd=0x%x dbd_type=0x%x\n", 
			size, pt, dbd, dbd?dbd->dbd_type:0));
		if(dbd  &&  dbd->dbd_type == DBD_SWAP){
			if (swfree1(dbd) == 0)
				(void) pbremove(rp, dbd);
		}

		/*
		 * Change to zero pte's.
		 */

		pg_zero(pt);
		if(dbd)
			dbd++->dbd_type = DBD_NONE;
	}


}
#endif NEW_PMMU




/*
 * Device number
 *	vp	-> vnode pointer
 * returns:
 *	dev	-> device number
 */
u_int
effdev(vp)
register struct vnode *vp;
{
	register int mode;

	if (vp->v_op == &svfs_vnodeops) {
		/* local file system */
		mode = VTOI(vp)->i_mode & IFMT;
		if (mode == IFREG || mode == IFDIR) 
			return(VTOI(vp)->i_dev);
		return(VTOI(vp)->i_rdev);
	}
	return((u_int)vp);
}

/*
 * Find page by looking on hash chain
 *	dbd	-> Ptr to disk block descriptor being sought.
 * returns:
 *	0	-> can't find it
 *	pfd	-> ptr to pfdat entry
 */

struct pfdat *
pagefind(rp, dbd)
register reg_t	*rp;
register dbd_t	*dbd;
{
	register u_int		dev;
	register daddr_t	blkno;
	register pfd_t		*pfd;

	/*	Hash on block and look for match.
	 */

	ASSERT(mem_lock);
	if(dbd->dbd_type == DBD_SWAP){
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
	} else {
		register struct vnode	*vp;

		/*	For pages on a file (rather than swap),
		 *	we use the first of the 2 or 4 block numbers
		 *	as the value to hash.
		 */

		vp = rp->r_vptr;
		ASSERT(vp != NULL);
		ASSERT(vp->v_map != NULL);
		dev = effdev(vp);

		/*	The following kludge is because of the
		 *	overlapping text and data block in a 413
		 *	object file.  We hash shared pages on the
		 *	first of the 2 or 4 blocks which make up
		 *	the page and private pages on the second
		 *	block.  This means that the block which
		 *	has the end of the text and the beginning
		 *	of the data will be in the hash twice,
		 *	once as text and once as data.  This is
		 *	necessary since the two cannot be shared.
		 */

		if(rp->r_type == RT_PRIVATE)
			blkno = vp->v_map[dbd->dbd_blkno + 1];
		else
			blkno = vp->v_map[dbd->dbd_blkno];
	}

	pfd = phash[(blkno>>DPPSHFT)&phashmask];

	for( ; pfd != NULL ; pfd = pfd->pf_hchain) {
		if((pfd->pf_blkno == blkno) && (pfd->pf_dev == dev)){
			if (pfd->pf_flags & P_BAD)
				continue;
			return(pfd);
		}
	}
	return(0);
}

/*
 * Insert page on hash chain
 *	dbd	-> ptr to disk block descriptor.
 *	pfd	-> ptr to pfdat entry.
 * returns:
 *	none
 */

pinsert(rp, dbd, pfd)
register reg_t	*rp;
register dbd_t	*dbd;
register pfd_t	*pfd;
{
	register u_int	dev;
	register int	blkno;

	/* Check page range, see if already on chain
	 */

	ASSERT(mem_lock);
	if(dbd->dbd_type == DBD_SWAP){
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
	} else {
		register struct vnode	*vp;

		/*	For pages on a file (rather than swap),
		 *	we use the first of the 2 or 4 block numbers
		 *	as the value to hash.
		 */

		vp = rp->r_vptr;
		ASSERT(vp != NULL);
		ASSERT(vp->v_map != NULL);
		dev = effdev(vp);

		/*	The following kludge is because of the
		 *	overlapping text and data block in a 413
		 *	object file.  We hash shared pages on the
		 *	first of the 2 or 4 blocks which make up
		 *	the page and private pages on the second
		 *	block.  This means that the block which
		 *	has the end of the text and the beginning
		 *	of the data will be in the hash twice,
		 *	once as text and once as data.  This is
		 *	necessary since the two cannot be shared.
		 */

		if(rp->r_type == RT_PRIVATE)
			blkno = vp->v_map[dbd->dbd_blkno + 1];
		else
			blkno = vp->v_map[dbd->dbd_blkno];

		/*
		 *	If blkno is zero, then we can't hash the page.
		 *	This happens for the last data page of a stripped
		 *	a.out that is an odd number of blocks long.
		 */

		if(blkno == 0)
			return;
	}

	ASSERT(pfd->pf_hchain == NULL);
	ASSERT(pfd->pf_use > 0);

	/*
	 * insert newcomers at tail of bucket
	 */

	{
		register struct pfdat *pfd1, **p;

		for(p = &phash[(blkno>>DPPSHFT)&phashmask] ; 
				pfd1 = *p ; p = &pfd1->pf_hchain) {
			if((pfd1->pf_blkno == blkno) &&
			   (pfd1->pf_dev == dev)){
#if OSDEBUG == YES
				printf("swapdev %x %x %x\n",swapdev,
				     pfd1->pf_dev,dev);
				printf("blkno %x %x\n",blkno,pfd1->pf_blkno);
				printf("swpi %x %x\n", pfd1->pf_swpi,
					pfd->pf_swpi);
				printf("pfd %x %x\n",pfd,pfd1);
				printf("use %x %x\n",pfd->pf_use,pfd1->pf_use);
				printf("flags %x %x\n",pfd->pf_flags,
				     pfd1->pf_flags);
#endif
				panic("pinsert dup");
			}
		}
		*p = pfd;
		pfd->pf_hchain = pfd1;
	}

	/*	Set up the pfdat.  Note that only swap pages are
	 *	put on the hash list for now.
	 */

	pfd->pf_dev = dev;
	if(dbd->dbd_type == DBD_SWAP){
		pfd->pf_swpi = dbd->dbd_swpi;
		pfd->pf_flags |= P_SWAP;
	} else {
		pfd->pf_flags &= ~P_SWAP;
	}
	pfd->pf_blkno = blkno;
	pfd->pf_flags |= P_HASH;
}


/*
 * remove page from hash chain
 *	pfd	-> page frame pointer
 * returns:
 *	0	Entry not found.
 *	1	Entry found and removed.
 */
premove(pfd)
register struct pfdat *pfd;
{
	register struct pfdat *pfd1, **p;
	int	rval;

	ASSERT(mem_lock);

	rval = 0;
	for(p = &phash[(pfd->pf_blkno>>DPPSHFT)&phashmask] ; 
			pfd1 = *p ; p = &pfd1->pf_hchain) {
		if (pfd1 == pfd) {
			*p = pfd->pf_hchain;
			rval = 1;
			break;
		}
	}

	/*
	 * Disassociate page from disk and
	 * remove from hash table
	 */
	pfd->pf_blkno = BLKNULL;
	pfd->pf_hchain = NULL;
	pfd->pf_flags &= ~P_HASH;
	pfd->pf_dev = 0;
	return(rval);
}

/*
 * Allocate system virtual address space but
 * don't allocate any physical pages (this is
 * done later with sptfill().  Expects a size
 * in pages, and returns a virtual address.
 */
#ifndef NEW_PMMU
caddr_t
sptreserve(size)
register int size;
{
	register sp;

TRACE(T_page, ("malloc(0x%x, 0x%x) ", sptmap, size));
	if ((sp = malloc(sptmap, size))  ==  0) {
		printf("No kernel virtual space\n");
#if OSDEBUG == YES
		printf("size=%d, mode=%d, base=%d\n",size, mode, base);
#endif
		return(NULL);
	}
TRACE(T_page, ("returned page number 0x%x\n", sp));
	return((caddr_t)ptosv(sp));
}
#endif NEW_PMMU

/*
 * Allocate pages for a reserved section of
 * of system virtual address space.
 *	return values:
 *		 0	normal return
 *		-1	nosleep is set and we would have 
 *			  had to sleep (no memory was allocated)
 * NOTE: 
 * 	This routine is intended to allow filling in a virtual 
 *	array at interrupt time (watch out - memory allocation 
 *	at interrupt time).  If called from an interrupt routine
 *	sptfill must be called with nosleep set, and the calling 
 *	routine must check for possible failure.  If nosleep is 
 *	not set, sptfill will not fail and need not be checked.
 */
#ifndef NEW_PMMU
sptfill(svaddr, size, mode, pbase, nosleep)
caddr_t svaddr;
register int size, mode;
int pbase, nosleep;
{
	register int sp, i;
	register pte_t *p;

	/*
	 * If we need to allocate memory and we're not allowed
	 * to sleep, make sure we won't run into any locks.
	 */
	if ((pbase == 0) && nosleep && 
	    (freemem < size || isreglock(&sysreg)))
		return(-1);

	/*
	 * Allocate and fill in pages
	 */
	sp = svtop(svaddr);
	if (pbase  ==  0){
		reglock(&sysreg);
		memlock();
		if(ptmemall(&sysreg, ptopte(sp), size, 1))
			panic("sptalloc - ptmemall failed");
		kmemory += size;
		memunlock();
		regrele(&sysreg);
	}

	/*
	 * Setup page table entries
	 */
	for (i = 0; i < size; i++) {
		if (pbase > 0) {
			p = (pte_t *)ptopte(sp + i);		
			pg_zero(p);
			wtpte(p->pgi, mode, pbase++);
		} else {
#ifdef MMB
			register j;
			
			p = (pte_t *)ptopte(sp + i);		
			for (j = 0; j < (1 << PPTOPSHFT); j++)
				p->pgi[j].pg_pte |= mode;
#else MMB
				((pte_t *)ptopte(sp+i))->pgi[0].pg_pte |= mode;
#endif MMB
		}
		invsatb(SYSATB, (caddr_t)(ptosv(sp + i)), 1);
	}
	return(0);
}
#endif NEW_PMMU

/*
 * Allocate system virtual address space and
 * allocate or link  pages.
 */
#ifndef NEW_PMMU
sptalloc(size, mode, base)
register int size, mode, base;
{
	register caddr_t svaddr;
	extern int kt_ppmem;

TRACE(T_page, ("sptalloc(0x%x, 0x%x, 0x%x)\n", size, mode, base));

 	if (base == 0 && kt_ppmem && kmemory > tune.t_ppmem) {
		if (!(suser() && kmemory <= kt_ppmem)) {
			printf("No kernel space\n");
			return(NULL);
		}
	}

	/*
	 * Allocate system virtual address space
	 */
	if ((svaddr = sptreserve(size)) == NULL)
		return(NULL);
		
	/*
	 * Allocate and fill in pages
	 */
	(void) sptfill(svaddr, size, mode, base, 0);

	return((int)svaddr);
}
#endif NEW_PMMU

#ifndef NEW_PMMU
sptfree(vaddr, size, flag)
register int size;
{
	register i, sp;

	sp = svtop(vaddr);
TRACE(T_page,("sptfree:vaddr=%x sp=%x\n",vaddr,sp));
	if (flag){
		reglock(&sysreg);
		memlock();
		pfree(&sysreg, ptopte(sp), (dbd_t *)NULL, size);
		kmemory -= size;
		memunlock();
		regrele(&sysreg);
	}
	for (i = 0; i < size; i++) {
		pg_zero(ptopte(sp+i));
	}
	mfree(sptmap, size, sp);
}
#endif NEW_PMMU

/*
 * Initialize memory map
 *	first	-> first free page #
 *	last	-> last free page #
 * returns:
 *	none
 */
#ifndef NEW_PMMU
meminit(first, last)
register int first;
{
	register struct pfdat *pfd;
	register int i;

	firstfree = first;
	maxfree = last;
	freemem = (last - first);
	maxmem = freemem;
	/*
	 * Setup queue of pfdat structures.
	 * One for each page of available memory.
	 */
	pfd = &pfdat[first];
	phead.pf_next = &phead;
	phead.pf_prev = &phead;
	/*
	 * Add pages to queue, high memory at end of queue
	 * Pages added to queue FIFO
	 */
	for (i = freemem; --i >= 0; pfd++) {
		pfd->pf_next = &phead;
		pfd->pf_prev = phead.pf_prev;
		phead.pf_prev->pf_next = pfd;
		phead.pf_prev = pfd;
		pfd->pf_flags = P_QUEUE;
	}
}
#endif NEW_PMMU

#ifdef NOTDEF

This code is broken now because the pfdat's are no longer contiguous

/*
 * flush all pages associated with a mount device
 *	mp	-> mount table entry
 * returns:
 *	none
 */
punmount(mp)
register struct mount *mp;
{
	register int i;
	register struct pfdat *pfd;

	bflush(mp->m_dev);
	memlock();
	for (i = firstfree,pfd = pfdat + i; i < maxfree; i++, pfd++) {
		if (mp->m_dev == pfd->pf_dev)
			if ((pfd->pf_flags & (P_HASH | P_SWAP))  == P_HASH)
				(void)premove(pfd);
	}
	memunlock();
}
#endif NOTDEF

/*
 * Find page by looking on hash chain
 *	dbd	Ptr to disk block descriptor for block to remove.
 * returns:
 *	0	-> can't find it
 *	i	-> page frame # (index into pfdat)
 */

pbremove(rp, dbd)
reg_t	*rp;
dbd_t	*dbd;
{
	register struct pfdat	*pfd;
	register struct pfdat	**p;
	register int		blkno;
	register u_int		dev;

	/*
	 * Hash on block and look for match
	 */

	ASSERT(mem_lock);

	if(dbd->dbd_type == DBD_SWAP){
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
	} else {
		register struct vnode	*vp;

		/*	For pages on a file (rather than swap),
		 *	we use the first of the 2 or 4 block numbers
		 *	as the value to hash.
		 */

		vp = rp->r_vptr;
		ASSERT(vp != NULL);
		ASSERT(vp->v_map != NULL);
		dev = effdev(vp);

		/*	The following kludge is because of the
		 *	overlapping text and data block in a 413
		 *	object file.  We hash shared pages on the
		 *	first of the 2 or 4 blocks which make up
		 *	the page and private pages on the second
		 *	block.  This means that the block which
		 *	has the end of the text and the beginning
		 *	of the data will be in the hash twice,
		 *	once as text and once as data.  This is
		 *	necessary since the two cannot be shared.
		 */

		if(rp->r_type == RT_PRIVATE)
			blkno = vp->v_map[dbd->dbd_blkno + 1];
		else
			blkno = vp->v_map[dbd->dbd_blkno];
	}

	for (p = &phash[(blkno>>DPPSHFT)&phashmask] ; 
			pfd = *p; p = &pfd->pf_hchain) {
		if ((pfd->pf_blkno == blkno) && (pfd->pf_dev== dev)) {
			*p = pfd->pf_hchain;

			pfd->pf_blkno = BLKNULL;
			pfd->pf_hchain = NULL;
			pfd->pf_flags &= ~P_HASH;
			pfd->pf_dev = 0;
			return(1);
		}
	}

	return(0);

}


/*
 * Reserve size memory pages.  Returns with freemem
 * decremented by size.  Return values:
 *	0 - Memory available immediately
 *	1 - Had to sleep to get memory
 */
memreserve(rp, size)
register reg_t *rp;
{
	register struct proc *p;

	ASSERT(rp->r_lock);
	ASSERT(mem_lock);
TRACE(T_page >= 2, ("memreserve: rp=0x%x freemem=0x%x size=0x%x\n",rp,freemem,size));

	if (freemem >= size) {
		freemem -= size;
		return(0);
	}
	p = u.u_procp;
	while (freemem < size) {
		regrele(rp);
		memunlock();
		p->p_stat = SXBRK;
		(void)wakeup(&runout);
TRACE(T_page, ("memreserve: calling swtch()\n"));
		swtch();
		reglock(rp);
		memlock();
	}
	freemem -= size;
	return(1);
}

#ifdef NEW_PMMU
static int sptsegbase = SPTSEG;
static int softseglimit = SPTSEG + SPTCOUNT;
static int hardseglimit = TOPSEG;

static pfd_t *pfdall();

static
sptseggrow(pgcount)
{
	register int segcnt = ptos(pgcount);
	register int i, j;
	register pfd_t *pfd;
	register int physaddr;
	register freeaddr, freecnt;

	TRACE(T_page, ("sptseggrow(cnt = 0x%x)\n", pgcount));

	reglock(&sysreg);
	memlock();

	if (sptsegbase + segcnt + NPTPP > hardseglimit)
		panic("sptseggrow: hard limit exceded\n");

	if (sptsegbase + segcnt > softseglimit) {
		printf("sptseggrow: soft limit exceded\n");
		softseglimit = sptsegbase + segcnt;
	}
	freeaddr = btop(kvsegntova(sptsegbase));
	freecnt = 0;
	for (i = 0; i < segcnt; i += NPTPP) {
		availsmem--;
		availrmem--;
		if (freemem < 1 || availsmem < tune.t_minasmem
				|| availrmem < tune.t_minarmem) {
			printf("sptseggrow: no memory\n");
			availsmem++;
			availrmem++;
			memunlock();
			regrele(&sysreg);
			return -1;
		}
		freemem--;
		if ((pfd = pfdall(&sysreg)) == (pfd_t *) 0) {
			printf("sptseggrow: no memory 2\n");
			freemem++;
			availsmem++;
			availrmem++;
			memunlock();
			regrele(&sysreg);
			return -1;
		}

#ifdef NEW_PMMU
		physaddr = ptob(pfdtopf(pfd));
#else
		physaddr = ptob(pfitop(pfd - pfdat));
#endif
		for (j = 0; j < NPTPP; j++) {
			wtste(kstbl[sptsegbase], SEG_RW, NPGPT, physaddr);
			physaddr += NPGPT * sizeof(pte_t);
			freecnt += NPGPT;
			sptsegbase++;
		}
	}
	memunlock();
	regrele(&sysreg);
	TRACE(T_page,("sptseggrow: mfree(sptmap, cnt = 0x%x, addr = 0x%x)\n",
			freecnt, freeaddr));
	mfree(sptmap, freecnt, freeaddr);
	return 0;
}

/*
 * Allocate system virtual address space and
 * allocate or link  pages.
 */
sptalloc(size, mode, base)
register int size, mode, base;
{
	register int svaddr;
	extern int kt_ppmem;
	caddr_t sptreserve();

TRACE(T_page, ("sptalloc(0x%x, 0x%x, 0x%x)\n", size, mode, base));
#ifdef NEW_PMMU
	if (base != -1) {
		svaddr = base;

		while ((svaddr < base + size) &&
				k1to1tbl[ptotbla(svaddr)])
			svaddr += stopg(1);
		if (svaddr >= base + size)
			return ptob(base);
	}
#endif

	if (base == -1 && size == 1) {
		pfd_t *pfd;
		reglock(&sysreg);
		memlock();
		memreserve(&sysreg, 1);
		pfd = pfdall(&sysreg);
#ifdef NEW_PMMU
		svaddr = ptob(pfdtopf(pfd));
		TRACE(T_page, ("sptalloc: 1 page at 0x%x (pfd = 0x%x, page frame 0x%x)\n", svaddr, pfd, pfdtopf(pfd)));
#else
		svaddr = ptob(pfitopf(pfd - pfdat));
		TRACE(T_page, ("sptalloc: 1 page at 0x%x (pfd = 0x%x, page frame 0x%x)\n", svaddr, pfd, pfitopf(pfd - pfdat)));
#endif
		memunlock();
		regrele(&sysreg);
		return svaddr;
	}

	/*
	 * Allocate system virtual address space
	 */
	if ((svaddr = (int) sptreserve(size)) == NULL)
		return NULL;
		
	/*
	 * Allocate and fill in pages
	 */
	(void) sptfill(svaddr, size, mode, base, 0);

	return svaddr;
}

sptfill(svaddr, size, mode, pbase, nosleep)
caddr_t svaddr;
register int size, mode;
int pbase, nosleep;
{
	register int sp;
	register pte_t *p;

	if (nosleep && (pbase == -1) &&
	    (freemem < size || isreglock(&sysreg)))
		return -1;

	sp = svtospg(svaddr);

	if (pbase  ==  -1) {		/* Allocate and fill in pages */
		reglock(&sysreg);
		memlock();
TRACE(T_page, ("sptfill: svaddr = 0x%x, page 0x%x\n", svaddr, sp));
		for (; size; size--, sp++) {
			p = ptopte(sp);
			pg_zero(p);

			if (ptmemall(&sysreg, p, 1))
				panic("sptfill - ptmemall failed");
#ifdef MMB
			{
			register short i;
			for (i = 0; i < (1 << PPTOPSHFT); i++)
				p->pgi[i].pg_pte |= mode;
			}
#else MMB
			p->pgi[0].pg_pte |= mode;
#endif MMB
			invsatb(SYSATB, (caddr_t)(ptosv(sp)), 1);
		}
		memunlock();
		regrele(&sysreg);
	}
	else for (; size; size--, sp++, pbase++) {	/* Map addresses */
		p = ptopte(sp);
		pg_zero(p);
		wtpte(p->pgi, mode, pbase);
		invsatb(SYSATB, (caddr_t)(ptosv(sp)), 1);
	}
	return 0;
}

sptfree(vaddr, size, flag)
register int size;
{
	register i, sp;

#ifdef NEW_PMMU
	if (k1to1tbl[btotbla(vaddr)]) {
		i = sp = vaddr;
		while ((i < sp + ptob(size)) && k1to1tbl[btotbla(i)])
			i += tblatob(1);
		if (i >= sp + ptob(size)) {
			TRACE(T_page,("sptfree:vaddr=%x\n",vaddr));
			if (flag) {
				for (;size; size--, sp += ptob(1)) {
					pfdfree(&sysreg, svtopfd(sp),
							(dbd_t *) NULL);
				}
			}
			return;
		}
	}
#endif
	sp = svtospg(vaddr);
TRACE(T_page,("sptfree:vaddr=%x sp=%x\n",vaddr,sp));

	if (flag) {
		reglock(&sysreg);
		memlock();
		for (i = 0; i < size; i++, sp++) {
			pfree(&sysreg, ptopte(sp), (dbd_t *) NULL);
			pg_zero(ptopte(sp));
		}
		memunlock();
		regrele(&sysreg);
	}
	else for (i = 0; i < size; i++)
		pg_zero(ptopte(sp + i));

	mfree(sptmap, size, svtop(vaddr));
}

ptmemall(rp, base, validate)
reg_t *rp;
pte_t *base;
{
	register pfd_t *pfd;

	ASSERT(mem_lock);
TRACE(T_page >= 2,("ptmemall(0x%x, 0x%x, 0x%x, 0x%x)\n",rp,base,validate,caller()));
	ASSERT(rp->r_lock);

	if(memreserve(rp, 1)  &&  base->pgm[0].pg_v) {    /*  Illegal size?  */
		freemem++;
		return 1;
	}

	ASSERT(mem_lock);
	ASSERT(rp->r_lock);

	if ((pfd = pfdall(rp)) == (pfd_t *) 0)
		return 1;

	pfdmap(base, pfd, validate);
	return 0;
}

static pfd_t *
pfdall(rp)
reg_t		*rp;
{
	register struct pfdat	*pfd;
	register pfd_t	*pfd_next;

	pfd = phead.pf_next;	/* Take page from head of queue */

	TRACE(T_page >= 2, ("pfdall found pfd=0x%x\n", pfd));
	ASSERT (pfd != &phead);
	ASSERT(pfd->pf_flags&P_QUEUE);
	ASSERT(pfd->pf_use == 0);

	/*
	 * Delink a page from free queue and set up pfd
	 */

	pfd_next = pfd->pf_next;
	pfd->pf_prev->pf_next = pfd_next;
	pfd_next->pf_prev = pfd->pf_prev;
	pfd->pf_next = NULL;
	pfd->pf_prev = NULL;
	if (pfd->pf_flags&P_HASH)
		(void) premove(pfd);
	pfd->pf_blkno = BLKNULL;
	pfd->pf_use = 1;
	pfd->pf_flags = 0;
	pfd->pf_rawcnt = 0;
#ifdef UCONTEXT
	pfd->pf_cx = rp->r_cx;
#endif UCONTEXT
	rp->r_nvalid++;
	return pfd;
}

pfdfree(rp, pfd, dbd)
reg_t		*rp;
register struct pfdat *pfd;
register dbd_t	*dbd;
{

	TRACE(T_page >= 2, ("pfdfree: pfd=0x%x dbd=0x%x dbd_type=0x%x\n",
			pfd, dbd, dbd->dbd_type));

	/* Free pages that aren't being used
	 * by anyone else
	 */
	if (--pfd->pf_use == 0) {

		/* Pages that are associated with disk go to end of
		 * queue in hopes that they will be reused.  All
		 * others go to head of queue.
		 */

		if (dbd == NULL || dbd->dbd_type == DBD_NONE) {
			/*
			 * put at head 
			 */
			pfd->pf_next = phead.pf_next;
			pfd->pf_prev = &phead;
			phead.pf_next = pfd;
			pfd->pf_next->pf_prev = pfd;
		} else {
			/*
			 * put at tail 
			 */
			pfd->pf_prev = phead.pf_prev;
			pfd->pf_next = &phead;
			phead.pf_prev = pfd;
			pfd->pf_prev->pf_next = pfd;
		}
		pfd->pf_flags |= P_QUEUE;
#ifdef UCONTEXT
		pfd->pf_cx = 0;
#endif UCONTEXT
		freemem++;
	}
	rp->r_nvalid--;
}

/*
 *	Make a given pte point to a given pfd. 
 *	Validate -> mark page valid
 */

pfdmap(base, pfd, validate)
pte_t *base;
pfd_t *pfd;
int validate;
{
	/*
	 * Insert in page table
	 */

#ifdef MMB
	{ register n;
#ifdef NEW_PMMU
	 register ppfn = pfdtopf(pfd);
#else
	 register ppfn = pfitopf(pfd - pfdat);
#endif
	for (n = 0; n < (1 << PPTOPSHFT); n++)
		base->pgm[n].pg_pfn = ppfn++;
	}
#else MMB
#ifdef NEW_PMMU
	base->pgm[0].pg_pfn = pfdtopf(pfd);
TRACE(T_page >= 2, ("pfdmap: mapping pte 0x%x to page number 0x%x\n", base, pfdtopf(pfd)));
#else
	base->pgm[0].pg_pfn = pfitopf(pfd - pfdat);
TRACE(T_page >= 2, ("pfdmap: mapping pte 0x%x to page number 0x%x\n", base, pfitopf(pfd-pfdat)));
#endif
#endif MMB

	pg_clrmod(base);
	pg_clrprot(base);

	if (npteget(base)->pgm[0].pg_cw)
		pg_setprot(base, PG_RO);
	else pg_setprot(base, PG_RW);

	if (validate)
		pg_setvalid(base);
}

/*
 * Shred page table and update accounting for swapped
 * and resident pages.
 *	rp	-> ptr to the region structure.
 *	pt	-> ptr to the first pte to free.
 *	dbd	-> ptr to disk block descriptor.
 *
 * Called with mem_lock set and returns the same way.
 */

pfree(rp, pt, dbd)
reg_t		*rp;
register pte_t	*pt;
register dbd_t	*dbd;
{
	ASSERT(mem_lock);
	TRACE(T_page >= 2, ("pfree: pt=0x%x\n", pt));

	/* 
	 * Zap page table entries
	 */

	if (pt->pgm[0].pg_v)
#ifdef NEW_PMMU
		pfdfree(rp, pftopfd(pt->pgm[0].pg_pfn), dbd);
#else
		pfdfree(rp, &pfdat[pftopfi(pt->pgm[0].pg_pfn)], dbd);
#endif

	if (dbd) {
		if (dbd->dbd_type == DBD_SWAP && swfree1(dbd) == 0) {
			(void) pbremove(rp, dbd);
		}
		dbd->dbd_type = DBD_NONE;
	}
	pg_zero(pt);
}

/*
 * Allocate system virtual address space but
 * don't allocate any physical pages (this is
 * done later with sptfill().  Expects a size
 * in pages, and returns a virtual address.
 */
caddr_t
sptreserve(size)
register int size;
{
	register sp;

TRACE(T_page, ("malloc(0x%x, 0x%x) ", sptmap, size));
	if ( ! (sp = malloc(sptmap, size)))
		sptseggrow(size);
	if ( ! sp && ! (sp = malloc(sptmap, size)))  {
		printf("sptreserve: No kernel virtual space\n");
#if OSDEBUG == YES
		printf("size=%d, mode=%d, base=%d\n",size, mode, base);
#endif
		return NULL;
	}
TRACE(T_page, ("returned page number 0x%x\n", sp));
	return (caddr_t) ptosv(sp);
}

pfdtopf(pfd)
register struct pfdat *pfd;
{
	register struct pfdat_desc *pfdd = pfdat_desc;

	for (; pfdd; pfdd = pfdd->pfd_next)
		if (pfd >= pfdd->pfd_head && 
				pfd < pfdd->pfd_head + pfdd->pfd_pfcnt)
			return pfdd->pfd_pfnum + (pfd - pfdd->pfd_head);

	return 0;
}

struct pfdat *
pftopfd(pf)
register u_int pf;
{
	register struct pfdat_desc *pfdd = pfdat_desc;

	for (; pfdd; pfdd = pfdd->pfd_next)
		if (pf >= pfdd->pfd_pfnum && 
				pf < pfdd->pfd_pfnum + pfdd->pfd_pfcnt)
			return pfdd->pfd_head + (pf - pfdd->pfd_pfnum);

	return (struct pfdat *) 0;
}

#endif NEW_PMMU
/* <@(#)page.c	1.5> */
