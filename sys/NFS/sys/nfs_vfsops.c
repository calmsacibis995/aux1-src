#ifndef lint	/* .../sys/NFS/sys/nfs_vfsops.c */
#define _AC_NAME nfs_vfsops_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1983-87 Sun Microsystems Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:21:54}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of nfs_vfsops.c on 87/11/11 21:21:54";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/* NFSSRC @(#)nfs_vfsops.c	2.2 86/05/15 */
/*      @(#)nfs_vfsops.c 1.1 86/02/03 SMI      */

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include "sys/param.h"
#include "sys/signal.h"
#include "sys/types.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/pathname.h"
#include "sys/uio.h"
#include "sys/socket.h"
#include "sys/sysent.h"
#include "netinet/in.h"
#include "rpc/types.h"
#include "rpc/xdr.h"
#include "rpc/auth.h"
#include "rpc/clnt.h"
#include "nfs/nfs.h"
#include "nfs/nfs_clnt.h"
#include "nfs/rnode.h"
#include "sys/mount.h"


#ifdef NFSDEBUG
extern int nfsdebug;
#endif

struct vnode *makenfsnode();
int nfsmntno;

/*
 * nfs vfs operations.
 */
int nfs_mount();
int nfs_unmount();
int nfs_root();
int nfs_statfs();
int nfs_sync();

struct vfsops nfs_vfsops = {
	nfs_mount,
	nfs_unmount,
	nfs_root,
	nfs_statfs,
	nfs_sync,
};

nfsinit()
{
	extern int async_daemon();
	extern int exportfs();
	extern int nfs_getfh();
	extern int nfs_svc();
	extern struct vfsops *vfssw[];

	vfssw[MOUNT_NFS] = &nfs_vfsops;

	sysent[119].sy_call = async_daemon;
	sysent[121].sy_call = nfs_svc;
	sysent[122].sy_call = nfs_getfh;
	sysent[140].sy_call = exportfs;
}

/*
 * nfs mount vfsop
 * Set up mount info record and attach it to vfs struct.
 */
/*ARGSUSED*/
nfs_mount(vfsp, path, data)
	struct vfs *vfsp;
	char *path;
	caddr_t data;
{
	int error;
	struct vnode *rootvp = NULL;	/* the server's root */
	struct mntinfo *mi = NULL;	/* mount info, pointed at by vfs */
	struct vattr va;		/* root vnode attributes */
	struct nfsfattr na;		/* root vnode attributes in nfs form */
	struct statfs sb;		/* server's file system stats */
	fhandle_t fh;			/* root fhandle */
	struct nfs_args args;		/* nfs mount arguments */

	/*
	 * get arguments
	 */
	error = copyin(data, (caddr_t)&args, sizeof (args));
	if (error) {
		goto errout;
	}

	/*
	 * create a mount record and link it to the vfs struct
	 */
	mi = (struct mntinfo *)kmem_alloc((u_int)sizeof(*mi));
	mi->mi_refct = 0;
	mi->mi_stsize = 0;
	mi->mi_hard = ((args.flags & NFSMNT_SOFT) == 0);
	if (args.flags & NFSMNT_RETRANS) {
		mi->mi_retrans = args.retrans;
		if (args.retrans < 0) {
			error = EINVAL;
			goto errout;
		}
	} else {
		mi->mi_retrans = NFS_RETRIES;
	}
	if (args.flags & NFSMNT_TIMEO) {
		mi->mi_timeo = args.timeo;
		if (args.timeo <= 0) {
			error = EINVAL;
			goto errout;
		}
	} else {
		mi->mi_timeo = NFS_TIMEO;
	}
	mi->mi_mntno = nfsmntno++;
	mi->mi_printed = 0;
	error = copyin((caddr_t)args.addr, (caddr_t)&mi->mi_addr,
	    sizeof(mi->mi_addr));
	if (error) {
		goto errout;
	}
	/*
	 * For now we just support AF_INET
	 */
	if (mi->mi_addr.sin_family != AF_INET) {
		error = EPFNOSUPPORT;
		goto errout;
	}
	if (args.flags & NFSMNT_HOSTNAME) {
		error = copyin((caddr_t)args.hostname, (caddr_t)mi->mi_hostname,
		    HOSTNAMESZ);
		if (error) {
			goto errout;
		}
	} else {
		addr_to_str(&(mi->mi_addr), mi->mi_hostname);
	}

	vfsp->vfs_data = (caddr_t)mi;

	/*
	 * Make the root vnode
	 */
	error = copyin((caddr_t)args.fh, (caddr_t)&fh, sizeof(fh));
	if (error) {
		goto errout;
	}
	rootvp = makenfsnode(&fh, (struct nfsfattr *) 0, vfsp);
	if (rootvp->v_flag & VROOT) {
		error = EBUSY;
		goto errout;
	}

	/*
	 * get attributes of the root vnode then remake it to include 
	 * the attributes.
	 */
	error = VOP_GETATTR(rootvp, &va, u.u_cred);
	if (error) {
		goto errout;
	}
	VN_RELE(rootvp);
	vattr_to_nattr(&va, &na);
	rootvp = makenfsnode(&fh, &na, vfsp);
	rootvp->v_flag |= VROOT;
	mi->mi_rootvp = rootvp;

	/*
	 * Get server's filesystem stats.  Use these to set transfer
	 * sizes, filesystem block size, and read-only.
	 */
	error = VFS_STATFS(vfsp, &sb);
	if (error) {
		goto errout;
	}
	mi->mi_tsize = min(NFS_MAXDATA, nfstsize());
	if (args.flags & NFSMNT_RSIZE) {
		if (args.rsize <= 0) {
			error = EINVAL;
			goto errout;
		}
		mi->mi_tsize = MIN(mi->mi_tsize, args.rsize);
	}
	if (args.flags & NFSMNT_WSIZE) {
		if (args.wsize <= 0) {
			error = EINVAL;
			goto errout;
		}
		mi->mi_stsize = MIN(mi->mi_stsize, args.wsize);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 1,
	    "nfs_mount: hard %d timeo %d retries %d wsize %d rsize %d\n",
	    mi->mi_hard, mi->mi_timeo, mi->mi_retrans, mi->mi_stsize,
	    mi->mi_tsize);
#endif
	/*
	 * Should set read only here!
	 */

	/*
	 * Set filesystem block size to at least CLBYTES and at most MAXBSIZE
	 */
	mi->mi_bsize = MAX(va.va_blocksize, SBUFSIZE);
	mi->mi_bsize = MIN(mi->mi_bsize, MAXBSIZE);
	vfsp->vfs_bsize = mi->mi_bsize;

errout:
	if (error) {
		if (mi) {
			kmem_free((caddr_t)mi, (u_int)sizeof(*mi));
		}
		if (rootvp) {
			VN_RELE(rootvp);
		}
	}
	return (error);
}

#ifdef notneeded
/*
 * Called by vfs_mountroot when nfs is going to be mounted as root
 */
nfs_mountroot()
{

	return(EOPNOTSUPP);
}
#endif

/*
 * vfs operations
 */

nfs_unmount(vfsp)
	struct vfs *vfsp;
{
	struct mntinfo *mi = (struct mntinfo *)vfsp->vfs_data;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_unmount(%x) mi = %x\n", vfsp, mi);
#endif
	rflush(vfsp);

	/*
	 * free vnodes held in buffer cache
	 */
	if (mi->mi_refct != 1) {
		rinval(vfsp);
	}
	if (mi->mi_refct != 1 || mi->mi_rootvp->v_count != 1) {
		return (EBUSY);
	}
	VN_RELE(mi->mi_rootvp);
	kmem_free((caddr_t)mi, (u_int)sizeof(*mi));
	return(0);
}

/*
 * find root of nfs
 */
int
nfs_root(vfsp, vpp)
	struct vfs *vfsp;
	struct vnode **vpp;
{

	*vpp = (struct vnode *)((struct mntinfo *)vfsp->vfs_data)->mi_rootvp;
	(*vpp)->v_count++;
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_root(0x%x) = %x\n", vfsp, *vpp);
#endif
	return(0);
}

/*
 * Get file system statistics.
 */
int
nfs_statfs(vfsp, sbp)
register struct vfs *vfsp;
struct statfs *sbp;
{
	struct nfsstatfs fs;
	struct mntinfo *mi;
	fhandle_t *fh;
	int error = 0;

	mi = vftomi(vfsp);
	fh = vtofh(mi->mi_rootvp);
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_statfs fh %o %d\n", fh->fh_fsid, fh->fh_fno);
#endif
	error = rfscall(mi, RFS_STATFS, xdr_fhandle,
	    (caddr_t)fh, xdr_statfs, (caddr_t)&fs, u.u_cred);
	if (!error) {
		error = geterrno(fs.fs_status);
	}
	if (!error) {
		if (mi->mi_stsize) {
			mi->mi_stsize = min(mi->mi_stsize, fs.fs_tsize);
		} else {
			mi->mi_stsize = fs.fs_tsize;
		}
		sbp->f_bsize = fs.fs_bsize;
		sbp->f_blocks = fs.fs_blocks;
		sbp->f_bfree = fs.fs_bfree;
		sbp->f_bavail = fs.fs_bavail;
		/*
		 * XXX	Cover up a botch in the protocol:  these two fields
		 *	aren't returned in the nfsstatfs structure.
		 */
		sbp->f_files = 0;
		sbp->f_ffree = 0;
		/*
		 * XXX This is wrong - should be a real fsid
		 */
		bcopy((caddr_t)&fh->fh_fsid, (caddr_t)sbp->f_fsid,
		    sizeof(fsid_t));
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_statfs returning %d\n", error);
#endif
	return (error);
}

/*
 * Flush any pending I/O.
 */
int
nfs_sync(vfsp)
	struct vfs * vfsp;
{

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_sync %x\n", vfsp);
#endif
	rflush(vfsp);
	return(0);
}

static char *
itoa(n, str)
	u_short n;
	char *str;
{
	char prbuf[11];
	register char *cp;

	cp = prbuf;
	do {
		*cp++ = "0123456789"[n%10];
		n /= 10;
	} while (n);
	do {
		*str++ = *--cp;
	} while (cp > prbuf);
	return (str);
}

/*
 * Convert a INET address into a string for printing
 */
static
addr_to_str(addr, str)
	struct sockaddr_in *addr;
	char *str;
{
	str = itoa(addr->sin_addr.s_net, str);
	*str++ = '.';
	str = itoa(addr->sin_addr.s_host, str);
	*str++ = '.';
	str = itoa(addr->sin_addr.s_lh, str);
	*str++ = '.';
	str = itoa(addr->sin_addr.s_impno, str);
	*str = '\0';
}
