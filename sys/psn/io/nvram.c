#ifndef lint	/* .../sys/psn/io/nvram.c */
#define _AC_NAME nvram_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.6 87/11/19 18:02:34}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.6 of nvram.c on 87/11/19 18:02:34";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)nvram.c	UniPlus VVV.2.1.4	*/
/*
 *	Non volatile ram/time of day clock driver
 *
 * (C) 1986 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */
#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/seg.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/reg.h"
#include "sys/uconfig.h"
#include <sys/debug.h>
#endif lint
#include "sys/uio.h"
#include <sys/nvram.h>
#include <sys/via6522.h>

#define DELAY1() 	{asm("	mov.l (%sp), (%sp)");}
#define DELAY2() 	{asm("	mov.l (%sp), (%sp)"); \
			 asm("	mov.l (%sp), (%sp)");}

static int nvram_opened = 0;			/* true if the device is open */

#define DELTA (((365*(1970-1904))+((1970-1904)/4)+1)*24*60*60)

/*
 *	Via interface bits
 */

#define CLK_DATA	0x01
#define CLK_CLOCK	0x02
#define CLK_CE		0x04

static int nvram_devinit();
static int nvram_send();
static unsigned char nvram_rcv();
static int nvram_timeout();
static unsigned long nvram_systime();

static unsigned char nvram_buffer[256];	/* local copy of the NVRAM */
static unsigned long nvram_time;	/* local copy of the time */
static unsigned long nvram_lastclock;	/* the clock time lst time we updated
					   the Unix time */
static long nvram_lasttime;		/* the Unix time last time we updated
					   it */

static unsigned char nvram_lock;	/* used to lock out clock code */

#define RETRY	(v.v_hz*120)		/* how often we set the time right */

/*
 *	Initialisation routine - starts periodic timeout to set the Unix clock
 *		straight every few minutes. Also reads the NVRAM into the local
 *		buffer.
 */

nvram_init()
{
	register unsigned long tm, tmp;

	nvram_devinit();
	tm = nvram_systime();
	for (;;) {
		tmp = nvram_systime();
		if (tmp == tm)
			break;
		tm = tmp;
	}
	nvram_lastclock = tmp;
	nvram_lasttime = time.tv_sec;
	timeout(nvram_timeout, 1, RETRY);
}

/*
 *	The timeout routine gets the current system time (If someone else is
 *	accessing the clock chip we just wait until next time), figures out how
 *	much time has changed since last time we set it and then updates the
 *	unix 'time'
 */

static
nvram_timeout(dotimeout)

int	dotimeout;	/* true if a timeout should be set */
{
	register unsigned long tm, tmp;
	register int s;
	register int diff;

	tm = nvram_systime();
	if (tm != 0xffffffff) {
		for (;;) {
			tmp = nvram_systime();
			if (tmp == tm)
				break;
			tm = tmp;
		}
		s = spl7();
		diff = (tmp - nvram_lastclock) - (time.tv_sec - nvram_lasttime);
		if (diff > 0 && diff < 60) {
			bumptime(&time, diff*1000000-time.tv_usec);
		}
		nvram_lastclock = tmp;
		nvram_lasttime = time.tv_sec;
		splx(s);
	}
	if(dotimeout)
		timeout(nvram_timeout, 1, RETRY);
}

/*	nvram_timewarp -- update kernel time from real time clock chip.
 *	     May be called if we might be losing time.
 */

nvram_timewarp()

{
	nvram_timeout(0);
}

/*
 *	The open routine enforces the "only one process open at a time"
 *		rule. 
 */

nvram_open(dev, flag)
dev_t dev;
int flag;
{
	if (nvram_opened)
		return(EBUSY);
	nvram_opened = 1;
	return(0);
}

/*
 *	The read routine copies out the local copy of the data. If the time
 *		is being read we access the chip.
 */

nvram_read(dev, uio)
dev_t dev;
register struct uio *uio;
{
	register int err, i;
	register unsigned long l;

	if (uio->uio_offset > 260)
		return(EIO);
	err = 0;
	if (uio->uio_offset < 256) {
		i = 256 - uio->uio_offset;
		err = uiomove((char *)&nvram_buffer[uio->uio_offset],
			(i < uio->uio_resid ? i : uio->uio_resid),
			UIO_READ,
			uio);
	}
	if (!err && uio->uio_resid) {
		nvram_gettime();
		do {
			l = nvram_time;
			nvram_gettime();
		} while (l != nvram_time);
		i = 256+4 - uio->uio_offset;
		err = uiomove(&((char *)&nvram_time)[uio->uio_offset-256],
			(i < uio->uio_resid ? i : uio->uio_resid),
			UIO_READ,
			uio);
		nvram_protect(1);
	}
	return(err);
}

/*
 *	The write routine copies the data into the local buffer and then
 *		zaps the changed bytes out to the nvram
 */

nvram_write(dev, uio)
dev_t dev;
register struct uio *uio;
{
	register int err, i, offset, resid;
	register long l;

	if (!suser())
		return(EPERM);
	if (uio->uio_offset > 260)
		return(EIO);
	err = 0;
	nvram_protect(0);
	if (uio->uio_offset < 256) {
		offset = uio->uio_offset;
		resid = uio->uio_resid;
		i = 256 - uio->uio_offset;
		err = uiomove((char *)&nvram_buffer[uio->uio_offset],
			(i < uio->uio_resid ? i : uio->uio_resid),
			UIO_WRITE,
			uio);
		nvram_zap(offset, resid - uio->uio_resid);
	}
	if (!err && uio->uio_resid) {
		i = 256+4 - uio->uio_offset;
		err = uiomove(&((char *)&nvram_time)[uio->uio_offset-256],
			(i < uio->uio_resid ? i : uio->uio_resid),
			UIO_WRITE,
			uio);
		nvram_lock = 1;
		nvram_lastclock = nvram_time;
		nvram_lasttime = time.tv_sec;
		l = nvram_time;
		do {
			nvram_time = l;
			nvram_settime();
			nvram_gettime();
		} while (l != nvram_time);
		nvram_lock = 0;
	}
	nvram_protect(1);
	return(err);
}

/*
 *	Close just marks the device as not open so that other processes can
 *		open it.
 */

nvram_close(dev)
dev_t dev;
{
	nvram_opened = 0;
}

/*
 *	The init routine initialises the interface and then reads in a local
 *		copy of the  NVRAM
 */

static
nvram_devinit()
{
	register struct via *vp;
	register int i;

	vp = (struct via *)VIA1_ADDR;
	nvram_lock = 1;
	vp->acr &= 0xfd;
	vp->pcr |= 0x20;
	vp->regb |= (CLK_CLOCK|CLK_CE);
	vp->ddrb &= ~CLK_DATA;
	vp->ddrb |= (CLK_CLOCK|CLK_CE);
	for (i = 0; i < 256; i++) {
		vp->regb &= ~CLK_CE;
		DELAY2();
		nvram_send(0xb8 | ((i>>5)&0x07));	/* extended read */
		nvram_send((i<<2)&0x7c);
		nvram_buffer[i] = nvram_rcv();
		DELAY2();
		vp->regb |= CLK_CE;
		DELAY2();
	}
	nvram_protect(1);
	nvram_lock = 0;
}

/*
 *	nvram_send sends a byte to the chip.
 *		assumptions:	CLK_CE has been pulled low
 *				nvram_lock is set
 */

static
nvram_send(val)
register int val;
{
	register struct via *vp;
	register int i;

	vp = (struct via *)VIA1_ADDR;
	vp->ddrb |= CLK_DATA;
	for (i = 0; i < 8; i++) {
		vp->regb &= ~CLK_CLOCK;		/* clock pulse goes low */
		DELAY1();
		vp->regb = (vp->regb&~CLK_DATA)|((val>>7)&CLK_DATA);/* output data */
		val <<= 1;
		DELAY1();
		vp->regb |= CLK_CLOCK;		/* clock pulse goes high */
		DELAY1();
	}
	vp->ddrb &= ~CLK_DATA;
}

/*
 *	nvram_rcv receives a byte from the chip.
 *		assumptions:	CLK_CE has been pulled low
 *				nvram_lock is set
 */

static
unsigned char
nvram_rcv()
{
	register struct via *vp;
	register int i;
	register unsigned char val;

	val = 0;
	vp = (struct via *)VIA1_ADDR;
	for (i = 0; i < 8; i++) {
		vp->regb &= ~CLK_CLOCK;		/* clock pulse goes low */
		DELAY1();
		val = (val<<1)|(vp->regb&CLK_DATA);	/* input data */
		DELAY1();
		vp->regb |= CLK_CLOCK;		/* clock pulse goes high */
		DELAY1();
	}
	return(val);
}

/*
 *	nvram_gettime reads the system time from the chip and places it in
 *		nvram_time
 */

static
nvram_gettime()
{
	register struct via *vp;
	register int i;
	register unsigned char *cp;
	register unsigned char old;

	cp = &((unsigned char *)&nvram_time)[3];
	vp = (struct via *)VIA1_ADDR;
	old = nvram_lock;
	nvram_lock = 1;
	for (i = 0; i < 4; i++) {
		vp->regb &= ~CLK_CE;
		DELAY2();
		nvram_send(0x81 | ((i<<2)&0x0c));
		*cp-- = nvram_rcv();
		DELAY2();
		vp->regb |= CLK_CE;
		DELAY2();
	}
	nvram_lock = old;
}

/*
 *	nvram_settime writes the time into the chip
 *		from nvram_time
 */

nvram_settime()
{
	register struct via *vp;
	register int i;
	register unsigned char *cp;
	register unsigned char old;

	cp = &((unsigned char *)&nvram_time)[3];
	vp = (struct via *)VIA1_ADDR;
	old = nvram_lock;
	nvram_lock = 1;
	for (i = 0; i < 4; i++) {
		vp->regb &= ~CLK_CE;
		DELAY2();
		nvram_send(0x01 | ((i<<2)&0x0c));
		nvram_send(*cp--);
		DELAY2();
		vp->regb |= CLK_CE;
		DELAY2();
	}
	nvram_lock = old;
}

/*
 *	nvram_zap writes changed data from the local NVRAM copy to the chip
 */

static
nvram_zap(offset, count) 
register int count;
register int offset;
{
	register struct via *vp;

	vp = (struct via *)VIA1_ADDR;
	nvram_lock = 1;
	while (count--) {
		vp->regb &= ~CLK_CE;
		DELAY2();
		nvram_send(0x38 | ((offset>>5)&0x07));	/* extended write */
		nvram_send((offset<<2)&0x7c);
		nvram_send(nvram_buffer[offset]);
		DELAY2();
		vp->regb |= CLK_CE;
		DELAY2();
		offset++;
	}
	nvram_lock = 0;
}

/*
 *	nvram protect sets/clears the write protect bits in the chip
 */

static
nvram_protect(x)
int x;
{
	register struct via *vp;

	vp = (struct via *)VIA1_ADDR;
	vp->regb &= ~CLK_CE;
	DELAY2();
	nvram_send(0x35);		/* write protect */
	nvram_send(x?0xd5:0x55);
	DELAY2();
	vp->regb |= CLK_CE;
	DELAY2();
}

/*
 *	nvram_systime returns the time in Unix form (ie 1970 based) with the
 *		GMT bias. It honors the locking protocol and therefore can be
 *		called from interrupt/timeout level (it is the only routine in
 *		this file, apart from nvram_timeout, that can). If a process is
 *		using the device it returns 0xffffffff which menas try again
 *		later (after a lower priority interrupt/process has completed
 *		its access)
 */

static unsigned long
nvram_systime()
{
	register struct via *vp;
	register int i, s;
	register unsigned long res;
	register short gmt;
	register unsigned char *cp;

	s = spl7();
	if (nvram_lock) {
		splx(s);
		return(0xffffffff);
	}
	nvram_lock = 1;
	splx(s);
	cp = &((unsigned char *)&nvram_time)[3];
	vp = (struct via *)VIA1_ADDR;
	for (i = 0; i < 4; i++) {
		vp->regb &= ~CLK_CE;
		DELAY2();
		nvram_send(0x81 | ((i<<2)&0x0c));
		*cp-- = nvram_rcv();
		DELAY2();
		vp->regb |= CLK_CE;
		DELAY2();
	}
	nvram_protect(1);
	gmt = (*(short *)nvram_buffer[NVRAM_GMTBIAS])&NVRAM_GMTMASK;
	if (gmt&((NVRAM_GMTMASK+1)>>1))
		gmt |= ~NVRAM_GMTMASK;
	if (gmt >= 1440 || gmt <= -1440)
		gmt = 0;
	res = nvram_time - gmt - DELTA;
	nvram_lock = 0;
	return(res);
}

