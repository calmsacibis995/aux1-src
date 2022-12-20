#ifndef lint	/* .../sys/SVFS/sys/5_subr.c */
#define _AC_NAME Z_5_subr_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1983-87 Sun Microsystems Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/11 21:32:30}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of 5_subr.c on 87/11/11 21:32:30";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*      @(#)ufs_subr.c 1.1 86/02/03 SMI; from UCB 4.5 83/03/21  */
/*      @(#)ufs_subr.c  2.1 86/04/14 NFSSRC */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/buf.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/sysmacros.h"
#ifdef QUOTA
#include "sys/quota.h"
#endif QUOTA
#include "svfs/inode.h"
#include "svfs/mount.h"
#include "svfs/filsys.h"
#include "sys/var.h"

int	syncprt = 0;
static	int updlock = 0;

/*
 * Update is the internal name of 'sync'.  It goes through the disk
 * queues to initiate sandbagged IO; goes through the inodes to write
 * modified nodes; and it goes through the mount table to initiate
 * the writing of the modified super blocks.
 */
update()
{
	register struct inode *ip;
	register struct mount *mp;
	struct filsys *fs;

	if(updlock) {
		updlock |= B_WANTED;
		sleep(&updlock, PRIBIO+1);
		return;
	}
	updlock = B_BUSY;
	/*
	 * Write back modified superblocks.
	 * Consistency check that the superblock
	 * of each file system is still in the buffer cache.
	 */
	for (mp = &mounttab[0]; mp < (struct mount *)v.ve_mount; mp++) {
		if (mp->m_bufp == NULL || mp->m_dev == NODEV)
			continue;
		fs = mp->m_bufp->b_un.b_fs;
		if (fs->s_fmod == 0 || fs->s_ronly != 0)
			continue;
		fs->s_fmod = 0;
		fs->s_time = time.tv_sec;
		sbupdate(mp);
	}
	/*
	 * Write back each (modified) inode.
	 */
	for (ip = inode; ip < (struct inode *)v.ve_inode; ip++) {
		if ((ip->i_flag & ILOCKED) != 0 || (ip->i_flag & IREF) == 0 ||
		    (ip->i_flag & (IACC | IUPD | ICHG)) == 0)
			continue;
		ip->i_flag |= ILOCKED;
		ip->i_lockedfile = __FILE__;
		ip->i_lockedline = __LINE__;
#ifdef ITRACE
		itrace(ip, caller(), 1);
		timeout(ilpanic, ip, 10 * v.v_hz);
#endif
		VN_HOLD(ITOV(ip));
		iupdat(ip, 0);
		iput(ip);
	}
	/*
	 * Force stale buffer cache information to be flushed,
	 * for all devices.
	 */
	bflush((struct vnode *) 0);
	if(updlock & B_WANTED)
		wakeup(&updlock);
	updlock = 0;
}

/*
 * Flush all the blocks associated with an inode.
 * Note that we make a more stringent check of
 * writing out any block in the buffer pool that may
 * overlap the inode. This brings the inode up to
 * date with recent mods to the cooked device.
 */
syncip(ip)
	register struct inode *ip;
{
	long lbn, lastlbn;
	daddr_t blkno;

	lastlbn = howmany(ip->i_size, FsBSIZE(ip->i_fs));
	for (lbn = 0; lbn < lastlbn; lbn++) {
		blkno = FsLTOP(ip->i_fs, bmap(ip, lbn, B_READ));
		blkflush(ip->i_devvp, blkno, (long) FsBSIZE(ip->i_fs));
	}
	imark(ip, ICHG);
	iupdat(ip, 1);
}

struct filsys *
trygetfs(dev)
	dev_t dev;
{
	register struct mount *mp;

	mp = getmp(dev);
	if (mp == NULL) {
		return(NULL);
	}
	return (mp->m_bufp->b_un.b_fs);
}

struct mount *
getmp(dev)
	dev_t dev;
{
	register struct mount *mp;
	register struct filsys *fs;

	for (mp = &mounttab[0]; mp < (struct mount *)v.ve_mount; mp++) {
		if (mp->m_bufp == NULL || mp->m_dev != dev)
			continue;
		fs = mp->m_bufp->b_un.b_fs;
		if (fs->s_magic != FsMAGIC) {
			printf("dev = 0x%x\n", dev);
			panic("getmp: bad magic");
		}
		return (mp);
	}
	return (NULL);
}
