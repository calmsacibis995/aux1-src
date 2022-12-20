#ifndef lint	/* .../sys/PAGING/os/bio.c */
#define _AC_NAME bio_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1983-87 Sun Microsystems Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:23:30}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of bio.c on 87/11/11 21:23:30";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)bio.c	UniPlus VVV.2.1.1	*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/mmu.h>
#include	<sys/sysmacros.h>
#include	<sys/time.h>
#include	<sys/page.h>
#include	<sys/systm.h>
#include	<sys/sysinfo.h>
#include	<sys/signal.h>
#include	<sys/seg.h>
#include	<sys/user.h>
#include	<sys/errno.h>
#include	<sys/buf.h>
#include	<sys/iobuf.h>
#include	<sys/conf.h>
#include	<sys/region.h>
#include	<sys/proc.h>
#include	<sys/var.h>
#include	<sys/uio.h>
/*
 *
 */
allocbuf(tp, size)
struct buf *tp;
int size;
{
	if (size > tp->b_bufsize)
		panic("allocbuf");
	tp->b_bcount = size;
	return (1);
}

/*
 * Release space associated with a buffer.
 */
bfree(bp)
        struct buf *bp;
{
        bp->b_bcount = 0;
}

