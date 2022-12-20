#ifndef lint	/* .../sys/psn/io/st225.c */
#define _AC_NAME st225_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.3 87/11/19 18:04:23}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.3 of st225.c on 87/11/19 18:04:23";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#ifdef	HOWFAR
int	T_st225 = 0;
#endif	HOWFAR
/*
 *  Seagate ST225 disk controller 
 *
 *  Copyright 1986 Unisoft Corporation of Berkeley CA
 *
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 *
 *	minor device assignment
 *	Bits 0-2	- disk partition 0-7
 *	Bit 3		- SCSI logical unit 0-1
 *	Bits 4-6	- SCSI ID 0-7
 *	Bit  7		- enable writing on block 0
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
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/utsname.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/iobuf.h"
#include "sys/var.h"
#include "sys/reg.h"
#include "sys/debug.h"
#include "sys/uio.h"
#endif lint
#include "sys/uioctl.h"
#include "sys/altblk.h"
#include "sys/diskformat.h"
#include "sys/ncr5380.h"

#define	NID	7			/* number of SCSI IDs */
#define NST	NID*2			/* number of devices */

#define	DEVBLKSIZE	512		/* length of a typical data read */
#define TRANSFER	65024		/* maximum transfer in bytes */
#define	SENSESIZE	sizeof(struct st_extsense)
					/* size of buffer for extended sense */
#define	INQSIZE		58		/* size of INQUIRY command buffer */
#define	MODESIZE	12		/* size of mode buffer */
#define	CAPSIZE		8		/* size of buffer for disk capacity */
#define MAXCMDSIZE	10		/* size of buffer for commands */
#define	SCSIDSKDEVICE	0		/* code for scsi direct access device */

#define unit(p)	(((p)>>3)&0xf)		/* unit number of device */
#define part(p) ((p)&7)			/* partition on unit */
#define okwrite0(p) ((p)&0x80)		/* can we write on block 0 */

#define xprintf if (st_unit[punit].st_xprintf) printf

#define	contr(x)	(x >> 1)	/* SCSI id from punit */
#define	lun(x)		(x & 1)		/* SCSI logical unit from punit */

/*
 *	the io queue headers
 */

struct iobuf st_tab[NID];		/* <<<< */
struct iostat st_iostat[NID];		/* errlog */
static int	ourmajor;		/* The driver's major device number */


/*
 *	SCSI request buffers.
 */

struct scsireq st_reqs[NID];
char	st_cmdbuf[NID][MAXCMDSIZE];
struct st_extsense {
	u_char	errinfo;
	u_char	fill1;
	u_char	key;
	u_char	blk[4];
	u_char	len;
	u_char	fill2[4];
	u_char	code;
	u_char	fill3[5];
	u_short	cyl;
	u_char	head;
	u_char	sect;
	u_char	rdcount[3];
	u_char	seekcount[3];
	u_char	uncor_read;
	u_char	uncor_write;
	u_char	seek;
	} st_sensebuf[NID];

/*
 *	partition information for this disk
 */

struct st_part {
	int	p_offset;
	int	p_size;
};


#define	STATE_NOINIT	0	/* device has not been initialized */
#define	STATE_NORMAL	1	/* a normal io is being done (default) */
#define	STATE_START	2	/* the device is being initialized */
#define	STATE_FMT	3	/* Drive is being formatted */


/*
 *	Commands
 */

#define	C_READ		SOP_READ	/* read multiple sector */
#define	C_WRITE		SOP_WRITE	/* write multiple sector */
#define C_FORMAT	SOP_FMT		/* format the device */
#define	C_TSTRDY	SOP_RDY		/* Test unit ready */
#define	C_INQ		SOP_INQ		/* inquire as to device type */
#define	C_SETMODE	SOP_SELMODE	/* Set device mode */
#define	C_GETMODE	SOP_GETMODE	/* Get device mode info */
#define	C_READCAP	SOP_READCAP	/* Get device capacity */

struct	deverreg st_sense_err[2] = {
	{0, "Sense key", -1, ""},
	{0, "Sense Code", -1, ""}
	};

struct	deverreg st_gen_err[2] = {
	{0, "Ret Value", -1, ""},
	{0, "Status Word", -1, ""}
	};

/*	Error actions.
 *	     Certain errors from the drive may or may not be a problem,
 *	depending on the state of the drive.
 *	Any unmarked error is assumed to be fatal.
 */


#define	S(n)	(1 << (n))
struct	erract	{
	u_char	code;	/* pertinent error code */
	u_char	mask;	/* mask of states effected */
	};
	
struct  erract st_fatal[] = {
	SST_MORE, S(STATE_START),
	SST_LESS, S(STATE_START),
	SST_STAT, S(STATE_START) | S(STATE_START),
	0, 0
	};

struct	erract st_initfatal[] = {
	SST_STAT,  S(1) | S(2),
	0, 0
	};
	
/*
 *	Each different "type" of disk has an entry in this
 *		table, for each disk the index of its
 *		"type" in this table should be in st_disktype
 */

struct st_unit {
	struct	buf *st_fmtbuf;	/* buffer when formatting */
	struct	buf *st_initb;	/* buffer for init requests */
	int	st_count;	/* count, used to update EOF */
	int	st_blknum;	/* block number for error messages */
	unsigned maxbn;		/* maximum block number */
	u_short	bsize;		/* block size of drive format */
	u_char	inter;		/* disk interleave */
	u_char	st_state;	/* current state of the device */
	u_char	st_xprintf;	/* flag for extended printing */
	u_char	st_retry;	/* flag that operation is a retry */
	u_char	st_initstate;	/* incremented as we perform init commands */
	u_char	st_fmtstate;	/* incremented as we do a format */
	u_char	st_needfmt;	/* concurrency control for formatting */
} st_unit[NST];


/*
 *	bad block information
 */

struct altblk st_altblk[NST];

/*
 *	disk partitioning information, one per unit
 *		the first number is the offset in blocks
 *		from the start of the disk, the second
 *		is the size in blocks of the partition.
 *	For this machine, partition sizes are supplied from the disk.
 *	XXX For now, we caluclate three partitions.
 */

struct st_part st_part[NST][8];
static	calcpart();		/* routine to calculate partitions */

/*
 *	initialisation routine called once during system startup,
 *		at spl7 before interrupts are enabled
 */

stinit()
{
	register int i;
	extern stintr(), ststrategy();
		
	for(ourmajor = 0; ourmajor < bdevcnt; ++ourmajor) {
		if(bdevsw[ourmajor].d_strategy == ststrategy)
			break;
	}
	if(ourmajor > bdevcnt)
		panic("can't find st major device in devsw");
/*
 *	for each controller set up its state and io buffers
 */

	for (i = 0; i < NID; ++i) {
		st_reqs[i].faddr = stintr;
		st_reqs[i].sensebuf = (caddr_t)&st_sensebuf[i];
		st_reqs[i].senselen = SENSESIZE;
		st_reqs[i].cmdbuf = st_cmdbuf[i];
		st_tab[i].b_dev = makedev(ourmajor,(i << 4));
		st_tab[i].io_stp = &st_iostat[i];
	}

/*	initialize each device
 */
	for (i = 0; i < NST; ++i) {
		st_unit[i].st_state = STATE_NOINIT;
		st_unit[i].st_xprintf = 1;
	}
}



/*	stinit0 -- initialize device.
 *	If the device shows as not initialized, perform initialization
 *	tasks for read or write.
 *	Always called at interrupt priority.
 */

stinit0(punit)
register punit;
{
	register struct st_unit *up;

	TRACE(T_st225, ("in stinit0\n"));
	up = &st_unit[punit];
	up->st_initstate = 1;
	up->st_state = STATE_START;
	ststart(punit);
}


/*	calcpart -- calculate partitions for device.
 *	     This routine is temporary.  It calculates device partitions
 *	as follows:
 *		0 -- greater of 3/4 of the disk, or total disk - 10 meg
 *		1 -- 1/4 of disk to a max of 10 meg.
 *		7 -- entire disk.
 *
 *	It is necessary for this routine to continue to support exisiting
 *	partitions for ST225 drives.  No effective attempt is made to align 
 *	to cylinder boundaries.  This is a mistake.
 *
 *	In the future all partition information will be read in from the disk.
 */

static
calcpart(punit)

{
	struct st_part *partp;
	register total;
	int	part1size;

	total = st_unit[punit].maxbn;
	partp = st_part[punit];
	TRACE(T_st225, ("total size %d ", total));
	if(total < 42000 && total > 40000) {	/* use initial ST225 map */
		partp[0].p_offset = 204;
		partp[0].p_size = 31280;
		partp[1].p_offset = 31280 + 204;
		partp[1].p_size = total - partp[1].p_offset;
		partp[7].p_size = total;
		TRACE(T_st225, ("ST225 mapping part1size = %d\n", 
				partp[1].p_size));
	}
	else {
		part1size = (total / 4);
		if(part1size > (10 * 1024 * 1024) / DEVBLKSIZE) {
			part1size = (10 * 1024 * 1024) / DEVBLKSIZE;
		}
		partp[0].p_offset = 204;
		partp[0].p_size = total - part1size - 204;
		partp[1].p_offset = total - part1size;
		partp[1].p_size = part1size;
		partp[7].p_size = total;
		TRACE(T_st225, ("part0size %d part1size %d\n", 
			partp[0].p_size, partp[1].p_size));
	}
}
	
/*
 * ststrategy - called by the system to read/write a buffer to/from
 *		the disk. It is declared in bdevsw (in conf.c) and
 *		is also called indirectly thru physio in the devices
 *		read and write routines below.
 */

#define b_cylin	b_resid

ststrategy(bp)
register struct buf *bp;
{
	register struct iobuf *dp;
	register int punit;

	(void) spl2();				/* protect critical section */
	punit = unit(bp->b_dev);		/* get unit number */
	dp = &st_tab[contr(punit)];		/* get the io Q header */
						/*   transfered             */
	bp->b_cylin = bp->b_blkno;		/* set up for next line */
	disksort(dp, bp);			/* call the disksort routine */
						/*   to insert the buffer in */
						/*   the queue at a 'more */
						/*   efficient' place */
	if (dp->b_active == 0)			/* if there are no i/os */
		ststart(punit);
	(void) spl0();
}

/* 
 *	start a disk I/O, this is always called with disk interrupts disabled 
 */

ststart(punit)
short punit;
{
	register struct buf *bp;
	register struct iobuf *dp;
	register struct st_unit *up;
	int i, bn;

	dp = &st_tab[contr(punit)];
	up = &st_unit[punit];
loop:
	if ((bp = dp->b_actf) == NULL){		/* if there is no work to do */
		return;				/*    just return */
	}

	up->st_retry = 0;
/*
 *	on the first time around set the residual correct
 */

	if (dp->b_active == 0) {
		bp->b_resid = bp->b_bcount;
		up->st_count = 0;
		dp->b_active++;
	}
	if (bp == up->st_fmtbuf && up->st_state != STATE_FMT) {	
		up->st_fmtstate = 0;		/* start a format command */
		up->st_state = STATE_FMT;
		stcmd(punit, C_TSTRDY, 0, 0, 0);
		return;
	}
	if(up->st_state == STATE_NOINIT) {
		stinit0(punit);
		return;
	}
	switch(up->st_state) {
	case STATE_START:
		bp = up->st_initb;
		switch(up->st_initstate) {
		case 1:	/* test device ready */
			stcmd(punit, C_TSTRDY, 0, 0, 0);
			break;
		case 2:	/* ask device what it is */
			stcmd(punit, C_SETMODE, 0, 0, 0);
			break;
		case 3:	/* ask device its size */
			stcmd(punit, C_READCAP, 0, bp->b_un.b_addr, CAPSIZE);
			break;
		default:
			printf("state == 0x%x\n", up->st_initstate);
			panic("unknown initialization state in ststart");
			break;
		}
		return;
	case STATE_FMT:
		switch(up->st_fmtstate) {
		case 1:
			stcmd(punit, C_SETMODE, 0, bp->b_un.b_addr, MODESIZE);
			break;
		case 2:
			stcmd(punit, C_FORMAT, 0, 0, 0);
			break;
		default:
			panic("ststart, internal error in format states");
			break;
		}
		return;
	case STATE_NORMAL:

/*
 *	make sure that this isnt a reference outside the partition
 */

		if ((bn = (bp->b_blkno + (up->st_count >> 9))) < 0) {
			bp->b_flags |= B_ERROR;
	     done:
			dp->b_actf = bp->av_forw;
			iodone(bp);
			dp->b_active = 0;
			goto loop;
		};

/*
 *	make sure that we do not transfer >= 65536 bytes at once
 */

		bp->b_bcount = bp->b_resid > TRANSFER ? TRANSFER : bp->b_resid;
		if(bp->b_bcount < DEVBLKSIZE) {	/* must xfer something */
			goto done;
		}
/*
 *	make sure that accessing past the end of a partition works 
 *		properly (you should get EOF)
 */

		i = (st_part[punit][(short)part(bp->b_dev)].p_size - bn) << 9;
		if (i > 0) {
			if (i < bp->b_bcount)
				bp->b_bcount = i;
		} else {
			goto done;
		}
		bn += st_part[punit][(short)part(bp->b_dev)].p_offset;

/*
 *	protect block 0 from writing (dont give an error, just step over it)
 */

		if (bn == 0 && !(bp->b_flags&B_READ) && !okwrite0(bp->b_dev)){
			if ((bp->b_resid -= DEVBLKSIZE) < DEVBLKSIZE)
				goto done;
			bp->b_bcount -= DEVBLKSIZE;
			up->st_count += DEVBLKSIZE;
			bn++;
		}
		stcmd(punit, (bp->b_flags&B_READ ?C_READ:C_WRITE), bn, 
			bp->b_un.b_addr + up->st_count, (int)bp->b_bcount);
		break;
	default:	/* can't happen */
		printf("Undefined state 0x%x in ststart for st%d\n",
			up->st_state, punit);
		up->st_state = STATE_NOINIT;
		break;
	}
}

/*
 *	this routine does all the work of setting up the SCSI request.
 */
stcmd(punit, cmd, bn, addr, count)
caddr_t addr;
{
	register struct scsireq *rqp;
	register struct st_unit *up;

	TRACE(T_st225, ("stcmd unit 0x%x cmd 0x%x bn 0x%x addr 0x%x count 0x%x\n", 
		punit, cmd, bn, addr, count));
	rqp = &st_reqs[contr(punit)];
	rqp->databuf = addr;
	rqp->driver = punit;
	up = &st_unit[punit];
	up->st_blknum = bn;
	switch(cmd) {
	case SOP_SELMODE:
	case SOP_GETMODE: 
	case SOP_INQ:
	case SOP_RDY:
		scsig0cmd(rqp, cmd, lun(punit), 0, count, 0);
		rqp->datalen = count;
		rqp->flags = (cmd != SOP_SELMODE) ? SRQ_READ : 0;
		up->st_blknum = 0;
		break;
	case SOP_READ:
	case SOP_WRITE:
		scsig0cmd(rqp, cmd, lun(punit), bn, count >> 9, 0);
		rqp->datalen = count;
		rqp->flags = (cmd == C_READ) ? SRQ_READ : 0; 
		break;
	case SOP_FMT:
		scsig0cmd(rqp, cmd, lun(punit), 0, (int)up->inter, 0);
		rqp->flags = SRQ_NOTIME;
		rqp->datalen = 0;
		up->st_blknum = 0;
		break;
	case SOP_READCAP:
		bzero(rqp->cmdbuf, 10);
		rqp->cmdbuf[0] = SOP_READCAP;
		rqp->cmdbuf[1] = lun(punit) << 5;
		rqp->datalen = count;
		rqp->flags = SRQ_READ;
		rqp->cmdlen = 10;
		up->st_blknum = 0;
		break;
	default:	/* Can't happen */
		panic("unimplemented command code in stcmd()");
		break;
	}
	(void) scsirequest(contr(punit), rqp);
	return;
}


/*
 * the device interrupt service routine
 */

stintr(rp)
register struct scsireq *rp;
{
	register struct	buf *bp;
	register struct iobuf *dp;
	register struct st_unit *up;
	register punit;
	struct	deverreg *erstuff;
	int	bn;
	int	wasfatal = 0;

	punit = rp->driver;
	dp = &st_tab[contr(punit)];
	up = &st_unit[punit];
	TRACE(T_st225, ("stintr rp=0x%x punit = 0x%x state = 0x%x\n", 
		rp, punit, up->st_state));

	if(dp->b_active == 0 || (bp = dp->b_actf) == 0) {
		dp->b_active = 0;
		printf("st%d: spurious interrupt\n",punit); /* impossible */
		return;
	}

	/* handle error printing
	 */

	if(st_isfatal((int)rp->ret, st_fatal, up->st_state)) {
		wasfatal = 1;
		xprintf("stintr: error on SCSI device %d\n", punit);
		xprintf("'%s' stat = 0x%x msg = 0x%x\n",
			scsi_strings[rp->ret], rp->stat, rp->msg);
		bn = up->st_blknum;
		if(rp->ret == SST_SENSE || rp->sensesent != 0) {
			xprintf("key = 0x%x code = 0x%x\n",
				st_sensebuf[contr(punit)].key,
				st_sensebuf[contr(punit)].code);
			st_sense_err[0].drvalue = st_sensebuf[contr(punit)].key;
			st_sense_err[1].drvalue = st_sensebuf[contr(punit)].code;
			erstuff = st_sense_err;
		}
		else {
			st_gen_err[0].drbits = scsi_strings[rp->ret];
			st_gen_err[0].drvalue = rp->ret;
			st_gen_err[1].drvalue = rp->stat;
			erstuff = st_gen_err;
		}

		fmtberr(dp, (unsigned)punit, -1, -1, (unsigned)bn,
			2, &erstuff[0], &erstuff[1]);
		logberr(dp, 1);
		if(rp->ret == SST_STAT || rp->sensesent != 0) {
			if(!up->st_retry) {
				up->st_retry = 1;
				xprintf("retry\n");
				(void)scsirequest(contr(punit), rp);
				return;
			}
		}
	}
			
	switch(up->st_state) {
	case STATE_NORMAL:
		if(wasfatal) {
			up->st_state = STATE_NOINIT;
			bp->b_flags |= B_ERROR;
			goto err_norm;
		}
/*
 *	now update the residual, this makes EOF work
 */

		bp->b_resid -= bp->b_bcount;
		up->st_count += bp->b_bcount;

/*
 *	then if there is no more to transfer then go to the next buffer
 */

		if (bp->b_resid < DEVBLKSIZE) {

/*
 *	now unlink the buffer from the queue and set us up for the
 *		next io
 */
err_norm:  /* error and normal exit */
			dp->b_actf = bp->av_forw;

/*
 *	wake up any processes waiting for this buffer
 */
			iodone(bp);
			dp->b_active = 0;
		}
		break;


	case STATE_START:
		if(st_isfatal((int)rp->ret, st_initfatal, up->st_initstate)) {
			TRACE(T_st225, ("ret 0x%x fatal for initstate %d\n",
				rp->ret, up->st_initstate));
			if(!wasfatal) {	/* if no message printed */
				printf("can't access st%d\n", punit);
			}
			bp = dp->b_actf;
			bp->b_flags |= B_ERROR;
			iodone(bp);
			dp->b_actf = bp->av_forw;
			up->st_state = STATE_NOINIT;
initdone:
			dp->b_active = 0;
			up->st_initstate = 0;
			break;
		}
		bp = up->st_initb;
		switch(up->st_initstate) {
		case 1:
		case 2:
			up->st_initstate++;
			break;
		case 3:
#ifndef lint			/* possible pointer alignment problem */
			if((up->maxbn = *((long *)bp->b_un.b_addr))
						< 100) {
				printf("st%d has inadequate capacity of %d blocks\n",
					punit, up->maxbn);
				printf("st%d not opened\n", punit);
				goto initdone;
			}
			up->bsize = *((long *)(&bp->b_un.b_addr[4]));
#endif lint
			if(up->bsize == 532) {
				(void) scsidevchar(contr(punit), SDC_532);
				TRACE(T_st225, ("532 byte format "));
			}
			else if(up->bsize == DEVBLKSIZE)
				(void) scsidevchar(contr(punit), 0);
			else if(up->bsize != DEVBLKSIZE) {
				printf("st%d has unsupported block size of %d bytes\n", 
					punit, up->bsize);
				printf("st%d not opened\n", punit);
				goto initdone;
			}
			calcpart(punit);
			up->st_state = STATE_NORMAL;	/* init finished ok */
			goto initdone;
		default:
			printf("software error in device initialization\n");
			goto initdone;
		}
		break;
	case STATE_FMT: 
		if(!wasfatal) {
			TRACE(T_st225, ("Format,  bp = 0%x fmtstate = 0x%x\n", 
				bp, up->st_fmtstate));
			switch(up->st_fmtstate) {
			case 0:
			case 1:
				up->st_fmtstate++;
				break;
			case 2:		/* all done, ok */
				up->st_state = STATE_NOINIT;
				dp->b_actf = bp->av_forw;
				iodone(bp);
				dp->b_active = 0;
				break;
			default:
				panic("impossible format stage");
				break;
			}
		}
		else {
			TRACE(T_st225, ("fatal error on format\n"));
			bp->b_flags = B_ERROR;
			up->st_state = STATE_NOINIT;
			goto err_norm;
		}
		break;
	default:
		panic("undefined state in stintr()");
		break;
	}

/*
 *	start the next io
 */

	ststart(punit);			/* start the next one */
	TRACE(T_st225, ("stintr complete\n"));
}

/*	st_isfatal -- determine if an error is fatal.
 */

st_isfatal(error, actp, state)

register error;
register struct erract *actp;
unsigned state;
{

	if(error == 0)
		return(0);
	while(actp->code != 0 && actp->mask != 0) {
		if(actp->code == error)
			return((actp->mask & (1 << state)) == 0);
		++actp;
	}
	return(1);
}


/*
 * stopen - open the device, check it is a valid device
 */

/*ARGSUSED*/
stopen(dev, flag)
{
	register punit = unit(dev);
	register struct st_unit *up;
	register struct buf *bp;

	up = &st_unit[punit];
	TRACE(T_st225, ("stopen punit = 0x%x state = 0x%x\n",
			punit, up->st_state));
	if (punit >= NST)  {
		TRACE(T_st225, ("Can't open st%d\n", dev));
		return(ENXIO);
	}
	if(up->st_initb == NULL) {	/* first open */
		bp = geteblk(DEVBLKSIZE);
		SPL2();
		if(up->st_initb == NULL) {
			up->st_initb = bp;
			bp->b_dev = makedev(ourmajor, minor(dev));
		}
		else {
			brelse(bp);
		}
		SPL0();
	}
	return(0);
}

/*
 * stclose - close the device
 */

/*ARGSUSED*/
stclose(dev)
{
}

/*
 * stioctl - preform ioctl operations on the raw device
 */

/*ARGSUSED*/
stioctl(dev, cmd, addr, flag)
dev_t dev;
int cmd;
caddr_t addr;
int flag;
{
	switch (cmd) {
case UIOCFORMAT:
		return(stformat(dev, addr));
		break;
case UIOCBDBK:
		st_altblk[unit(dev)].a_magic = 0;
		break;	
case UIOCEXTE:
		st_unit[unit(dev)].st_xprintf = 1;
		break;
case UIOCNEXTE:
		st_unit[unit(dev)].st_xprintf = 0;
		break;
	default:
		return(ENOTTY);
	}
	return(0);
}

/*	stformat -- format the SCSI disk.
 *	this routine understands the data structure made by the diskformat
 *		program
 *	Formatting is a two stage process.  First set the mode of the 
 *	drive.  Second tell the drive to format.
 *	There is a race condition.  If another process sets the extended print
 *	mode of the driver while the drive is formatting, the new mode will
 *	be lost.
 *	XXX the two stages are canidates for linked commands.
 */


stformat(dev, addr)
caddr_t addr;
dev_t dev;
{
	struct diskformat *df;
	register struct st_unit *up;
	register int punit;
	register caddr_t cp;
	struct buf *bp;
	int	savxprint;

/*
 *	only the superuser can format disks
 */

	punit = unit(dev);
	if (!suser())
		return(EPERM);
	up = &st_unit[punit];
	df = (struct diskformat *) addr;

/*
 * 	allocate the buffer
 */

	SPL2();
	while(up->st_fmtbuf != NULL) {
		up->st_needfmt = 1;
		(void)sleep((caddr_t)(&up->st_needfmt), PZERO+1);
	}
	SPL0();
	savxprint = up->st_xprintf;
	up->st_xprintf = 1;
	bp = geteblk(DEVBLKSIZE);
	up->st_fmtbuf = bp;
	bp->b_dev = makedev(ourmajor, minor(dev));
	cp = bp->b_un.b_addr;
	bzero(cp, MODESIZE);	/* set up mode command output buffer */
	cp[3] = 8;
#ifndef lint	/* possible pointer alignment problem */
	if(df->d_secsize != DISKDEFAULT && 
	    (df->d_secsize == DEVBLKSIZE || df->d_secsize == 532))
		*((long *)(&cp[4+4])) = df->d_secsize & 0xFFFF;
	else
		*((long *)(&cp[4+4])) = DEVBLKSIZE;
#endif lint
	if(df->d_ileave != DISKDEFAULT && df->d_ileave > 0)
		up->inter = df->d_ileave;
	else
		up->inter = 2;	

		
/*
 *	queue the format request and wait for its completion
 */

	ststrategy(bp);
	iowait(bp);

/*
 *	dispose of the "dirty" buffer
 */

	bp->b_flags |= B_ERROR;		/* make sure it is not reused */
	bp->b_dev = NODEV;
	up->st_fmtbuf = NULL;
	brelse(bp);
	up->st_xprintf = savxprint;	
	SPL2();
	if(up->st_needfmt) {
		up->st_needfmt = 0;
		wakeup((caddr_t)(up->st_needfmt));
	}
	SPL0();
	return(0);
}

/*
 * diskread - process read
 */

stread(dev, uio)
dev_t  dev;
struct uio *uio;
{
	return(physio(ststrategy, (struct buf *)NULL, dev, B_READ, uio));
}

/*
 * diskwrite - process write
 */
stwrite(dev, uio)
dev_t  dev;
struct uio *uio;
{
	return(physio(ststrategy, (struct buf *)NULL, dev, B_WRITE, uio));
}

/*
 *	error message print routine
 */

stprint(dev, str)
char *str;
{
	printf("%s on disk drive id %d unit %d, slice %d\n", 
		str, (dev>>4)&0x7, (dev>>3)&1, dev&7);
}

stdump()
{
}
