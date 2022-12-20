#ifndef lint	/* .../sys/psn/os/5_vfsops.c */
#define _AC_NAME Z_5_vfsops_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1983-87 Sun Microsystems Inc., All Rights Reserved.  {Apple version 1.4 87/11/11 21:33:12}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.4 of 5_vfsops.c on 87/11/11 21:33:12";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*      @(#)ufs_vfsops.c 1.1 86/02/03 SMI; from UCB 4.1 83/05/27        */
/*      @(#)ufs_vfsops.c        2.2 86/05/14 NFSSRC */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/page.h"
#include "sys/region.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/buf.h"
#include "sys/pathname.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/uio.h"
#include "sys/conf.h"
#include "svfs/filsys.h"
#include "svfs/mount.h"
#include "svfs/inode.h"
#include "sys/var.h"
#include "sys/sysmacros.h"
#undef	NFS
#include "sys/mount.h"
#include "sys/ssioctl.h"

/*
 * svfs vfs operations.
 */
extern int svfs_mount();
extern int svfs_unmount();
extern int svfs_root();
extern int svfs_statfs();
extern int svfs_sync();

struct vfsops svfs_vfsops = {
	svfs_mount,
	svfs_unmount,
	svfs_root,
	svfs_statfs,
	svfs_sync,
};

/*
 * this is the default filesystem type.
 * this should be setup by the configurator
 */
extern int svfs_mountroot();
int (*rootfsmount)() = svfs_mountroot;

/*
 * Default device to mount on.
 */
extern dev_t rootdev;

/*
 * svfs_mount system call
 */
svfs_mount(vfsp, path, data)
	struct vfs *vfsp;
	char *path;
	caddr_t data;
{
	int error;
	dev_t dev;
	struct vnode *vp;
	struct svfs_args args;

	/*
	 * Get arguments
	 */
	error = copyin(data, (caddr_t)&args, sizeof (struct svfs_args));
	if (error) {
		return (error);
	}
	if ((error = getmdev(args.fspec, &dev)) != 0)
		return (error);
	/*
	 * Mount the filesystem.
	 */
	error = mountfs(dev, path, vfsp);
	return (error);
}

/*
 * Called by vfs_mountroot when svfs is going to be mounted as root
 */
svfs_mountroot()
{
	struct vfs *vfsp;
	register struct filsys *fsp;
	register int error;

	vfsp = (struct vfs *)kmem_alloc(sizeof (struct vfs));
        VFS_INIT(vfsp, &svfs_vfsops, (caddr_t)0);
        error = mountfs(rootdev, "/", vfsp);
        if (error) {
                kmem_free((caddr_t)vfsp, sizeof (struct vfs));
                return (error);
        }
        error = vfs_add((struct vnode *)0, vfsp, 0);
        if (error) {
                unmount1(vfsp, 0);
                kmem_free((caddr_t)vfsp, sizeof (struct vfs));
                return (error);
        }
        vfs_unlock(vfsp);
        fsp = ((struct mount *)(vfsp->vfs_data))->m_bufp->b_un.b_fs;
	clkset(fsp->s_time);
	return (0);
}

int
mountfs(dev, path, vfsp)
	dev_t dev;
	char *path;
	struct vfs *vfsp;
{
	register struct filsys *fsp;
	register struct mount *mp = 0;
	register struct buf *bp = 0;
	struct buf *tp = 0;
	struct vnode *dev_vp;
	int error;

#ifdef	lint
	path = path;
#endif
	/*
	 * Open block device mounted on.
	 * When bio is fixed for vnodes this can all be vnode operations
	 */
	error =
	    (*bdevsw[major(dev)].d_open)(
		dev, (vfsp->vfs_flag & VFS_RDONLY) ? FREAD : FREAD|FWRITE);
	if (error) {
		return (error);
	}
	/*
	 * check for dev already mounted on
	 */
	for (mp = &mounttab[0]; mp < (struct mount *)v.ve_mount; mp++) {
		if (mp->m_bufp != NULL && dev == mp->m_dev) {
			return (EBUSY);
		}
	}
	/*
	 * find empty mount table entry
	 */
	for (mp = &mounttab[0]; mp < (struct mount *)v.ve_mount; mp++) {
		if (mp->m_bufp == 0)
			goto found;
	}
	return (EBUSY);
found:
	vfsp->vfs_data = (caddr_t)mp;
        mp->m_vfsp = vfsp;
	/*
	 * read in superblock
	 */
	dev_vp = devtovp(dev);
	tp = bread(dev_vp, SBLOCK, SBSIZE);
	if (tp->b_flags & B_ERROR) {
		error = EIO;
		goto out;
	}
	/*
	 * Copy the super block into a buffer in its native size.
	 */
	mp->m_bufp = tp;	/* just to reserve this slot */
	mp->m_dev = NODEV;
	fsp = tp->b_un.b_fs;
	if (fsp->s_magic != FsMAGIC) {
		tp->b_flags |= B_NOCACHE;
		error = EINVAL;
		goto out;
	}
	bp = geteblk(SBSIZE);
	mp->m_bufp = bp;
	bcopy((caddr_t)tp->b_un.b_addr, (caddr_t)bp->b_un.b_addr,
	   sizeof(struct filsys));
	brelse(tp);
	tp = 0;
	fsp = bp->b_un.b_fs;
	fsp->s_flock = 0;
	fsp->s_ilock = 0;
	fsp->s_ninode = 0;
	fsp->s_inode[0] = 0;
	if (vfsp->vfs_flag & VFS_RDONLY) {
                fsp->s_ronly = 1;
        } else {
                fsp->s_fmod = 1;
                fsp->s_ronly = 0;
        }
	vfsp->vfs_bsize = FsBSIZE(fsp);
	mp->m_dev = dev;
	VN_RELE(dev_vp);
        (*cdevsw[major(dev)].d_ioctl)(dev, GD_SBZBTMOUNT, 
					&time.tv_sec, FREAD|FWRITE);
	return (0);
out:
	mp->m_bufp = 0;
	if (bp)
		brelse(bp);
	if (tp)
		brelse(tp);
	VN_RELE(dev_vp);
	return (error);
}

/*
 * The System V Interface Definition requires a "umount" operation
 * which takes a device pathname as an argument.  This requires this
 * to be a system call.
 */

umount(uap)
	struct a {
		char	*fspec;
	} *uap;
{
	register struct mount *mp;
	dev_t dev;

	if (!suser())
		return;

	if ((u.u_error = getmdev(uap->fspec, &dev)) != 0)
		return;

	if ((mp = getmp(dev)) == NULL) {
		u.u_error = EINVAL;
		return;
	}

	dounmount(mp->m_vfsp);
}

/*
 * vfs operations
 */

svfs_unmount(vfsp)
	struct vfs *vfsp;
{

	return (unmount1(vfsp, 0));
}

unmount1(vfsp, forcibly)
	register struct vfs *vfsp;
	int forcibly;
{
	dev_t dev;
	register struct mount *mp;
	register struct filsys *fs;
	register int stillopen;
	int flag;

	mp = (struct mount *)vfsp->vfs_data;
	dev = mp->m_dev;
#ifdef QUOTA
	if ((stillopen = iflush(dev, mp->m_qinod)) < 0 && !forcibly)
#else
	if ((stillopen = iflush(dev)) < 0 && !forcibly)
#endif
		return (EBUSY);
	if (stillopen < 0)
		return (EBUSY);			/* XXX */
#ifdef QUOTA
	(void)closedq(mp);
	/*
	 * Here we have to iflush again to get rid of the quota inode.
	 * A drag, but it would be ugly to cheat, & this doesn't happen often
	 */
	(void)iflush(dev, (struct inode *)NULL);
#endif
	fs = mp->m_bufp->b_un.b_fs;
	flag = !fs->s_ronly;
	brelse(mp->m_bufp);
	mp->m_bufp = 0;
	mp->m_dev = 0;
	if (!stillopen) {
		register struct vnode *dev_vp;

		(*cdevsw[major(dev)].d_ioctl)(dev, GD_SBZBTUMOUNT, &time.tv_sec,
			(vfsp->vfs_flag & VFS_RDONLY) ? FREAD : FREAD|FWRITE);
		(*bdevsw[major(dev)].d_close)(dev, flag);
		dev_vp = devtovp(dev);
		binval(dev_vp);
		VN_RELE(dev_vp);
	}
	return (0);
}

/*
 * find root of svfs
 */
int
svfs_root(vfsp, vpp)
	struct vfs *vfsp;
	struct vnode **vpp;
{
	register struct mount *mp;
	register struct inode *ip;

	mp = (struct mount *)vfsp->vfs_data;
	ip = iget(mp->m_dev, mp->m_bufp->b_un.b_fs, (ino_tl)ROOTINO);
	if (ip == (struct inode *)0) {
		return (u.u_error);
	}
	iunlock(ip);
	*vpp = ITOV(ip);
	return (0);
}

/*
 * Get file system statistics.
 */
int
svfs_statfs(vfsp, sbp)
register struct vfs *vfsp;
struct statfs *sbp;
{
	register struct filsys *fsp;

	fsp = ((struct mount *)vfsp->vfs_data)->m_bufp->b_un.b_fs;
	if (fsp->s_magic != FsMAGIC)
		panic("svfs_statfs");
	sbp->f_bsize = FsBSIZE(fsp);
	sbp->f_blocks = fsp->s_fsize;
	sbp->f_bfree = fsp->s_tfree;
	sbp->f_bavail = fsp->s_tfree;
	/*
	 * inodes
	 */
	sbp->f_files =  (long) ((fsp->s_isize - 2) * FsINOPB(fsp));
	sbp->f_ffree = (long) fsp->s_tinode;
	sbp->f_fsid[0] = sbp->f_fsid[1] = 0;
	return (0);
}

/*
 * Flush any pending I/O.
 */
int
svfs_sync()
{
	update();
	return (0);
}

/*
 * Common code for mount and umount.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */
static int
getmdev(fspec, pdev)
	char *fspec;
	dev_t *pdev;
{
	register int error;
	struct vnode *vp;

	/*
	 * Get the device to be mounted
	 */
	error =
	    lookupname(fspec, UIOSEG_USER, FOLLOW_LINK,
		(struct vnode **)0, &vp);
	if (error)
		return (error);
	if (vp->v_type != VBLK) {
		VN_RELE(vp);
		return (ENOTBLK);
	}
	*pdev = vp->v_rdev;
	VN_RELE(vp);
	if (major(*pdev) >= bdevcnt)
		return (ENXIO);
	return (0);
}

sbupdate(mp)
	struct mount *mp;
{
	register struct filsys *fs = mp->m_bufp->b_un.b_fs;
	register struct buf *bp;
	register struct vnode *dev_vp;

	dev_vp = devtovp(mp->m_dev);
	bp = getblk(dev_vp, SBLOCK, SBSIZE);
	bcopy((caddr_t)fs, bp->b_un.b_addr, SBSIZE);
	bwrite(bp);
	VN_RELE(dev_vp);
}
