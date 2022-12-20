#ifndef lint	/* .../sys/COMMON/os/genassym.c */
#define _AC_NAME genassym_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/18 18:37:57}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of genassym.c on 87/11/18 18:37:57";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)genassym.c	UniPlus 2.1.3	*/
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <sys/mmu.h>
#include <sys/time.h>
#include <sys/page.h>
#include <sys/seg.h>
#include <sys/user.h>
#include <sys/reg.h>


main()
{
	register struct sysinfo *s = (struct sysinfo *)0;
	register struct user *u = (struct user *)0;

	printf("\tglobal\tsysinfo\n");
	printf("\tset\tV_INTR%%,%d\n", &s->intr);
	printf("\tset\tU_USER%%,%d\n", &u->u_user[0]);
	printf("\tset\tU_SIGCODE%%,%d\n", &u->u_sigcode);
        exit(0);        /* sai -6/17/85- Vax make blows up without this */
}
