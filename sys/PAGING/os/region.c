#ifndef lint	/* .../sys/PAGING/os/region.c */
#define _AC_NAME region_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.7 87/12/18 17:43:54}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.7 of region.c on 87/12/18 17:43:54";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

/*	@(#)region.c	UniPlus VVV.2.1.17	*/

#ifdef HOWFAR
extern int T_region;
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
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/pfdat.h"
#include "sys/signal.h"
#include "sys/dir.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vnode.h"
#include "sys/var.h"
#include "sys/buf.h"
#include "sys/debug.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/tuneable.h"
#ifdef NEW_PMMU
#include "sys/pmmu.h"
#endif
#endif lint

/* reg_t	nullregion; */
preg_t	nullpregion;
int	rlist_lock;

void
reginit()
{
	register reg_t *rp;

	rfree.r_forw = &rfree;
	rfree.r_back = &rfree;

	ractive.r_forw = &ractive;
	ractive.r_back = &ractive;

	for (rp = region; rp < &region[v.v_region]; rp++) {
		rp->r_back = rfree.r_back;
		rp->r_forw = &rfree;
		rfree.r_back->r_forw = rp;
		rfree.r_back = rp;
	}
}


#ifndef reglock

reglock(rp)
register reg_t *rp;
{
	register int s;

	s = splhi();
	while (rp->r_lock) {
		rp->r_lock |= 2;
		sleep(&rp->r_lock, PZERO);
	}
	rp->r_lock = 1;
	splx(s);
}

#endif reglock


#ifndef regrele

regrele(rp)
register reg_t *rp;
{
	ASSERT(rp->r_lock);
	if (rp->r_lock&2)
		wakeup(&rp->r_lock);
	rp->r_lock = 0;
}

#endif regrele

#ifndef rlstlock

rlstlock()
{
	register int s;

	s = splhi();
	while (rlist_lock) {
		rlist_lock |= 2;
		sleep(&rlist_lock, PZERO);
	}
	rlist_lock = 1;
	splx(s);
}

#endif rlstlock

#ifndef rlstunlock

rlstunlock()
{
	ASSERT(rlist_lock);
	if (rlist_lock&2)
		wakeup(&rlist_lock);
	rlist_lock = 0;
}

#endif rlstunlock

/*
 * Allocate a new region.
 * Returns a locked region pointer or NULL on failure
 * The region is linked into the active list.
 */

reg_t *
allocreg(vp, type)
register struct vnode	*vp;
{
	register reg_t *rp;

	rlstlock();

	if ((rp = rfree.r_forw) == &rfree) {
		rlstunlock();
		printf("Region table overflow\n");
		u.u_error = ENOMEM;
		return(NULL);
	}
	/*
	 * Remove from free list
	 */
	rp->r_back->r_forw = rp->r_forw;
	rp->r_forw->r_back = rp->r_back;

	/* Initialize region fields and bump vnode reference
	 * count. Vnode is locked by the caller
	 */

/* BOBJ: why doe the vax code do a
	rp->r_incore = 0; ????
 */
	rp->r_type = type;
	rp->r_vptr = vp;
	reglock(rp);

	if(vp != NULL)
		VN_HOLD(vp);

	/*
	 * Link onto active list
	 */
	rp->r_forw = ractive.r_forw;
	rp->r_back = &ractive;
	ractive.r_forw->r_back = rp;
	ractive.r_forw = rp;

	rlstunlock();
	
	return(rp);
}

/*
 * Free an unused region table entry.
 */
void
freereg(rp)
register reg_t *rp;	/* pointer to a locked region */
{
	register struct vnode	*vp;
	register int		i;
	register int		lim;
	register int		size;
	register pte_t		*pt;
	register int		tsize;

	ASSERT(rp->r_lock);

	/* If the region is still in use, then don't free it.
	 */

	vp = rp->r_vptr;

	if(rp->r_refcnt != 0){
		regrele(rp);
		return;
	}

	/*
	 * Decrement use count on associated vnode
	 * Vnode is locked by the caller.
	 */

	if(vp) {
		if (rp->r_type == RT_STEXT)
			vp->v_flag &= ~(VTEXT|VTEXTMOD);
		VN_RELE(vp);
	}

	/*	Free the memory pages and the page tables and
	 *	disk use maps.  These latter are always allocated
	 *	together in pairs in a contiguous 128 word piece
	 *	of kernel virtual address space.  Note that the
	 *	pfree for the first page table is special because
	 *	pages may not have been allocated from the beginning
	 *	of the segment.  The last page table is also special
	 *	since it may not have been fully allocated. 
	 *	NOTE: phys() regions don't actually have pages out there
	 *		to free. So we skip that part. This means we
	 *		don't need "pinvalidate()" anymore
	 */
	
	tsize = rp->r_pgsz;
	lim = ptos(tsize);

	if (rp->r_stack) {
		for(i = 0  ;  i < lim  ;  i++){
			if ((rp->r_type & RT_PHYSCALL) == 0) {
				pt = rp->r_list[NSEGP - 1 - i];
				size = tsize - stopg(i);
				if(size > NPGPT)
					size = NPGPT;
				pt += (NPGPT - size);
				memlock();
				while (size--) {
#ifdef NEW_PMMU
					pfree(rp, pt, dbdget(pt));
#else
					pfree(rp, pt, dbdget(pt), 1);
#endif NEW_PMMU
					pt++;
				}
				memunlock();
			}
			uptfree(rp->r_list[NSEGP - 1 - i], 2);
		}
	} else {
		for(i = 0  ;  i < lim  ;  i++){
			if ((rp->r_type & RT_PHYSCALL) == 0) {
				pt = rp->r_list[i];
				size = tsize - stopg(i);
				if(size > NPGPT)
					size = NPGPT;
				memlock();
				while (size--) {
#ifdef NEW_PMMU
					pfree(rp, pt, dbdget(pt));
#else
					pfree(rp, pt, dbdget(pt), 1);
#endif NEW_PMMU
					pt++;
				}
				memunlock();
			}
			uptfree(rp->r_list[i], 2);
		}
	}

#ifdef NEW_PMMU
	if ((rp->r_type & RT_PHYSCALL) == 0) {
	TRACE(T_availmem,("freereg: returning %d avail[S]mem pages\n", 
				tsize));
	availsmem += tsize;
	ASSERT(rp->r_noswapcnt >= 0  &&  rp->r_noswapcnt <= 1);
	if (rp->r_noswapcnt) {
		TRACE(T_availmem,("freereg: returning %d avail[R]mem pages\n", tsize));
		availrmem += tsize;
	}
	}
#endif

	/*
	 * Free the list.
	 */
	
	uptfree(rp->r_list, rp->r_listsz);

	/*
	 * Remove from active list
	 * and clean up region fields.
	 */

	rlstlock();

	rp->r_back->r_forw = rp->r_forw;
	rp->r_forw->r_back = rp->r_back;

	rp->r_flags = 0;
	rp->r_listsz = 0;
	rp->r_pgsz = 0;
	rp->r_stack = 0;
	rp->r_nvalid = 0;
	rp->r_type = 0;
	rp->r_filesz = 0;
#ifdef NEW_PMMU
	rp->r_noswapcnt = 0;
#endif
	rp->r_list = NULL;

	regrele(rp);
/* BOBJ: why does vax code do a
	*rp = nullregion; ???
 */

	/*
	 * Link into free list
	 */

	rp->r_forw = rfree.r_forw;
	rp->r_back = &rfree;
	rfree.r_forw->r_back = rp;
	rfree.r_forw = rp;

	rlstunlock();
}

/*
 * Attach a region to a process' address space
 */
preg_t *
attachreg(rp, up, vaddr, type, prot)
register reg_t	*rp;	/* pointer to region to be attached */
register user_t	*up;	/* pointer to u-block (needed by fork) */
register caddr_t vaddr;	/* virtual address to attach at */
register int type;	/* Type to make the pregion. */
register int prot;	/* permissions for segment table entries. */
{
	register preg_t *prp;


	ASSERT(rp->r_lock);

TRACE(T_region, ("attachreg(0x%x, 0x%x, 0x%x, 0x%x,\n", rp, up, vaddr, type));
TRACE(T_region, (" 0x%x)\n", prot));
	/*	Check attach address.
	 *	It must be segment aligned.
	 */

	 if((int)vaddr & SOFFMASK) {
		u.u_error = EINVAL;
		return(NULL);
	 }

	/*	Allocate a pregion.  We should always find a
	 *	free pregion because of the way the system
	 *	is configured.
	 */

	prp = findpreg(up->u_procp, PT_UNUSED);
	if(prp == NULL) {
		u.u_error = EMFILE;
		return(NULL);
	}

	/*	init pregion
	 */

	prp->p_reg = rp;
	prp->p_regva = vaddr;
	prp->p_type = type;

	if (type == PT_STACK)
		rp->r_stack = 1;
	else
		rp->r_stack = 0;
	if(prot == SEG_RO)
		prp->p_flags |= PF_RDONLY;

	/*	Check that region does not go beyond end of virtual
	 *	address space.
	 */


	if(chkgrowth(up, prp, 0, rp->r_pgsz)){

		/* Region beyond end of address space.
		 * Undo what has been done so far
		 */

		*prp = nullpregion;
		u.u_error = EINVAL;
		return(NULL);
	}

	/*	Load the segment table.
	 */

#ifdef STKDEBUG
	if (rp->r_stack)
		TRACE(T_region, ("attachreg: PT_STACK: reg=0x%x regva=0x%x\n",
			prp->p_reg, prp->p_regva));
#endif STKDEBUG
	loadstbl(up, prp, 0);

/* BOBJ: why does vax code do a
	++rp->r_incore;
 */
	++rp->r_refcnt;
	up->u_procp->p_size += rp->r_pgsz;
	return(prp);
}

/*
 * Detach a region from the current process' address space
 */
void
detachreg(prp, up)
register preg_t *prp;
register user_t *up;
{
	register reg_t		*rp;
	register int		i;
	register int		*st;

	rp = prp->p_reg;

	ASSERT(rp);
	ASSERT(rp->r_lock);

	/*
	 * Invalidate segment table entries pointing at the region
	 */
	if (!rp->r_stack) {
		st = (int *)snum(prp->p_regva);
	} else {
		st = (int *)(NSEGP - 1 - ptots(rp->r_pgsz));
#ifdef STKDEBUG
	TRACE(T_region, ("detachreg: PT_STACK st=0x%x\n", st));
#endif STKDEBUG
	}
	for(i = 0;  i < rp->r_pgsz ; i += NPGPT, st = (int *)((int)st+1))
		wtste(up->u_stbl[(int)st], 0, 0, 0);
	
	/*	Decrement process size by size of region.
	*/

	up->u_procp->p_size -= rp->r_pgsz;

	/*
	 * Decrement use count and free region if zero
	 * and RG_NOFREE is not set, otherwise unlock.
	 */

/* BOBJ: why does vax code do a
	--rp->r_incore;
 */
	if(--rp->r_refcnt == 0 && !(rp->r_flags & RG_NOFREE)){
		freereg(rp);
	} else {
		regrele(rp);
	}
	
	/*	Clear the proc region we just detached.
	 */
	
	for(i = prp - up->u_procp->p_region; i < pregpp-1; i++, prp++) {
		*prp = *(prp+1);
	}
	*prp = nullpregion;
}

/*
 * Duplicate a region
 */
reg_t *
dupreg(rp)
register reg_t *rp;
{
	register int	i;
	register int	j;
	register int	size;
	register pte_t	*ppte;
	register pte_t	*cpte;
	register pte_t	**plp;
	register pte_t	**clp;
	register reg_t	*rp2;
	extern pte_t	*uptalloc();
	register int	ptincr;


	ASSERT(rp->r_lock);

	/* If region is shared, there is no work to do.
	 * Just return the passed region.  The region reference
	 * counts are incremented by attachreg
	 */


TRACE(T_region,("dupreg:1\n"));
	if(rp->r_type != RT_PRIVATE)
		return(rp);

#ifdef NEW_PMMU
	availsmem -= rp->r_pgsz;	/* Allocate space for the new region's rlist */

	if (availsmem < tune.t_minasmem) {
		TRACE(T_region, ("dupreg - can't get %d pages\n", rp->r_pgsz));
		availsmem += rp->r_pgsz;
		u.u_error = EAGAIN;
		return(NULL);
	}
	TRACE(T_availmem,("dupreg: taking %d avail[S]mem pages\n", rp->r_pgsz));
#endif

	/*
	 * Need to copy the region.
	 * Allocate a region descriptor
	 */

	if((rp2 = allocreg(rp->r_vptr, rp->r_type)) == NULL) {
#ifdef NEW_PMMU
		availsmem += rp->r_pgsz;
#endif
		u.u_error = EAGAIN;
		return(NULL);
	}

TRACE(T_region,("dupreg:2\n"));
	/*	Allocate a list for the new region.
	 */

	rp2->r_listsz = rp->r_listsz;
	rp2->r_list = (pte_t **)uptalloc(rp2->r_listsz);
	if (rp2->r_list == (pte_t **)-1) {
		rp2->r_listsz = 0;
		rp2->r_list = NULL;
#ifdef NEW_PMMU
		availsmem += rp->r_pgsz;
#endif
		freereg(rp2);
		u.u_error = EAGAIN;
		return(NULL);
	}

TRACE(T_region,("dupreg:j\n"));

	/*
	 * Copy pertinent data to new region
	 */

	rp2->r_pgsz = rp->r_pgsz;
	rp2->r_filesz = rp->r_filesz;
	rp2->r_nvalid = rp->r_nvalid;
	rp2->r_stack = rp->r_stack;

	/* Scan the parents page table  list and fix up each page table.
	 * Allocate a page table and map table for the child and
	 * copy it from the parent.
	 */

TRACE(T_region,("dupreg:ptos(rp->r_pgsz)=%x\n",ptos(rp->r_pgsz)));

	for(i = 0  ;  i < ptos(rp->r_pgsz)  ;  i++){
TRACE(T_region,("dupreg:4\n"));
		if (!rp->r_stack) {
			plp = &rp->r_list[i];
			clp = &rp2->r_list[i];
			if ((cpte = uptalloc(2)) == (pte_t *)-1) {
				rp2->r_pgsz = stopg(i);
				freereg(rp2);
#ifdef NEW_PMMU
				availsmem += rp->r_pgsz;
#endif
				u.u_error = EAGAIN;
				return(NULL);
			}
			*clp = cpte;
			ppte = *plp;
/*
			rp2->r_list[i] = cpte;
			ppte = rp->r_list[i];
*/
			ptincr = 1;
		} else {
TRACE(T_region,("dupreg:5\n"));
			plp = &rp->r_list[NSEGP - 1 - i];
			clp = &rp2->r_list[NSEGP - 1 - i];
			if ((*clp = uptalloc(2)) == (pte_t *)-1) {
				rp2->r_pgsz = stopg(i);
				freereg(rp2);
#ifdef NEW_PMMU
				availsmem += rp->r_pgsz;
#endif
				u.u_error = EAGAIN;
				return(NULL);
			}
			cpte = *clp + NPGPT - 1;
			ppte = *plp + NPGPT - 1;
/*
			rp2->r_list[NSEGP - 1 - i] = cpte;
			ppte = &rp->r_list[NSEGP - 1 - i][NPGPT - 1];
			cpte += NPGPT - 1;
*/
			ptincr = -1;
		}
		size = rp->r_pgsz - stopg(i);
		if(size > NPGPT)
			size = NPGPT;
TRACE(T_region,("dupreg:size=%x\n",size));
		for(j = 0  ;  j < size  ;  j++, ppte += ptincr, cpte += ptincr){
#ifdef MMB
			uint	map;
#else MMB
			dbd_t	map;
#endif MMB
TRACE(T_region,("dupreg:cw ppte=%x,(ppte)=%x\n",ppte, ppte->pgi[0].pg_pte));
			if(!(ppte->pgi[0].pg_pte&PG_RO)) {
#ifdef AWS
TRACE(T_region,("dupreg:cw %x\n",ppte));
#else AWS
TRACE(T_region,("dupreg:cw %x p(%x)\n",ppte,svirtophys(ppte)));
#endif AWS
				pg_clrprot(ppte);
				pg_setprot(ppte, PG_RO);
				pg_setcw(ppte);
			}
			*cpte = *ppte;
			if(ppte->pgm[0].pg_v){
				struct pfdat *pfd;

#ifdef NEW_PMMU
				pfd = pftopfd(ppte->pgm[0].pg_pfn);
#else
				pfd = pfdat + pftopfi(ppte->pgm[0].pg_pfn);
#endif
				ASSERT(pfd->pf_use > 0); 
				memlock();
				pfd->pf_use++;
				memunlock();
			}
#ifdef MMB
			map = *((uint *)(dbdget(ppte)));
			if(((dbd_t *)&map)->dbd_type == DBD_SWAP)
#else MMB
			map = *(dbdget(ppte));
			if(map.dbd_type == DBD_SWAP)
#endif MMB
			{
				ASSERT(swpuse((dbd_t *)&map) != 0);
				if(!swpinc(((dbd_t *)&map), "dupreg")){
					(dbdget(cpte))->dbd_type = DBD_NONE;
					freereg(rp2);
#ifdef NEW_PMMU
					availsmem += rp->r_pgsz;
#endif NEW_PMMU
					u.u_error = EAGAIN;
					return(NULL);
				}
			}
#ifdef MMB
			*((uint *)(dbdget(cpte))) = map;
#else MMB
			*(dbdget(cpte)) = map;
#endif MMB
		}
	}
	return(rp2);
}

/*
 * Change the size of a region
 *  change == 0  -> no-op
 *  change  < 0  -> shrink
 *  change  > 0  -> expand
 * For expansion, you get (fill) real pages (change-fill) demand zero pages
 * For shrink, the caller must flush the ATB
 * Returns 0 on no-op, -1 on failure, and 1 on success.
 */

growreg(prp, change, type)
register preg_t *prp;
{
	register pte_t	*pt;
	register int	i;
	register reg_t	*rp;
	register int	size;
	register int	osize;
	register int	rlim;
	register int	ptincr;

TRACE(T_region, ("growreg(0x%x, 0x%x, 0x%x)", prp, change, type));
	rp = prp->p_reg;
TRACE(T_region, (" rp=0x%x\n", rp));

#ifdef NEW_PMMU
	ASSERT(rp->r_noswapcnt >=0);
#endif

	if(change == 0)
		return(0);
	
	osize = rp->r_pgsz;

	if(change < 0) {

		/*	The region is being shrunk.  Compute the new
		 *	size and free up the unneeded space.
		 */
#ifdef NEW_PMMU
		if ( ! (rp->r_type & RT_PHYSCALL)) {
			TRACE(T_availmem,("growreg: returning %d avail[S]mem pages\n", 
						change));
			availsmem -= change;    /* change < 0.  */

			if (rp->r_noswapcnt) {
				TRACE(T_availmem,("growreg: returning %d avail[R]mem pages\n", change));
				availrmem -= change;
			}
		}
#endif

		if (!rp->r_stack) {
			i = ptots(osize + change);
			rlim = ptos(osize);
		} else {
			i = NSEGP - ptos(osize + change);
			rlim = NSEGP - ptots(osize);
TRACE(T_region, ("growreg: shrink stack: i=%d rlim=%d\n", i, rlim));
		}

		for(  ;  i < rlim ;  i++){
			/*	Free up the allocated pages for
			 *	this segment.
			 */

			pt = rp->r_list[i];
			size = osize - stopg(i);
			if(size > NPGPT)
				size = NPGPT;
			if(osize + change > stopg(i)){
				size -= osize + change - stopg(i);
				if (!rp->r_stack)
					pt += osize + change - stopg(i);
				else {
					pt += NPGPT - (osize - stopg(i));
TRACE(T_region, ("growreg: shrink stack: pt=0x%x\n", pt));
				}
			}
			memlock();
			while (size--) {
#ifdef NEW_PMMU
				pfree(rp, pt, dbdget(pt));
#else
				pfree(rp, pt, dbdget(pt), 1);
#endif NEW_PMMU
				pt++;
			}
			memunlock();
		}

		/*	Free up the page tables which we no
		 *	longer need.
		 */

		(void) ptexpand(rp, change);
		if (rp->r_stack) {	/* update attach point for stack */
			prp->p_regva = (caddr_t)stob(ptots(stopg(NSEGP) -
							(rp->r_pgsz+change)));
TRACE(T_region, ("growreg: shrunk prp->p_regva=0x%x\n", prp->p_regva));
		}
	} else {
		/*	We are expanding the region.  Make sure that
		 *	the new size is legal and then allocate new
		 *	page tables if necessary.
		 */

#ifdef NEW_PMMU
		if ( ! (rp->r_type & RT_PHYSCALL)) {
			availsmem -= change;

			if (availsmem < tune.t_minasmem) {
				TRACE(T_region,
					("growreg - can't get %d pages\n",
					change));
				availsmem += change;
				u.u_error = EAGAIN;
				return(-1);
			}

			if (rp->r_noswapcnt) {
				availrmem -= change;
				if (availrmem < tune.t_minarmem) {
					TRACE(T_region, ("growreg - can't get %d pages\n", change));
					availrmem += change;
					availsmem += change;
					u.u_error = EAGAIN;
					return(-1);
				}
			}
		}
#endif NEW_PMMU

TRACE(T_region, ("growreg: chkgrowth(0x%x, 0x%x, 0x%x, 0x%x)\n",
&u, prp, osize, osize+change));
		if (chkgrowth(&u, prp, osize, osize + change)
				|| ptexpand(rp, change)) {
TRACE(T_region, ("growreg: chkgrowth/ptexpand failed\n"));
			u.u_error = ENOMEM;
#ifdef NEW_PMMU
			if ( ! (rp->r_type & RT_PHYSCALL)) {
				availsmem += change;
				if (rp->r_noswapcnt)
					availrmem += change;
			}
#endif
			return(-1);
		}
		if (rp->r_stack) {
			/* update attach point for stack after ptexpand() */
			prp->p_regva = (caddr_t)stob(ptots(stopg(NSEGP) -
							(rp->r_pgsz+change)));
TRACE(T_region, ("growreg: expanded prp->p_regva=0x%x\n", prp->p_regva));
		}

		/*	Initialize the new page tables and allocate
		 *	pages if required.
		 */

		i = ptots(osize);
		for( ; i <  ptos(osize + change); i++){
			size = osize + change - stopg(i);
			if(size >= NPGPT)
				size = NPGPT;
			if (!rp->r_stack) {
				pt = rp->r_list[i];
				ptincr = 1;
				if(osize > stopg(i)) {
					size -= osize - stopg(i);
					pt += osize - stopg(i);
				}
			} else {
				pt = &rp->r_list[NSEGP - 1 - i][NPGPT - 1];
				ptincr = -1;
				if(osize > stopg(i)) {
					size -= osize - stopg(i);
					pt -= osize - stopg(i);
				}
			}
TRACE(T_region, ("growreg: expand: osize=0x%x size=0x%x pt=0x%x *pt=0x%x\n",
osize,size,pt,*pt));

			while(--size >= 0){
				pg_zero(pt);
				pg_setprot(pt, PG_RW);
TRACE(T_region, ("growreg: pteset'ed. pt=0x%x *pt=0x%x\n", pt, *pt));
				(dbdget(pt))->dbd_type = type;
				pt += ptincr;
			}
		}
	}

TRACE(T_region, ("loadstbl(0x%x, 0x%x, 0x%x)\n", &u, prp, change));
	loadstbl(&u, prp, change);
TRACE(T_region, ("growreg: loadstbl retd\n"));

	rp->r_pgsz += change;
	u.u_procp->p_size += change;
	return(1);
}

/*
 * Check that grow of a pregion is legal
 */

chkgrowth(up, prp, osize, nsize)
register user_t	*up;
register preg_t	*prp;
register int	osize;	/* Old size in pages. */
register int	nsize;	/* New size in pages. */
{
	register	*st1;	/* First segment table entry	*/
				/* following old size.		*/
	register	*st2;	/* First segment table entry	*/
				/* following new size.		*/


	/*	Set up pointers to the segment table.
	 */

	if (prp->p_reg->r_stack) {
TRACE(T_region,("chkgrowth: PT_STACK\n"));
		if (ptos(nsize) > NSEGP) {
			return(-1);
		}
		st2 = (int *)(NSEGP - 1 - ptots(osize));
		st1 = (int *)(NSEGP - 1 - ptots(nsize));
	} else {
		if(btotp(prp->p_regva + ptob(nsize)) >= (stopg(NSEGP) -1))
			return(-1);
		st1 = (int *)(snum(prp->p_regva) + ptos(osize));
		st2 = (int *)(snum(prp->p_regva) + ptos(nsize));
	}

	/*	Make sure we are not trying to grow the
	**	stack beyond the end f the virtual address
	**	space of a process.
	*/


	/*	Last page of last segment is not used.
	 * 	This prevents address from wrapping.
	 */
/* BOBJ: is this necessary???? I don't think this is true for ccv */

	/*	If we are not growing into a new segment,
	 *	then everything is O.K.
	 */
	
	if(st2 < st1)
		return(0);

	/*	If we are checking a null region, then just make
	 *	sure that we are not trying to attach on top of
	 *	an already attached region.
	 */
	
	if(nsize == 0){
		if(steprot(up->u_stbl[(int)st1]) & S_VALID)
			return(-1);
		else
			return(0);
	}

	/*	We are growing into at least one more segment.
	 *	Make sure that all of the new segments are unused;
	 *	that is , not part of another region.
	 */
	for( ;  st1 < st2  ;  st1 = (int *)((int)st1 + 1))
		if(steprot(up->u_stbl[(int)st1]) & S_VALID)
			return(-1);
TRACE(T_region,("chkgrowth:ok\n"));
	return(0);
}

loadstbl(up, prp, change)
user_t		*up;
preg_t		*prp;
register int	change;
{
	register reg_t	*rp;
	register int	st;
	register caddr_t regva;
	register int	size;
	register pte_t	**lp;

	rp = prp->p_reg;
	regva = prp->p_regva;

	if(change < 0) {
		register int	stlim;

		size = rp->r_pgsz;
		if (rp->r_stack) {
			stlim = NSEGP-ptos(size+change);
			st = NSEGP+ptos(size);
TRACE(T_region, ("loadstbl stack: st=0x%x stlim=0x%x\n", st, stlim));
		} else {
			st= snum((int)regva) + ptos(size + change);
			stlim = snum((int)regva) + ptos(size);
		}

		for(; st < stlim ; st++) {
			TRACE(T_region, ("loadstbl1: u_stbl[%d] == 0\n", st));
			wtste(up->u_stbl[st], 0, 0, 0);
		}
	} else {
		register pte_t	**lplim;
		register int	physaddr;
		register int	prot;

		prot = prp->p_flags & PF_RDONLY ? SEG_RO : SEG_RW;
		if (rp->r_stack) {
			st = NSEGP-ptos(rp->r_pgsz + change);
			lp= &rp->r_list[st];
			lplim= &rp->r_list[NSEGP];
TRACE(T_region, ("loadstbl stack: st=0x%x lp=0x%x lplim=0x%x\n",st,lp,lplim));
		} else {
			st = snum((int)regva);
			lp = rp->r_list;
			lplim = &lp[ptos(rp->r_pgsz + change)];
TRACE(T_region, ("loadstbl non-stack: st=0x%x lp=0x%x lplim=0x%x\n",st,lp,lplim));
		}

		for (; lp < lplim ; lp++, st++) {
#ifdef AWS
			wtste(up->u_stbl[st], prot, NPGPT, *lp);
TRACE(T_region, ("loadstbl2: *lp=0x%x u_stbl[0x%x]=0x%x\n",
*lp, st, up->u_stbl[st].segm.ld_addr));
#else AWS
			physaddr = (int) svirtophys(*lp);
			wtste(up->u_stbl[st], prot, NPGPT, physaddr);
TRACE(T_region, ("loadstbl2: *lp=0x%x physaddr=0x%x u_stbl[0x%x]=0x%x\n",
*lp, physaddr, st, up->u_stbl[st].segm.ld_addr));
#endif AWS
		}
	}
}


/*
 * Expand user page tables for a region 
 */
ptexpand(rp, change)
reg_t		*rp;
{
	register pte_t	**lp;
	register int	osize;
	register int	nsize;
	register pte_t	**lp1;
	register pte_t	**lp2;
	extern pte_t	*uptalloc();

	/* Calculate the new size in pages.
	 */

	osize = rp->r_pgsz;
	nsize = osize + change;

	/*	If we are shrinking the region, then free up
	 *	the page tables and map tables.  Use a smaller
	 *	list if possible.
	 */

	if(ptos(nsize) < ptos(osize)){
		if (rp->r_stack) {
			/* (bottom of stack) - (stack size) == (top of stack) */
			lp = &rp->r_list[NSEGP - ptos(osize)];
			lp2 = &rp->r_list[NSEGP - ptos(nsize)];
		} else {
			lp = &rp->r_list[ptos(nsize)];
			lp2 = &rp->r_list[ptos(osize)];
		}
		for(  ;  lp < lp2;  lp++){
			uptfree(*lp, 2);
			*lp = 0;
		}
		if (nsize <= 0) {
			uptfree(rp->r_list, rp->r_listsz);
			rp->r_listsz = 0;
			rp->r_list = (pte_t **)0;
		}
	}

	/*	If the region shrunk, then we are done.
	 */

	if(change <= 0){
		return(0);
	}
	
	/*	If the region grew, make sure an r_list has been allocated. 
	 */
	
#define RLISTSZ	(((NSEGP*(sizeof(pte_t *))) + PTSIZE-1) >> PTSZSHFT)

	if (!rp->r_list) {
		lp2 = (pte_t **)uptalloc(RLISTSZ);
		if (lp2 == (pte_t **)-1) 
			return(-1);
		else {
			rp->r_list = lp2;
			rp->r_listsz = RLISTSZ;
		}
	}

	/*
	 * Allocate a new set of page tables and disk maps.
	 */

	if (rp->r_stack) {
		lp1 = &rp->r_list[NSEGP - ptos(nsize)];
		lp2 = &rp->r_list[NSEGP - ptos(osize)];
TRACE(T_region, ("ptexpand:stack: lp=&rp->r_list[0x%x]=0x%x lp2=&rp->r_list[0x%x]=0x%x\n",
NSEGP-ptos(nsize), lp, NSEGP-ptos(osize), lp2));
	} else {
		lp1 = &rp->r_list[ptos(osize)];
		lp2 = &rp->r_list[ptos(nsize)];
	}

	for(lp = lp1 ; lp < lp2 ; lp++) {

		/*
		 * Release this region while we grow the system
		 * region to contain its new page tables.  To get
		 * this memory, we may have to go to sleep and vhand
		 * may have to steal pages from this region.  The
		 * vhand routines use r_pgsz to determine the current
		 * size of the region, and r_pgsz will not be increased
		 * from its original value until the end of growreg.
		 *
		 * This fixes a deadlock which occurred when trying to
		 * load a data region with huge data and bss sections.
		 * In particular, the system previously hung when
		 * trying to expand the page tables for a huge bss
		 * area (say 100 Meg), after having used up all of
		 * physical memory with initialized data.
		 */

		regrele(rp);
		*lp = (pte_t *)uptalloc(2);
		reglock(rp);

		if (*lp == (pte_t *)-1) {
			/*
			 * Release what we've grabbed so far.  This is
			 * necessary because we haven't changed r_pgsz yet.
			 */
			for (; lp1 < lp ; lp1++) {
				uptfree(*lp1, 2);
				*lp1 = 0;
			}
			return(-1);
		}
		TRACE(T_region,("ptexpand: new pgtbl uptalloc'ed=0x%x\n", *lp));
	}

	return(0);
}

loadreg(prp, vaddr, vp, off, count)
register preg_t		*prp;
caddr_t			vaddr;
register struct vnode	*vp;
{
	register struct user *up;
	register reg_t	*rp;
	register int	gap;

	/*	Grow the region to the proper size to load the file.
	 */

	up = &u;
	rp = prp->p_reg;
	ASSERT(rp->r_lock);
	gap = vaddr - prp->p_regva;

	if(growreg(prp, (int)btotp(gap), DBD_NONE) < 0) 
		return(-1);
	if(growreg(prp, (int)(btop(count+gap) - btotp(gap)), DBD_DFILL) < 0) 
		return(-1);


	/*	We must unlock the region here because we are going
	 *	to fault in the pages as we read them.  No one else
	 *	will try to use the region before we finish because
	 *	the RG_DONE flag is not set yet.
	 */
	regrele(rp);

	/*	Set up to do the I/O.
	 */

	up->u_error =
	    vn_rdwr(UIO_READ, vp, vaddr, count,
		off, UIOSEG_USER, IO_UNIT, (int *) 0);
	if (up->u_error) {
		reglock(rp);
		return(-1);
	}


	/*	Clear the last (unused)  part of the last page.
	 */
	
	vaddr += count;
	count = ptob(1) - poff(vaddr);

	if(count > 0  &&  count < ptob(1))
		/* BOBJ: how about a little error checking */
		if (uclear(vaddr, count) == -1) {
			u.u_error = EFAULT;
			reglock(rp);
			return(-1);
		}

	reglock(rp);
	rp->r_flags |= RG_DONE;
	if (rp->r_flags & RG_WAITING) {
		rp->r_flags &= ~RG_WAITING;
		wakeup(&rp->r_flags);
	}

	return(0);
}

mapreg(prp, vaddr, vp, off, count)
preg_t	*prp;
caddr_t		vaddr;
struct vnode	*vp;
int		off;
register int	count;
{
	register int	i;
	register int	j;
	register int	blkspp;
	register reg_t	*rp;
	int		bsize;
	int		gap;
	int		seglim;
	dbd_t		*dbd;

	/*	If the block number list is not built,
	 *	then build it now.
	 */
	
	if(vp->v_map == 0)
		if (mapfile(vp) < 0)
			return(-1);

	/*	Get region pointer and effective device number.
	 */

	rp = prp->p_reg;
	ASSERT(rp->r_lock);
	bsize = vp->v_vfsp->vfs_bsize;

	/*	Compute the number of file system blocks in a page.
	 *	This depends on the file system block size.
	 */

	blkspp = NBPP/bsize;

	/*	Allocate invalid pages for the gap at the start of
	 *	the region and demand-fill pages for the actual
	 *	text.
	 */
	
	gap = vaddr - prp->p_regva;
	if(growreg(prp, (int)btotp(gap), DBD_NONE) < 0)
		return(-1);
	if(growreg(prp, (int)(btop(count+gap) - btotp(gap)), DBD_DFILL) < 0)
		return(-1);
	
	rp->r_filesz = count + off;
	
	/*	Build block list pointing to map table.
	 */

	gap = btotp(gap);  /* Gap in pages. */
	off = btotp(off) * blkspp;  /* File offset in blocks. */
	i = ptots(gap);
	seglim = ptos(rp->r_pgsz);

	for(  ;  i < seglim  ;  i++){
		register int	lim;
		register pte_t	*pt;

		if(gap > stopg(i))
			j = gap - stopg(i);
		else
			j = 0;

		lim = rp->r_pgsz - stopg(i);
		if(lim > NPGPT)
			lim = NPGPT;

		pt = (pte_t *)rp->r_list[i] + j;
		dbd = dbdget(pt);

		for(  ;  j < lim  ;  j++, pt++, dbd++){

			/*	If these are private pages, then make
			 *	them copy-on-write since they will
			 *	be put in the hash table.
			 */

			if(rp->r_type == RT_PRIVATE){
				pg_clrprot(pt);
				pg_setprot(pt, PG_RO);
				pg_setcw(pt);
			}
			dbd->dbd_type  = DBD_FILE;
			dbd->dbd_blkno = off;
			off += blkspp;
		}
	}

	/*	Mark the last page for special handling
	 */
	
	dbd[-1].dbd_type = DBD_LSTFILE;

	rp->r_flags |= RG_DONE;
	if (rp->r_flags & RG_WAITING) {
		rp->r_flags &= ~RG_WAITING;
		wakeup(&rp->r_flags);
	}
	return(0);

}

/*	Create the block number list for an vnode.
 */

/* BOBJ: mapfile() is a dumb routine. What we should do is call bldblklst()
 * directly.  The only thing this routine does is figure out some
 * parameters for bldblklst(), calls bldblklst(), rounds up count to
 * next click's worth.  Bldblklst() could figure out the parameters itself
 * from the vnode (vp) and could also do the rounding.  Then we could get rid
 * of this routine and call bldblklst() directly from mapreg().
 */
mapfile(vp)
struct vnode	*vp;
{
	register int	i;
	register int	*bnptr;
	register int	blkspp;
	register int	nblks;
	register int	bsize;
	struct vattr vattr;

	/*	Get number of blocks to be mapped.
	 */

	VOP_GETATTR(vp, &vattr, u.u_cred);
	ASSERT(vp->v_map == 0);
	bsize = vp->v_vfsp->vfs_bsize;
	nblks = (vattr.va_size + bsize - 1)/bsize;
	blkspp = NBPP/bsize;


	/*	Round up the file size in block to an
	 *	integral number of pages.  Allocate
	 *	space for the block number list.
	 */

	i = ((nblks + blkspp - 1) / blkspp) * blkspp;
	bnptr = vp->v_map = (int *)uptalloc(ptos(i));
	if (bnptr == (int *)-1) {
		u.u_error = ENOMEM;
		return(-1);
	}

	/*	Build the actual list of block numbers
	 *	for the file.
	 */

	bldblklst(bnptr, vp, nblks);

	/*	If the size is not an integral number of
	 *	pages long, then the last few block
	 *	number up to the next page boundary are
	 *	made zero so that no one will try to
	 *	read them in.  See code in fault.c/vfault.
	 */

	while(i%blkspp != 0){
		*bnptr++ = 0;
		i++;
	}
	return(0);
}

/*	Find the region corresponding to a virtual address.
 */

reg_t	*
findreg(p, vaddr)
register struct proc	*p;
register caddr_t	vaddr;
{
	register preg_t	*prp;
	register preg_t	*oprp;

	oprp = 0;
	for(prp = &p->p_region[0] ; prp->p_reg ; prp++)
		if(vaddr >= prp->p_regva)
		if (oprp) {
			if(prp->p_regva > oprp->p_regva)
				oprp = prp;
		} else oprp = prp;
	if(oprp && vaddr >= oprp->p_regva) {
		reglock(oprp->p_reg);
		return(oprp->p_reg);
	}
	panic("findreg - no match");
	/*NOTREACHED*/
}

/*	Find the pregion of a particular type.
 */

preg_t *
findpreg(pp, type)
register proc_t	*pp;
register int	type;
{
	register preg_t	*prp;

	for(prp = pp->p_region ; prp->p_reg ; prp++){
		if(prp->p_type == type)
			return(prp);
	}

	/*	We stopped on an unused region.  If this is what
	 *	was called for, then return it unless it is the
	 *	last region for the process.  We leave the last
	 *	region unused as a marker.
	 */

	if((type == PT_UNUSED)  &&  (prp < &pp->p_region[pregpp - 1]))
		return(prp);
	return(NULL);
}

/*
 * Change protection of ptes for a region
 */
void
chgprot(prp, prot)
preg_t	*prp;
{
	if(prot == SEG_RO)
		prp->p_flags |= PF_RDONLY;
	else
		prp->p_flags &= ~PF_RDONLY;
	loadstbl(&u, prp, 0);

	clratb(USRATB);
}

/* Locate process region for a given virtual address. */

preg_t *
vtopreg(p, vaddr)
register struct proc *p;
register caddr_t vaddr;
{
	register preg_t *prp;
	register reg_t *rp;

	for (prp = p->p_region; rp = prp->p_reg; prp++) {
		caddr_t lo = prp->p_regva;
		caddr_t hi = lo + ptob(rp->r_pgsz);
		if ((unsigned long)vaddr >= (unsigned long)lo
		  && (unsigned long)vaddr < (unsigned long)hi)
			return(prp);
	}
	return(NULL);
}

/* <@(#)region.c	1.7> */
