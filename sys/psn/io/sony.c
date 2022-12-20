#ifndef lint	/* .../sys/psn/io/sony.c */
#define _AC_NAME sony_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.12 87/11/19 18:04:07}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.12 of sony.c on 87/11/19 18:04:07";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

#ifdef HOWFAR
extern int	T_sony;
extern int	T_sony2;
int	T_sonyfull = 0;		/* turning this on harms operation */
#endif	HOWFAR
/*	@(#)sony.c	UniPlus VVV.2.1.16	*/
/*
 * (C) Copyright 1985 UniSoft Systems of Berkeley CA
 *
 * Sony driver
 * and eject driver
 *
 */ 

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/param.h"
#include "sys/debug.h"
#include "sys/uconfig.h"
#include "sys/mmu.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/seg.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/utsname.h"
#include "sys/buf.h"
#include "sys/file.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/var.h"
#include "setjmp.h"
#endif lint
#include "sys/uioctl.h"
#include "sys/ssioctl.h"
#include "sys/diskformat.h"
#include "sys/sony.h"
#include "sys/via6522.h"

 
extern char  TagData[];
short GapSync;			/* globals for fmt.s */
short	fmt2side;

static  int iwmflag;	/* flag for concurrency control of IWM chip */

short	tout;	/* flag that disk motor timeout process started */
int	misscount;
#define	TOUTTIME 8	/* time in seconds to wait before turning off drive */
int	toff();

struct sonyinfo {
	char drvthere;		/* flag to show whether the drive was found */
	char drvheads;		/* flag for number of heads 0=1, 1=2 */
	char dskthere;		/* flag for diskette being in drive */
	char dskclamped;	/* flag for disk being up to speed */
	short curtrk;		/* current track */
	char dsk2side;		/* true if current disk is two side format */
	char dskwrprot;		/* diskette write protected flag */
	char dskbsyflag;	/* disk was opened and not closed if 1 */
	char active;		/* activity down counter */
	char closing		/* flag that device is closing */
} sntb[SNCNT];			/* On-line status table */
short snIntExt[SNCNT] = { IntDrv,ExtDrv }; /* Internal/External drive */
				/* the code assumes drive 0 is Internal */

extern char  SyncTbl[];
#define NRETRY	5

#define physical(d) ((minor(d)>>4)&0x1)	/* physical unit number 0-1 */
#define logical(d) (minor(d)&0x7)	/* logical unit number, 0-7 */
#define splsn	spl5			/* mask ALL interrupts */

#define GETBUF(bp)					\
	(void) splsn();					\
	while (bp->b_flags & B_BUSY) {			\
		bp->b_flags |= B_WANTED;		\
		(void)sleep((caddr_t)bp, PRIBIO+1);	\
	}						\
	bp->b_flags |= B_BUSY;				\
	(void) spl0()

#define FREEBUF(bp)			\
	(void) splsn();			\
	if (bp->b_flags & B_WANTED)	\
		wakeup((caddr_t)bp);	\
	bp->b_flags = 0;		\
	(void) spl0()

struct	buf	sncbuf;		/* command buffer */
struct iostat snstat[SNCNT];
#ifndef SN1
#define SN1 1
#endif SN1
struct iobuf sntab = tabinit(SN1,snstat);	/* active buffer header */

/*	Two sided drives have twice the capacity of one sided drives.
 *	If a drive is two sided, we multiply these numbers times 2.
 */
struct sn_sizes {
	daddr_t sn_offset, sn_size;
} sn_sizes[] = {
	0,		800,		/* a = filesystem */
	0,		0,		/* b = unused */
	0,		0,		/* c = unused */
	0,		0,		/* d = unused */
	0,		0,		/* e = unused */
	0,		0,		/* f = unused */
	0,		0,		/* g = unused */
	0,		800		/* h = entire disk */
};


/*
 *	called from oem7init
 */
sninit()
{
	register char *sndisk = (char *)IWM_ADDR;	
	register result, mask;
	int i,j;

	mask = 0x17;
	for(i = 0; i < 400000; ++i) {		/* initialize the chip */
		result = sndisk[MtrOff];
		result = sndisk[Q6H];		/* set sense mode */
		result = sndisk[Q7L];		/* get the mode register */
		if(result & 0x20)      /* if bit 5 set interface not disabled */
			continue;
		if(mask == (result & mask))	/* check if mode now set */
			break;
		sndisk[Q7H] = mask;
		result = sndisk[Q7L];
	}
	result = sndisk[Q6L];
	for (i=0,j=0;i<SNCNT;i++) {
		if(chkdrvexst(i))  /* find out whether the drive is attached */
			j++;
	}
	printf("floppy: %d floppy %s\n",j,j==1?"drive":"drives");
	for(i=0;i<=j;i++) {
	    if(sntb[i].drvthere) {
		    printf("floppy: drive %d is %s drive\n",i,
			   sntb[i].drvheads?"an 800K":"a 400K");
	    }
	}
}

chkdrvexst(n)
register int n;
{
	register char *sndisk = (char *)IWM_ADDR;	
	register uchar_t reslt;

	TRACE(T_sony2, ("chkdrvexst for sony drive %d\n", n));
	if(n < 0 || n >= SNCNT) {
		return(0);
	}
	if(sndisk[snIntExt[n]])	;	/* tst.b */
	if(sndisk[MtrOn])	;	/* tst.b */
	reslt = adrandsense(DrvExstAdr);
	if(reslt < 0x80)		/* IWM < 0x80 if valid result */	
		sntb[n].drvthere++;
	else
		return(0);
	reslt = adrandsense(SidesAdr);	/* IWM < 0x80 if valid result */	
	if (reslt >= 0x80)
		sntb[n].drvheads++;
	return(1);
}

adrandsense(addr)
register int addr;
{
	register char *sndisk = (char *)IWM_ADDR;	
	register char stat;
	register int opri;

	opri = splsn();
	TRACE(T_sony2, ("adrandsense 0x%x == ", addr));
	(void)adrdisk(addr);
	if(sndisk[Q6H])	;		/* tst.b to go into sense mode */
	stat = sndisk[Q7L];		/* get result */
	if(sndisk[Q6L])	;		/* tst.b to go back to read mode */
	splx(opri);
	TRACE(T_sony2, ("0x%x\n", stat));
	return(stat);
}

adrdisk(adr)
register int adr;
{
	register char *sndisk = (char *)IWM_ADDR;	
	register struct via *vp = (struct via *)VIA1_ADDR;
	register int opri = splsn();

	if(sndisk[Ph0H])	;	/* tst.b */
	if(sndisk[Ph1H])	;	/* tst.b */
	if(sndisk[Ph2L])	;	/* tst.b */
	if(adr & CA2)
		if(sndisk[Ph2H]) ;	/* tst.b to set CA2 */
	if(adr & SEL)
		vp->rega |= VRA_HEAD;
	else
		vp->rega &= ~VRA_HEAD;
	if(!(adr & CA0))
		if(sndisk[Ph0L]) ;	/* tst.b to set CA0 */
	if(!(adr & CA1))
		if(sndisk[Ph1L]) ;	/* tst.b to set CA1 */
	splx(opri);
}

snopen(dev, flag)
dev_t dev;
{
	register char *sndisk = (char *)IWM_ADDR;
	register unit = physical(dev);
	register struct sonyinfo *si = &sntb[unit];

	if (unit >= SNCNT || si->drvthere == 0) {
		TRACE(T_sony, ("can't open 0x%x drvthere = 0x%x\n", unit, 
			si->drvthere));
		return(ENXIO);
	}
	(void)splsn();
	while(si->closing) {
		sleep(&si->closing, PZERO+1);
	}
	while(iwmflag & B_BUSY) {
		iwmflag |= B_WANTED;
		sleep((caddr_t) &iwmflag, PZERO+1);
	}
	iwmflag |= B_BUSY;
	SPL0();

	TRACE(T_sony, ("motor on in snopen for %d\n", unit));
/* select disk, turn on motor */
	if(sndisk[snIntExt[unit]]);		/* tst.b */
	if(sndisk[MtrOn])	;		/* tst.b */
	sntb[(unit == 0 ? 1 : 0)].dskclamped = S_OFF;
/*
 * check whether there is a diskette in the drive, status returned is < 0
 * if not. If no diskette, treat as error condition.
*/
	if(adrandsense(DIPAdr) < 0) {
		si->dskthere = 0;
		si->dskclamped = S_OFF;
		(void)splsn();
		if(iwmflag & B_WANTED)
			wakeup((caddr_t)&iwmflag);
		iwmflag = 0;
		SPL0();
		return(EIO);
	}
	si->dskthere = 1;
	if(si->dskclamped == S_DONE) { /*if true then disk running & clamped*/
		(void)splsn();
		if(iwmflag & B_WANTED)
			wakeup((caddr_t)&iwmflag);
		iwmflag = 0;
		SPL0();
		if(si->dskwrprot && (flag & FWRITE))
			return(EROFS);
		return(0);
	}

/*
 * the code below is executed only if the disk status flag indicates that 
 * the disk hasn't already been powered up, and recalibrated
 */
	PwrUp(unit, (v.v_hz/10) * 8);			/* crank on the juice */
	if(recal(unit) < 0) {		/* do recal on first open */
		(void)splsn();
		IPwrOff(unit);
		if(iwmflag & B_WANTED)
			wakeup((caddr_t)&iwmflag);
		iwmflag = 0;
		SPL0();
		return(EIO);
	}
	if(adrandsense(WrProtAdr) > 0) {
		si->dskwrprot = 1;	/* disk write protected */
		if(flag & FWRITE) {
			(void)splsn();
			if(iwmflag & B_WANTED)
				wakeup((caddr_t)&iwmflag);
			iwmflag = 0;
			SPL0();
			return(EROFS);
		}
	}
	else
		si->dskwrprot = 0;	/* disk not write protected */
	if(chkhdtrk(0, unit, 0) < 0)
		si->dsk2side = 0;
	si->dskclamped = S_DONE;		/* mark startup done */
	TRACE(T_sony, ("exit open for %d\n", unit));
	(void)splsn();
	if(iwmflag & B_WANTED)
		wakeup((caddr_t)&iwmflag);
	iwmflag = 0;
	SPL0();
	return(0);
}

#ifdef notdef
snbopen(dev)
dev_t dev;
{
	int err;

	if((err = snopen(dev)) != 0)
		return(err);			/* snopen sets u_error */	
	sntb[physical(dev)].dskbsyflag = 1;
	return(0);
}
#endif notdef

FPwrUp(dev)
{
	PwrUp(dev,0);
}

PwrUp(dev,ticks)
register dev;		/* actually physical(dev) from calling proc. */
{
	register char *sndisk = (char *)IWM_ADDR;
	register int opri = splsn();

	TRACE(T_sony, ("Pwr Up 0x%x, delay %d\n", dev, ticks));
/* select disk, turn on motor */
	if(sndisk[snIntExt[dev]]);		/* tst.b */
	if(sndisk[MtrOn])	;		/* tst.b */
	if(!tout) {
		sntb[dev].active = 2;
		tout = 1;
		timeout(toff, 0, TOUTTIME * v.v_hz);
	}
	if(adrandsense(MtrOnAdr) <= 0) {
	/* already running if > 0 */
		if(sndisk[Ph3H]) ;		/* tst.b to set line high */
		asm("nop");
		asm("nop");			/* waste some time */
		if(sndisk[Ph3L]) ;		/* set line low again */
	}
	/* force spin up before other drive's next access */
	sntb[(dev == 0 ? 1 : 0)].dskclamped = S_OFF;
	if(!ticks) {
		splx(opri);
		return;
	}
	delay(ticks);
	splx(opri);
}

recal(dev)
register dev;
{
	register char *sndisk = (char *)IWM_ADDR;
	register int opri = splsn();
	register char mask = 0x17;
        register struct sonyinfo *si = &sntb[dev];
	register int reslt;
	register int cnt= 0x20000;
	register int tmp = snIntExt[dev];
	int stepicnt = 0;

	TRACE(T_sony, ("recal for %d\n", dev));
	if(sndisk[Q6H]);		/* set IWM to sense mode */
	reslt = sndisk[Q7L];		/* read status */
	if(sndisk[Q6L]);		/* set back to read mode */
	if(mask == (reslt & mask))	/* if mode already set, go 
					 * see if need power up */
		goto pwrup;
	TRACE(T_sony, ("Set IWM mode\n"));
	while(cnt--) {
		if(sndisk[MtrOff]);	/* change IWM mode */
		if(sndisk[Q6H]);	/* set sense mode */
		si->dskclamped = S_OFF;
		reslt = sndisk[Q7L];
		if(reslt & 0x20)      /* if bit 5 set interface not disabled */
			continue;
		if(mask == (reslt & mask))	/* check if mode now set */
			goto pwrup;		/* get out if so */
		sndisk[Q7H] = mask;
		if(sndisk[Q7L]);
	}
pwrup:
	if(si->dskclamped == S_DONE)
		goto arnd;
	si->dskclamped = S_INPROG;		/* mark startup in progress */
/* select disk, turn on motor */
	if(sndisk[Q6L]);			/* back to read mode */
	PwrUp(dev, (v.v_hz / 10) * 8);
	si->dskclamped = S_DONE;

arnd:		
	cnt = 85;				/* max. no. of tracks */	
	tmp = DirLAdr;
	while(cnt--) {
		adrdisk(tmp);			/* step out first */
		if(sndisk[Ph3H]) ;		/* tst.b to set line high */
		asm("nop");
		asm("nop");			/* waste some time */
		if(sndisk[Ph3L]) ;		/* set line low again */
		if((reslt = adrandsense(StepLAdr)) >= 0) { /* fail if + */
			si->dskclamped = S_OFF;
			si->curtrk = -1;
			splx(opri);
			TRACE(T_sony, ("StepLAdr failed in recal()\n"));
			return(-1);
		}
		if(sndisk[Ph3H]) ;		/* tst.b to set line high */
		asm("nop");
		asm("nop");			/* waste some time */
		if(sndisk[Ph3L]) ;		/* set line low again */
		delay(1);
		if((reslt = adrandsense(Tk0Adr)) > 0)  /* OK if + */
			goto recaled;
		if(++stepicnt < 3)
			tmp = DirLAdr;
		else
			tmp = DirHAdr;
	}
/* here only if couldn't recal in 82 steps, set error */
	si->dskclamped = S_OFF;
	si->curtrk = -1;
	splx(opri);
	return(-1);
recaled:
	si->curtrk = 0;
	splx(opri);
	return(0);			/* successfully recal'ed */
}

IPwrOff(dev)
register dev;			/* really physical(dev) */
{
	PwrOff(dev,0);
}

PwrOff(dev, ticks)
register dev;
{
	register char *sndisk = (char *)IWM_ADDR;
	register struct sonyinfo *si;
	register int opri = splsn();

	TRACE(T_sony, ("PwrOff unit 0x%x delay 0x%x\n", dev, ticks));
	si = &sntb[dev];
	adrdisk(MtrOffAdr);
	if(sndisk[Ph3H]) ;			/* tst.b to set line high */
	asm("nop");
	asm("nop");				/* waste some time */
	if(sndisk[Ph3L]) ;			/* set line low again */
	sntb[0].dskclamped = sntb[1].dskclamped = S_OFF;
	si->active = -1;
	if(!ticks) {
		splx(opri);
		return;
	}
	delay(ticks);
	splx(opri);
}


snseek(trk, unit)
register short trk;
{
	register char *sndisk = (char *)IWM_ADDR;
	register int opri = splsn();
	register struct sonyinfo *si = &sntb[unit];
	register short dirflag;
	register int   stepdir;

	if(si->curtrk < 0) {		/* had a prev. error if < 0 */ 
		TRACE(T_sony, ("curtrk bad coming into snseek\n"));
		if(recal(unit) < 0){	/* get to known state */
			splx(opri);
			return(-1);
		}
	}
	if(trk == si->curtrk) {		/* just do any pending waiting */
wait:
#ifdef	notdef
		/* Currently dead code */
		if(si->dsktimeout) {
			timeout(mywake, &seekflag, si->dsktimeout);
			(void) sleep((caddr_t)&seekflag, PRIBIO+1);
			si->dsktimeout = 0;
		}
#endif	notdef
		splx(opri);
		return(si->curtrk);	/* < 0 if error */
	}
	if(trk < si->curtrk) {
		dirflag = -1;
		stepdir = DirHAdr;
	} else {
		dirflag = 1;
		stepdir = DirLAdr;
	}
	TRACE(T_sony,("snseek trk == 0x%x curtrk == 0x%x\n", trk, si->curtrk));
	while(trk != si->curtrk) {
		si->curtrk += dirflag;
		adrdisk(stepdir);
		if(sndisk[Ph3H]) ;		/* tst.b to set line high */
		asm("nop");
		asm("nop");			/* waste some time */
		if(sndisk[Ph3L]) ;		/* set line low again */
		if(adrandsense(StepLAdr) > 0) {
			TRACE(T_sony, ("StepLAdr error in snseek\n"));
			goto errxit;
		}
		if(sndisk[Ph3H]) ;		/* tst.b to set line high */
		asm("nop");
		asm("nop");			/* waste some time */
		if(sndisk[Ph3L]) ;		/* set line low again */
		delay(1);
	}		
	delay(2);
	goto wait;
errxit:
	si->curtrk = -1;
	goto wait;
}


reseek(track, unit)
short track;
{

	if(snseek(track, unit) < 0) {
		if(recal(unit) < 0) 
			return(-1);
		if(snseek(track, unit) < 0)
			return(-1);
	}
	return(0);
}

/*	chkhdtrk -- check head and track.
 *	    If the drive is positioned on the correct head and track,
 *	this routine returns the current sector number.
 *	    If not positioned correctly, it returns a negative number.
 *	SIDE EFFECT:
 *	    The number of sides according to the current disk format
 *	is set in the disk status table.
 */

chkhdtrk(trk, unit, side)

{
	register int opri = splsn();
	struct	astuff {
		short	sidetrk;
		u_char	cursec;
		u_char	vol;
		} rd;
	register retval;

retry:
	if(side)
		(void)adrdisk(RdData1Adr);
	else
		(void)adrdisk(RdDataAdr);
	/* retval: if positive then low word == secnum
	 *	   high word == <side><track>
	 * else error code.
	 */
	TRACE(T_sonyfull, ("rdhead in chkhdtrk()\n"));
	if((retval = rdhead((caddr_t)&rd)) < 0) {
		if(++sntab.b_errcnt < NRETRY)
			goto retry;
		splx(opri);
		TRACE(T_sony, ("error 0x%x from rdhead\n", retval));
		return(retval);
	}
	if((rd.sidetrk & 0xFF) != trk) {
		TRACE(T_sony, ("wrong track in chkhdtrk\n"));
		return(-1);
	}
	if(((rd.sidetrk >> 11) & 1) != side) {
		TRACE(T_sony, ("wrong side in chkhdtrk 0x%x\n", rd.sidetrk));
		return(-1);
	}
#ifdef	HOWFAR
	if(sntb[unit].dsk2side != (rd.vol > 0x1f)) {
		if(rd.vol > 0x1f)
			printf("two sided!\n");
		else
			printf("one sided!\n");
	}
#endif  HOWFAR
	sntb[unit].dsk2side = (rd.vol > 0x1f);
	splx(opri);
	TRACE(T_sonyfull, ("return 0x%x from chkhdtrk\n", rd.cursec));
	return(rd.cursec);
}

snclose(dev)
dev_t dev;
{
	register punit;

	if ((punit = physical(dev)) >= SNCNT) {
		return(ENXIO);
	}
	SPL2();
	while(sntab.b_active || sntb[punit].closing) {
		sntb[punit].closing = 1;
		sleep(&sntb[punit].closing, PRIBIO);
	}
	SPL0();
	return(0);
}

#ifdef notdef
snbclose(dev)
dev_t dev;
{
	int err;

	if((err = snclose(dev)) < 0)
		return(err);	
	sntb[physical(dev)].dskbsyflag = 0;
	return(0);
}
#endif notdef

/*	toff -- turn off drive.
 *	     This routine is called from a timeout.  Each time a drive
 *	is accesed, the active counter is set to 2.  This routine knocks
 *	it down.  When active reaches zero, the drive is turned off.  Inactive
 *	drives are marked by -1 in their activity counters.
 */

toff()
{
	register i;
	register struct	sonyinfo *si;

	si = sntb;
	tout = 0;
	for(i = 0; i < SNCNT; ++i, ++si) {
		if(si->active > 0 && --(si->active) > 0)
			tout = 1;
	}
	if(iwmflag)
		tout = 1;
	if(tout)
		timeout(toff, 0, TOUTTIME * v.v_hz);
	else {
		for(i = 0, si = sntb; i < SNCNT; ++i, ++si) {
			if(si->active == 0)
				PwrOff(i, 0);
		}
	}
}

snstrategy(bp)
register struct buf *bp;
{
	register punit;

	punit = physical(bp->b_dev);
	if (bp == &sncbuf) {				/* if command */
		snstat[punit].io_misc++; /* errlog: */
		(void) splsn();
		if (sntab.b_actf == (struct buf *)NULL) /* set up links */
			sntab.b_actf = bp;
		else
			sntab.b_actl->av_forw = bp;
		sntab.b_actl = bp;
		bp->av_forw = (struct buf *)NULL;
	} else {
		snstat[punit].io_ops++; /* errlog: */
			/* resid for disksort */
		bp->b_resid = bp->b_blkno + 
		(sn_sizes[logical(bp->b_dev)].sn_offset << sntb[punit].dsk2side);
		(void) splsn();
		disksort(&sntab, bp);
	}
	if (sntab.b_active == 0)
		snstart();
	(void) spl0();
}

snstart()
{
	register struct buf *bp;
	register lunit;
	register struct sonyinfo *si;
	register daddr_t bn;
	register lastblk;
	int	punit;
	caddr_t addr;

	while(iwmflag & B_BUSY) {
		iwmflag |= B_WANTED;
		/* ASSERT: always called from process context */
		sleep((caddr_t) &iwmflag, PRIBIO+1);
	}
	iwmflag |= B_BUSY;
loop:
	if ((bp = sntab.b_actf) == (struct buf *)NULL) {
		if(iwmflag & B_WANTED)
			wakeup((caddr_t)&iwmflag);
		iwmflag = 0;
		for(si = sntb; si < &sntb[SNCNT]; ++si) {
			if(si->closing) {
				si->closing = 0;
				wakeup(&si->closing);
			}
		}
		return;
	}
	if (sntab.b_active == 0) {
		sntab.b_active = 1;
		if (bp != &sncbuf) {
			bp->b_resid = bp->b_bcount;
		}
	}
	blkacty |= (1<<SN1);
	if(misscount >= 1000) {	/* if potentially 10 seconds clock lost */
		nvram_timewarp();	/* Lets do the time warp again */
		misscount = 0;
	}
	punit = physical(bp->b_dev);
	lunit = logical(bp->b_dev);
	si = &sntb[punit];
	if(si->dskclamped == S_OFF) {
		si->dskclamped = S_INPROG;
		PwrUp(punit, (v.v_hz / 10) * 8);
		si->dskclamped = S_DONE;
	}
	si->active = 2;
	if (bp == &sncbuf) {
		sncmd(bp);	/* b_resid holds the command */
		bp->b_resid = 0;
		sntab.b_errcnt = 0; 
		goto done;
	}
	bn = bp->b_blkno + ((bp->b_bcount - bp->b_resid) >> 9);
	lastblk = sn_sizes[lunit].sn_size << si->dsk2side;
	if(bn >= lastblk) {
		if (bn > lastblk || (!(bp->b_flags & B_READ)
		  && bp->b_resid == bp->b_bcount)) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		goto done;	/* will return EOF */
	}
	if (bp->b_resid < 512 ||
	    (!(bp->b_flags&B_READ) && si->dskwrprot)) {
error:
		if (bp->b_resid != 0) {
			bp->b_flags |= B_ERROR;
#ifdef HOWFAR
			printf("Unix snstart: blkno=%d resid=%d error=%d\n",
				 bp->b_blkno, bp->b_resid, bp->b_error);
#endif HOWFAR
		}
		bp->b_error = EIO;
done:
		blkacty &= ~(1<<SN1);
		if (sntab.b_errcnt)
			logberr(&sntab, 0); /* errlog non-fatal errors */
		sntab.b_active = 0;
		sntab.b_errcnt = 0;
		sntab.b_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}
	addr = bp->b_un.b_addr + bp->b_bcount - bp->b_resid;
	TRACE(T_sony, ("snstart bp = 0x%x ", bp));
	if(snrw(punit, bn, bp->b_flags&B_READ, addr) < 0)  {
		goto error;
	}
	bp->b_resid -= 512;
	goto loop;
}

snread(dev, uio)
dev_t	dev;
struct uio *uio;
{
	return(physio(snstrategy, (struct buf *)NULL, dev, B_READ, uio));
}

snwrite(dev, uio)
dev_t	dev;
struct uio *uio;
{
	return(physio(snstrategy, (struct buf *)NULL, dev, B_WRITE, uio));
}


struct sn_blockmap {
	int maxblock, sectors;
} sn_blockmap[] = {
		192, 12,
		176, 11,
		160, 10,
		144, 9,
		128, 8 
};

/*
 * this routine is responsible for seeking to the correct track, finding the
 * sector and then reading or writing it, as appropriate
 */
 
/* ARGSUSED */
snrw(unit, bn, rw, addr)    /* always called at priority sn (see splsn above) */
int unit, rw;
register daddr_t bn;
register caddr_t addr;
{

	register struct sn_blockmap *p = sn_blockmap;
	register struct sonyinfo *si = &sntb[unit];
	register int cursec, sector;
	register int side;
	int	stat;
	int track, beenhere = 0;

	TRACE(T_sony, ("in snrw %s for unit %d, bn 0x%x addr 0x%x\n", 
		rw ? "read" : "write", unit, bn, addr));

	side = si->dsk2side;
	track = 0;
	while( bn >= (p->maxblock << side)) {
		bn -= p->maxblock << side;
		track += 16 << side;
		p++;
	}
	sector = bn % p->sectors;
	track += bn / p->sectors;
	if(side) {
		side = (track & 1);
		track >>= 1;
	}
	TagData[0] = sector;
	(void) spl5();
	if(reseek(track, unit) < 0) {
		TRACE(T_sony, ("reseek failed in snrw\n"));
		return(-1);
	}
/*
 * check that we have the right track and side. If we fail, force a
 * recal and reseek and check again
 */
retry:
	sntab.b_errcnt = 0;
	if((cursec = chkhdtrk(track, unit, side)) < 0) {
		TRACE(T_sony, ("about to reseek in srw\n"));
		si->curtrk = -1;		/* to force recal. */
		sntab.b_errcnt = 0;
		if((cursec = chkhdtrk(track, unit, side)) < 0) { /* try again */
			TRACE(T_sony, ("2 errors in snrw\n"));
			return(-1);	/* only 2 tries */
		}
	}		
	if(cursec != sector) {
		TRACE(T_sony2, ("%x", cursec));
		if(beenhere++ > 100) {
			TRACE(T_sony2, ("?%x? ", sector));
			return(-1);
		}
		++misscount;
		delaysec();
		goto retry;
	}
	if(rw) {
		stat = snrd(addr);
		TRACE((T_sony && stat < 0), ("snrd stat = 0x%x\n", stat));
		TRACE(T_sony2, ("-%x ", cursec));
		return(stat);
	}
	else {
		stat = snwr(addr);
		TRACE((T_sony && stat < 0), ("snwr stat = 0x%x\n", stat));
		TRACE(T_sony2, ("-%x ", cursec));
		return(stat);
	}
}

delaysec()
{
#define	DELAY_CNT	2500	/* # of counts in 9.5 ms */
#define DELAY()		{int i; for (i = 0; i < DELAY_CNT; i++);}

    	/* a sector passes under the head in about 11 ms, to avoid missing
   	 * the next one, use the VIA as a counter and poll it.
    	 */

	DELAY();
#undef	DELAY_CNT
}

/* ARGSUSED */
snioctl(dev, cmd, adr, flag)
dev_t dev;
caddr_t adr;
{
	int err, unit;
	struct diskformat *df;
	register struct buf *bp;

	if ((unit = physical(dev)) >= SNCNT) {
		return(ENXIO);
	}
	bp = &sncbuf;
	switch (cmd) {
		case AL_EJECT:
			if (sntb[unit].dskbsyflag > 0) {
				return(EINVAL);
			}
			break;
		case GD_PARTSIZE:
			u.u_rval1 = sn_sizes[logical(dev)].sn_size 
					<< sntb[unit].dsk2side;
			return(0);
		case UIOCFORMAT:
			if (!(flag & FWRITE)) {
				return(EPERM);
			}
			if (sntb[unit].dskbsyflag > 0) {
				return(EINVAL);
			}
			if(!sntb[unit].dskthere || 
			   sntb[unit].dskclamped == S_OFF) {
				return(EINVAL);
			}
			/*
			 *	read the data block from user space
			 */

			df = (struct diskformat *)adr;
			if(df->d_lhead == DISKDEFAULT || df->d_lhead == 1) {
				cmd = 2;
				TRACE(1, ("format two sides\n"));
			}
			else {
				TRACE(1, ("format one side\n"));
			}
			break;
		default:
			return(ENOTTY);
	}
	err = 0;
	GETBUF(bp);
	bp->b_dev = dev;
	bp->b_resid = cmd;		/* stash the command in resid */
	bp->b_error = 0;
	snstrategy(bp);
	iowait(bp);
	if (bp->b_flags & B_ERROR) {
		if (bp->b_error) {
			err = bp->b_error;
		} else {
			err = EIO;
		}
	}
	FREEBUF(bp);
	sntb[unit].dskclamped = S_OFF;
	return(err);
}
/*
 * this is global so the assembly language code can
 * find it 
 */
char fmtbuf[1024];

sncmd(bp)	/* always called at priority sn (see splsn above) */
struct buf *bp;
{
	unsigned int cmd = bp->b_resid;
	register int unit = physical(bp->b_dev);  /* d2 */
	register char *sndisk = (char *) IWM_ADDR;  /* a2 */
	register struct sonyinfo *si = &sntb[unit];  /* a3 */
	register char *bufptr = fmtbuf, *bptr; /* a4 a5 */
	register int i, j;  /* d3 d4 */

	TRACE(T_sony, ("sncmd\n"));
	switch (cmd) {

		case UIOCFORMAT:
		case 2:
			GapSync = 7;
		/* turn on the disk */
			FPwrUp(unit);
			if(cmd == 2)
				fmt2side = -1;
			else
				fmt2side = 0;
	/* fill the buffer with info., 12 sectors & 27 bytes per sector */
			for(i=0; i < 12; i++) {
				bptr = SyncTbl;
				for(j = 0; j < 27; j++)
					*bufptr++ = *bptr++;
			}
	/* loop for each of 80 tracks */
			for(j=0; j < 80; j++) {
	/* seek to track, error if can't */

				if((i = reseek(j, unit)) != 0) {
					bp->b_flags |= B_ERROR;
					return;
				}
				si->active = 2;
				if((i = FormatTrack(j)) != 0) { /* assembly language */
					bp->b_flags |= B_ERROR;
					return;
				}
			}
			(void) recal(unit);
			IPwrOff(unit);
			break;

		case AL_EJECT:
			FPwrUp(unit);	
			(void) recal(unit);
			adrdisk(EjectHAdr);	/* tell it to pop out disk */
			if(sndisk[Ph3H])  ;	/* set strobe high */
			delay(v.v_hz << 1);	/* XXX review eject code */
			si->dskthere = 0;
			si->dskclamped = S_OFF;
			if(sndisk[Ph3L]) ;	/* set strobe low */
			IPwrOff(unit);
			break;
		default:
			TRACE(1, ("Unknown sncmd number = 0x%x\n", cmd));
			bp->b_error = EINVAL;
			bp->b_flags |= B_ERROR;
			return;
	}
	return;
}

snprint(dev, str)
char *str;
{
	printf("%s on sn drive %d\n", str, (dev>>4)&0xF);
}
