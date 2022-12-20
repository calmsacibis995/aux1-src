#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/ast.c */
#define _AC_NAME ast_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:06:15}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of ast.c on 87/11/11 21:06:15";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*
 * SCC device driver for the AST board
 *
 *	Copyright 1986 UniSoft Corporation
 *
 *	Richard Kiessig, Intelligent Decisions, Inc.
 *	Jan. 1987
 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/time.h"
#include "sys/sysinfo.h"
#include "sys/mmu.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/seg.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/var.h"
#include "sys/reg.h"
#include <sys/scc.h>
#include "scc.h"
#include "via.h"
#include <sys/debug.h>
#include "sys/proc.h"
#include "setjmp.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/ttx.h"
#include <sys/uconfig.h>
#include <sys/sysmacros.h>

/*
 * External function references.
 */
extern int nulldev();
extern int qenable();

#define	S(b) (((3686400 / 16) / ((b) * 2)) - 2)	/* baud rate setting macro */
#define NSCC 4
#define SCTIME 1				/* scc_scan interval */
#define	isAside(a)  (((int)(a) & 0x4) == 4)	/* check address for A port */

static int scc_scan();
static int scc_proc();
static int scc_open();
static int scc_close();
static int scc_ioctl();

static unsigned char	scc_modem[NSCC];/* current modem control state */
static unsigned char	scc_dtr[NSCC];	/* current state of dtr flow control */
static unsigned char	scc_wdtr[NSCC];	/* we are waiting for dtr flow cntrl */
static unsigned char	scc_emodem[NSCC]; /* UIOCEMODEM is active */
static unsigned char	scc_flow[NSCC];	/* UIOCFLOW is active */
static unsigned char	scc_wflow[NSCC];/* wait for UIOCFLOW */
static unsigned char	scc_wclose[NSCC];/* wait for close */
char 			scc_isopen[NSCC];/* open status (hooks) */
struct ttx		scc_ttx[NSCC];	/* the per-device ttx structures */

/*
 * Note: to select baud rate
 *	k = chip_input_frequency/(2 * baud * factor) - 2
 *	put factor in register 9 and k in registers D & C
 *	NOTE:
 *		normally, factor = 16
 *		for this driver, chip_input_frequency = 3684600 Hz
 * scc_speeds is a table of these numbers by UNIX baud rate
 */
static int scc_speeds[] =
{
	S(1),	S(50),	S(75),	S(110),	S(134),	S(150),	S(200),	S(300),
	S(600),	S(1200), S(1800), S(2400), S(4800), S(9600), S(19200), S(38400)
};

static struct device *scc_addr[] =
{
	(struct device *)0xB0004,
	(struct device *)0xB0000,
	(struct device *)0xB4004,
	(struct device *)0xB4000
};

#define VIA	((via_chip_t *)0xC0000)

static struct module_info scc_info = {5322, "scc", 0, 256, 256, 256};
static struct qinit scc_rq = {NULL, ttx_rsrvc, scc_open, scc_close,
				nulldev, &scc_info, NULL};
static struct qinit scc_wq = {ttx_wputp, ttx_wsrvc, scc_open, scc_close,
				nulldev, &scc_info, NULL};
struct streamtab scc_tab = {&scc_rq, &scc_wq, NULL, NULL};

/*
 * Macro to write a value to a register in the SCC.
 */
extern int T_scc;
#ifdef SCC_DEBUG2
#define ch_write(ch, reg, value) \
	(debugit("ch_write(%d, 0x%x)\n",reg,value), scc_delay(), (ch)->csr = reg, scc_delay(), (ch)->csr = value)

debugit(a, b, c, d, e)
{
	TRACE(T_scc, (a, b, c, d, e));
	TRACE(T_scc, ("from: 0x%x\n", ((int*)&a)[-1]));
}
#else
#define ch_write(ch, reg, value) \
	(scc_delay(), (ch)->csr = reg, scc_delay(), (ch)->csr = value)
#endif SCC_DEBUG2

/*
 * Macro to read a value from a register in the SCC.
 */
#define ch_read(ch, reg) (scc_delay(), (ch)->csr = reg, scc_delay(), (ch)->csr)

/*
 * Table to initialize a port to 9600 baud.
 */
static scc_rvd scc_rvd_scitable[] =
{
	SCC_WR1, 0,
	SCC_WR4, SCC_WR4_PEV | SCC_WR4_1SB | SCC_WR4_X16,
	SCC_WR5, SCC_WR5_TX8 | SCC_WR5_TXE,
	SCC_WR11, SCC_WR11_RXN | SCC_WR11_RCB | SCC_WR11_TCB,
	SCC_WR12, S(9600) & 0xFF,
	SCC_WR13, (S(9600) >> 8) & 0xFF,
	SCC_WR14, 0,
	SCC_WR15, 0,
	SCC_WR14, SCC_WR14_BGS | SCC_WR14_BGE,
	SCC_WR3, SCC_WR3_RX8 | SCC_WR3_RXE,
	SCC_WR10, 0,
	SCC_WR1, W1RXIALL | W1TXIEN /*| W1EXTIEN*/,
	SCC_WR2, 0,			/* auto vector */
	SCC_WR0, W0RXINT,
	SCC_WR9, W9NV | W9MIE | W9DLC,
	SCC_EOT, 0
};

static int 
scc_open(q, dev, flag)
   register queue_t *q;
   register int dev;
   int flag;
{
	register struct ttx *tp;
	register struct device *addr;
	register int d;

	d = minor(dev)&0x03;
printf("scc_open(0x%x, %d)\n", dev, d);
	if (d >= NSCC)
		return OPENFAIL;
	tp = &scc_ttx[d];
	SPL5();
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0)
	{
		/*
		 * Make sure the device is not
		 * being used for other purposes
		 */
		if (scc_isopen[d])
		{
			SPL0();
			return OPENFAIL;
		}
		/*
		 * initialize port; don't raise RTS or DTR yet.
		 */
		scinit(d);
		scc_isopen[d] = 1;
		tp->t_proc = scc_proc;
		tp->t_ioctl = scc_ioctl;
		addr = scc_addr[d];
		tp->t_addr = (caddr_t)addr;
		tp->t_state &= ~CARR_ON;
		tp->t_dev = d;
		if (!scc_modem[d] && !scc_emodem[d])
			tp->t_state |= CARR_ON;
		ttxinit(q, tp, 4);
		/*
		 * Set speeds, etc.; only raise DTR if we have CARR_ON.
		 */
		scc_param(tp);
	}
	if (!(flag & FNDELAY)) 
	{
#ifdef NEWWAY
		if ((tp->t_state & CARR_ON) == 0)
		{
			tp->t_state |= WOPEN;
			SPL0();
			return OPENSLEEP;
		}
#endif NEWWAY
	}
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
	SPL0();
	return dev;
}

/*
 * initialize a port
 */
static int
scinit(dev)
   int dev;
{
	register struct device *addr;
	register int s;

	addr = scc_addr[dev];
	s = spltty();
	if(isAside(addr)) 
		ch_write(addr, SCC_WR9, W9NV | W9ARESET);
	else
		ch_write(addr, SCC_WR9, W9NV | W9BRESET);
	/*
	 * Need 4 times the clock cycles after reset.
	 */
	{
		register int count;

		for (count = 4; count; --count)
			scc_delay();
	}
	scc_cmd(scc_rvd_scitable, &sccs[dev]);
	if (scc_flow[dev])
		ch_write(addr, SCC_WR3, SCC_WR3_RX8|SCC_WR3_RXE|SCC_WR3_AUE);
	scc_scan(dev);			/* start modem scanning */
	splx(s);
}

static int 
scc_close(q, flag)
   queue_t *q;
   int flag;
{
	register struct ttx *tp;
	register int dev;

	tp = (struct ttx *)q->q_ptr;
	dev = tp->t_dev;
#ifdef NEWWAY
	if (tp->t_xm)
	{
		scc_wclose[dev] = 1;
		return CLOSESLEEP;
	}
	return 0;
#else  NEWWAY
	scc_isopen[dev] = 0;
	tp->t_xm = 0;				/* BUG: to avoid sleep */
	scc_fclose(tp);
#endif NEWWAY
}

static int 
scc_fclose(tp)
   register struct ttx *tp;
{
	register int dev;

	dev = tp->t_dev;
	ttx_close(tp);
	/*
	 * Drop RTS and DTR for *all* closes
	 */
	schup(tp);				/* hang up */
	/*
	 * Reset emodem state
	 */
	if (scc_emodem[dev] >= 1)
		scc_emodem[dev] = 1;
	untimeout(scc_scan, dev);		/* stop scanning */
}

static int 
scc_proc(tp, cmd)
   register struct ttx *tp;
{
	register mblk_t *m, *m1;
	register struct device *addr;
	register dev_t dev;
	register int s;

printf("proc(%d): cmd=%d\n", tp->t_dev, cmd);
	s = spltty();
	dev = tp->t_dev;
	addr = (struct device *)tp->t_addr;
	switch (cmd) 
	{
	case T_TIME:
		scw5(tp, 0);			/* clear break */
		tp->t_state &= ~TIMEOUT;
		goto start;

	case T_WFLUSH:
		if (tp->t_xm) {
			freemsg(tp->t_xm);
			tp->t_xm = NULL;
		}
		/* fall through */
	case T_RESUME:
		tp->t_state &= ~TTSTOP;
		goto start;

	case T_OUTPUT:
	{
		register int timer;

start:
printf("OUTPUT 1\n");
		if (tp->t_state&(TIMEOUT|TTSTOP|BUSY|XMT_DELAY) || !tp->t_q)
			break;
printf("OUTPUT 2\n");
		if (tp->t_state & TTXON) {
printf("OUTPUT 3\n");
			scc_delay();
			addr->data = CSTART;
			tp->t_state |= BUSY;
			tp->t_state &= ~TTXON;
			break;
		} else if (tp->t_state & TTXOFF)
		{
printf("OUTPUT 4\n");
			scc_delay();
			addr->data = CSTOP;
			tp->t_state |= BUSY;
			tp->t_state &= ~TTXOFF;
			break;
		}
		/*
		 * If nothing to transmit, then wake up the streams
		 * output handler.
		 */
		if ((m = tp->t_xm) == NULL) {
printf("OUTPUT 5\n");
			qenable(WR(tp->t_q));
			break;
		}
		tp->t_state |= BUSY;
		/*
		 * Always assert RTS before transmitting anything
		 */
		scw5(tp, W5RTS);
		/*
		 * If DTR flow control is enabled and the other side's DSR
		 * isn't asserted, then wait until it is
		 */
		if (scc_dtr[dev] && !scc_dsr(dev))
		{
printf("OUTPUT 6\n");
			scc_wdtr[dev] = 1;
			break;
		}
		/*
		 * If RTS/CTS flow control is enabled, wait for CTS to
		 * come up.
		 */
		if (scc_flow[dev] && !(ch_read(addr, SCC_RR0) & R0CTS))
		{
printf("OUTPUT 7\n");
			scc_wflow[dev] = 1;
			break;
		}
#ifndef NEWWAY
		for (timer = 100; timer; --timer)
			;
#endif  NEWWAY
		for (timer = 10000; timer; --timer)
		{
			if (ch_read(addr, SCC_RR0) & SCC_RR0_TBE)
			{
printf("OUTPUT 8\n");
				scc_delay();
				addr->data = *m->b_rptr++;
				break;
			}
		}
		/*
		 * Remove any empty messages from the output buffer.
		 */
		while (m->b_rptr >= m->b_wptr)
		{
			m1 = unlinkb(m);
			freeb(m);
			tp->t_xm = m = m1;
			if (m == NULL) {
				qenable(WR(tp->t_q));
				break;
			}
		}
printf("OUTPUT 9\n");
		break;
	}
	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:
		tp->t_state |= (TTXOFF|TBLOCK);
		tp->t_state &= ~TTXON;
		goto start;

	case T_RFLUSH:
		if (m = tp->t_rm) {
			tp->t_count = tp->t_size;
			m->b_wptr = m->b_rptr;
		}
		if (!(tp->t_state&TBLOCK))
			break;
		/* fall through */

	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF|TBLOCK);
		tp->t_state |= TTXON;
		goto start;

	case T_PARM:
		scc_param(tp);
		break;
	
	case T_BREAK:
		scw5(tp, W5BREAK);
		ttx_break(tp);
		break;
	}
	splx(s);
}

static int 
scw5(tp, d)
   register struct ttx *tp;
   int d;
{
	register int w5, s;
	register struct device *addr;

	addr = (struct device *)tp->t_addr;
	w5 = W5TXENABLE | d;
	if (tp->t_state & CARR_ON)
		w5 |= W5DTR;
	switch (tp->t_cflag & CSIZE)
	{
	case CS5: w5 |= W55BIT; break;
	case CS6: w5 |= W56BIT; break;
	case CS7: w5 |= W57BIT; break;
	case CS8: w5 |= W58BIT; break;
	}
	s = spltty();
	ch_write(addr, SCC_WR5, w5);
	splx(s);
}

/*
 * All SCC ioctl functions happen here.
 */
static int 
scc_ioctl(tp, iocbp, m)
   struct ttx *tp;
   struct iocblk *iocbp;
   mblk_t *m;
{
	int dev;	
	register int i;
	register char *arg;
	register struct device *addr;

	dev = tp->t_dev;
	iocbp->ioc_count = 0;
	addr = (struct device *)tp->t_addr;
	switch (iocbp->ioc_cmd)
	{
	/*
	 * UIOCMODEM:	turns on modem control. If DTR flow control
	 *		was on turn it off and start output.
	 */
	case UIOCMODEM:
		if (scc_dtr[dev])
		{
			scc_dtr[dev] = 0;
			if (scc_wdtr[dev])
			{
				scc_wdtr[dev] = 0;
				tp->t_state &= ~BUSY;
				(*tp->t_proc)(tp, T_OUTPUT);
			}
		}
		scc_modem[dev] = 1;
		break;
	/*
	 * UIOCNOMODEM:	turns off modem control and DTR flow control.
	 *		If DTR flow control was on turn it off and
	 *		start output. If modem control is on allow
	 *		processes waiting to open to do so.
	 */
	case UIOCNOMODEM:
 		if (scc_dtr[dev])
 		{
 			scc_dtr[dev] = 0;
 			if (scc_wdtr[dev])
 			{
 				scc_wdtr[dev] = 0;
 				tp->t_state &= ~BUSY;
 				(*tp->t_proc)(tp, T_OUTPUT);
 			}
 		}
 		scc_modem[dev] = 0;
		scc_emodem[dev] = 0;
 		if (tp->t_state&WOPEN)
 		{
 			tp->t_state |= CARR_ON;
#ifdef NEWWAY
			tp->t_state &= ~WOPEN;
			opencomplete(tp);
#endif NEWWAY
 		}
 		break;
 	/*
 	 * UIOCDTRFLOW:	turns on DTR flow control. If modem control
 	 *		is on allow processes waiting to open to do so.
 	 */
#ifdef NOTDEF
	case UIOCHSKFLOW:
#endif NOTDEF
 	case UIOCDTRFLOW:
 		if (scc_modem[dev])
 		{
 			scc_modem[dev] = 0;
 			if (tp->t_state&WOPEN)
 			{
#ifdef NEWWAY
				tp->t_state &= ~WOPEN;
				opencomplete(tp);
#endif NEWWAY
 				tp->t_state |= CARR_ON;
 			}
 		}
 		scc_dtr[dev] = 1;
 		break;
	/*
	 * UIOCNOFLOW	- hardware flow control is disabled. RTS is asserted
	 *		  before transmitting (or asserted all the time). CTS
	 *		  is ignored
	 */
	case UIOCNOFLOW:
		ch_write(addr, SCC_WR3, SCC_WR3_RX8|SCC_WR3_RXE);
		scc_flow[dev] = 0;
		break;
	/*
	 * UIOCFLOW	- hardware flow control is enabled. RTS is asserted
	 *		  before transmitting. CTS must be asserted by the other
	 *		  end before transmission can start. (this is required
	 *		  for every character).
	 */
	case UIOCFLOW:
		ch_write(addr, SCC_WR3, SCC_WR3_RX8|SCC_WR3_RXE|SCC_WR3_AUE);
		scc_flow[dev] = 1;
		break;
	/*
	 * UIOCEMODEM	- European-style modem control.  Wait for RI to come
	 *		  up during open(), then assert DTR and wait for 
	 *		  DCD before completing the open.
	 */
	case UIOCEMODEM:
		scc_emodem[dev] = 1;
		break;
 	/*
 	 * UIOCTTSTAT: return the modem control/flow control state
 	 */
 	case UIOCTTSTAT:
		if (m->b_cont == NULL) 
			m->b_cont = allocb(4, BPRI_MED);
		if (m->b_cont) {
			arg = m->b_cont->b_rptr;
			if (scc_modem[dev])
				*arg++ = 1;
			else if (scc_emodem[dev])
				*arg++ = 2;
			else
				*arg++ = 0;
			*arg++ = scc_dtr[dev] ? 1 : 0;
			*arg = scc_flow[dev] ? 1 : 0;
			iocbp->ioc_count = 3;
			m->b_cont->b_wptr += 3;
		}
 		break;
#ifdef NOTDEF
	/*
	 * UIOCMSTAT: modem status
	 */
	case UIOCMSTAT:
	{
		struct device *addr;
		unsigned char a, b;

		addr = (struct device *)tp->t_addr;
		if (m->b_cont == NULL) 
			m->b_cont = allocb(2, BPRI_MED);
		if (m->b_cont) {
			short *arg;

			arg = (short *)m->b_cont->b_rptr;
			*arg = 0x5;	/* DTR & RTS always on (outputs) */
			if (ch_read(addr, SCC_RR0) & R0DCD)
				*arg |= 0x2;
			if (ch_read(addr, SCC_RR0) & R0CTS)
				*arg |= 0x8;
			a = scc_dsr(dev);
			b = scc_ri(dev);
			if (a)
				*arg |= 0x10;
			if (b)
				*arg |= 0x20;
			iocbp->ioc_count = 2;
			m->b_cont->b_wptr += 2;
		}
		break;
	}
#endif NOTDEF
	/*
	 * Unknown ioctl.
	 */
 	default:
		return 1;
 	}
 	return 0;
 }
 
static int 
scc_param(tp)
    register struct ttx *tp;
 {
 	struct device *addr;
 	int flag, s;
 	register int speed, w4;
 
 	flag = tp->t_cflag;
 	if (((flag & CBAUD) == B0) && scc_modem[tp->t_dev]) /* hang up line */
 	{
 		schup(tp);
 		return;
 	}
 	addr = (struct device *)tp->t_addr;
 	w4 = W4CLK16 | (flag & CSTOPB ? W42STOP : W41STOP);/* CLK & Stop bits */
	scw5(tp, W5RTS);
 	if (flag & PARENB)		/* If parity enable */
 	{
 		w4 |= W4PARENABLE;	/* Enable parity */
 		if (!(flag & PARODD))	/* If not odd parity */
 			w4 |= W4PAREVEN;/* Set even parity */
 	}
 	speed = scc_speeds[flag & CBAUD];/* Set baud rate */
 	s = spltty();
	ch_write(addr, SCC_WR4, w4);
	ch_write(addr, SCC_WR12, speed);
	ch_write(addr, SCC_WR13, speed >> 8);
 	splx(s);
 }
 
 /*
  * Hang up the line by dropping DTR and RTS
  */
 static int 
 schup(tp)
    register struct ttx *tp;
 {
 	register struct device *addr;
 	register int s;
 
 	addr = (struct device *)tp->t_addr;
 	s = spltty();
	ch_write(addr, SCC_WR5, W5TXENABLE | W58BIT);
 	splx(s);
 }
 
 /*
  * Top level scc interrupt handler.
  */
 scintr(ap)
    register struct args *ap;
 {
 	register struct device *addr;
 	register struct ttx *tp;
	register int i;
 
printf("intr(%d)\n", ap->a_dev);
 	for (i = 0; i < NSCC; i++)
	{
 		tp = &scc_ttx[i];
 		addr = (struct device *)tp->t_addr;
 		while (ch_read(addr, SCC_RR0) & R0RXRDY)
		{
 			if (ch_read(addr, SCC_RR1) &
			   (R1PARERR | R1OVRERR | R1FRMERR))
 				scc_sintr(tp);
 			else
 				scc_rintr(tp);
		}
 		if (ch_read(addr, SCC_RR0) & R0TXRDY)
 			scc_xintr(tp);
	}
 }
 
 /*
  * Receive scc interrupt.
  */
static int 
scc_rintr(tp)
    register struct ttx *tp;
 {
 	register struct device *addr;
 	register mblk_t *m;
 	register int c;
 	register int lcnt;
 	register int flg;
 	register char ctmp;
 	char lbuf[3];
 
 	addr = (struct device *)tp->t_addr;
	ch_write(addr, SCC_WR0, W0RXINT);	/* reenable Rx interrupt */
	ch_write(addr, SCC_WR0, W0RIUS);	/* reset interrupt */
	c = addr->data & 0xFF;
	if (tp->t_iflag & IXON)
	{
		ctmp = c & 0177;
		if (tp->t_state & TTSTOP)
		{
			if (ctmp == CSTART || tp->t_iflag & IXANY)
				(*tp->t_proc)(tp, T_RESUME);
		}
		else if (ctmp == CSTOP)
				(*tp->t_proc)(tp, T_SUSPEND);
		if (ctmp == CSTART || ctmp == CSTOP)
			return;
	}
	if ((m = tp->t_rm) == NULL)
		return;
	lcnt = 1;
	flg = tp->t_iflag;
	if (flg&ISTRIP)
		c &= 0177;
	else
	{
		/* c &= 0377; not needed */
		if (c == 0377 && flg&PARMRK)
		{
			lbuf[1] = 0377;
			lcnt = 2;
		}
	}
	/*
	 * Stash character in message
	 */
	if (lcnt != 1)
	{
		lbuf[0] = c;
		while (lcnt)
		{
			*m->b_wptr++ = lbuf[--lcnt];
			if (--tp->t_count == 0)
			{
				if (ttx_put(tp))
					return;
				if ((m = tp->t_rm) == NULL)
					return;
			}
		}
	}
	else
	{
		*m->b_wptr++ = c;
		tp->t_count--;
	}
	if (m && m->b_wptr != m->b_rptr)
		ttx_put(tp);
}

/*
 * Transmitter scc interrupt.
 */
static int scc_xintr(tp)
   register struct ttx *tp;
{
	register struct device *addr;

#ifdef KERNEL
	sysinfo.xmtint++;
#endif KERNEL
	addr = (struct device *)tp->t_addr;
	ch_write(addr, SCC_WR0, W0RTXPND);	/* reset Tx interrupt */
	ch_write(addr, SCC_WR0, W0RIUS);	/* reset interrupt */
	tp->t_state &= ~BUSY;
	scw5(tp, 0);				/* clear RTS */
	scc_proc(tp, T_OUTPUT);
}

/*
 * Scc status interrupt.
 */
static int scc_sintr(tp)
   register struct ttx *tp;
{
	register struct device *addr;
	register mblk_t *m;
	register int c;
	register int lcnt;
	register int flg;
	register char ctmp;
	register unsigned char stat;
	char lbuf[3];

#ifdef KERNEL
	sysinfo.rcvint++;
#endif KERNEL
	addr = (struct device *)tp->t_addr;
	c = addr->data & 0xFF;			/* read data BEFORE reset err */
	stat = ch_read(addr, SCC_RR1);
	ch_write(addr, SCC_WR0, W0RERROR);	/* reset error condition */
	ch_write(addr, SCC_WR0, W0RXINT);	/* reinable Rx interrupt */
	ch_write(addr, SCC_WR0, W0RIUS);	/* reset int under service */
	if (tp->t_iflag & IXON)
	{
		ctmp = c & 0177;
		if (tp->t_state & TTSTOP)
		{
			if (ctmp == CSTART || tp->t_iflag & IXANY)
				(*tp->t_proc)(tp, T_RESUME);
		}
		else if (ctmp == CSTOP)
			(*tp->t_proc)(tp, T_SUSPEND);
		if (ctmp == CSTART || ctmp == CSTOP)
			return;
	}
	if ((m = tp->t_rm) == NULL)
		return;
	/*
	 * Check for errors
	 */
	lcnt = 1;
	flg = tp->t_iflag;
	if (stat & (R1PARERR |R1OVRERR|R1FRMERR))
	{
		if ((stat & R1PARERR ) && (flg & INPCK))
			c |= PERROR;
		if (stat & R1OVRERR)
			c |= OVERRUN;
		if (stat & R1FRMERR)
			c |= FRERROR;
	}
	if (c&(FRERROR|PERROR|OVERRUN))
	{
		if ((c&0377) == 0)
		{
			if (flg&IGNBRK)
				return;
			if (flg&BRKINT)
			{
				putctl1(tp->t_q->q_next, M_CTL, L_BREAK);
				return;
			}
		}
		else if (flg&IGNPAR)
			return;
		if (flg&PARMRK)
		{
			lbuf[2] = 0377;
			lbuf[1] = 0;
			lcnt = 3;
#ifdef KERNEL
			sysinfo.rawch += 2;
#endif KERNEL
		}
		else
			c = 0;
	}
	else if (flg&ISTRIP)
		c &= 0177;
	else
	{
		/* c &= 0377; not needed */
		if (c == 0377 && flg&PARMRK)
		{
			lbuf[1] = 0377;
			lcnt = 2;
		}
	}
	/*
	 * Stash character in message
	 */
	if (lcnt != 1)
	{
		lbuf[0] = c;
		while (lcnt)
		{
			*m->b_wptr++ = lbuf[--lcnt];
			if (--tp->t_count == 0)
			{
				if (ttx_put(tp))
					return;
				if ((m = tp->t_rm) == NULL)
					return;
			}
		}
	}
	else
	{
		*m->b_wptr++ = c;
		tp->t_count--;
	}
	if (m && m->b_wptr != m->b_rptr)
		ttx_put(tp);
}

static int 
scc_scan(dev)
   register int dev;
{
	register struct ttx *tp;
	register struct device *addr;
	register int s;

	s = spltty();
	timeout(scc_scan, dev, SCTIME);
	addr = (struct device *)scc_addr[dev];
	ch_write(addr, SCC_WR0, W0REXT);	/* reset external status/intr */
	tp = &scc_ttx[dev];
	if (scc_modem[dev])
	{
		/*
		 * If DCD goes away in UIOCMODEM, then hang up the line.
		 */
		if (!(ch_read(addr, SCC_RR0) & R0DCD))
		{
			if (tp->t_state & CARR_ON)
			{
				tp->t_state &= ~CARR_ON;
				if (tp->t_state&ISOPEN)
				{
					putctl1(tp->t_q->q_next, M_FLUSH, 
						FLUSHRW);
					putctl1(tp->t_q->q_next, M_SIG, SIGHUP);
				}
				schup(dev);
			}
		}
		/*
		 * Otherwise, if DCD is there and we're waiting for an open
		 * to complete, then do it.
		 */
		else
		{
			if (!(tp->t_state & CARR_ON) && (tp->t_state & WOPEN))
			{
				tp->t_state &= WOPEN;
				tp->t_state |= CARR_ON;
				scw5(tp, 0);
#ifdef NEWWAY
				opencomplete(tp);
#endif NEWWAY
			}
		}
	}
	switch (scc_emodem[dev])
	{
	case 1:
		/* waiting for RI */
		if ((tp->t_state & WOPEN) && scc_ri(dev))
		{
			scc_emodem[dev] = 2;
			scw5(tp, W5DTR);	/* assert DTR */
			break;
		}
	case 2:
		/* got RI; wait for DCD */
		if (ch_read(addr, SCC_RR0) & R0DCD)
		{
			scc_emodem[dev] = 3;
			tp->t_state |= CARR_ON;
			tp->t_state &= ~WOPEN;
#ifdef NEWWAY
			opencomplete(tp);
#endif NEWWAY
			break;
		}
	}
	if (scc_dtr[dev] && scc_wdtr[dev])
	{
		if (scc_dsr(dev))
		{
			scc_wdtr[dev] = 0;
			tp->t_state &= ~BUSY;
			(*tp->t_proc)(tp, T_OUTPUT);
		}
	}
	if (scc_flow[dev] && scc_wflow[dev])
	{
		if (ch_read(addr, SCC_RR0) & R0CTS)
		{
			scc_wflow[dev] = 0;
			tp->t_state &= ~BUSY;
			(*tp->t_proc)(tp, T_OUTPUT);
		}
	}
	if (scc_wclose[dev])
	{
		if (tp->t_xm == NULL)
		{
			scc_wclose[dev] = 0;
			scc_fclose(tp);
#ifdef NEWWAY
			closecomplete(tp);
#endif NEWWAY
		}
	}
	splx(s);
}

static int
scc_ri(dev)
   int dev;
{
	int b;

	b = 0;
	switch(dev)
	{
	case 0:
		b = VIA->orb_irb & VIA_IRB_SCC1_RIA;
		break;
	case 1:
		b = VIA->orb_irb & VIA_IRB_SCC1_RIB;
		break;
	case 2:
		b = VIA->orb_irb & VIA_IRB_SCC2_RIA;
		break;
	case 3:
		b = VIA->orb_irb & VIA_IRB_SCC2_RIB;
		break;
	}
	return !b;
}

static int
scc_dsr(dev)
   int dev;
{
	int a;

	a = 0;
	switch(dev)
	{
	case 0:
		a = VIA->orb_irb & VIA_IRB_SCC1_DSRA;
		break;
	case 1:
		a = VIA->orb_irb & VIA_IRB_SCC1_DSRB;
		break;
	case 2:
		a = VIA->orb_irb & VIA_IRB_SCC2_DSRA;
		break;
	case 3:
		a = VIA->orb_irb & VIA_IRB_SCC2_DSRB;
		break;
	}
	return !a;
}
