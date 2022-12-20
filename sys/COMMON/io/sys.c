#ifndef lint	/* .../sys/COMMON/io/sys.c */
#define _AC_NAME sys_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:15:06}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of sys.c on 87/11/11 21:15:06";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)sys.c	UniPlus 2.1.3	*/

/*
 *	indirect driver for controlling tty.
 *		now with streams support ....
 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#endif PAGING
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/conf.h"
#ifdef PAGING
#include "sys/page.h"
#include "sys/region.h"
#endif PAGING
#include "sys/proc.h"
#include "sys/ioctl.h"
#include "sys/uio.h"
#include "sys/stream.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/var.h"
#ifdef PAGING
#include "sys/tty.h"
#endif PAGING

static struct vnode *sy_vnode();

/* ARGSUSED */
syopen(dev, flag)
dev_t dev;
{
	int error;
	struct vnode *vp;

	if (error = sycheck())
		return(error);
	if (cdevsw[(short)major(u.u_ttyd)].d_str) {
		vp = sy_vnode(u.u_ttyd);
		if (vp == NULL)
			return(ENXIO);
		return((*stream_open)(&u.u_ttyd, flag, vp));
	}
	return((*cdevsw[(short)major(u.u_ttyd)].d_open)(minor(u.u_ttyd), flag));
}

/* ARGSUSED */
syread(dev, uio)
dev_t dev;
struct uio *uio;
{
	int error;
	struct vnode *vp;

	if (error = sycheck())
		return(error);
	if (cdevsw[(short)major(u.u_ttyd)].d_str) {
		vp = sy_vnode(u.u_ttyd);
		if (vp == NULL)
			return(ENXIO);
		return((*stream_read)(vp, uio));
	}
	return((*cdevsw[(short)major(u.u_ttyd)].d_read)(minor(u.u_ttyd), uio));
}

/* ARGSUSED */
sywrite(dev, uio)
dev_t dev;
struct uio *uio;
{
	int error;
	struct vnode *vp;

	if (error = sycheck())
		return(error);
	if (cdevsw[(short)major(u.u_ttyd)].d_str) {
		vp = sy_vnode(u.u_ttyd);
		if (vp == NULL)
			return(ENXIO);
		return((*stream_write)(vp, uio));
	}
	return((*cdevsw[(short)major(u.u_ttyd)].d_write)(minor(u.u_ttyd), uio));
}

/* ARGSUSED */
syioctl(dev, cmd, arg, mode)
dev_t dev;
{
	int error;
	struct vnode *vp;

	if (cmd == TIOCNOTTY) {
		u.u_ttyp = 0;
		u.u_ttyd = 0;
		u.u_procp->p_pgrp = 0;
		return (0);
	}
	if (error = sycheck())
		return(error);
	if (cdevsw[(short)major(u.u_ttyd)].d_str) {
		vp = sy_vnode(u.u_ttyd);
		if (vp == NULL)
			return(ENXIO);
		return((*stream_ioctl)(vp, cmd, arg, mode, u.u_cred));
	}
	return((*cdevsw[(short)major(u.u_ttyd)].d_ioctl)(minor(u.u_ttyd), cmd, arg, mode));
}

/*ARGSUSED*/
syselect(dev, flag)
	dev_t dev;
	int flag;
{
	int error;

	if (error = sycheck()) {
		u.u_error = error;
		return(0);
	}
	return ((*cdevsw[major(u.u_ttyd)].d_select)(u.u_ttyd, flag));
}

sycheck()
{
	if (u.u_ttyp == NULL) {
		return(ENXIO);
	}
	if (!(u.u_procp->p_flag & SPGRP42) && *u.u_ttyp != u.u_procp->p_pgrp) {
		return(EIO);
	}
	return(0);
}

static struct vnode *
sy_vnode(dev)
dev_t dev;
{
	register struct vnode *vp;
	register struct file *fp;

	for (fp = file; fp < (struct file *)v.ve_file; fp++) {
		if (fp->f_type == DTYPE_VNODE && fp->f_count) {
			vp = (struct vnode *)fp->f_data;
			if (vp->v_type == VCHR && 
			    vp->v_rdev == dev &&
			    vp->v_sptr)
				return(vp);
		}
	}
	return(0);
}
