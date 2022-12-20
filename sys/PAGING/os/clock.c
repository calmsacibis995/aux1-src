#ifndef lint	/* .../sys/PAGING/os/clock.c */
#define _AC_NAME clock_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1984-85 AT&T-IS, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.6 87/11/11 21:23:42}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.6 of clock.c on 87/11/11 21:23:42";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

/*	@(#)clock.c	UniPlus VVV.2.1.8	*/

#include "sys/types.h"
#include "sys/tuneable.h"
#include "sys/param.h"
#include "sys/psl.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/callout.h"
#include "sys/time.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/swap.h"
#include "sys/reg.h"
#include "sys/debug.h"
#include "sys/protosw.h"
#include "sys/socket.h"
#include "net/if.h"
#include "net/netisr.h"
#include <sys/errno.h>

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	alarm clock signals
 *	jab the scheduler
 */

#define	PRF_ON	01
extern	unsigned prfstat;

time_t	lbolt;
struct timeval time;
int one_sec = 1;
extern int switching;
extern char queueflag;

extern int	lticks;
extern int	mtimer;
extern int	tick;
extern int	vhand();
int	vhandcnt;	/* Counter for t_vhandr.		*/
extern	freemem;	/* Amount of memory available		*/

clock(ap)
struct args *ap;
{
	register struct proc *pp;
	register a;
	register int ps = ap->a_ps;
	register caddr_t pc = ap->a_pc;
	static rqlen, sqlen;
	extern char vhandwakeup;
	int i;
	swpt_t *st;
	register struct callout *p1;
	register struct user *up;
	extern int adjtimedelta, tickadj, doresettodr;

	up = &u;
	if (panicstr) {
		clkreset(0);
		return;
	} else
		clkreset(1);

	if (up->u_stack[0] != STKMAGIC)
		panic("Interrupt stack overflow");

	/*
	 * Update real-time timeout queue.
	 * At front of queue are some number of events which are ``due''.
	 * The time to these is <= 0 and if negative represents the
	 * number of ticks which have passed since it was supposed to happen.
	 * The rest of the q elements (times > 0) are events yet to happen,
	 * where the time for each is given as a delta from the previous.
	 * Decrementing just the first of these serves to decrement the time
	 * to all events.
	 */
	p1 = calltodo.c_next;
	while(p1) {
		if(--p1->c_time > 0)
			break;
		if(p1->c_time == 0)
			break;
		p1 = p1->c_next;
	}
	if (!BASEPRI(ps) && !queueflag)
		if(calltodo.c_next && calltodo.c_next->c_time <= 0)
			timein();
	if (prfstat & PRF_ON)
		prfintr((unsigned)pc, ps);
	pp = up->u_procp;
	if (usermode(ps)) {
		if (timerisset(&u.u_timer[ITIMER_VIRTUAL].it_value) &&
		    itimerdecr(&u.u_timer[ITIMER_VIRTUAL], tick) == 0)
			psignal(u.u_procp, SIGVTALRM);
		if (pp->p_nice > NZERO)
			a = CPU_NICE;
		else
			a = CPU_USER;
		up->u_utime++;
	} else {
		if (ap->a_dev != 0) {	/* dev has old idleflg in it */
			if (syswait.iowait+syswait.swap+syswait.physio) {
				a = CPU_WAIT;
				if (syswait.iowait)
					sysinfo.wait[W_IO]++;
				if (syswait.swap)
					sysinfo.wait[W_SWAP]++;
				if (syswait.physio)
					sysinfo.wait[W_PIO]++;
			} else
				a = CPU_IDLE;
		} else {
			a = CPU_KERNEL;
			if (!switching)
				up->u_stime++;
		}
		if (a != CPU_IDLE)
			if (timerisset(&u.u_timer[ITIMER_PROF].it_value) &&
			    itimerdecr(&u.u_timer[ITIMER_PROF], tick) == 0)
				psignal(u.u_procp, SIGPROF);
	}
	sysinfo.cpu[a]++;
#ifdef IN_NFS_SWAP
	for (a = 0; a < MIN(dk_nunits, DK_NDRIVE); a++)
		if (dk_busy & (1 << a))
			dk_time[a]++;
#endif IN_NFS_SWAP
	pp = up->u_procp;
	if (pp->p_stat==SRUN  || pp->p_stat==SONPROC) {
		register preg_t	*prp;
		register reg_t	*rp;

		/*	Update memory usage for the currently
		*	running process.
		*/

		for(prp = pp->p_region ; rp = prp->p_reg ; prp++){
			if(rp->r_type == RT_PRIVATE){
				up->u_mem += rp->r_nvalid;
			} else {
				if(rp->r_refcnt)
					up->u_mem += rp->r_nvalid/rp->r_refcnt;
			}
		}
	}
	if (!switching && pp->p_cpu < 80)
		pp->p_cpu++;
	lbolt++;	/* time in ticks */
	if (adjtimedelta == 0) {
		bumptime(&time, tick);
	} else {
		register delta;

		if (adjtimedelta < 0) {
			delta = tick - tickadj;
			adjtimedelta += tickadj;
		} else {
			delta = tick + tickadj;
			adjtimedelta -= tickadj;
		}
		bumptime(&time, delta);
		if (-tickadj < adjtimedelta && adjtimedelta < tickadj) {
			adjtimedelta = 0;
			if (doresettodr) {
				doresettodr = 0;
				resettodr();
			}
		}
	}
	if(--lticks <= 0)
		runrun++;
	if (--one_sec <= 0) {
		if (critical(ps))
			return;
		one_sec += v.v_hz;
		minfo.freemem = freemem;
		minfo.freeswap = 0;

		for(i = 0, st=swaptab;i < MSFILES ;  i++, st++){
			if(st->st_ucnt == NULL)
				continue;
			minfo.freeswap += st->st_nfpgs << DPPSHFT;
		}
		if ((time.tv_sec & 3) == 0)	/* entry to load average */
			loadav();
		rqlen = 0;
		sqlen = 0;
		for(pp = &proc[0]; pp < (struct proc *)v.ve_proc; pp++)
		if (pp->p_stat) {
			if (pp->p_time != 127)
				pp->p_time++;
			if (pp->p_clktim) 
				if (--pp->p_clktim == 0) 
					psignal(pp, SIGALRM);
			pp->p_cpu >>= 1;
			if (pp->p_pri >= (PUSER-NZERO))
				pp->p_pri = calcppri(pp);

			if (pp->p_stat == SRUN)
				if (pp->p_flag & SLOAD)
					rqlen++;
				else 
					sqlen++;
		}
		if (rqlen) {
			sysinfo.runque += rqlen;
			sysinfo.runocc++;
		}
		if (sqlen) {
			sysinfo.swpque += sqlen;
			sysinfo.swpocc++;
		}

		/*	Wake up page aging process every 
		 *	t_vhandr seconds unless we have lots of
		 *	memory.
		 */

		if (--vhandcnt <= 0) {
			vhandcnt = tune.t_vhandr;
			if(freemem < tune.t_vhandl && vhandwakeup) {
				vhandwakeup = 0;
				wakeup(&vhandwakeup);
			}
		}
		/*
		* Wakeup sched if
		* memory is tight or someone is not loaded (runin set)
		*/
		if (runin!=0) {
			runin = 0;
			setrun(&proc[0]);
		}
		/* wakeup lbolt (for 4.2 job control) */
		wakeup(&lbolt);
	}
	if (usermode(ps) && up->u_prof.pr_scale)
		addupc((unsigned)pc, &up->u_prof, 1);
}

int bkmscnt = 0;
timein()
{
	register struct callout *p1;
	register s;
	register caddr_t arg;
	register int (*func)();
	register int a;

	s = splclk();
	for (;;) {

		(void) spl7();
		if ((p1 = calltodo.c_next) == 0 || p1->c_time > 0)
			break;
		arg = p1->c_arg; func = p1->c_func; a = p1->c_time;
		calltodo.c_next = p1->c_next;
		p1->c_next = callfree;
		callfree = p1;
		(void) spltty();
		(*func)(arg, a);
	}
	(void) splx(s);
}
/*
 *  Arrange that (*fun)(arg) is called in t/hz seconds.
 */
timeout(fun, arg, t)
	int (*fun)();
	caddr_t arg;
	register int t;
{
	register struct callout *p1, *p2, *pnew;
	register int s = spl7();

	if (t == 0)
		t = 1;
	pnew = callfree;
	if (pnew == NULL)
		panic("timeout table overflow");
	callfree = pnew->c_next;
	pnew->c_arg = arg;
	pnew->c_func = fun;
	for (p1 = &calltodo; (p2 = p1->c_next) && p2->c_time < t; p1 = p2)
		if (p2->c_time > 0)
			t -= p2->c_time;
	p1->c_next = pnew;
	pnew->c_next = p2;
	pnew->c_time = t;
	if (p2)
		p2->c_time -= t;
	(void) splx(s);
}

/*
 * untimeout is called to remove a function timeout call
 * from the callout structure.
 */
untimeout(fun, arg)
	int (*fun)();
	caddr_t arg;
{
	register struct callout *p1, *p2;
	register int s;

	s = spl7();
	for (p1 = &calltodo; (p2 = p1->c_next) != 0; p1 = p2) {
		if (p2->c_func == fun && p2->c_arg == arg) {
			if (p2->c_next && p2->c_time > 0)
				p2->c_next->c_time += p2->c_time;
			p1->c_next = p2->c_next;
			p2->c_next = callfree;
			callfree = p2;
			break;
		}
	}
	(void) splx(s);
}

#define	PDELAY	(PZERO-1)
delay(ticks)
{
	extern wakeup();
	int s;

	if (ticks<=0)
		return;
	s = spl7();
	timeout(wakeup, (caddr_t)u.u_procp+1, ticks);
	(void) sleep((caddr_t)u.u_procp+1, PDELAY);
	(void) splx(s);
}

/*
 * From here down is load average code
 */
struct lavnum {
	unsigned short high;
	unsigned short low;
};

struct lavnum avenrun[3];

/*
 * Constants for averages over 1, 5, and 15 minutes
 * when sampling at 4 second intervals.
 * (Using 'fixed-point' with 16 binary digits to right)
 */
struct lavnum cexp[3] = {
	{ 61309, 4227 },	/* (x = exp(-1/15) * 65536) , 1 - x */
	{ 64667, 869 },		/* (x = exp(-1/75) * 65536) , 1 - x */
	{ 65245, 291 },		/* (x = exp(-1/225) * 65536) , 1 - x */
};

/* called once every four seconds */
loadav()
{
	register struct lavnum *avg;
	register struct lavnum *rcexp;
	register unsigned int j;
	register unsigned short nrun;
	register struct proc *p;

	nrun = 0;
	for (p = &proc[0]; p < (struct proc *)v.ve_proc; p++) {
		if (p->p_flag & SSYS)
			continue;
		if (p->p_stat) {
			switch (p->p_stat) {
			case SSLEEP:
			case SSTOP:
				if (p->p_pri <= PZERO)
					nrun++;
				break;
			case SONPROC:
			case SRUN:
			case SIDL:
				nrun++;
				break;
			}
		}
	}
	/*
	 * Compute a tenex style load average of a quantity on
	 * 1, 5 and 15 minute intervals.
	 * (Using 'fixed-point' with 16 binary digits to right)
	 */
	avg = avenrun;
	rcexp = cexp;
	for ( ; avg < &avenrun[3]; avg++, rcexp++) {
		j = ((avg->low * rcexp->high + 32768) >> 16)
		    + (avg->high * rcexp->high)
		    + (nrun * rcexp->low);
		avg->low = j & 65535;
		avg->high = j >> 16;
	}
}

/*
 * Compute number of hz until specified time.
 * Used to compute third argument to timeout() from an
 * absolute time.
 */
hzto(tv)
	struct timeval *tv;
{
	register long ticks;
	register long sec;
	int s = spl7();

	/*
	 * If number of ticks will fit in 32 bit arithmetic,
	 * then compute number of ticks to time.
	 * Otherwise round times greater than representible to maximum value.
	 */
	sec = tv->tv_sec - time.tv_sec;
	if (sec <= ((0x7fffffff / v.v_hz) - v.v_hz))
		ticks = (sec * v.v_hz) +
			(((tv->tv_usec - time.tv_usec) * v.v_hz) / 1000000);
	else
		ticks = 0x7fffffff;
	(void) splx(s);
	return (ticks);
}

/*
 * Bump a timeval by a number of usec's.
 */
bumptime(tp, usec)
	register struct timeval *tp;
	int usec;
{

	tp->tv_usec += usec;
	if (tp->tv_usec >= 1000000) {
		tp->tv_sec += tp->tv_usec / 1000000;
		tp->tv_usec %= 1000000;
	}
}

uniqtime(tv)
        register struct timeval *tv;
{
        static struct timeval last;
        static int uniq;

        while (last.tv_usec != time.tv_usec || last.tv_sec != time.tv_sec) {
                last = time;
                uniq = 0;
        }
        *tv = last;
        tv->tv_usec += uniq++;
}
/* <@(#)clock.c	6.3> */
