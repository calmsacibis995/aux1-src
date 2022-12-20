#ifndef lint	/* .../sys/COMMON/os/subr.c */
#define _AC_NAME subr_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/11 21:12:29}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of subr.c on 87/11/11 21:12:29";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/vnode.h"
#include "sys/signal.h"
#ifdef PAGING
#include "sys/seg.h"
#endif PAGING
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/vfs.h"
#include "sys/var.h"

/*
 * Routine which sets a user error; placed in
 * illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{
	return(ENODEV);
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{
	return (0);
}

imin(a, b)
{
 	return (a < b ? a : b);
}
