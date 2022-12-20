#ifndef lint	/* .../sys/psn/io/linea.c */
#define _AC_NAME linea_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.3 87/11/19 18:02:22}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of linea.c on 87/11/19 18:02:22";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)linea.c	UniPlus VVV.2.1.5	*/
/*
 *	This routine is the 'high level' a-line trap handler ... see 
 *		*ivec.s for the rest of this code.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <sys/mmu.h>
#include <sys/time.h>
#include <sys/page.h>
#include <sys/seg.h>
#include <sys/user.h>
#include <sys/reg.h>

#ifdef PMMU
long
lineA(ap)
register struct args *ap;
{
	register caddr_t *ptr;

	ptr = (caddr_t *) u.u_user[0];	/* We already know that u.u_user[0] */
					/* is non-zero */
	ptr[1] = ap->a_pc;
	ap->a_pc = ptr[0];
}
#endif PMMU
