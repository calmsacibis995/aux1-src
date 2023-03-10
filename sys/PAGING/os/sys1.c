#ifndef lint	/* .../sys/PAGING/os/sys1.c */
#define _AC_NAME sys1_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:27:15}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of sys1.c on 87/11/11 21:27:15";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)sys1.c	UniPlus VVV.2.1.1	*/

#ifdef HOWFAR
extern int T_fork;
#include "sys/debug.h"
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
#include "sys/errno.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/reg.h"
#endif lint


/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

exec()
{
	((struct execa *)u.u_ap)->envp = NULL;
	exece();
}

/*
 * exit system call:
 * pass back caller's arg
 */
rexit()
{
	register struct a {
		int	rval;
	} *uap;

	uap = (struct a *)u.u_ap;
	exit((uap->rval & 0377) << 8);
}

/*
 * fork system call.
 */
fork()
{
	register struct user *up;
	
	up = &u;
	sysinfo.sysfork++;
	/*
	 * Disallow if
	 *  No processes at all;
	 *  not su and too many procs owned; or
	 *  not su and would take last slot.
	 * Check done in newproc().
	 */
	switch( newproc(1) ) {
	case 1: /* child  -- successful newproc */
		up->u_rval1 = up->u_procp->p_ppid;
		up->u_rval2 = 1;  /* child */
		up->u_start = time.tv_sec;
		up->u_ticks = lbolt;
		up->u_mem = up->u_procp->p_size;
		up->u_ior = up->u_iow = up->u_ioch = 0;
		up->u_cstime = 0;
		up->u_stime = 0;
		up->u_cutime = 0;
		up->u_utime = 0;
		up->u_acflag = AFORK;
		up->u_lock = 0;
		return;
	case 0: /* parent -- successful newproc */
		/* up->u_rval1 = pid-of-child; */
		break;
	default: /* unsuccessful newproc */
		break;
	}
	up->u_rval2 = 0; /* parent */
	if (!(up->u_procp->p_flag & SCOFF))
		/*
		 *	We infer from the fact that the binary was not in
		 *	Common Object File Format that the fork interface
		 *	is the old one.
		 */
		up->u_ar0[PC] += 2;
}

/* <@(#)sys1.c	6.1> */
