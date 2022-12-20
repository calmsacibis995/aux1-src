#ifndef lint	/* .../sys/PAGING/os/init.c */
#define _AC_NAME init_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/11 21:25:00}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of init.c on 87/11/11 21:25:00";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)init.c	UniPlus VVV.2.1.3	*/


#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/time.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/file.h"

#include "sys/debug.h"
#include "sys/conf.h"
#include "sys/utsname.h"
#endif lint

/*
 * Initialize hash links for buffers.
 */
bhinit()
{
        register int i;
        register struct bufhd *bp;

        for (bp = bufhash, i = 0; i < v.v_hbuf; i++, bp++)
                bp->b_forw = bp->b_back = (struct buf *)bp;
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device hash buffer lists to empty.
 */
binit()
{
	register struct buf *bp, *dp;
	register unsigned i;
	register long b;
	extern  caddr_t iobufs;
	extern struct buf *sbuf;

	for (dp = bfreelist; dp < &bfreelist[BQUEUES]; dp++) {
		dp->b_forw = dp->b_back = dp->av_forw = dp->av_back = dp;
		dp->b_flags = B_HEAD;
	}
	b = (((long)iobufs + (sizeof(int) - 1)) & (-sizeof(int)));
	for (i=0, bp = sbuf; i < v.v_buf; i++, bp++) {
		bp->b_dev = NODEV;
		bp->b_bcount = 0;
#ifdef NEW_PMMU
		bp->b_un.b_addr = (caddr_t) svirtophys(b);
#else
		bp->b_un.b_addr = (caddr_t)b;
#endif
		bp->b_bufsize = v.v_sbufsz;
		b += v.v_sbufsz;
		binshash(bp, &bfreelist[BQ_AGE]);
		bp->b_flags = B_BUSY|B_INVAL;
/*
**	this notdef'd code shoudl not be here.  it breaks AES
** 	doesn't lock sysreg, and if the kernel is the right size
**	will not allow it to boot.
*/
#ifdef notdef
		if ((((int)bp->b_un.b_addr)&POFFMASK) == 0) {
			memlock();
			if(ptmemall(&sysreg, svtopte(bp->b_un.b_addr), 1, 1))
				panic("binit - ptmemall failed");
			memunlock();
		}
#endif
		brelse(bp);
	}
        pfreelist.av_forw = bp = pbuf;
        for (; bp < &pbuf[(short)(v.v_pbuf-1)]; bp++)
                bp->av_forw = bp+1;
        bp->av_forw = NULL;
}

/*
 * iinit is called once (from main) very early in initialization.
 * It reads the root's super block and initializes the current date
 * from the last modified date.
 *
 * panic: cannot mount the root -- cannot read the super block.
 * Usually because of an IO error.
 */
iinit()
{
	/*
	 * mount the root (gets rootdir)
	 */
	vfs_mountroot();
	/*
	 * get vnodes and do open call for swapdev and pipedev
	 */
	swapdev_vp = devtovp(swapdev);
	pipedev_vp = devtovp(pipedev);
#ifndef	PASS_MAJOR
	(*bdevsw[major(pipedev)].d_open)(minor(pipedev), FREAD | FWRITE | FKERNEL);
	(*bdevsw[major(swapdev)].d_open)(minor(swapdev), FREAD | FWRITE | FKERNEL);
#else	PASS_MAJOR
	(*bdevsw[major(pipedev)].d_open)(pipedev, FREAD | FWRITE | FKERNEL);
	(*bdevsw[major(swapdev)].d_open)(swapdev, FREAD | FWRITE | FKERNEL);
#endif	PASS_MAJOR
}
