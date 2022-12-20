#ifndef lint	/* .../sys/PAGING/os/swtch.c */
#define _AC_NAME swtch_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.3 87/11/11 21:27:09}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of swtch.c on 87/11/11 21:27:09";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)swtch.c	UniPlus VVV.2.1.6	*/

#ifdef HOWFAR
extern int	T_swtch;
#endif HOWFAR
#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/debug.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/pfdat.h"
#endif lint

/*
 * put the current process on
 * the Q of running processes and
 * call the scheduler.
 */
qswtch()
{

	setrq(u.u_procp);
	swtch();
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 * There is a race here. A process may become
 * ready after it has been examined.
 * In this case, idle() will be called and
 * will return in at most 1HZ time.
 * i.e. its not worth putting an spl() in.
 */
swtch()
{
	TRACE(T_swtch, ("swtch:\n"));
	if (save(u.u_rsav))
 		return;
	TRACE(T_swtch, ("swtch: calling resched()\n"));
	resched();
}

/*
 * This routine is called by the breakpoint trap
 * handler to reschedule the CPU.
 */

int switching;		/* don't bill running process */

extern int lticks;

resched()
{
	register struct proc *p, *q, *pp, *pq;
	register n;

	TRACE(T_swtch, ("resched:\n"));
	switching = 1;
	sysinfo.pswitch++;

loop:
	SPLHI();

	/*
	 * Search for highest-priority runnable process
	 */
	TRACE(T_swtch, ("resched: searching proc's\n"));

	runrun = 0;
	pp = NULL;
	q = NULL;
	n = 128;
	for(p = runq; p; q = p, p = p->p_link) {
		TRACE(T_swtch, ("\nresched: p==0x%x proc[%d] p_flag=0x%x ", p, p-proc, p->p_flag));
		if(( p->p_flag & SLOAD)== 0)
			continue;
		TRACE(T_swtch, ("p_pri=0x%x ", p->p_pri));
		if( p->p_pri > n )
			continue;
		TRACE(T_swtch, ("runnable"));
		pp = p;
		pq = q;
		n = p->p_pri;
	}
	TRACE(T_swtch, ("\n"));
	/*
	 * If no process is runnable, idle.
	 */
	p = pp;
	if (p == NULL) {
		TRACE(T_swtch, ("resched: no proc runnable\n"));
		curpri = PIDLE;
#ifndef NEW_PMMU
		/* before idling, switch to dummy uarea for this processor */
		SPLHI();

		/* wtste(kstbl[USEG], SEG_RW, USIZE, iduptbl); */
		/*
		 * use in-line code because system stack will be blown away
		 * and so will any return address for subroutines
		 */
		nlduptbl(iduptbl);
		flush_atc;
		asm("	mov.l	kstack%,%a6");
		asm("	mov.l	%a6,%sp");
		clr_cache(1);
#ifdef ASADEBUG	/*debug*/
		if (syswait.iowait)
{
extern int T_dophys;
if (T_dophys) debugit();
			idle1();
}
		else 
		if (syswait.swap)
			idle2();
		else
#endif ASADEBUG	/*debug*/
#endif ! NEW_PMMU
		idle();
		goto loop;
	}
	q = pq;
	/* Check that the selected process p has not changed since it
	 * was selected. Processes p and q * must still be on the runq.
	 * However, it is possible that p
	 * could have been swapped out in the meantime. Check that p
	 * is still loaded before resched-ing it.
	 */


	/* Take p off the runq */
	if (q == NULL)
		runq = p->p_link;
	else
		q->p_link = p->p_link;
	p->p_stat = SONPROC;	/* process p will be running	*/
	lticks = v.v_slice;	/* allocate process a time slice  */
	curpri = n;

#ifdef NEW_PMMU
	if (u.u_procp->p_stat != SZOMB)		/* Allow interrupts if we  */
		SPL0();				/* have a stack */
	else {
		register pte_t *pt;		/* If we are a dead proc */

		invsatb(USRATB, 0, -1);         /* clear old vals */
		reglock(&sysreg);
		memlock();

		pt = u.u_procp->p_uptbl;
		for (n = 0; n < USIZE; n++, pt++)
			pfree(&sysreg, pt, (dbd_t *)NULL);
		availrmem += USIZE;
		availsmem += USIZE;

		memunlock();
		regrele(&sysreg);
	}
#else NEW_PMMU

	SPL0();
#endif NEW_PMMU

	switching = 0;
	TRACE(T_swtch, ("resched: resume(0x%x, 0x%x)\n", u.u_rsav, p->p_uptbl));
	resume(u.u_rsav, p->p_uptbl);
	/* NOTREACHED */
}


/* <@(#)swtch.c	1.2> */
