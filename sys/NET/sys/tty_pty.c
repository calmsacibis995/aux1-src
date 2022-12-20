#ifndef lint	/* .../sys/NET/sys/tty_pty.c */
#define _AC_NAME tty_pty_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., 1980-87 The Regents of the University of California, 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/11 21:17:51}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of tty_pty.c on 87/11/11 21:17:51";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	pty.c	4.21	82/03/23	*/

/*
 * Pseudo-teletype Driver
 * (Actually two drivers, requiring two entries in 'cdevsw')
 */

/*
 *  billn -- 12/15/82.  Mercilessly hacked for system 3.
 *  5/22/85 billn:	Close each side by hand via ptxproc().  Add
 *			ptsxtproc() for shell layering.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/time.h"
#ifdef PAGING
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/page.h"
#include "sys/region.h"
#endif PAGING
#include "sys/systm.h"
#include "sys/ioctl.h"
#include "sys/tty.h"
#include "sys/ttold.h"
#include "sys/termio.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/buf.h"
#include "sys/file.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/sxt.h"
#include "sys/uio.h"

#define BUFSIZ 100		/* Chunk size iomoved from user */

/*
 * pts == /dev/tty[pP]?
 * ptc == /dev/pty[pP]?
 */

#ifndef AUTOCONFIG
extern struct tty	pts_tty[];
extern struct pt_ioctl	pts_ioctl[];
#else
extern struct tty	*pts_tty;
extern struct pt_ioctl	*pts_ioctl;
#endif AUTOCONFIG

#define	PF_RCOLL	0x01
#define	PF_WCOLL	0x02
#define	PF_NBIO		0x04
#define	PF_PKT		0x08		/* packet mode */
#define	PF_STOPPED	0x10		/* user told stopped */
#define	PF_REMOTE	0x20		/* remote and flow controlled input */
#define	PF_NOSTOP	0x40
#define PF_WTIMER       0x80            /* waiting for timer to flush */
#define ptck_output(tp)	((((tp)->t_state&TTSTOP) == 0) && (tp)->t_outq.c_cc)

int ptproc(), ptxproc();

/*ARGSUSED*/
ptsopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;

	dev =  minor(dev);
	if (dev >= v.v_npty) {
		return(ENXIO);
	}
	tp = &pts_tty[dev];
	if ((tp->t_state & ISOPEN) == 0)
		ttinit(tp);
	while ((tp->t_state & CARR_ON) == 0) {
		tp->t_state |= WOPEN;
		(void) sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	(*linesw[tp->t_line].l_open)(tp);
	return(0);
}

ptsclose(dev)
	dev_t dev;
{
	register struct tty *tp;

	dev = minor(dev);
	tp = &pts_tty[dev];
	tp->t_proc = ptxproc;		/* a no-wait dummy proc */
	(*linesw[tp->t_line].l_close)(tp);
}

ptsread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &pts_tty[minor(dev)];
	register struct pt_ioctl *pti = &pts_ioctl[minor(dev)];
	int rv;

	dev = minor(dev);
	if (tp->t_proc == ptproc)
		rv = (*linesw[tp->t_line].l_read)(tp, uio);
	else
		rv = 0;
	wakeup((caddr_t)&tp->t_rawq.c_cf);
	if (pti->pt_selw) {
		selwakeup(pti->pt_selw, pti->pt_flags & PF_WCOLL);
		pti->pt_selw = 0;
		pti->pt_flags &= ~PF_WCOLL;
	}
	return(rv);
}

/*
 * Write to pseudo-tty.
 * Wakeups of controlling tty will happen
 * indirectly, when tty driver calls ptproc.
 */
ptswrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;
	int rv;

	dev = minor(dev);
	tp = &pts_tty[dev];
	if (tp->t_proc == ptproc)
		rv = (*linesw[tp->t_line].l_write)(tp, uio);
	else
		rv = 0;
	return(rv);
}

ptcwakeup(tp)
	struct tty *tp;
{
	struct pt_ioctl *pti = &pts_ioctl[tp - &pts_tty[0]];
	int s = spl5();         /* any NZ spl will lockout ptctimer */

	if (pti->pt_selr) {
		selwakeup(pti->pt_selr, pti->pt_flags & PF_RCOLL);
		pti->pt_selr = 0;
		pti->pt_flags &= ~PF_RCOLL;
	}
	if (pti->pt_selw) {
		selwakeup(pti->pt_selw, pti->pt_flags & PF_WCOLL);
		pti->pt_selw = 0;
		pti->pt_flags &= ~PF_WCOLL;
	}
	if (tp->t_wsel) {
		if (!tp->t_wsel->p_wchan)
			tp->t_wsel = 0;
		else if (tp->t_outq.c_cc <= ttlowat[tp->t_cflag&CBAUD]) {
			selwakeup (tp->t_wsel, tp->t_state & TS_WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
		}
	}
	wakeup((caddr_t)&tp->t_outq.c_cf);
	splx(s);
}

ptctimer()
{
	register struct tty *tp = &pts_tty[0];
	register struct pt_ioctl *pti = &pts_ioctl[0];
	register i;

	timeout(ptctimer, (caddr_t)0, v.v_hz >> 2);
	for (i = 0; i < v.v_npty; i++, pti++, tp++) {
		if (((pti->pt_flags & PF_WTIMER) == 0) && !tp->t_wsel)
			continue;
		pti->pt_flags &= ~PF_WTIMER;
		if (tp->t_proc == 0)
			continue;
		ptcwakeup(tp);
	}
}

/*ARGSUSED*/
ptcopen(dev, flag)
dev_t dev;
int flag;
{
	register struct tty *tp;
	struct pt_ioctl *pti;
	static first;

	dev = minor(dev);
	if (dev >= v.v_npty) {
		return(ENXIO);
	}
	if (first == 0) {
		first++;
		ptctimer();
	}
	tp = &pts_tty[dev];
	if (tp->t_state & CARR_ON) {
		return(EIO);
	}
	tp->t_iflag = ICRNL|ISTRIP|IGNPAR;
	tp->t_oflag = OPOST|ONLCR|TAB3;
	tp->t_lflag = ISIG|ICANON; /* no echo */
	tp->t_proc = ptproc;
	tp->t_state |= CARR_ON;
	if (tp->t_state & WOPEN)
		wakeup((caddr_t)&tp->t_rawq);
	pti = &pts_ioctl[dev];
	pti->pt_flags = 0;
	pti->pt_send = 0;
	return(0);
}

ptcclose(dev)
	dev_t dev;
{
	register struct tty *tp;

	dev = minor(dev);
	tp = &pts_tty[dev];
	if (tp->t_state & ISOPEN)
		signal(tp->t_pgrp, SIGHUP);
	tp->t_state &= ~CARR_ON;	/* virtual carrier gone */
	ttyflush(tp, FREAD|FWRITE);
	tp->t_proc = ptxproc;		/* mark closed */
}

ptcread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;
	register struct pt_ioctl *pti;
	register c;
	int error = 0;

	dev = minor(dev);
	tp = &pts_tty[dev];
	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return(EIO);
	pti = &pts_ioctl[dev];
	if (pti->pt_flags & PF_PKT) {
		if (pti->pt_send) {
			error = ureadc(pti->pt_send, uio);
			if (error)
				return(error);
			pti->pt_send = 0;
			return(0);
		}
		error = ureadc(0, uio);
	}
	while (!ptck_output(tp)) {
		if (pti->pt_flags&PF_NBIO)
			return (EWOULDBLOCK);
		(void) sleep((caddr_t)&tp->t_outq.c_cf, TTIPRI);
	}
	while (tp->t_outq.c_cc && (uio->uio_resid > 0) &&
	       ((c = getc(&tp->t_outq)) >= 0))
		if (ureadc(c, uio) < 0) {
			error = EFAULT;
			break;
		}
	tp->t_state &= ~BUSY;
	if (tp->t_state&OASLP) {
		tp->t_state &= ~OASLP;
		wakeup((caddr_t)&tp->t_outq);
	}
	if (tp->t_state&TTIOW /*&& tp->t_outq.c_cc==0*/) {
		tp->t_state &= ~TTIOW;
		wakeup((caddr_t)&tp->t_oflag);
	}
	return(error);
}

ptcwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp;
	register char *cp, *ce;
	register int cc, c;
	char locbuf[BUFSIZ];
	int cnt = 0;
	int error = 0;

	dev = minor(dev);
	tp = &pts_tty[dev];
	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return(EIO);
	do {
		register struct iovec *iov;

		if (uio->uio_iovcnt == 0)
			break;
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iovcnt--;
			uio->uio_iov++;
			if (uio->uio_iovcnt < 0)
				panic("ptcwrite");
			continue;
		}
		cc = MIN(iov->iov_len, BUFSIZ);
		cp = locbuf;
		error = uiomove(cp, cc, UIO_WRITE, uio);
		if (error)
			break;
		ce = cp + cc;

again:
		while (cp < ce) {
			while (tp->t_delct && tp->t_rawq.c_cc >= TTYHOG - 2) {
				tp->t_state &= ~IASLP;
				wakeup((caddr_t)&tp->t_rawq);
				if (tp->t_state & TS_NBIO) {
					iov->iov_base -= ce - cp;
					iov->iov_len += ce - cp;
					uio->uio_resid += ce - cp;
					uio->uio_offset -= ce - cp;
					if (cnt == 0)
						return(EWOULDBLOCK);
					return(0);
				}
				/* Better than just flushing it! */
				/* Wait for something to be read */
				(void) sleep((caddr_t)&tp->t_rawq.c_cf, TTOPRI);
				goto again;
			}
			if (tp->t_iflag & ISTRIP)
				c = *cp++ & 0x7f;
			else
				c = *cp++;
			if (tp->t_iflag & IXON) {
				if (tp->t_state & TTSTOP) {
					if (c == CSTART || (tp->t_iflag & IXANY))
						(*tp->t_proc)(tp, T_RESUME);
				} else
					if (c == CSTOP)
						(*tp->t_proc)(tp, T_SUSPEND);
				if (c == CSTART || c == CSTOP)
					continue;
			}
			if (tp->t_rbuf.c_ptr != NULL) {
				*tp->t_rbuf.c_ptr = c;
				tp->t_rbuf.c_count--;
				(*linesw[tp->t_line].l_input)(tp,L_BUF);
			}
			cnt++;
		}
	} while (uio->uio_resid);
	return(error);
}

ptcioctl(dev, cmd, data, flag)
caddr_t data;
dev_t dev;
{
	register struct tty *tp = &pts_tty[minor(dev)];
	register struct pt_ioctl *pti = &pts_ioctl[minor(dev)];

	if (cmd == TIOCPKT) {
		if (*(int *)data)
			pti->pt_flags |= PF_PKT;
		else
			pti->pt_flags &= ~PF_PKT;
		return(0);
	}
	if (cmd == FIONBIO) {
		if (*(int *)data)
			pti->pt_flags |= PF_NBIO;
		else
			pti->pt_flags &= ~PF_NBIO;
		return(0);
	}
	/* IF CONTROLLER STTY THEN MUST FLUSH TO PREVENT A HANG ???  */
        if ((cmd == TIOCSETP) || (cmd == TCSETAW)) {
		while (getc(&tp->t_outq) >= 0)
			;
		tp->t_state &= ~BUSY;
	}
	return(ptsioctl(dev, cmd, data, flag));
}

/*ARGSUSED*/
ptsioctl(dev, cmd, addr, flag)
register caddr_t addr;
register dev_t dev;
{
	register struct tty *tp = &pts_tty[minor(dev)];
	register struct pt_ioctl *pti = &pts_ioctl[minor(dev)];
	register int stop;

	if (ttiocom(tp, cmd, (int)addr, dev) == 0)
		;
	/* else...?? */
	stop = tp->t_iflag & IXON;
	if (pti->pt_flags & PF_NOSTOP) {
		if (stop) {
			pti->pt_send &= ~TIOCPKT_NOSTOP;
			pti->pt_send |= TIOCPKT_DOSTOP;
			pti->pt_flags &= ~PF_NOSTOP;
			ptcwakeup(tp);
		}
	} else {
		if (stop == 0) {
			pti->pt_send &= ~TIOCPKT_DOSTOP;
			pti->pt_send |= TIOCPKT_NOSTOP;
			pti->pt_flags |= PF_NOSTOP;
			ptcwakeup(tp);
		}
	}
	ptcwakeup(tp);		/* just for fun */
	return(u.u_error);
}

ptproc(tp, cmd)
register struct tty *tp;
{
	register struct pt_ioctl *pti = &pts_ioctl[tp - &pts_tty[0]];
        extern ttrstrt(), sxtout();

        switch(cmd) {
        case T_TIME:
                tp->t_state &= ~TIMEOUT;
                goto start;

        case T_WFLUSH:
		pti->pt_send = TIOCPKT_FLUSHWRITE;
		tp->t_state &= ~BUSY;
		/* fall through */

        case T_RESUME:
                tp->t_state &= ~TTSTOP;
		pti->pt_send &= ~TIOCPKT_STOP;
		pti->pt_send |= TIOCPKT_START;
		wakeup((caddr_t)&tp->t_outq.c_cf);
		/* fall through */

        case T_OUTPUT:
start:
		/*
		 * The following is not a general solution.  Someone may use
		 * some other line disc. besides sxt someday.
		 */
		if (linesw[(short)tp->t_line].l_output == sxtout)
			ptsxtproc(tp);
                if (tp->t_state&(TIMEOUT|TTSTOP|BUSY))
                        break;
		if (tp->t_state&TTIOW /*&& tp->t_outq.c_cc==0*/) {
			tp->t_state &= ~TTIOW;
			wakeup((caddr_t)&tp->t_oflag);
		}
		if (tp->t_outq.c_cc < 200) {
			pti->pt_flags |= PF_WTIMER;
			return;
		}
		tp->t_state |= BUSY;
		pti->pt_flags &= ~PF_WTIMER;
		ptcwakeup(tp);
                if (tp->t_outq.c_cc <= ttlowat[tp->t_cflag&CBAUD]) {
			if (tp->t_state & OASLP) {
				tp->t_state &= ~OASLP;
				wakeup((caddr_t)&tp->t_outq);
			}
			if (tp->t_wsel) {
				selwakeup(tp->t_wsel, (int) (tp->t_state & TS_WCOLL));
				tp->t_wsel = 0;
				tp->t_state &= ~TS_WCOLL;
			}
                }
                break;

        case T_SUSPEND:
                tp->t_state |= TTSTOP;
		tp->t_state &= ~BUSY;
		pti->pt_send &= ~TIOCPKT_START;
		pti->pt_send |= TIOCPKT_STOP;
                break;

        case T_BLOCK:
		tp->t_state |= TBLOCK | TTXOFF;
                tp->t_state &= ~TTXON;
		ptcwakeup(tp);
                break;

        case T_RFLUSH:
		pti->pt_send = TIOCPKT_FLUSHREAD;
                if (!(tp->t_state&TBLOCK))
                        break;
		/* fall through */

        case T_UNBLOCK:
                tp->t_state &= ~(TTXOFF|TBLOCK);
		tp->t_state |= TTXON;
		ptcwakeup(tp);
		goto start;

        case T_BREAK:
                tp->t_state |= TIMEOUT;
                timeout(ttrstrt, (caddr_t)tp, v.v_hz/4);
                break;

	case T_PARM:
		ptcwakeup(tp);
		break;
        }
}

/* ptxproc -- "null" proc for closing */
ptxproc(tp, cmd)
register struct tty *tp;
{
	ptcwakeup(tp);
	while (getc(&tp->t_outq) >= 0)
		;

        switch(cmd) {
        case T_WFLUSH:
		/* fall through */

        case T_RESUME:
                tp->t_state &= ~(TTSTOP|TIMEOUT);
		/* fall through */
	
        case T_OUTPUT:
start:
		tp->t_state &= ~BUSY;
		if (tp->t_state&TTIOW) {
			tp->t_state &= ~TTIOW;
			wakeup((caddr_t)&tp->t_oflag);
		}
		if (tp->t_state & OASLP) {
			tp->t_state &= ~OASLP;
			wakeup((caddr_t)&tp->t_outq);
		}
		if (tp->t_wsel) {
			selwakeup(tp->t_wsel, (int) (tp->t_state & TS_WCOLL));
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
		}
                break;

        case T_RFLUSH:
		/* fall through */

        case T_UNBLOCK:
                tp->t_state &= ~(TTXOFF|TBLOCK);
		tp->t_state |= TTXON;
		/* fall through */

	case T_PARM:
		goto start;
		/*NOTREACHED*/
        }
}

ptcselect(dev, rw)
	dev_t dev;
	int rw;
{
	register struct tty *tp = &pts_tty[minor(dev)];
	struct pt_ioctl *pti = &pts_ioctl[minor(dev)];
	struct proc *p;
	int s;

	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return (1);   	/* ??? billn */

	switch (rw) {
	case FREAD:
		if (ptck_output(tp))
			return (1);
		s = spl6();
		if ((p = pti->pt_selr) && p->p_wchan == (caddr_t)&selwait)
			pti->pt_flags |= PF_RCOLL;
		else
			pti->pt_selr = u.u_procp;
		splx(s);
		break;

	case FWRITE:
		return 1;
		/*NOTREACHED*/
	}
	return (0);
}

ptsxtproc(tp)
	register struct tty *tp;
{
	register struct clist *outq = &tp->t_outq;
	register struct cblock *this;
	register struct ccblock *tbuf = &tp->t_tbuf;
	register s = spl6();

	/* get cblocks from virt tty(s) & link them to real tty */
	while (CPRES & (*linesw[(short)tp->t_line].l_output)(tp)) {
		this = CMATCH(tbuf->c_ptr);
		this->c_next = NULL;
		if (outq->c_cl == NULL) {
			outq->c_cl =  this;
			outq->c_cf =  this;
		}
		else {
			outq->c_cl->c_next = this;
			outq->c_cl = this;
		}
		outq->c_cc += tbuf->c_count;
		/* fixup clist structure -- should be done by ttout() */
		this->c_last = this->c_first + tbuf->c_count;
		/* clear tbuf */
		tbuf->c_ptr = NULL;
		tbuf->c_count = 0;
		/* wait for drainage */
		while (outq->c_cc > tthiwat[tp->t_cflag&CBAUD]) {
			tp->t_state |= OASLP;
			ptcwakeup(tp);
			(void) sleep((caddr_t) outq, TTOPRI);
		}
	}
	splx(s);
}
