#ifndef lint	/* .../sys/COMMON/os/vfs_io.c */
#define _AC_NAME vfs_io_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1983-87 Sun Microsystems Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.4 87/11/11 21:13:22}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.4 of vfs_io.c on 87/11/11 21:13:22";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*      @(#)vfs_io.c 1.1 86/02/03 SMI   */
/*      NFSSRC @(#)vfs_io.c     2.1 86/04/15 */
#include "compat.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/signal.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/page.h"
#include "sys/region.h"
#endif PAGING
#include "sys/time.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/ostat.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "sys/errno.h"
#include "sys/var.h"
#include "svfs/mount.h"

int vno_rw();
int vno_ioctl();
int vno_select();
int vno_close();
int vno_stat();

struct fileops vnodefops = {
        vno_rw,
        vno_ioctl,
        vno_select,
        vno_close,
	vno_stat
};

int
vno_rw(fp, rw, uiop)
	struct file *fp;
	enum uio_rw rw;
	struct uio *uiop;
{
	register struct vnode *vp;
	register int count;
	register int error;

	vp = (struct vnode *)fp->f_data;
	/*
	 * If write make sure filesystem is writable
	 */
	if ((rw == UIO_WRITE) && (vp->v_vfsp->vfs_flag & VFS_RDONLY))
		return(EROFS);
	count = uiop->uio_resid;
	if (vp->v_type == VREG || vp->v_type == VFIFO) {
		error =
		    VOP_RDWR(vp, uiop, rw,
			((fp->f_flag & FAPPEND) != 0?
			    IO_APPEND|IO_UNIT: IO_UNIT), fp->f_cred);
	} else {
		error =
		    VOP_RDWR(vp, uiop, rw,
			((fp->f_flag & FAPPEND) != 0?
			    IO_APPEND: 0), fp->f_cred);
	}
	if (error)
		return(error);
	if (fp->f_flag & FAPPEND) {
		/*
		 * The actual offset used for append is set by VOP_RDWR
		 * so compute actual starting location
		 */
		fp->f_offset = uiop->uio_offset - (count - uiop->uio_resid);
	}
	return(0);
}

int
vno_ioctl(fp, com, data)
	struct file *fp;
	int com;
	caddr_t data;
{
	struct vattr vattr;
	struct vnode *vp;
	int error = 0;

	vp = (struct vnode *)fp->f_data;
	switch((int) vp->v_type) {

	case VREG:
	case VDIR:
	case VFIFO:
		switch (com) {

		case FIONREAD:
			error = VOP_GETATTR(vp, &vattr, u.u_cred);
			if (error == 0)
				if (vp->v_type==VFIFO)
					*(off_t *) data = vattr.va_size;
				else
					*(off_t *) data = vattr.va_size - fp->f_offset;
			break;

		case FIONBIO:
		case FIOASYNC:
			break;

		default:
			error = ENOTTY;
			break;
		}
		break;

	case VCHR:
		u.u_rval1 = 0;
		if ((u.u_procp->p_compatflags & COMPAT_SYSCALLS) && save(u.u_qsav)) {
			u.u_eosys = RESTARTSYS;
		} else {
			error = VOP_IOCTL(vp, com, data, fp->f_flag,fp->f_cred);
		}
		break;

	default:
		error = ENOTTY;
		break;
	}
	return (error);
}

int
vno_select(fp, flag)
	struct file *fp;
	int flag;
{
	struct vnode *vp;

	vp = (struct vnode *)fp->f_data;
	switch((int) vp->v_type) {
	case VCHR:
	case VFIFO:
		return (VOP_SELECT(vp, flag, fp->f_cred));

	default:
		/*
		 * Always selected
		 */
		return (1);
	}
}

int
vno_stat(vp, sb)
	register struct vnode *vp;
	register struct stat *sb;
{
	register int error;
	struct vattr vattr;

	error = VOP_GETATTR(vp, &vattr, u.u_cred);
	if (error)
		return (error);
	sb->st_mode = vattr.va_mode;
	sb->st_uid = vattr.va_uid;
	sb->st_gid = vattr.va_gid;
	sb->st_dev = vattr.va_fsid;
	sb->st_ino = vattr.va_nodeid;
	sb->st_inol = vattr.va_nodeid;
	sb->st_nlink = vattr.va_nlink;
	sb->st_size = vattr.va_size;
        sb->st_blksize = vattr.va_blocksize;
	sb->st_atime = vattr.va_atime.tv_sec;
	sb->st_mtime = vattr.va_mtime.tv_sec;
        sb->st_spare2 = 0;
	sb->st_ctime = vattr.va_ctime.tv_sec;
        sb->st_spare3 = 0;
	sb->st_rdev = (dev_t)vattr.va_rdev;
        sb->st_blocks = vattr.va_blocks;
        sb->st_spare4[0] = sb->st_spare4[1] = 0;
	return (0);
}

int
vno_close(fp)
	register struct file *fp;
{
	register struct vnode *vp;
	register struct file *ffp;
	register struct vnode *tvp;
	register struct mount *mp;
	register enum vtype type;
	register dev_t dev;

	vp = (struct vnode *)fp->f_data;
	if (fp->f_flag & (FSHLOCK | FEXLOCK))
		vno_unlock(fp, (FSHLOCK | FEXLOCK));

	type = vp->v_type;
	if ((type == VBLK) || (type == VCHR)) {
		/*
		 * check that another vnode for the same device isn't active.
		 * This is because the same device can be referenced by two
		 * different vnodes.
		 */
		dev = vp->v_rdev;
		for (ffp = file; ffp < (struct file *) v.ve_file; ffp++) {
			if (ffp == fp)
				continue;
			if (ffp->f_type != DTYPE_VNODE)		/* XXX */
				continue;
			if (ffp->f_count &&
			    (tvp = (struct vnode *)ffp->f_data) &&
			    tvp->v_rdev == dev && tvp->v_type == type) {
				VN_RELE(vp);
				return (0);
			}
		}
		if(type == VBLK) {
			for (mp = &mounttab[0]; 
			    mp < (struct mount *)v.ve_mount; mp++) {
				if (mp->m_bufp != NULL && dev == mp->m_dev) {
					VN_RELE(vp);
					return (0);
				}
			}
		}
	}
	u.u_error = vn_close(vp, fp->f_flag);
	VN_RELE(vp);
	return (u.u_error);
}

/*
 * Place an advisory lock on an inode.
 */
int
vno_lock(fp, cmd)
	register struct file *fp;
	int cmd;
{
	register int priority;
	register struct vnode *vp;

	/*
	 * Avoid work.
	 */
	if ((fp->f_flag & FEXLOCK) && (cmd & LOCK_EX) ||
	    (fp->f_flag & FSHLOCK) && (cmd & LOCK_SH))
		return (0);

	priority = PLOCK;
	vp = (struct vnode *)fp->f_data;

	if ((cmd & LOCK_EX) == 0)
		priority++;
	/*
	 * If there's a exclusive lock currently applied
	 * to the file, then we've gotta wait for the
	 * lock with everyone else.
	 */
again:
	while (vp->v_flag & VEXLOCK) {
		/*
		 * If we're holding an exclusive
		 * lock, then release it.
		 */
		if (fp->f_flag & FEXLOCK) {
			vno_unlock(fp, FEXLOCK);
			goto again;
		}
		if (cmd & LOCK_NB)
			return (EWOULDBLOCK);
		vp->v_flag |= VLWAIT;
		(void) sleep((caddr_t)&vp->v_exlockc, priority);
	}
	if (cmd & LOCK_EX) {
		cmd &= ~LOCK_SH;
		/*
		 * Must wait for any shared locks to finish
		 * before we try to apply a exclusive lock.
		 */
		while (vp->v_flag & VSHLOCK) {
			/*
			 * If we're holding a shared
			 * lock, then release it.
			 */
			if (fp->f_flag & FSHLOCK) {
				vno_unlock(fp, FSHLOCK);
				goto again;
			}
			if (cmd & LOCK_NB)
				return (EWOULDBLOCK);
			vp->v_flag |= VLWAIT;
			(void) sleep((caddr_t)&vp->v_shlockc, PLOCK);
		}
	}
	if (fp->f_flag & (FSHLOCK|FEXLOCK))
		panic("vno_lock");
	if (cmd & LOCK_SH) {
		vp->v_shlockc++;
		vp->v_flag |= VSHLOCK;
		fp->f_flag |= FSHLOCK;
	}
	if (cmd & LOCK_EX) {
		vp->v_exlockc++;
		vp->v_flag |= VEXLOCK;
		fp->f_flag |= FEXLOCK;
	}
	return (0);
}

/*
 * Unlock a file.
 */
int
vno_unlock(fp, kind)
	register struct file *fp;
	int kind;
{
	register struct vnode *vp;
	register int flags;

	vp = (struct vnode *)fp->f_data;
	kind &= fp->f_flag;
	if (vp == NULL || kind == 0)
		return;
	flags = vp->v_flag;
	if (kind & FSHLOCK) {
		if ((flags & VSHLOCK) == 0)
			panic("vno_unlock: SHLOCK");
		if (--vp->v_shlockc == 0) {
			vp->v_flag &= ~VSHLOCK;
			if (flags & VLWAIT)
				wakeup((caddr_t)&vp->v_shlockc);
		}
		fp->f_flag &= ~FSHLOCK;
	}
	if (kind & FEXLOCK) {
		if ((flags & VEXLOCK) == 0)
			panic("vno_unlock: EXLOCK");
		if (--vp->v_exlockc == 0) {
			vp->v_flag &= ~(VEXLOCK|VLWAIT);
			if (flags & VLWAIT)
				wakeup((caddr_t)&vp->v_exlockc);
		}
		fp->f_flag &= ~FEXLOCK;
	}
}

int
vno_ostat(vp, sb)
	register struct vnode *vp;
	register struct ostat *sb;
{
	register int error;
	struct vattr vattr;

	error = VOP_GETATTR(vp, &vattr, u.u_cred);
	if (error)
		return (error);
	sb->ost_mode = vattr.va_mode;
	sb->ost_uid = vattr.va_uid;
	sb->ost_gid = vattr.va_gid;
	sb->ost_dev = vattr.va_fsid;
	sb->ost_ino = vattr.va_nodeid;
	sb->ost_nlink = vattr.va_nlink;
	sb->ost_size = vattr.va_size;
	sb->ost_atime = vattr.va_atime.tv_sec;
	sb->ost_mtime = vattr.va_mtime.tv_sec;
	sb->ost_ctime = vattr.va_ctime.tv_sec;
	sb->ost_rdev = (dev_t)vattr.va_rdev;
	return (0);
}
