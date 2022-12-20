#ifndef lint	/* .../sys/PAGING/io/pwr.c */
#define _AC_NAME pwr_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:23:14}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of pwr.c on 87/11/11 21:23:14";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)pwr.c	UniPlus VVV.2.1.1	*/

#ifdef lint
#include "sys/sysinclude.h"
#else
#include "sys/types.h"
#ifndef ORIG3B20
#include "sys/mmu.h"
#include "sys/seg.h"
#endif ORIG3B20
#include "sys/param.h"
#include "sys/page.h"
#include "sys/systm.h"
#ifdef ORIG3B20
#include "sys/seg.h"
#endif ORIG3B20
#include "sys/signal.h"
#include "sys/region.h"
#include "sys/time.h"
#include "sys/proc.h"
#include "sys/var.h"
#endif lint

#ifdef ORIG3B20
extern	int	sysbase[];	/* interrupt masks */
extern	int	procid;

/*
 * pwr - called from (ml/trap.s) when the power
 * circuit interrupts via source 6.
 */
pwr()
{
	register struct proc *pp;

	if(procid != 0)
		return;
	printf("\n\nPOWER FAIL IMMINENT\n\n");
	/* Ignore power fail interrupt */
	psetmask(6);
	/* Send SIGPWR to all processes */
	for (pp = &proc[1]; pp<(struct proc *)v.ve_proc; pp++)
		if (pp->p_stat)
			psignal(pp, SIGPWR);
	/* Release all disk jobs */
	dfcrele();
}

/*
 * pwralrm - called from (ml/trap.s) when a
 * a power alarm interrupt (source 7) is received.
 */
pwralrm()
{
	int pclrmask();

	if(procid != 0)
		return;
	printf("\n\nPOWER ALARM EXISTS IN MACHINE\n\n");
	/* Ignore power alarm interrupt */
	psetmask(7);
	/* Re-enable interrupt in a bit */
	timeout(pclrmask, 7, 20*60*HZ);
}

/*
 * psetmask - set a bit in the interrupt masks,
 * disabling the corresponding interrupt source
 */
psetmask(n)
{
	register mask, i;

	mask = 1 << n;
	/*
	 * Set the bit for all execution levels
	 */
	for(i = 0; i < 8; i++)
		sysbase[i] |= mask;
}

/*
 * pclrmask - clear a bit in the interrupt masks,
 * enabling the corresponding interrupt source
 */
pclrmask(n)
{
	register i, mask;

	mask = 1 << n;
	/*
	 * Clear the bit for all execution levels
	 * except 7 and 15 which correspond to spl7()
	 */
	for(i = 0; i < 8; i++) {
		if(i == 7)
			continue;
		sysbase[i] &= ~mask;
	}
}
#endif ORIG3B20

/* <@(#)pwr.c	6.1> */
