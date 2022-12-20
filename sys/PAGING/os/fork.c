#ifndef lint	/* .../sys/PAGING/os/fork.c */
#define _AC_NAME fork_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.4 87/11/11 21:24:32}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.4 of fork.c on 87/11/11 21:24:32";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)fork.c	UniPlus VVV.2.1.9	*/

#include "compat.h"
#ifdef HOWFAR
extern int T_fork;
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
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/pfdat.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/vnode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/mmu.h"
#include "sys/tuneable.h"
#endif lint


/*
 * Create a new process-- the internal version of
 * sys fork.
 *
 * This changes the new proc structure and
 * alters only u.u_procp of the uarea.
 *
 * It returns 1 in the new process, 0 in the old.
 */


newproc(failok)
{
	register struct user *up;
	register struct proc *cp, *pp, *pend;
	register n, a;
	register int (**fptr)();
	register struct var *vp;
	extern int (*forkfunc[])();
	static mpid;

	up = &u;
	vp = &v;
	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
retry:
	mpid++;
	if (mpid >= MAXPID) {
		mpid = 0;
		goto retry;
	}
	pp = &proc[0];
	cp = NULL;
	n = (struct proc *)vp->ve_proc - pp;
	a = 0;
	do {
		if (pp->p_stat == NULL) {
			if (cp == NULL)
				cp = pp;
			continue;
		}
		if (pp->p_pid==mpid)
			goto retry;
		if (pp->p_uid == up->u_ruid)
			a++;
		pend = pp;
	} while(pp++, --n);
	if (cp==NULL) {
		if ((struct proc *)vp->ve_proc >= &proc[vp->v_proc]) {
			if (failok) {
				tablefull("proc");
				syserr.procovf++;
				up->u_error = EAGAIN;
				return(-1);
			} else
				panic("no procs");
		}
		cp = (struct proc *)vp->ve_proc;
	}
	if (cp > pend)
		pend = cp;
	pend++;
#ifdef lint
	vp->ve_proc = (struct proc *)pend;
#else lint
	vp->ve_proc = (char *)pend;
#endif lint
	if (up->u_uid && up->u_ruid) {
		if (cp == &proc[vp->v_proc-1] || a > vp->v_maxup) {
			up->u_error = EAGAIN;
			return(-1);
		}
	}

	/*
	 * make proc entry for new proc
	 */

	pp = up->u_procp;
	cp->p_uid = pp->p_uid;
	cp->p_suid = pp->p_suid;
	cp->p_pgrp = pp->p_pgrp;
	cp->p_nice = pp->p_nice;
	cp->p_stat = SIDL;

	/*
	 * p_clktim should be inheirited (per the manual)
	 * but it drives us nuts
	 */

	cp->p_clktim = 0;
	timerclear(&cp->p_realtimer.it_value);
	cp->p_flag = SLOAD | (pp->p_flag & (SCOFF|SPGRP42));
	cp->p_pid = mpid;
	cp->p_ppid = pp->p_pid;
	cp->p_time = 0;
	cp->p_sigmask = pp->p_sigmask;
	cp->p_sigcatch = pp->p_sigcatch;
	cp->p_sigignore = pp->p_sigignore;
	cp->p_cpu = 0;
	cp->p_pri = PUSER + pp->p_nice - NZERO;
	cp->p_size = USIZE;
	cp->p_compatflags = pp->p_compatflags;

	/* link up to parent-child-sibling chain---
	 * no need to lock generally since only a free proc call
	 * (done by same parent as newproc) diddles with child chain.
	 */

	if (pp == &proc[1])
	{
		cp->p_sibling = pp->p_child;
		cp->p_parent = pp;
		pp->p_child = cp;
	}
	else
	{
		cp->p_sibling = pp->p_child;
		cp->p_parent = pp;
		pp->p_child = cp;
	}

	/*
	 * make duplicate entries
	 * where needed
	 */

	for(n=0; n<NOFILE; n++)
		if (up->u_ofile[n] != NULL)
			up->u_ofile[n]->f_count++;
	VN_HOLD(up->u_cdir);
	if (up->u_rdir)
		VN_HOLD(up->u_rdir);

	crhold(up->u_cred);

	for (fptr = forkfunc; *fptr; fptr++)
		(**fptr)(cp, pp);

	/* Can't rsav because parent may have to sleep while sptalloc'ing
	 * the udot for the child.  So ssav now, and copy to rsav in setuctxt.
	 */
TRACE(T_fork, ("newproc: child cp==proc[%d]==0x%x saving to ssav\n", cp-proc, cp));
	if (save(up->u_ssav)) {
TRACE(T_fork, ("newproc: child cp==proc[%d]==0x%x returning\n", cp-proc, cp));
		return(1);		/* child will return here */
	}
	switch (procdup(cp, pp)) {
	case 0:
		/* Successful copy */
		break;
	case -1:
		/* reset all incremented counts */

		pexit(cp);

		/* Could not duplicate or attach region.
		 * clean up parent-child-sibling pointers--
		 * No lock necessary since nobody else could
		 * be diddling with them here.
		 */

		pp->p_child = cp->p_sibling;
		cp->p_parent = NULL;
		cp->p_sibling = NULL;
		cp->p_stat = NULL;
		up->u_error = EAGAIN;
		return(-1);
	}
/* BOBJ: the vax code does a 
 *	pp->p_stat = SRUN;
 *	cp->p_flag &= ~SLOCK;
 * here.
 */
	cp->p_stat = SRUN;

	setrq(cp);
	up->u_rval1 = cp->p_pid;		/* parent returns pid of child */

	/* have parent give up processor after
	 * its priority is recalculated so that
	 * the child runs first (its already on
	 * the run queue at sufficiently good
	 * priority to accomplish this).  This
	 * allows the dominant path of the child
	 * immediately execing to break the multiple
	 * use of copy on write pages with no disk home.
	 * The parent will get to steal them back
	 * rather than uselessly copying them.
	 */

	runrun = 1;
	return(0);
}

/*
 * Create a duplicate copy of a process
 */
procdup(cp, pp)
struct proc *cp, *pp;
{
	register preg_t	*p_prp;
	register preg_t	*c_prp;
	register user_t *uservad;

	/*	Allocate a u-block for the child process
	 *	in the kernel virtual space.
	 */

#ifdef NEW_PMMU
	availrmem -= USIZE;
	availsmem -= USIZE;

	if ((availrmem < tune.t_minarmem)  ||
	    (availsmem < tune.t_minasmem)) {
		printf("procdup - can't get %d pages for udot\n", USIZE);
		availrmem += USIZE;
		availsmem += USIZE;
		return(-1);
	}
	TRACE(T_availmem,("procdup: taking %d avail[RS]mem pages for udot\n", 
					USIZE));
#endif

#ifdef NEW_PMMU
	if((uservad = (struct user *)sptalloc(USIZE, PG_V|PG_RW, -1)) == NULL)
#else NEW_PMMU
	if((uservad = (struct user *)sptalloc(USIZE, PG_V|PG_RW, 0)) == NULL)
#endif NEW_PMMU
	{
#ifdef NEW_PMMU
		availrmem += USIZE;
		availsmem += USIZE;
#endif
		return(-1);
	}
	
	/*	Setup child u-block
	 */
	
	setuctxt(cp, uservad);

	/*	Duplicate all the regions of the process.
	 */

	p_prp = pp->p_region;
	c_prp = cp->p_region;

	for( ; p_prp->p_reg ; p_prp++, c_prp++){
		register int		prot;
		register reg_t		*rp;

		prot = (p_prp->p_flags & PF_RDONLY ? SEG_RO : SEG_RW);
		reglock(p_prp->p_reg);
		rp = dupreg(p_prp->p_reg);
		if(rp == NULL) {
			regrele(p_prp->p_reg);
			if(c_prp > cp->p_region)
				do {
					c_prp--;
					reglock(c_prp->p_reg);
					detachreg(c_prp, uservad);
				} while(c_prp > cp->p_region);
			sptfree(uservad, USIZE, 1);
#ifdef NEW_PMMU
			availrmem += USIZE;
			availsmem += USIZE;
#endif NEW_PMMU
			return(-1);
		}

		ASSERT(uservad->u_procp == cp);

		if(attachreg(rp, uservad, p_prp->p_regva,
			     p_prp->p_type, prot) == NULL){
			freereg(rp);
			if(rp != p_prp->p_reg){
				regrele(p_prp->p_reg);

				/* Note that we don't want to
				** do a VN_RELE(vp) here since
				** rp while have had the same
				** vp value and the freereg
				** will have unlocked it.
				*/
			}
			if(c_prp > cp->p_region)
				do {
					c_prp--;
					reglock(c_prp->p_reg);
					detachreg(c_prp, uservad);
				} while(c_prp > cp->p_region);
			sptfree(uservad, USIZE, 1);
#ifdef NEW_PMMU
			availrmem += USIZE;
			availsmem += USIZE;
#endif NEW_PMMU
			return(-1);
		}

		regrele(p_prp->p_reg);
		if(rp != p_prp->p_reg){
			regrele(rp);
		}
	}

	/*	Flush ATB.  Copy-on-write page permissions
	 *	have changed from RW to RO
	 */

	clratb(USRATB);

	sptfree(uservad, USIZE, 0);
	return(0);
}

/*	Setup context of child process
 */

setuctxt(p, up)
register struct proc *p;	/* child proc pointer */
register struct user *up;	/* child u-block pointer */
{
	register pte_t *pt;
#ifdef NEW_PMMU
	extern caddr_t realsvtop();
	extern pte_t *realsvtopte();
	register int i;
#endif NEW_PMMU

	/*	Copy u-block and u-block page tables
	 */
#ifdef NEW_PMMU /* No point in copying seg tables - clear at end of routine */
	blt512((caddr_t)&up->u_tbla[0],(caddr_t)&u.u_tbla[0], 
				(ptob(USIZE) - sizeof(u.u_stbl)) >> 9);
	pt = p->p_uptbl;
	
	for (i = 0; i < ptob(USIZE); i += ptob(1), ++pt)
		*pt = *(realsvtopte((int) up + i));

	for ( i = 0; i < UNTBLA; i++)
		wtrte(up->u_tbla[i], RT_VALID, NSTBL,
				(long)realsvtop(&up->u_stbl[i * NSTBL]));
#else NEW_PMMU

	bcopy(&u, up, ptob(USIZE));
TRACE(T_fork,("setuctxt: copied udot to 0x%x\n", up));
	bcopy(svtopte(up), (caddr_t)p->p_uptbl, USIZE*sizeof(pte_t));
TRACE(T_fork,("setuctxt: loaded udot's pte into 0x%x=0x%x\n", p->p_uptbl, p->p_uptbl[0]));
#endif


	/*	Reload the u-block kernel segment table entry
	 *	(p_addr), and reset u_procp
	 */
#ifndef	AWS
	pt = (pte_t *)uptbl;
	wtste(p->p_addr, SEG_RW, USIZE, pt);
#endif	AWS
	up->u_procp = p;
TRACE(T_fork,("setuctxt: set u_procp=0x%x=0x%x\n", p, up->u_procp));


	/*	Clear segment table in child
	 *	This makes it look as if no regions are attached
	 */
	/*	Child was saved in ssav, but will be resumed on rsav.
	 */
	bcopy((caddr_t)up->u_ssav, (caddr_t)up->u_rsav, sizeof(label_t));
	bzero((caddr_t)up->u_stbl, NSEGP*sizeof(ste_t));
#ifdef AWS
	bzero((caddr_t)up->u_tlb, sizeof(up->u_tlb));
#endif AWS
}

/* <@(#)fork.c	1.4> */

