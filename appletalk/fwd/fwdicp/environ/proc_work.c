#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/proc_work.c */
#define _AC_NAME proc_work_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:07:23}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of proc_work.c on 87/11/11 21:07:23";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/process.h>

#define	STACKSIZE	4096

char proc_space[PROCESS_COUNT*STACKSIZE];

static int x[2];

init_proc()
{
	int i;

	for (i = 0; i < PROCESS_COUNT;i++) 
		make_proc(&proc_space[i*STACKSIZE], STACKSIZE);
}
