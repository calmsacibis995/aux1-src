#ifndef lint	/* .../sys/psn/os/main.c */
#define _AC_NAME main_c
#define _AC_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.11 87/11/18 18:43:59}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.11 of main.c on 87/11/18 18:43:59";
#endif		/* _AC_HISTORY */
#endif		/* lint */

char _ac_s[] = "Copyright (c) 1987 Apple Computer, Inc., 1985 Adobe Systems \
Incorporated, 1983-87 AT&T-IS, 1985-87 Motorola Inc., \
1980-87 Sun Microsystems Inc., 1980-87 The Regents of the \
University of California, 1985-87 Unisoft Corporation, All \
Rights Reserved.";

#define _AC_MODS

#ifdef HOWFAR
extern int T_main;
#endif HOWFAR

#include "compat.h"
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
#include "sys/time.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/var.h"

#include "sys/debug.h"
#include "sys/conf.h"
#include "sys/utsname.h"
#endif lint

/*
 *	kstack used to locate start of kernel stack; else size
 *	of user.h would need to coded into start.s
 *	p0uptbl used to set up physical pages for proc[0] u_area.
 *	istack used to set up per process interrupt stack.
 */

int	physmem, maxmem, lastaddr;
int	minmem;
struct vnode	*swapdev_vp;
struct vnode	*pipedev_vp;
extern 	int 	mtimer;
extern struct utsname utsname;

extern  short icode[];
extern	szicode;
/*
 *	Initialization code.
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *
 *	loop at low address in user mode -- /etc/init
 *	cannot be executed.
 */

main()
{
	register int i;
	register struct user *up;
	register  struct  proc  *p;
	register  int  (**fptr)();
	extern	int	(*initfunc[])();
	extern int		schar();
	extern int		callzero;
	extern char oemmsg[], timestamp[];
	extern int cmask, cdlimit;
	extern int compatflags;


	printf("A/UX Copyright 1987 by Apple Computer, Inc.  ");
	printf("All  rights reserved.   Portions   of  this\n");
	printf("product have been  previously copyrighted by");
	printf(",  and  are  licensed from  AT&T-IS, UniSoft\n");
	printf("Corporation,  The Regents of the  University ");
	printf("of California,  Sun Microsystems Inc.,  and\n");
	printf("Adobe Systems.\n\n");

	printf("\245A/UX RELEASE %s VERSION %s\245  A/UX kernel created %s.\n\n",
	       utsname.release,utsname.version,timestamp);

	if (!cputype)
		panic("CPU can't page fault properly\n");

	/* set up system process */

	proc[0].p_stat = SRUN;
	proc[0].p_flag |= SLOAD|SSYS;
	proc[0].p_nice = NZERO;
	proc[0].p_compatflags = compatflags;

	up = &u;
	up->u_cmask = cmask;
	up->u_limit = cdlimit;
	up->u_rdir = NULL;
	for (i = 0; i < sizeof(up->u_rlimit)/sizeof(up->u_rlimit[0]); i++)
		up->u_rlimit[i].rlim_cur = up->u_rlimit[i].rlim_max =
		    RLIM_INFINITY;

	bcopy("sched", up->u_comm, 5);
	bcopy("sched", up->u_psargs, 5);

	up->u_stack[0] = STKMAGIC;

	startup();
#if OSDEBUG == YES
	callzero = 1;
#endif

        /*
         * initialize kernel memory allocator
         */
        kmem_init();
 
	/*
	 * call all initialization routines
	 */

TRACE(T_main,("main:  calling initializion  routines\n"));
	for (fptr = initfunc; *fptr; fptr++) {
		TRACE(T_main,("main calling 0x%x ",*(int *)fptr));
		(**fptr)();
		TRACE(T_main,("\n"));
	}

	up->u_start = time.tv_sec;
	/*
	 * Setup credentials
	 */
	up->u_cred = crget();
	{
	register int i;

	for (i = 1; i < NGROUPS; i++)
		up->u_groups[i] = NOGROUP;
	} 

	/*	This call of swapadd must come after devinit in case
	 *	swap is moved by devinit.  It must also come after
	 *	dskinit so that the disk is don'ed.  This call should
	 *	return 0 since the first slot of swaptab should be used.
	 */

TRACE(T_main,("main:  swapadd(0x%x, 0x%x, 0x%x)\n",swapdev,(int)swaplow,swapcnt));
	if(swapadd(swapdev, (int)swaplow, swapcnt) != 0)
		panic("startup - swapadd failed");

	   
	/* The following is a check on some system configuration
	 * parameters. It checks that the stack has been
	 * defined correctly. */

#ifndef lint	/* causes lint errors when OSDEBUG turned on */
	ASSERT((USRSTACK - ptob(SSIZE) - NCARGS) > stob(NSEGP-1));
#endif lint

	/*
	 * reset sanity timers in offline processor and reset it
	 */

	/*
	 * make init process
	 * enter scheduling loop
	 * with system process
	 */

TRACE(T_main,("main:  making init process\n"));
	if(newproc(0)) {
		register preg_t *prp;
		register reg_t *rp;

TRACE(T_main, ("main:  init child\n"));
		up->u_cstime = up->u_stime = up->u_cutime = up->u_utime = 0;
		rp = allocreg((struct inode *)NULL, RT_PRIVATE);
		prp = attachreg(rp, &u, (caddr_t)v.v_ustart, PT_DATA, SEG_RW);
		if (growreg(prp, btop(szicode), DBD_DFILL) < 0)
			panic("main.c - icode growreg failure.");
		regrele(rp);
TRACE(T_main,("main:  copying icode\n"));
		if(copyout((caddr_t)icode, (caddr_t)v.v_ustart, szicode))
			panic("main.c - copyout of icode failed.");

		/*	The following line returns to user mode
		 *	and transfers to location zero where we
		 *	have just copied the code to exec init.
		 */

		return(0);
	}

	if(newproc(0)) {
TRACE(T_main, ("main:  vhand child\n"));
		maxmem -= (up->u_ssize + 1);
		up->u_procp->p_flag |= SLOAD|SSYS;
		up->u_cstime = up->u_stime = up->u_cutime = up->u_utime = 0;
		bcopy("vhand", up->u_psargs,5);
		bcopy("vhand", up->u_comm, 5);
TRACE(T_main,("main:  vhand()\n"));
		vhand();
	}

#ifdef AUTOCONFIG
	/*
	 *	Add init_last ... the last init routine to be called during
	 *		system startup, it is here so we can start system
	 *		processes
	 */
	{
		register  int  (**fptr)();
		extern	int	(*init_last[])();
		extern 	int	init_lastl;
		register int i;

		for (i = 0, fptr = init_last; i < init_lastl && *fptr; fptr++) {
			TRACE(T_main,("init_last: %x \n",**fptr));
			(**fptr)();
		}
	}

#endif AUTOCONFIG

TRACE(T_main,("main:  sched()\n"));
	sched();
#ifdef lint
	return(0);
#endif lint
}
/* <@(#)main.c	6.6> */
