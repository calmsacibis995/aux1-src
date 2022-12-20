#ifndef lint	/* .../sys/COMMON/os/sys3.c */
#define _AC_NAME sys3_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1983-87 Sun Microsystems Inc., 1980-87 The Regents of the University of California, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:12:39}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of sys3.c on 87/11/11 21:12:39";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)sys3.c	UniPlus 2.1.2	*/

/*	@(#)kern_descrip.c 1.1 85/05/30 SMI; from UCB 6.3 83/11/18	*/

#include "sys/param.h"
#include "sys/types.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/uio.h"
#ifdef PAGING
#include "sys/seg.h"
#endif PAGING
#include "sys/user.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#ifdef PAGING
#include "sys/region.h"
#endif PAGING
#include "sys/proc.h"
#include "sys/conf.h"
#include "sys/file.h"
#include "sys/stat.h"
#include "sys/errno.h"
#include "sys/ioctl.h"
#include "sys/var.h"
#include "sys/acct.h"
#include "sys/flock.h"
#include "sys/sysinfo.h"

/*
 * Descriptor management.
 */

/*
 * TODO:
 *	increase NOFILE
 *	eliminate u.u_error side effects
 */

/*
 * System calls on descriptors.
 */
getdtablesize()
{

	u.u_rval1 = NOFILE;
}

getdopt()
{

}

setdopt()
{

}

dup()
{
	register struct a {
		int	i;
	} *uap = (struct a *) u.u_ap;
	struct file *fp;
	int j;

	if (uap->i &~ 077) { uap->i &= 077; dup2(); return; }	/* XXX */

	fp = getf(uap->i);
	if (fp == 0)
		return;
	j = ufalloc(0);
	if (j < 0)
		return;
	dupit(j, fp, u.u_pofile[uap->i]);
}

dup2()
{
	register struct a {
		int	i, j;
	} *uap = (struct a *) u.u_ap;
	register struct file *fp;
	register struct user *up;

	up = &u;
	fp = getf(uap->i);
	if (fp == 0)
		return;
	if (uap->j < 0 || uap->j >= NOFILE) {
		up->u_error = EBADF;
		return;
	}
	up->u_rval1 = uap->j;
	if (uap->i == uap->j)
		return;
	if (up->u_ofile[uap->j]) {
		closef(up->u_ofile[uap->j]);
		if (up->u_error)
			return;
	}
	dupit(uap->j, fp, up->u_pofile[uap->i]);
}

dupit(fd, fp, flags)
	int fd;
	register struct file *fp;
	register int flags;
{
	register struct user *up;

	up = &u;
	up->u_ofile[fd] = fp;
	up->u_pofile[fd] = flags & ~UF_EXCLOSE;
	fp->f_count++;
}

/*
 * The file control system call.
 */
fcntl()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;
 	struct flock bf;
	int error;
	register i;
	register char *pop;
	register struct user *up = &u;

	uap = (struct a *)up->u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	pop = &up->u_pofile[uap->fdes];
	switch(uap->cmd) {
	case F_DUPFD:
		i = uap->arg;
		if (i < 0 || i >= NOFILE) {
			up->u_error = EINVAL;
			return;
		}
		if ((i = ufalloc(i)) < 0)
			return;
		dupit(i, fp, *pop);
		break;

	case F_GETFD:
		up->u_rval1 = *pop;
		break;

	case F_SETFD:
		*pop = uap->arg;
		break;

	case F_GETFL:
		up->u_rval1 = fp->f_flag+FOPEN;
		break;

	case F_SETFL:
		fp->f_flag &= FCNTLCANT;
		fp->f_flag |= (uap->arg-FOPEN) &~ FCNTLCANT;
		up->u_error = fset(fp, FNDELAY, fp->f_flag & FNDELAY);
		if (up->u_error)
			break;
		up->u_error = fset(fp, FASYNC, fp->f_flag & FASYNC);
		if (up->u_error)
			(void) fset(fp, FNDELAY, 0);
		break;

	case F_GETLK:
		/* get record lock */
		up->u_error = copyin((caddr_t)uap->arg, (caddr_t)&bf, sizeof bf);
		if (up->u_error)
			break;
		up->u_error = getflck(fp, &bf);
		if (up->u_error)
			break;
		up->u_error = copyout((caddr_t)&bf, (caddr_t)uap->arg, sizeof bf);
		break;

	case F_SETLK:
		/* set record lock and return if blocked */
		up->u_error = copyin((caddr_t)uap->arg, (caddr_t)&bf, sizeof bf);
		if (up->u_error)
			break;
		up->u_error = setflck(fp, &bf, 0);
		break;

	case F_SETLKW:
		/* set record lock and wait if blocked */
		up->u_error = copyin((caddr_t)uap->arg, (caddr_t)&bf, sizeof bf);
		if (up->u_error)
			break;
		up->u_error = setflck(fp, &bf, 1);
		break;

	case F_GETOWN:
		up->u_error = fgetown(fp, &up->u_rval1);
		break;

	case F_SETOWN:
		up->u_error = fsetown(fp, uap->arg);
		break;

	default:
		up->u_error = EINVAL;
	}
}

fset(fp, bit, value)
	struct file *fp;
	int bit, value;
{

	if (value)
		fp->f_flag |= bit;
	else
		fp->f_flag &= ~bit;
	return (fioctl(fp, (int)(bit == FNDELAY ? FIONBIO : FIOASYNC),
	    (caddr_t)&value));
}

fgetown(fp, valuep)
	struct file *fp;
	int *valuep;
{
	int error;

	switch (fp->f_type) {

	case DTYPE_SOCKET:
		*valuep = ((struct socket *)fp->f_data)->so_pgrp;
		return (0);

	default:
		error = fioctl(fp, (int)TIOCGPGRP, (caddr_t)valuep);
		*valuep = -*valuep;
		return (error);
	}
}

fsetown(fp, value)
	struct file *fp;
	int value;
{

	if (fp->f_type == DTYPE_SOCKET) {
		((struct socket *)fp->f_data)->so_pgrp = value;
		return (0);
	}
	if (value > 0) {
		struct proc *p = pfind(value);
		if (p == 0)
			return (EINVAL);
		value = p->p_pgrp;
	} else
		value = -value;
	return (fioctl(fp, (int)TIOCSPGRP, (caddr_t)&value));
}

fioctl(fp, cmd, value)
	struct file *fp;
	int cmd;
	caddr_t value;
{

	return ((*fp->f_ops->fo_ioctl)(fp, cmd, value));
}

close()
{
	register struct a {
		int	i;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register u_char *pf;
	register struct user *up;

	up = &u;
	fp = getf(uap->i);
	if (fp == 0)
		return;
	pf = (u_char *)&up->u_pofile[uap->i];
	up->u_ofile[uap->i] = NULL;
	*pf = 0;
	closef(fp);
	/* WHAT IF up->u_error ? */
}

fstat()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		struct	stat *sb;
	} *uap;
	struct stat ub;
	register struct user *up;

	up = &u;
	uap = (struct a *)up->u_ap;
	fp = getf(uap->fdes);
	if (fp == 0)
		return;
	up->u_error = (*fp->f_ops->fo_stat)(fp->f_data, &ub);
	if (up->u_error == 0)
		up->u_error = copyout((caddr_t)&ub, (caddr_t)uap->sb,
		    sizeof (ub));
}

/*
 * Allocate a user file descriptor.
 */
ufalloc(i)
	register int i;
{
	register struct user *up;

	up = &u;
	for (; i < NOFILE; i++)
		if (up->u_ofile[i] == NULL) {
			up->u_rval1 = i;
			up->u_pofile[i] = 0;
			return (i);
		}
	up->u_error = EMFILE;
	return (-1);
}

ufavail()
{
	register int i, avail = 0;

	for (i = 0; i < NOFILE; i++)
		if (u.u_ofile[i] == NULL)
			avail++;
	return (avail);
}

struct	file *lastf;
/*
 * Allocate a user file descriptor
 * and a file structure.
 * Initialize the descriptor
 * to point at the file structure.
 */
struct file *
falloc()
{
	register struct file *fp;
	register i;
	register struct user *up;

	up = &u;
	i = ufalloc(0);
	if (i < 0)
		return (NULL);
	if (lastf == 0)
		lastf = file;
	for (fp = lastf; fp < (struct file *) v.ve_file; fp++)
		if (fp->f_count == 0)
			goto slot;
	for (fp = file; fp < lastf; fp++)
		if (fp->f_count == 0)
			goto slot;
	tablefull("file");
	syserr.fileovf++;
	up->u_error = ENFILE;
	return (NULL);
slot:
	up->u_ofile[i] = fp;
	fp->f_count = 1;
	fp->f_data = 0;
	fp->f_offset = 0;
	crhold(up->u_cred);
	fp->f_cred = up->u_cred;
	lastf = fp + 1;
	return (fp);
}

/*
 * Convert a user supplied file descriptor into a pointer
 * to a file structure.  Only task is to check range of the descriptor.
 * Critical paths should use the GETF macro.
 */
struct file *
getf(f)
	register int f;
{
	register struct file *fp;

	if ((unsigned)f >= NOFILE || (fp = u.u_ofile[f]) == NULL) {
		u.u_error = EBADF;
		return (NULL);
	}
	return (fp);
}

/*
 * Internal form of close.
 * Decrement reference count on file structure.
 * If last reference not going away, but no more
 * references except in message queues, run a
 * garbage collect.  This would better be done by
 * forcing a gc() to happen sometime soon, rather
 * than running one each time.
 */
closef(fp)
	register struct file *fp;
{

	if (fp == NULL)
		return;
	cleanlocks(fp);
	if (fp->f_count > 1) {
		fp->f_count--;
		return;
	}
	if (fp->f_count < 1)
		panic("closef: count < 1");
	(*fp->f_ops->fo_close)(fp);
	crfree(fp->f_cred);
	fp->f_count = 0;
}

/*
 * Apply an advisory lock on a file descriptor.
 */
flock()
{
	register struct a {
		int	fd;
		int	how;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;

	fp = getf(uap->fd);
	if (fp == NULL)
		return;
	if (fp->f_type != DTYPE_VNODE) {
		u.u_error = EOPNOTSUPP;
		return;
	}
	if (uap->how & LOCK_UN) {
		vno_unlock(fp, FSHLOCK|FEXLOCK);
		return;
	}
	/* avoid work... */
	if ((fp->f_flag & FEXLOCK) && (uap->how & LOCK_EX) ||
	    (fp->f_flag & FSHLOCK) && (uap->how & LOCK_SH))
		return;
	u.u_error = vno_lock(fp, uap->how);
}

/*
 * Test if the current user is the super user.
 */
suser()
{

        if (u.u_uid == 0) {
                u.u_acflag |= ASU;
                return(1);
        }
        u.u_error = EPERM;
        return(0);
}
/*
 * Warn that a system table is full.
 */
tablefull(tab)
        char *tab;
{

        printf("%s: table is full\n", tab);
}
