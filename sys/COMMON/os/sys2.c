#ifndef lint	/* .../sys/COMMON/os/sys2.c */
#define _AC_NAME sys2_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:12:35}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of sys2.c on 87/11/11 21:12:35";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)sys2.c	UniPlus 2.1.3	*/

/*	@(#)sys_generic.c 1.1 85/05/30 SMI; from UCB 5.42 83/06/24	*/

#include "compat.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/page.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/signal.h"
#ifdef PAGING
#include "sys/seg.h"
#endif PAGING
#include "sys/user.h"
#include "sys/ioctl.h"
#include "sys/tty.h"
#include "sys/sxt.h"
#include "sys/termio.h"
#include "sys/file.h"
#ifdef PAGING
#include "sys/region.h"
#endif PAGING
#include "sys/proc.h"
#include "sys/uio.h"
#include "sys/stat.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/vnode.h"
#include "sys/sysinfo.h"
#include "sys/diskformat.h"
#include "sys/disktune.h"
#include "sys/sysmacros.h"

/*
 * Read system call.
 */
read()
{
	register struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap = (struct a *)u.u_ap;
	struct uio auio;
	struct iovec aiov;

	sysinfo.sysread++;
	aiov.iov_base = (caddr_t)uap->cbuf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	rwuio(&auio, UIO_READ);
}

readv()
{
	register struct a {
		int	fdes;
		struct	iovec *iovp;
		int	iovcnt;
	} *uap = (struct a *)u.u_ap;
	struct uio auio;
	struct iovec aiov[16];		/* XXX */

	if (uap->iovcnt <= 0 || uap->iovcnt > sizeof(aiov)/sizeof(aiov[0])) {
		u.u_error = EINVAL;
		return;
	}
	sysinfo.sysread++;
	auio.uio_iov = aiov;
	auio.uio_iovcnt = uap->iovcnt;
	u.u_error = copyin((caddr_t)uap->iovp, (caddr_t)aiov,
	    (unsigned)(uap->iovcnt * sizeof (struct iovec)));
	if (u.u_error)
		return;
	rwuio(&auio, UIO_READ);
}

/*
 * Write system call
 */
write()
{
	register struct a {
		int	fdes;
		char	*cbuf;
		int	count;
	} *uap = (struct a *)u.u_ap;
	struct uio auio;
	struct iovec aiov;

	sysinfo.syswrite++;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	aiov.iov_base = uap->cbuf;
	aiov.iov_len = uap->count;
	rwuio(&auio, UIO_WRITE);
}

writev()
{
	register struct a {
		int	fdes;
		struct	iovec *iovp;
		int	iovcnt;
	} *uap = (struct a *)u.u_ap;
	struct uio auio;
	struct iovec aiov[16];		/* XXX */

	if (uap->iovcnt <= 0 || uap->iovcnt > sizeof(aiov)/sizeof(aiov[0])) {
		u.u_error = EINVAL;
		return;
	}
	sysinfo.syswrite++;
	auio.uio_iov = aiov;
	auio.uio_iovcnt = uap->iovcnt;
	u.u_error = copyin((caddr_t)uap->iovp, (caddr_t)aiov,
	    (unsigned)(uap->iovcnt * sizeof (struct iovec)));
	if (u.u_error)
		return;
	rwuio(&auio, UIO_WRITE);
}

rwuio(uio, rw)
	register struct uio *uio;
	enum uio_rw rw;
{
	struct a {
		int	fdes;
	};
	register struct file *fp;
	register struct iovec *iov;
	int i, count;

	GETF(fp, ((struct a *)u.u_ap)->fdes);
	if ((fp->f_flag&(rw==UIO_READ ? FREAD : FWRITE)) == 0) {
		u.u_error = EBADF;
		return;
	}
	u.u_fmode = fp->f_flag;
	uio->uio_resid = 0;
	uio->uio_seg = 0;
	iov = uio->uio_iov;
	for (i = 0; i < uio->uio_iovcnt; i++) {
		if (iov->iov_len < 0) {
			u.u_error = EINVAL;
			return;
		}
		uio->uio_resid += iov->iov_len;
		if (uio->uio_resid < 0) {
			u.u_error = EINVAL;
			return;
		}
		iov++;
	}
	count = uio->uio_resid;
	uio->uio_offset = fp->f_offset;
	if ((u.u_procp->p_compatflags & COMPAT_SYSCALLS) && save(u.u_qsav)) {
		if (uio->uio_resid == count)
			u.u_eosys = RESTARTSYS;
	} else
		u.u_error = (*fp->f_ops->fo_rw)(fp, rw, uio);
	u.u_rval1 = count - uio->uio_resid;
	fp->f_offset += u.u_rval1;
	if (rw == UIO_READ)
		sysinfo.readch += (unsigned) u.u_rval1;
	else
		sysinfo.writech += (unsigned) u.u_rval1;
}

/*
 * Ioctl system call
 */
ioctl()
{
        register struct file *fp;
        register struct a {
                int     fdes;
                int     cmd;
                caddr_t	cmarg;
        } *uap;
        register int    com;
        register u_int size;
        char data[IOCPARM_MASK+1];

        uap = (struct a *)u.u_ap;
        if ((fp = getf(uap->fdes)) == NULL)
                return;
        if ((fp->f_flag & (FREAD|FWRITE)) == 0) {
                u.u_error = EBADF;
                return;
        }
	com = uap->cmd;

        /*
         * Map old style ioctl's into new for the
         * sake of backwards compatibility (sigh).
         */
	if ((com & (IOC_IN | IOC_OUT | IOC_VOID)) == 0)
		com = mapioctl(com);

        /*
         * Interpret high order word to find
         * amount of data to be copied to/from the
         * user's address space.
         */
        size = (com &~ (IOC_INOUT|IOC_VOID)) >> 16;
        if (size > sizeof (data)) {
                u.u_error = EINVAL;
                return;
        }
        if (com&IOC_IN) {
                if (size == sizeof (int) && uap->cmarg == NULL)
			*(int *)data = 0;
		else if (size > 0) {
                        u.u_error =
                            copyin(uap->cmarg, (caddr_t)data, (u_int)size);
                        if (u.u_error)
                                return;
                } else
                        *(caddr_t *)data = uap->cmarg;
        } else if ((com&IOC_OUT) && size)
                /*
                 * Zero the buffer on the stack so the user
                 * always gets back something deterministic.
                 */
                bzero((caddr_t)data, size);
        else if (com&IOC_VOID)
                *(caddr_t *)data = uap->cmarg;

        switch (com) {
 
        case FIONBIO:
                u.u_error = fset(fp, FNDELAY, *(int *)data);
                return;
         
        case FIOASYNC:
                u.u_error = fset(fp, FASYNC, *(int *)data);
                return;
         
        case FIOSETOWN:
                u.u_error = fsetown(fp, *(int *)data);
                return;
         
        case FIOGETOWN:
                u.u_error = fgetown(fp, (int *)data);
                return;
        }       
        u.u_error = (*fp->f_ops->fo_ioctl)(fp, com, data);
        /*      
         * Copy any data to user, size was
         * already set and checked above.
         */     
        if (u.u_error == 0 && (com&IOC_OUT) && size)
                u.u_error = copyout(data, uap->cmarg, (u_int)size);
}

mapioctl(com)
{
#ifdef	DEBUG
	static int badcom = 0;
#endif

        switch (com) {
		/* for kernel profiling */
		case 1:
			return(_IO(0, 1));
		case 2:
			return(_IO(0, 2));
		case 3:
			return(_IO(0, 3));

		case (('D' << 8) | 0):
			return(LDOPEN);

		case (('D' << 8) | 1):
			return(LDCLOSE);

		case (('D' << 8) | 2):
			return(LDCHG);

		case (('D' << 8) | 8):
			return(LDGETT);

		case (('D' << 8) | 9):
			return(LDSETT);

                case (('F' << 8) | 127):
                        return (FIONREAD);
                
		case (('T' << 8) | 1):
			return(TCGETA);

		case (('T' << 8) | 2):
			return(TCSETA);

		case (('T' << 8) | 3):
			return(TCSETAW);

		case (('T' << 8) | 4):
			return(TCSETAF);

		case (('T' << 8) | 5):
			return(TCSBRK);

		case (('T' << 8) | 6):
			return(TCXONC);

		case (('T' << 8) | 7):
			return(TCFLSH);

		case (('U' << 8) | 0):
			return(UIOCFORMAT);

		case (('U' << 8) | 1):
			return(UIOCEXTE);

		case (('U' << 8) | 2):
			return(UIOCNEXTE);

		case (('U' << 8) | 3):
			return(UIOCWCHK);

		case (('U' << 8) | 4):
			return(UIOCNWCHK);

		case (('U' << 8) | 8):
			return(UIOCGETDT);

		case (('U' << 8) | 9):
			return(UIOCSETDT);

		case (('U' << 8) | 10):
			return(UIOCBDBK);

		case (('b' << 8) | 0):
			return(SXTIOCLINK);

		case (('b' << 8) | 1):
			return(SXTIOCTRACE);

		case (('b' << 8) | 2):
			return(SXTIOCNOTRACE);

		case (('b' << 8) | 3):
			return(SXTIOCSWTCH);

		case (('b' << 8) | 4):
			return(SXTIOCWF);

		case (('b' << 8) | 5):
			return(SXTIOCBLK);

		case (('b' << 8) | 6):
			return(SXTIOCUBLK);

		case (('b' << 8) | 7):
			return(SXTIOCSTAT);

                case (('f' << 8) | 125):
                        return (FIOASYNC);
 
                case (('f' << 8) | 126):
                        return (FIONBIO);
 
                default:
#ifdef	DEBUG
			if (((com & (IOC_IN | IOC_OUT | IOC_VOID)) == 0) && com != badcom) {
				printf("mapioctl:  unknown ioctl:  0x%x\n", com);
				badcom = com;
			}
#endif
                        return (com);
                        /*NOTREACHED*/
        }
}
/* <@(#)sys2.c	6.2> */
