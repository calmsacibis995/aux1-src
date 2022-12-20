#ifndef lint	/* .../sys/PAGING/os/usys4.c */
#define _AC_NAME usys4_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/11 21:28:35}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of usys4.c on 87/11/11 21:28:35";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

/*	@(#)usys4.c	UniPlus VVV.2.1.2	*/


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
#include "sys/signal.h"
#include "sys/dir.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/region.h"
#include "sys/proc.h"
#endif lint
/*
 * phys - Set up a physical address in user's address space.
 */
phys()
{
	register struct a {
		unsigned phnum;		/* phys segment number */
		unsigned laddr;		/* logical address */
		unsigned bcount;	/* byte count */
		unsigned phaddr;	/* physical address */
	} *uap;

	if (!suser()) return;
	uap = (struct a *)u.u_ap;
	dophys(uap->phnum, uap->laddr, uap->bcount, uap->phaddr);
}

/*
 * reboot the system
 */
reboot()
{
	register i;

	if (!suser())
		return;
	sync();
	for (i = 0; i < 1000000; i++)
		;
	doboot();
}

/*
 * powerdown the system
 */
powerdown()
{
	register i;

	if (!suser())
		return;
	sync();
	for (i = 0; i < 1000000; i++)
		;
	dopowerdown();
}
