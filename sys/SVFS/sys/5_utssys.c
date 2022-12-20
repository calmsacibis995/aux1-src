#ifndef lint	/* .../sys/SVFS/sys/5_utssys.c */
#define _AC_NAME Z_5_utssys_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1983-87 Sun Microsystems Inc., 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:32:34}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of 5_utssys.c on 87/11/11 21:32:34";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/* @(#)utssys.c	1.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/vfs.h"
#include "svfs/filsys.h"
#include "svfs/mount.h"
#include "sys/signal.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#endif PAGING
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/var.h"
#include "sys/utsname.h"

utssys()
{
	register i;
	register struct a {
		char	*cbuf;
		int	mv;
		int	type;
	} *uap;
	struct {
		daddr_t	f_tfree;
		ino_t	f_tinode;
		char	f_fname[6];
		char	f_fpack[6];
	} ust;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	switch(uap->type) {

case 0:		/* uname */
	if (copyout((caddr_t)&utsname, uap->cbuf, sizeof(struct utsname)))
		up->u_error = EFAULT;
	return;

/* case 1 was umask */

case 2:		/* ustat */
	for(i=0; i<v.v_mount; i++) {
		register struct mount *mp;

		mp = &mounttab[i];
		if (mp->m_bufp && mp->m_dev == uap->mv) {
			register struct filsys *fp;

			fp = mp->m_bufp->b_un.b_fs;
			ust.f_tfree = FsLTOP(fp, fp->s_tfree);
			ust.f_tinode = fp->s_tinode;
			bcopy(fp->s_fname, ust.f_fname, sizeof(ust.f_fname));
			bcopy(fp->s_fpack, ust.f_fpack, sizeof(ust.f_fpack));
			if (copyout((caddr_t)&ust, uap->cbuf, 18))
				up->u_error = EFAULT;
			return;
		}
	}
	up->u_error = EINVAL;
	return;

case 33:	/* uvar */
	if (copyout((caddr_t)&v, uap->cbuf, sizeof(struct var)))
		up->u_error = EFAULT;
	return;

default:
	up->u_error = EFAULT;
	}
}
