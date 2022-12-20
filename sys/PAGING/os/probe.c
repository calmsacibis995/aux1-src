#ifndef lint	/* .../sys/PAGING/os/probe.c */
#define _AC_NAME probe_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.4 87/11/11 21:25:58}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.4 of probe.c on 87/11/11 21:25:58";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)probe.c	UniPlus VVV.2.1.6	*/

#ifdef HOWFAR
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
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/file.h"
#include "sys/vnode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/pfdat.h"
#include "sys/proc.h"
#include "sys/tuneable.h"
#include "sys/debug.h"
#endif lint

/*
 * Calculate number of pages transfer touchs
 */
#define len(base, count)	\
	btop(base + count) - btotp(base)

/*
 * Calulate starting user PTE address for transfer
 */
#define upt(base)	\
	(pte_t *) segvptbl(&u.u_stbl[snum(base)]) + pnum(base)
union ptbl *segvptbl();

/*
 * Check user read/write access
 * If rw has B_PHYS set, the user PTEs will be faulted
 * in and locked down.  Returns 0 on failure, 1 on success
 */
useracc(base, count, rw)
register base, count;
{
	register	i;
	register	npgs;
	register int	x;
	register pte_t	*pt;
	register reg_t  *rp;
	pfd_t		*pfd;

	/*
	 * If physio, two checks are made:
	 *	1) Base must be word aligned
	 *	2) Transfer must be contained in one segment
	 */

	if (rw & B_PHYS && ((base & 1)
	 || snum(base) != snum(base + count - 1)))
		return(0);

	/*
	 * Check page permissions and existence
	 */

	npgs = len(base, count);

	rp = findreg(u.u_procp, (caddr_t)base);
	if (rp == NULL)
		return(0);
#ifndef NEW_PMMU
	rp->r_flags |= RG_NOSWAP;
#endif
	regrele(rp);

	for(i = npgs; --i >= 0; base += ptob(1)) {
		x = fubyte((caddr_t)base);
		if(x == -1)
			goto out;

		/*	In the following test we check for a copy-
		**	on-write page and if we find one, we break
		**	the copy-on-write even if we are not going
		**	to write to the page.  This is necessary
		**	in order to make the lock accounting
		**	work correctly.  The problem is that the
		**	lock bit is in the pde but the count of
		**	the number of lockers is in the pfdat.
		**	Therefore, if more than one pde refers
		**	to the same pfdat, the accounting gets
		**	wrong and we could fail to unlock a
		**	page when we should.  Note that this is
		**	not very likely to happen since it means
		**	we did a fork, no exec, and then started
		**	doing raw I/O.  Still, it could happen.
		*/

		if(rw & B_READ)
			if(subyte((caddr_t)base, x) == -1)
				goto out;
#ifdef NEW_PMMU
		if ((rw & B_PHYS) && ! rp->r_noswapcnt && (rp->r_type&RT_PHYSCALL) == 0)
#else
		if ((rw & B_PHYS) && (rp->r_type&RT_PHYSCALL) == 0)
#endif
		{
#ifndef lint
			pt = (pte_t *)upt(base);
#else
			pt = (pte_t *)0;
#endif
			if (pt->pgm[0].pg_cw  &&  !(rw & B_READ))
				if (subyte(base, x) == -1)
					goto out;
			memlock();
			pg_setlock(pt);
#ifdef NEW_PMMU
			pfd = pftopfd(pt->pgm[0].pg_pfn);
			if (pfd->pf_rawcnt++ == 0){
				availrmem--;
				if (availrmem < tune.t_minarmem) {
					printf("useracc - couldn't lock page\n");
					pg_clrlock(pt);
					--pfd->pf_rawcnt;
					u.u_error = EAGAIN;
					availrmem++;
					memunlock();
					goto out;
				}
				TRACE(T_availmem,("useracc: taking 1 avail[R]mem page\n"));
                        }
#else
			pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt++;
#endif NEW_PMMU
			memunlock();
		}
	}
#ifndef NEW_PMMU
	reglock(rp);
	rp->r_flags &= ~RG_NOSWAP;
	regrele(rp);
#endif NEW_PMMU
	return(1);
out:
#ifdef NEW_PMMU
	if ((rw & B_PHYS)  &&  ! rp->r_noswapcnt) {
		for (i++, base -= ptob(1) ; i < npgs ; i++, base -= ptob(1)) {
#ifndef lint
			pt = (pte_t *)upt(base);
#else
			pt = (pte_t *)0;
#endif
			pfd = pftopfd(pt->pgm[0].pg_pfn);
			if (--pfd->pf_rawcnt == 0){
				pg_clrlock(pt);
				TRACE(T_availmem,("useracc: returning 1 avail[R]mem page\n"));
				availrmem += 1;
			}
		}
	}
#else NEW_PMMU
	reglock(rp);
	rp->r_flags &= ~RG_NOSWAP;
	regrele(rp);
#endif NEW_PMMU
	return(0);
}


/*
 * Terminate user physio
 */
undma(base, count, rw)
register int base, rw;
{
	register pte_t *pt;
	register npgs;
	register reg_t *rp;


	/*
	 * Unlock PTEs, set the reference bit.
	 * Set the modify bit if B_READ
	 */
	rp = findreg(u.u_procp, (caddr_t)base);
	ASSERT(rp != (reg_t *)NULL);
	if ((rp->r_type&RT_PHYSCALL) == 0)
#ifndef lint
	for (pt = (pte_t *)upt(base), npgs = len(base, count); --npgs >= 0; pt++) {
#else lint
	for (pt = (pte_t *)upt(0), npgs = len(base, count); --npgs >= 0; pt++) {
#endif lint
#ifdef NEW_PMMU
		{
		struct pfdat *pfd = pftopfd(pt->pgm[0].pg_pfn);

		ASSERT(rp->r_noswapcnt  ||  npteget(pt)->pgm[0].pg_lock);
		ASSERT(rp->r_noswapcnt  ||  pfd->pf_rawcnt > 0);
		memlock();
		if (! rp->r_noswapcnt  &&  --pfd->pf_rawcnt == 0) {
			pg_clrlock(pt);
			TRACE(T_availmem,("undma: returning 1 avail[R]mem page\n"));
			availrmem++;
		}
		}
#else
		ASSERT(pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt > 0);
		memlock();
		if(--pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt == 0)
			pg_clrlock(pt);
#endif
		memunlock();
		pg_setref(pt);
		if (rw == B_READ) 
			pg_setmod(pt);
	}
	regrele(rp);
}

/* <@(#)probe.c	1.2> */
