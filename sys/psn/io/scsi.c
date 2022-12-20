#ifndef lint	/* .../sys/psn/io/scsi.c */
#define _AC_NAME scsi_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer Inc., All Rights Reserved.  {Apple version 1.9 87/11/19 18:03:36}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.9 of scsi.c on 87/11/19 18:03:36";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

#ifdef HOWFAR
int	T_scsi = 0;
#endif HOWFAR
/*	@(#)scsi.c	1.16 - 8/24/87
 *
 * NCR 5380 SCSI interface.
 *
 * (C) 1987 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#ifndef	STANDALONE
#include "sys/param.h"
#include "sys/uconfig.h"
#include "sys/var.h"

#include <sys/mmu.h>
#include <sys/signal.h>
#include <sys/seg.h>
#include <sys/time.h>
#include <sys/user.h>
#include "setjmp.h"
#include <sys/page.h>
#include <sys/systm.h>
#endif	STANDALONE
#include "sys/debug.h"
#include "sys/stream.h"
#endif lint
#include "sys/ncr5380.h"
#include "sys/via6522.h"


/*	DELAY -- A generic delay.  Not tuned to SCSI delay values */

#ifdef	lint
#define DELAY
#else	lint
#define	DELAY { jnkvar1 = 1; jnkvar2 = jnkvar1; }
static	jnkvar1, jnkvar2;
#endif	lint
#define	FRESET_TICKS (v.v_hz/4)	/* timeout ticks to hold RST line */
#define	WRESET_TICKS (v.v_hz*8)	/* ticks to stay off after a reset */
#define	NSCSI 8			/* Number of SCSI devices */
#define OURID	7		/* Our SCSI ID */
#ifndef	NULL
#define NULL 0
#endif	NULL

/* DODIS -- determine if we should disconnect job.
 */
#ifdef	STANDALONE
#define	DODIS(x)	0
#else	STANDALONE
#define	DODIS(tskp) (lastmult > jobstep - 400 \
	&& !(tskp->devchar & (SDC_EXCL | SDC_NODISC)) \
	&& tskp->lastnotify > lastreset \
	&& !(tskp->flags & SF_EXCL))
#endif	STANDALONE

static	scsiin_poll(), scsiin_dma(), scsiout_poll(), scsiout_dma();
static	choosetask(), doabort(), dorun(), doreset(), doreset2(), donotify();
static	dopanic(), finishreset(), scsitask(), watchdog(); 


/*	SCSI global state.
 *	     This reflects the state of the current activity of the ncr
 *	chip.
 */

#define	SG_IDLE		0	/* no activity */
#define SG_CHOOSE	1	/* Choose the next job to run */
#define	SG_RUN		2	/* Running the current job */
#define	SG_WPHASE	3	/* Running job waiting for phase match */
#define	SG_WRECON	4	/* Idle & waiting for job(s) to reconnect */
#define	SG_WBUS		5	/* SCSI bus is busy */
#define SG_WRESET	6	/* Waiting for a reset to complete */
#define	SG_ABORTJ	7	/* Aborting this job */
#define	SG_RESET	8	/* Begin a reset, reset the bus */
#define	SG_NOTIFY	9	/* Call the user code */
#define SG_PANIC	10	/* Impossible state transition */


/*	Input types.
 *	     These types encompass both hardware events and software
 *	events.  The first group describes types of interrupts, later
 *	codes are used for events in processing.
 *	     We decode various chip registers into these types.
 *	Warning: the irqtype table is indexed by these codes.  Do not
 *	change numeric values without changing this table.
 */

#define	SI_FSEL		0	/* Both lost BSY and pending SEL */
#define	SI_SEL		1	/* A target is reselecting us */
#define	SI_RESET	2	/* The bus is being reset */
#define	SI_PHASE	3	/* Lost Phase match */
#define	SI_PARITY	4	/* A parity error occured */
#define	SI_FREE		5	/* The bus is currently free */
#define	SI_UNK		6	/* Unknown interrupt, last hadware event */
#define SI_NEWTASK	7	/* A new task has been scheduled */
#define SI_DONE		8	/* Finished a task */
#define SI_ABT		9	/* The current task has been aborted */
#define	SI_BSY		10	/* The bus is busy, we cannot use it */
#define SI_WAITP	11	/* The current task is waiting for phase */


#if defined(DEBUG) || defined(HOWFAR) || defined(STANDALONE)
char	*eventnames[] = {	/* for trace messages */
	"SI_FSEL",	/* 0 */
	"SI_SEL",	/* 1 */
	"SI_RESET",	/* 2 */
	"SI_PHASE",	/* 3 */
	"SI_PARITY",	/* 4 */
	"SI_FREE",	/* 5 */
	"SI_UNK",	/* 6 */
	"SI_NEWTASK",	/* 7 */
	"SI_DONE",	/* 8 */
	"SI_ABT",	/* 9 */
	"SI_BSY",	/* 10 */
	"SI_WAITP",	/* 11 */
	};
#endif	DEBUG

/*	statetable -- table of global state transitions.
 *	     There are a set of input tokens and a group of states defined
 *	for processing.  This table defines the expected transitions between
 *	states.
 *	     Note: SG_PANIC does not appear in this table.
 */

#define _____ SG_PANIC
char statetable[SI_WAITP+1][SG_NOTIFY+1] = {

	     /* SG_IDLE, SG_CHOOSE, SG_RUN, SG_WPHASE, SG_WRECON, 
		SG_WBUS, SG_WRESET, SG_ABORTJ, SG_RESET, SG_NOTIFY */

/* SI_FSEL */	_____, _____, _____, SG_ABORTJ, SG_CHOOSE,
		SG_CHOOSE, _____, _____, _____, _____,

/* SI_SEL */	_____, SG_RUN, SG_CHOOSE, SG_CHOOSE, SG_CHOOSE,
		SG_CHOOSE, _____, _____, _____, _____,

/* SI_RESET */	SG_RESET, SG_RESET, SG_RESET, SG_RESET, SG_RESET, 
		SG_RESET, _____, SG_RESET, _____, SG_RESET, 

/* SI_PHASE */	_____, _____, _____, SG_RUN, _____,
		_____, _____, _____, _____, _____,

/* SI_PARITY */	_____, _____, _____, _____, _____, 
		_____, _____, _____, _____, _____, 

/* SI_FREE */	_____, _____, _____, SG_ABORTJ, _____,
		SG_CHOOSE, _____, _____, _____, _____,

/* SI_UNK */	SG_IDLE, _____, _____, SG_ABORTJ, SG_WRECON,
		SG_WBUS, _____, _____, _____, _____,

/* SI_NEWTASK*/	SG_CHOOSE, SG_RUN, _____, _____, SG_CHOOSE,
		_____, _____, _____, _____, _____,

/* SI_DONE */	_____, SG_IDLE, SG_NOTIFY, _____, _____,
		_____, SG_CHOOSE, SG_NOTIFY, SG_WRESET, SG_WBUS,

/* SI_ABT */	_____, SG_RESET, SG_ABORTJ, SG_ABORTJ, SG_ABORTJ,
		SG_ABORTJ, _____, SG_RUN, _____, _____,

/* SI_BSY */	_____, SG_WRECON, SG_WBUS, _____, _____,
		_____, _____, _____, _____, _____,

/* SI_WAITP */	_____, _____, SG_WPHASE, _____, _____,
		_____, _____, _____, _____, _____,

		};

/*	state_atribs -- attributes of the states.
 *	    The basic hardware setup required for each of the states is
 *	defined in this structure.  Entering a new state becomes a
 *	matter of setting the hardware and calling a routine.  If no
 *	routine is defined the state scheduler returns, assuming the
 *	task will be restarted by an interrupt.
 */

static	struct	state_atribs {
	int	(*fcn)();	/* processing routine */
	u_char	modereg;	/* Values for the ncr chip mode register */
	u_char	modemsk;	/* Set only mode bits where mask == 1 */
	u_char	irq : 1;	/* Enable ncr chip interrupts */
	u_char	select : 1;	/* Set our id in select enable register */
	u_char	mtrbsy : 1;	/* Monitor BSY via software or hardware */
	char	*name;		/* State name for debugging */
	} state_atribs[SG_PANIC+1] = {
		{ NULL,		0, 6, 1, 0, 0, "SG_IDLE"},
		{ choosetask,	0, 6, 0, 1, 0, "SG_CHOOSE"},
		{ dorun,	0, 6, 0, 1, 0, "SG_RUN"},
		{ NULL,		2, 2, 1, 0, 1, "SG_WPHASE"},
		{ NULL,		0, 6, 1, 1, 0, "SG_WRECON"},
		{ NULL,		0, 2, 1, 1, 1, "SG_WBUS"},
		{ NULL, 	0, 6, 0, 0, 0, "SG_WRESET"},
		{ doabort,	0, 6, 0, 0, 0, "SG_ABORTJ"},
		{ doreset,	0, 6, 0, 0, 0, "SG_RESET"},
		{ donotify,	0, 0, 0, 1, 1, "SG_NOTIFY"},
		{ dopanic, 	0, 0, 0, 0, 0, "SG_PANIC"}
	};


/*	SCSI task flags
 *	    These flags modify the state.  They are one bit flags.
 */

#define	SF_JOB		1	/* There is a job for this ID */
#define	SF_SENSE	2	/* Performing a sense command */
#define	SF_ABTSET	4	/* Task has been set up for abort processing */
#define	SF_DISCON	8	/* The job is disconnected */
#define	SF_LINK		0x10	/* A linked job has completed */
#define	SF_LINKFLAG	0x20	/* A linked job with flag has completed */
#define	SF_READ		0x40	/* Data direction of command is read */
#define	SF_RUN		0x80	/* The task has begun to run */
#define	SF_EXCL		0x100	/* The job must have exclusive use */
#define	SF_NOINTR	0x200	/* job must not be interrupted, use spl7 */

/*	SCSI phase states.
 *	    In addition to the task states, there are lower level phase states
 *	these match what is occuring, or last occured on the SCSI bus.
 */

#define SP_SEL		0	/* Selected the device */
#define	SP_IDENT	1	/* Sent the identify message for disconnect */
#define	SP_CMD		2	/* Sent the command */
#define	SP_DATA		3	/* Did the data transfer */
#define SP_STAT		4	/* Stat phase */
#define SP_ABT		5	/* Aborting this transaction */

#if	defined(DEBUG) || defined(HOWFAR) || defined(STANDALONE)
char *pstatenames[] = {
	"SP_SEL",	/* 0 */
	"SP_IDENT",	/* 1 */
	"SP_CMD",	/* 2 */
	"SP_DATA",	/* 3 */
	"SP_STAT",	/* 4 */
	"SP_ABT"	/* 5 */
	};
#endif	DEBUG

/*	Phase transitions.
 *	    This defines legal state transitions between the basic phases.
 *	The message phases are not tracked in this table, nor is bus free.
 *	The table defines what to do on phase transitions from the target.  
 *	If the last processing state was x, and the new phase is y, the 
 *	table gives the new processing state.  This table drives activities
 *	in scsitask().
 */

static	char	spstates [SP_ABT+1][4] = {
		/* Data out */	/* Data in */	/* Cmd */	/* Stat */
/* SP_SEL */	SP_ABT,		SP_ABT,		SP_CMD,		SP_ABT,
/* SP_IDENT */	SP_ABT,		SP_ABT,		SP_CMD,		SP_ABT,
/* SP_CMD */	SP_DATA,	SP_DATA,	SP_ABT,		SP_STAT,
/* SP_DATA */	SP_DATA,	SP_DATA,	SP_ABT,		SP_STAT,
/* SP_STAT */	SP_ABT,		SP_ABT,		SP_CMD,		SP_ABT,
/* SP_ABT */	SP_ABT,		SP_ABT,		SP_ABT,		SP_ABT
		};

/*	If an error occurs in this state, and it is not tagged by special
 *	purpose code, assign the following error value to it.
 */
char	sp_errs[SP_ABT+1] = {
	SST_SEL,		/* SP_SEL */
	SST_CMD,		/* SP_IDENT */
	SST_CMD,		/* SP_CMD */
	SST_PROT,		/* SP_DATA */
	SST_COMP,		/* SP_STAT */
	SST_PROT		/* SP_ABT */
	};

#define	splintr()	spl2()	/* priority when we are interrupted */
#define SPLINTR()	SPL2()
#ifdef	STANDALONE
#define	SPL2	spl7
#define	spl2	spl7
#define	SPL0	spl7
#define	spl0	spl7
#define bzero(cp, n)	memset(cp, 0, n)
long	sdma_addr = SDMA_ADDR_R7;
#endif	STANDALONE

static  short needed = 0;		/* flag that a request is waiting */
static 	short sg_state = SG_IDLE;	/* SCSI global state variable */
static 	short sg_oldstate = SG_IDLE;	/* for debug, last state */
static	short curtask;			/* index of current task */
static	u_char	discjobs;		/* a bit is on for each disconnected */
static	int taskinp;			/* parameter for scsitask */
static	int	ourid = OURID;		/* the host computer SCSI ID */
static  scsifree;			/* True if a BSY has been off recently */
static	berrflag;			/* True if bus error on DMA/IO */
static	checkconcur = 0;		/* Mostly used for debugging */

static	struct	tasks {
	struct scsireq *req;
	long	devchar;	/* device characteristics */
	caddr_t	cmdbuf;		/* Current command */
	int	cmdlen;		/* Length of the command */
	caddr_t	databuf;	/* Area to read or write data */
	int	datalen;	/* size of data area */
	int	datasent;	/* Data read so far */
	long	lastnotify;	/* last time we returned ok */
	u_char	msgout;		/* Message to be output */
	u_char	stat;		/* Stat byte */
	u_char	msgin;		/* Message read from device */
	u_char	lun;		/* logical unit number */
	short	pstate;		/* phase state */
	short	flags;		/* task flags */
	u_char	maxtime;	/* limit on time for this task */
	u_char	curtime;	/* current time running on this task */
	} tasks[NSCSI];

static	struct scsig0cmd sense_cmd[NSCSI] = {
	{SOP_SENSE, 0, 0, 0, sizeof(struct scsig0cmd), 0},
	{SOP_SENSE, 0, 0, 0, sizeof(struct scsig0cmd), 0},
	{SOP_SENSE, 0, 0, 0, sizeof(struct scsig0cmd), 0},
	{SOP_SENSE, 0, 0, 0, sizeof(struct scsig0cmd), 0},
	{SOP_SENSE, 0, 0, 0, sizeof(struct scsig0cmd), 0},
	{SOP_SENSE, 0, 0, 0, sizeof(struct scsig0cmd), 0},
	{SOP_SENSE, 0, 0, 0, sizeof(struct scsig0cmd), 0},
	};
static	char jnkbuf[100];	/* A junk buffer */

/*	scsistrings -- error message strings.
 *	This global data structure is indexed by scsi return code.  It is
 *	available to any driver which may want to print an error message.
 */

char	*scsi_strings[] = {
	"SCSI driver implementation error",	/* code 0, there is no error */
	"SCSI bus dropped busy",		/* 1 */
	"Error during SCSI command",		/* 2 */
	"Error during SCSI status",		/* 3 */
	"Error during SCSI sense command",	/* 4 */
	"Cannot select SCSI device",		/* 5 */
	"SCSI timeout",				/* 6 */
	"Driver(s) for SCSI device placed multiple conncurent requests", /* 7 */
	"Protocol Error Processing SCSI request",	/* 8 */
	"More data than SCSI device requested",	/* 9 */
	"Less data than SCSI device requested", /* 10 */
	"SCSI extended status returned",	/* 11 */
	"SCSI Retry required",			/* 12 */
	};

#define	TOUTTIME	2
static	short tout;		/* flag that time out daemon running */
static	short unjam;		/* flag for timeouts of active transactions */
static	time_t jamtick;		/* timer for stuck devices (lbolt units) */
static	short stuckdisc = -1;	/* number of stuck disconnected task */
static	short isinit;		/* flag that driver has initialized */
static	short neverdisc;	/* if true, never disconnect jobs */
static	short nextexcl = -1;	/* id of next job to gain exclusive use */
static	short nextexpri;	/* priority of nextexcl */

/*	Macros for detecting stuck devices.
 *	    These check if an operation is taking too long.  The standalone
 * 	versions are based on best guess.
 */

#ifdef	STANDALONE
#define CLRJAM	(jamtick = 0)	/* init counter to start of interval */
#define UNJAM	(jamtick++ > 1000000)	/* True if timeout has occured */
#else	STANDALONE
#define CLRJAM	(jamtick = lbolt + TOUTTIME * v.v_hz)
#define UNJAM	(jamtick < lbolt)
#endif	STANDALONE

#define	METRICS
#ifdef	METRICS
/*	Some metrics.
 */

static	 long numdisc;		/* number of disconnects since reboot */
static	 long numindma;		/* number of times DMA in called */
static	 long numoutdma;	/* number of times DMA out called */
static	 long numreq;		/* number of requests */
#define	meter(n)	(++n)
#else	METRICS
#define	meter(n)	
#endif	METRICS


/*	Job steps are an internal counter that is simply incremented
 *	each time the driver "does something".  They are a method of 
 *	detecting the passage of time.
 */
static	long jobstep = 1000;	/* incremented each scheduler event */
static	long lastmult = 0;	/* set when multiple requests are present */
static	long laststuck;		/* used to process timeouts on disc jobs */
static	long lastreset;		/* last time a reset was detected */


#ifndef	STANDALONE
/*	The following streams data structures allow us to use the stream
 *	scheduler to run copy operations at cpu chip priority 0.  We do
 *	not use any other part of streams besides the queue scheduler.
 */

extern	nulldev();
static struct 	module_info scsimodinfo = { 93, "scsi", 0, 256, 256, 256, NULL };
static struct	qinit scsidata = { NULL, scsitask, NULL, NULL,
			nulldev, &scsimodinfo, NULL};
static struct	queue scsiqueue = { &scsidata };
#endif	STANDALONE


/*	choosetask -- select next id.
 *	     The routine is called either when software wants to do some
 *	I/O, or when a device is selecting us, presumably following a
 *	disconnect.
 */

static
choosetask(inp)

{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register struct tasks *stp;
	static	lastsched = 0;
	register i, n;
	register id;
	int	s;

	TRACE(T_scsi, ("choosetask: inp %s\n", eventnames[inp]));
	s = spl7();
again:
	switch(inp) {
	case SI_DONE:
	case SI_NEWTASK:
	case SI_FREE:
		if((ncr->curr_stat & 0xE6) == (SCS_SEL | 4) &&
			(ncr->curr_data & (1 << ourid))) { /* Check SEL interrupt */
			inp = SI_SEL;
			goto again;
		}
		if(stuckdisc >= 0) {
			if(!discjobs) {
				splx(s);
				scsisched(SI_ABT);
				return;
			}
			break;
		}
		n = 0;
		id = lastsched;
		stp = &tasks[id];
		for(i = SRQ_PRIMASK; i >= 0; i -= 1 << SRQ_PRISHIFT) {
			do {
				++stp;
				if(++id >= NSCSI) {
					id = 0;
					stp = tasks;
				}
				if((stp->flags
				   & (SF_JOB|SF_RUN)) == SF_JOB 
				   && (stp->req->flags
				   & SRQ_PRIMASK) == i) {
					n = 1;
				}
			} while(!n && id != lastsched);
			if(nextexcl >= 0 && id != nextexcl && i <= nextexpri) {
				if(discjobs)
					break;
				else
					id = nextexcl;
			}
			if(n) {
				if((stp->flags & SF_EXCL) 
				  || (stp->devchar & SDC_EXCL)) {
					if(discjobs) {
						nextexcl = id;
						nextexpri = i;
						break;
					}
				}
				nextexcl = -1;
				lastsched = id;
				curtask = id;
				stp->flags |= SF_RUN;
				TRACE(T_scsi, ("choosetask id %d\n", id)); 
				splx(s);
				scsisched(SI_NEWTASK);
				return;
			}
		}
		break;
	case SI_FSEL:
	case SI_SEL:
		n = ncr->curr_data;
		if(!(n & (1 << ourid))) {
			ncr->sel_ena = 0;
			ncr->sel_ena = (1 << ourid);
			splx(s);
			scsisched(SI_BSY);	/* Busy, but need to wait */
			return;
		}
		n &= ~(1 << ourid);
		ncr->sel_ena = 0;
		for(i = 1, id = 0; i < 0x100; i <<= 1, ++id) {
			if(n & i) {
				if((n & ~i) || !(discjobs & i)) {	
				/* two id's active or not disc */
					ncr->sel_ena = (1 << ourid);
					scsisched(SI_BSY);
					return;
				}
				TRACE(T_scsi, ("select id = 0x%x\n", id));
				break;
			}
		}
		if(ncr->curr_stat & SCS_SEL) {
			ncr->init_comm = SIC_BSY;
			i = 0;
			while(ncr->curr_stat & SCS_SEL) {
				if(++i > 20000)
					break;
			}
			ncr->init_comm = 0;
			DELAY;
			DELAY;
			if(ncr->curr_stat & SCS_BSY) {
				if(stuckdisc == id)
					stuckdisc = -1;
				curtask = id;
				stp = &tasks[id];
				stp->flags &= ~SF_DISCON;
				discjobs &= ~(1 << curtask);
				TRACE(T_scsi, ("recon %d\n", id));
				ncr->sel_ena = (1 << ourid);
				splx(s);
				scsisched(SI_SEL);
				return;
			}
			else {
				TRACE(T_scsi, ("Lost BSY somehow\n"));
			}
		}
		break;
	default:
		panic("unknown input in choosetask");
		break;
	}
	splx(s);
	if(discjobs)
		scsisched(SI_BSY);	/* Busy, but need to wait */
	else
		scsisched(SI_DONE);	/* signals idle state */
	return;
}


/*	doabort -- give up on job.
 *	     When an I/O fails, we must get the device off the bus.  A new
 *	job will be scheduled to do this, but first we switch over to a
 *	scratch buffer area, and mark the process as being aborted.
 */

static
doabort(inp)

{
	register struct tasks *stp;

	TRACE(T_scsi, ("do abort\n"));
	stp = &tasks[curtask];
	stp->flags |= SF_ABTSET;
	stp->databuf = jnkbuf;
	stp->datalen = sizeof(jnkbuf);
	stp->datasent = 0;
	stp->maxtime = 2;
	stp->cmdbuf = jnkbuf;
	if(!stp->req->ret)
		stp->req->ret = SST_PROT;
	if(stp->flags & SF_SENSE)
		stp->req->ret = SST_SENSE;

	/* There is a race here, busy could have dropped and been re-asserted 
	 * by another device.
	 */
	if(((struct ncr5380 *)SCSI_ADDR)->curr_stat & SCS_BSY) {
#ifdef	STANDALONE
		if(!(stp->flags & SF_RUN)) {
			stp->req->ret = SST_SEL;
			scsisched(SI_DONE);
			return;
		}
#endif	STANDALONE
		scsisched(SI_ABT);
	}
	else
		scsisched(SI_DONE);
}


/*	donotify -- notify user handler.
 *	When the scsi request was placed, the user included a call back
 *	address.  Now we call it back.
 */

static
donotify(inp)

{
	register struct tasks *stp;
	register struct scsireq *rqp;
	int i;

	ASSERT((spl2() & 0x700) >= 0x200);	/* Call at spl2() or more */
	TRACE(T_scsi, ("donotify: inp %s\n", eventnames[inp]));
	stp = &tasks[curtask];
	rqp = stp->req;
	if(stp->flags & SF_DISCON) {
		discjobs |= 1 << curtask;
	}
	else {
		TRACE(T_scsi, ("complete id %d\n", curtask));
		rqp->msg = stp->msgin;
		rqp->stat = stp->stat;
		if((stp->stat & STA_CHK) && !(stp->flags & SF_SENSE)) {
			TRACE(1, 
			("stat command required id %d\n", curtask));
			stp->flags &= ~SF_RUN;
			stp->flags |= SF_SENSE | SF_READ;
			stp->cmdbuf = (caddr_t)&sense_cmd[curtask];
			stp->cmdlen = sizeof(struct scsig0cmd);
			sense_cmd[curtask].addrH = stp->lun << 5;
			if(rqp->senselen && rqp->sensebuf) {
				stp->databuf = rqp->sensebuf;
				stp->datalen = rqp->senselen;
			}
			else {
				stp->databuf = jnkbuf;
				stp->datalen = sizeof(jnkbuf);
			}
			sense_cmd[curtask].len = stp->datalen;
			stp->datasent = 0;
			rqp->ret = SST_STAT;
		}
		else {
			stp->flags &= ~(SF_JOB | SF_RUN | SF_DISCON);
			if(rqp->ret == 0)
				stp->lastnotify = jobstep;
			if(rqp->faddr)
				(*rqp->faddr)(rqp);
		}
	}
	scsisched(SI_DONE);
	return;
}


/*	dopanic -- Aieeeeee.
 */

static
dopanic(inp)

{
	TRACE(1, ("dopanic input = %s %s -> %s\n", 
		eventnames[inp], 
		state_atribs[sg_oldstate].name, state_atribs[sg_state].name));
	panic("SCSI manager software error in state table");
	/* Not normally executed.  Might work, though */
	sg_state = SG_IDLE;
	scsisched(SI_RESET);
}


/*	doreset -- reset the SCSI bus.
 *	     We respond to some timeout error conditions by resetting
 *	the SCSI bus.  This begins the process.  doreset2() turns off
 *	the reset pulse a fraction of a second later, and finishreset
 *	restores things to normalacy a few seconds later.
 *	     The reset pulse will cause the next operation on an ST225
 *	drive, to generate a sense condition.  The driver should not
 *	consider this an error.
 */

static
doreset(inp)

{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	int	i;

	ASSERT(sg_state == SG_RESET);
	printf("SCSI reset\n");
	ncr->out_data = 0;
	ncr->mode = 0;
	ncr->targ_comm = 0;
	if(!(ncr->curr_stat & SCS_RST)) {
		ncr->init_comm = SIC_RST;
	}
	if(ncr->init_comm & SIC_RST) {
#ifndef	STANDALONE
		timeout(doreset2, 0, FRESET_TICKS);
#else	STANDALONE
		for(i = 0; i < 200000; ++i)
			;
		ncr->init_comm = 0;
#endif	STANDALONE
	}
	lastreset = jobstep;
#ifndef	STANDALONE
	timeout(finishreset, 0, WRESET_TICKS);
#endif	STANDALONE
	scsisched(SI_DONE);
}


/*	doreset2 -- turn off reset pulse.
 *	     Started by a timeout, this routine cancels the reset pulse.
 */

static
doreset2()

{
	((struct ncr5380 *)SCSI_ADDR)->init_comm = 0;
}


/*	dorun -- enable SCSI service queue.
 *	     Most activity using the SCSI chip is done at low processor
 *	priority and is scheduled via the streams mechanism.  This routine
 *	enables our stream service routine.
 */

static
dorun(inp)

{
#ifndef	STANDALONE
	TRACE(T_scsi, ("dorun: inp = %s\n", eventnames[inp]));
	taskinp = inp;
	qenable(&scsiqueue);
#else	STANDALONE
	static	level, needrun;

	if(level++ == 0) {
		needrun = 1;
		taskinp = inp;
		while(needrun) {
			needrun = 0;
			scsitask();
		}
	}
	else  {
		taskinp = inp;
		needrun = 1;
	}
	--level;
#endif	STANDALONE
}	


/*	finishreset -- clean up following reset.
 *	    After pulsing the RST line, we give devices 2 seconds, and then
 *	we complete the offending request and go back to business as usual.
 */
 
static
finishreset()

{
	register struct tasks *stp;
	register struct scsireq *rqp;
	register i;
	
	SPLINTR();
	TRACE(T_scsi, ("finish reset called\n"));
	if(sg_state != SG_WRESET) {
		panic("software error in SCSI finish reset");
	}
	discjobs = 0;
	for(i = 0, stp = tasks; i < NSCSI; ++i, ++stp) {
		if((stp->flags & (SF_JOB | SF_RUN)) == (SF_JOB | SF_RUN)) {
			rqp = stp->req;
			if(i == stuckdisc || (stuckdisc < 0 && i == curtask)) {
				rqp->ret = SST_TIMEOUT;
			}
			else {
				rqp->ret = SST_AGAIN;
			}
			rqp->datasent = 0;
			stp->flags &= ~(SF_JOB | SF_RUN | SF_DISCON);
			if(rqp->faddr)
				(*rqp->faddr)(rqp);
			break;
		}
	}
	if(stuckdisc >= 0) {
		tasks[stuckdisc].devchar |= SDC_NODISC;
		stuckdisc = -1;
	}
	i = ((struct ncr5380 *)SCSI_ADDR)->respar_int;
	VIA_CLRSIRQ();
	scsisched(SI_DONE);
}


/*	scsidevchar -- set scsi device characteristics.
 *	     Set the bit mapped word of device characteristics for this
 *	device id.
 */

scsidevchar(id, stuff)

long	stuff;
{
	if(id < 0 || id >= NSCSI)
		return(-1);
	tasks[id].devchar = stuff;
	if(neverdisc)
		tasks[id].devchar |= SDC_NODISC;
	return(0);
}


/*	scsig0cmd -- make group 0 command block.
 *	Fill in the blanks of a six byte command.  A buffer with space
 *	for the command block is initialized and linked into the general
 *	scsi request format being prepared.
 */

scsig0cmd(req, op, lun, addr, len, ctl)

struct scsireq *req;	/* request being assembled */
int	op;		/* the SCSI op code */
int	lun;		/* The logical unit number for the comand (1..8) */
register addr;		/* The address field of the SCSI command block */
int	len;		/* The length field of the command */
int	ctl;		/* contents of byte 5 */
{
	struct scsig0cmd *cmd;

#ifdef lint	/* possible pointer alignment problem */
	cmd = (struct scsig0cmd *)NULL;
#else lint
	cmd = (struct scsig0cmd *)req->cmdbuf;
#endif lint
	cmd->op = op;
	cmd->addrH = ((lun & 0x7) << 5) | (((int)addr >> 16) & 0x1F);
	cmd->addrM = (int)addr >> 8;
	cmd->addrL = (int)addr;
	cmd->len = len;
	cmd->ctl = ctl;
	req->cmdbuf = (caddr_t) cmd;
	req->cmdlen = 6;
}


/*	scsiget -- perform SCSI bus arbitration.
 *	Must be called at spl7().
 */

int
scsiget()

{
	register int i;
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	int	tsthigh;

	ASSERT(!(ncr->mode & (SMD_BSY | SMD_DMA)));
	if((ncr->curr_stat & 0xE6) == (SCS_SEL | 4) &&
	    (ncr->curr_data & (1 << ourid))) { /* Check SEL interrupt */
		return(SI_SEL);
	}
	if(ncr->bus_status & SBS_IRQ)
		return(SI_BSY);
	i = 0;
	tsthigh = 1 << (ourid + 1);
	while(!(ncr->curr_stat & (SCS_BSY | SCS_SEL))) {
		ncr->curr_data = 1 << ourid;
		ncr->mode = SMD_ARB;
		while(!(ncr->init_comm & SIC_AIP)) {
			if((ncr->curr_stat & SCS_SEL)
					|| ++i >= 200000) {	/* TUNE */
				ncr->mode = 0;
				return(SI_BSY);
			}
			DELAY;
		}
		DELAY;		/* TUNED 2.2 microseconds */

		/* Assert:  The compiler will not optimize common expressions
		 */
		if((ncr->init_comm & SIC_LA) || ncr->curr_data >= tsthigh 
						|| (ncr->init_comm & SIC_LA)) {
			ncr->mode = 0;
		}
		else {
			ncr->init_comm = SIC_SEL | SIC_BSY;
			ncr->mode = 0;
		/* Delay at least 1.2 usec */
		/* Assert: It will take at least 1.2 usec to rts, do some 
		 * branches, jsr to scsiselect, and do more processing
		 */
			return(SI_DONE);
		}
	}
	if((ncr->curr_stat & 0xE6) == (SCS_SEL | 4) &&
	    (ncr->curr_data & (1 << ourid))) { /* Check SEL interrupt */
		return(SI_SEL);
	}
	else {
		return(SI_BSY);
	}
}
/*	scsiin_dma -- input bytes from the scsi bus.
 *	    After insuring that a phase match is present, this routine
 *	reads bytes from the SCSI device.  It relies on the "four byte
 *	at a time" psuedo DMA interface.
 */

static int
scsiin_dma(buf, len, extend)

register long *buf;
int	len;
int	extend;		/* use 532 byte sectoring if true */
{
#ifndef	STANDALONE
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register bstat;
	register len2;
	register int current;
	jmp_buf jb;
	int	*saved_jb;

	TRACE(T_scsi, ("scsiin_dma phase 0x%x\n", (ncr->curr_stat >> 2) & 7));
	/* Assert: it is ok to access nofault outside of process context */
	meter(numindma);
	saved_jb = u.u_nofault;
	if (setjmp(jb)) {
		/* FYI: setjmp invalidates all register variables */
		u.u_nofault = saved_jb;
		berrflag = 1;
		TRACE(1, ("scsiin_dma: benign handshake port bus error\n"));
		return(0);
	}
	u.u_nofault = jb;
	for(current = 0; current < len; current += sizeof(long)) {
		if(!(current & (256-1))) {/* pause each 256 bytes */
		    CLRJAM;
		    if(extend && !(current & (512-1)) && current > 0) {
			for(len2 = 20; len2 > 0; --len2) {
			    while((bstat = 	/* wait device ready */
			        (ncr->bus_status & (SBS_PHASE | SBS_DMA))) != 
			            (SBS_PHASE | SBS_DMA)) {
				if(!(bstat & SBS_PHASE) || UNJAM)
				    goto errout;
			    }
			    bstat = *((char *)SDMA_ADDR);	/* junk */
			}
		    }
		    while((bstat = 	/* wait device ready */
			(ncr->bus_status & (SBS_PHASE | SBS_DMA))) != 
			    (SBS_PHASE | SBS_DMA)) {
			if(!(bstat & SBS_PHASE) || UNJAM) {
    errout:
			    TRACE(T_scsi,
		    ("scsiin_dma: phase changed to 0x%x with %d bytes left\n",
				(ncr->curr_stat>>2) & 0x7, len - current));
			    u.u_nofault = saved_jb;
			    return(current);	/* bytes transferred */
			}
		    }
		}
		*buf++ = *((long *)SHSK_ADDR);
	}
	u.u_nofault = saved_jb;
	TRACE(T_scsi, ("scsiin_dma complete\n"));
	return(current);
#endif	STANDALONE
}


/*	scsiin_poll -- input bytes from the scsi bus.
 *	The "expected" bus phase is set to the given phase, and bytes
 *	are read.  The routine returns the number of bytes actually
 *	transferred.  This routine checks the availability of bytes by
 *	reading the ncr chip.  It uses the byte at a time psuedo DMA
 *	interface.
 */

static int
scsiin_poll(buf, len, extend)

register caddr_t buf;
register int len;
int	extend;		/* use 532 byte sectoring if true */
{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register bstat;
	register len2;
	register phase;
	int	start;

	TRACE(T_scsi, ("scsiin_poll phase 0x%x\n", (ncr->curr_stat >> 2) & 7));
	start = len;
	while(len > 0) {
		while((bstat = (ncr->bus_status & (SBS_PHASE | SBS_DMA))) != 
						(SBS_PHASE | SBS_DMA)) {
			if(!(bstat & SBS_PHASE)) {
errout:
				TRACE(T_scsi,
		("scsiin_poll: phase changed to 0x%x with %d bytes left\n",
					(ncr->curr_stat>>2) & 0x7, len));
				return(start - len);	/* bytes transferred */
			}
			if(UNJAM) {
				goto errout;
			}
		}
		*buf++ = *((char *)SDMA_ADDR);
		if(!(--len & (512-1))) {
		    CLRJAM;
		    if(extend && !(start & (512-1))) {
			for(len2 = 20; len2 > 0; --len2) {
			    while((bstat = 
			    (ncr->bus_status & (SBS_PHASE | SBS_DMA))) != 
				    (SBS_PHASE | SBS_DMA)) {
				if(!(bstat & SBS_PHASE) || UNJAM)
				    goto errout;
			    }
			    bstat = *((char *)SDMA_ADDR);	/* junk */
			}
		    }
		}
	}
	TRACE(T_scsi, ("scsiin_poll complete\n"));
	return(start);
}


/*	scsiinit -- do initialization.
 *	     Called at boot up.
 */

scsiinit()

{
	register struct ncr5380 *ncr;
	int	jnk;


#ifdef	STANDALONE
	VIA_SIRQ(NO);
	{
		register struct via *vp;

		vp = (struct via *)VIA1_ADDR;
		if(!(vp->rega & VRA_REV8)) {	/* Rev 8 board */
			sdma_addr = SDMA_ADDR_R8;
		}
	}
#endif	STANDALONE

	ncr = (struct ncr5380 *)SCSI_ADDR;
	ncr->mode = 0;
	ncr->init_comm = 0;
	ncr->sel_ena = 0;
#ifdef STANDALONE
	if(ncr->curr_stat & SCS_BSY) {
		int	i;
		
		S_WR(ncr->init_comm) = SIC_RST;
		for(i = 0; i < 7500; ++i)	/* TUNE 5 or 10 millisec */
				;
		S_WR(ncr->init_comm) = 0;
		for(i = 0; i < 6000000; ++i)	/* TUNE to over 5 seconds */
				;
	}
#endif	STANDALONE		
	jnk = ncr->respar_int;
	ncr->sel_ena = 1 << ourid;
	VIA_CLRSIRQ();
	isinit = 1;
	TRACE(T_scsi, ("scsiinit -- complete\n"));
}


/*	irqtype -- table of interrupt types.
 *	     Open up those hardware manuals and follow along.  We translate
 *	bit patterns from two registers into a small integer code.
 */

static struct	irqtype {
	short	value;		/* value required in bus_stat + curr_stat */
	short	mask;		/* but first mask with this value */
	} irqtype[]  = {
	0x1402,	0xf5e2,		/* SI_FSEL */
	0x1002, 0xf5e2,		/* SI_SEL */
	0x1080,	0x1080,		/* SI_RESET */
	0x1040,	0xfdc2,		/* SI_PHASE */
	0x1060,	0xb462,		/* SI_PARITY */
	0x1400,	0xf700,		/* SI_FREE */
	};			/* SI_UNK */

/*	scsiirq -- interrupt request from chip.
 *	     We determine the cause of the interupt, and pass control to the
 *	scheduler.
 */

scsiirq() 

{ 
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register short statbytes;
	register type;
	register struct irqtype *irp;
	
	TRACE(T_scsi, ("scsiirq\n"));
again:
	statbytes = (ncr->bus_status << 8) |  ncr->curr_stat;
	if(!(ncr->bus_status & SBS_IRQ)) {
		if(scsifree) {
			scsifree = 0;
			scsisched(SI_FREE);
			return;
		}
		else if(sg_state == SG_WPHASE && !(ncr->bus_status & SBS_PHASE)
		     && (ncr->curr_stat & SCS_BSY)) {
			scsisched(SI_PHASE);
			return;
		}
		else {
			/* Silently ignore bogus interrupts */
			VIA_CLRSIRQ();
			return;
		}
	}
	for(type = 0, irp = irqtype; type < SI_UNK; ++type, ++irp) {
		if((statbytes & irp->mask) == irp->value)
			break;
	}
	if(type == SI_UNK || type == SI_PARITY) {
		type = SI_UNK;
		if(((ncr->bus_status << 8) | ncr->curr_stat) != statbytes)
			goto again;
		TRACE(1, ("unknown interrupt info 0x%x\n", statbytes));
		printscsi();
	}
	switch(type) {
	case SI_FSEL:
		ncr->mode &= ~SMD_BSY;
		ncr->sel_ena = 0;
		break;
	case SI_SEL:
		ncr->sel_ena = 0;
		if(scsifree) {
			scsifree = 0;
			type = SI_FSEL;
		}
		break;
	case SI_FREE:
		ncr->mode &= ~SMD_BSY;
		break;
	case SI_PHASE:
		ncr->mode &= ~SMD_DMA;
		break;
	}
	statbytes = ncr->respar_int;		/* clear interrupt */
	VIA_SIRQ(NO);
	VIA_CLRSIRQ();
	if((ncr->bus_status & SBS_IRQ)) {
		statbytes = (ncr->bus_status << 8) |  ncr->curr_stat;
		if((statbytes & irqtype[SI_SEL].mask) == irqtype[SI_SEL].value) {
			ncr->sel_ena = 0;
			statbytes = ncr->respar_int;
			VIA_CLRSIRQ();
			scsisched(SI_SEL);
			return;
		}
	}
	scsisched(type);
	return;
}


/*	scsiout_dma -- output bytes on the scsi bus.
 *	The "expected" bus phase is set to the given phase, and bytes 
 *	are written.  The routine returns the number of bytes actually
 *	transferred.
 *	    Output is accomplised via the handshake dma port.  If an error
 *	occurs, the operation should be retried.
 */

static int
scsiout_dma(buf, len, extend)

register caddr_t buf;
register len;
int	extend;		/* true if 532 byte disk sectors */
{
#ifndef	STANDALONE
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register bstat;
	register len2;
	int start;
	jmp_buf jb;
	int	*saved_jb;

	TRACE(T_scsi, ("scsiout_dma phase 0x%x\n", (ncr->curr_stat >> 2) & 7));
	/* Assert: it is ok to access nofault outside of process context */
	meter(numoutdma);
	saved_jb = u.u_nofault;
	if (setjmp(jb)) {
		u.u_nofault = saved_jb;
		berrflag = 1;
		TRACE(1, ("scsiout_dma: benign handshake port bus error count\n"));
		return(0);
	}
	u.u_nofault = jb;
	for(start = len; len > 0; len -= sizeof(long)) {
		if(!(len & (256-1))) {	/* look for phase match each 256 bytes */
		    CLRJAM;
		    /* if on a 512 boundary, and an extend format device
		     * and a multiple of 512 request, output the extend bytes
		     */
		    if(extend && !(start & (512-1)) && start-len > 0) {
			for(len2 = 20; len2 > 0; --len2) {
			    while((bstat = 
			    (ncr->bus_status & (SBS_PHASE | SBS_DMA))) !=
				    (SBS_PHASE | SBS_DMA)) {
				if(!(bstat & SBS_PHASE) || UNJAM)
				    goto errout;
			    }
			    *((char *) SDMA_ADDR) = 0;
			}
		    }
		    while((bstat = (ncr->bus_status & 
			    (SBS_PHASE | SBS_DMA))) != (SBS_PHASE | SBS_DMA)) {
			if(UNJAM || !(bstat & SBS_PHASE)) {
errout:
				TRACE(T_scsi,
		("scsiout_dma: phase changed to 0x%x with %d bytes left\n",
				    (ncr->curr_stat>>2) & 0x7, len));
				u.u_nofault = saved_jb;
				return(start - len);
			}
		    }
		}
		*((long *)SHSK_ADDR) = *((long *)buf)++;
	}
	while((ncr->bus_status & (SBS_PHASE | SBS_DMA)) == SBS_PHASE) {
		if(UNJAM) {
			break;
		}
	}
	*((char *) SDMA_ADDR) = 0;	/* Write one extra to generate phase err */
	u.u_nofault = saved_jb;
	TRACE(T_scsi, ("scsiout_dma complete\n"));
	return(start);
#endif	STANDALONE
}


/*	scsiout_poll -- output bytes on the scsi bus.
 *	The "expected" bus phase is set to the given phase, and bytes 
 *	are written.  The routine returns the number of bytes actually
 *	transferred.
 */

static int
scsiout_poll(buf, len, extend)

register caddr_t buf;
register len;
int	extend;		/* true if 532 byte disk sectors */
{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register bstat;
	register len2;
	int start;

	TRACE(T_scsi, ("scsiout_poll phase 0x%x\n", (ncr->curr_stat >> 2) & 7));
	start = len;
	while(len > 0) {
		while((bstat = (ncr->bus_status & (SBS_PHASE | SBS_DMA))) !=
						(SBS_PHASE | SBS_DMA)) {
			if(!(bstat & SBS_PHASE)) {
errout:
				TRACE(T_scsi,
		("scsiout_poll: phase changed to 0x%x with %d bytes left\n",
					(ncr->curr_stat>>2) & 0x7, len));
				ncr->init_comm &= ~SIC_DB;
				return(start - len);
			}
			if(UNJAM) {
				goto errout;
			}
		}
		*((char *) SDMA_ADDR) = *buf++;
		/* if on a 512 boundary, and an extend format device
		 * and a multiple of 512 request, output the extend bytes
		 */
		if(!(--len & (512-1))) {
		    CLRJAM;
		    if(extend && !(start & (512-1))) {
			for(len2 = 20; len2 > 0; --len2) {
			    while((bstat = 
			    (ncr->bus_status & (SBS_PHASE | SBS_DMA))) !=
				    (SBS_PHASE | SBS_DMA)) {
				if(!(bstat & SBS_PHASE) || UNJAM)
				    goto errout;
			    }
			    *((char *) SDMA_ADDR) = 0;
			}
		    }
		}
	}
	while((ncr->bus_status & (SBS_PHASE | SBS_DMA)) == SBS_PHASE) {
		if(UNJAM) {
			break;
		}
	}
	*((char *) SDMA_ADDR) = 0;	/* Write one extra to generate phase err */
	TRACE(T_scsi, ("scsiout_poll complete\n"));
	return(start);
}


/*	scsiout_slow -- output bytes on the scsi bus.
 *	     This output routine uses the slowest possible SCSI output.  It is
 *	required by certain 10 byte commands on ST225N drives.  Only certain
 *	drives have the problem, but we do it for all.  Use of this routine
 *	was a last minute addition to compensate for deficient hardware.
 */

static int
scsiout_slow(buf, len)

register caddr_t buf;
register len;
{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register bstat;
	register len2;
	int start;

	TRACE(T_scsi, ("scsiout_slow phase 0x%x\n", (ncr->curr_stat >> 2) & 7));
	start = len;
	ASSERT(!(ncr->mode & SMD_DMA));
	while(len > 0) {
		while((bstat = (ncr->curr_stat & (SCS_BSY | SCS_REQ))) !=
						(SCS_BSY | SCS_REQ)) {
			if(!(bstat & SCS_BSY) || !(ncr->bus_status & SBS_PHASE)) {
errout:
				TRACE(T_scsi,
		("scsiout_slow: phase changed to 0x%x with %d bytes left\n",
					(ncr->curr_stat>>2) & 0x7, len));
				ncr->init_comm &= ~SIC_DB;
				return(start - len);
			}
			if(UNJAM) {
				goto errout;
			}
		}
		ncr->init_comm = SIC_DB;
		ncr->out_data = *buf++;
		ncr->init_comm |= SIC_ACK;
		while(((bstat = ncr->curr_stat) & SCS_REQ)) {
			if(UNJAM)
				goto errout;
		}
		ncr->init_comm = 0;
		--len;
	}
	TRACE(T_scsi, ("scsiout_slow complete\n"));
	return(start);
}


/*	scsirecon -- inhibit reconnection.
 *	    If called, devices will be inhibited from reconnection until
 *	the next reboot.  It is better to spend more money on hardware
 *	than make this request.
 */

scsirecon()
{
	int	id;

	neverdisc = 1;
	for(id = 0; id < NSCSI; ++id)
		tasks[id].devchar |= SDC_NODISC;
}


/*	scsirequest -- place a request.
 *	     This is the interface between drivers and the ncr chip.  If 
 *	the chip is not busy, we act on this request.  If the chip is busy, 
 *	we place the request where it will be executed later.  If there is
 *	already a request for this ID, we wait for the current 
 *	request to complete.
 */

scsirequest(id, req)

register struct scsireq *req;
{
	register s;
	register struct tasks *stp;

	TRACE(T_scsi, ("scsirequest id = %d\n", id));
	meter(numreq);
#ifdef	STANDALONE
	if(!isinit)
		scsiinit();
#endif	STANDALONE
	if(!tout) {
#ifndef	STANDALONE
		timeout(watchdog, 0, TOUTTIME * v.v_hz);
#endif	STANDALONE
		tout = 1;
	}
	ASSERT(isinit);
	req->sensesent = req->datasent = req->stat = req->msg = req->ret = 0;
	if(id >= NSCSI) {
		return(req->ret = SST_SEL);
	}
	stp = &tasks[id];
	s = splintr();
	if(stp->flags & SF_JOB)
		return(req->ret = SST_MULT);
	stp->cmdbuf = req->cmdbuf;
	stp->cmdlen = req->cmdlen;
	stp->databuf = req->databuf;
	stp->datasent = 0;
	stp->datalen = req->datalen;
	stp->flags = SF_JOB;
	stp->maxtime = req->timeout;
	stp->lun = ((struct scsig0cmd *)(req->cmdbuf))->addrH >> 5 & 7;
	if((req->flags & SRQ_READ))
		stp->flags |= SF_READ;
	if((req->flags & SRQ_EXCL))
		stp->flags |= SF_EXCL;
	if((req->flags & SRQ_NOINTR))
		stp->flags |= SF_NOINTR;
	stp->req = req;
	if(sg_state == SG_IDLE || sg_state == SG_WRECON)
		scsisched(SI_NEWTASK);
	else if(!(sg_state == SG_NOTIFY || sg_state == SG_WRESET))
		lastmult = jobstep;
	splx(s);
	return(req->ret);
}


/*	scsisched -- schedule the next SCSI state.
 *	     This routine turns the crank on the global state machine.
 *	Interrupts and software occurances generate input events to
 *	this routine.  The processing of the event is controlled by
 *	the global state.
 *	     There are some assumptions in the state machine about when
 *	events will be delivered.  For example, scsirequest() will only
 *	post a SI_NEWTASK event if the scsi bus is idle or waiting on
 *	disconnected jobs.  These assumptions eased implementation at the
 *	expense of a general purpose state machine.
 */

scsisched(inp)

int	inp;
{
	struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register struct state_atribs *atp;
	register newstate;
	int	i;
	int	s;

	s = spl7();
	ASSERT((s & 0x700) >= 0x200);
	++jobstep;
	sg_oldstate = sg_state;
	sg_state = statetable[inp][sg_state];
	atp = &state_atribs[sg_state];
	TRACE(T_scsi, ("scsisched input = %s %s -> %s\n", 
		eventnames[inp], state_atribs[sg_oldstate].name, atp->name ));
	TRACE(T_scsi, ("bus_status 0x%x curr_status 0x%x entry mode = 0x%x\n",
		ncr->bus_status, ncr->curr_stat, ncr->mode));
	ncr->mode = (ncr->mode & ~atp->modemsk) | atp->modereg;
	VIA_CLRSIRQ();		/* We clear it, knowing we'll check 
				 * the chip and not lose it
				 */
#ifdef	STANDALONE
	VIA_SIRQ(0);
#else	STANDALONE
	VIA_SIRQ(atp->irq);
#endif	STANDALONE
	if(atp->mtrbsy) {
		if((ncr->curr_stat & SCS_BSY) && !scsifree) {
			ncr->mode |= SMD_BSY;
		}
		else if(!scsifree) {
			scsifree = 1;
			ncr->mode &= ~SMD_BSY;
			if(ncr->bus_status & SBS_BSY)
				i = ncr->respar_int;
		}
	}
	else {
		scsifree = 0;
		if(ncr->bus_status & SBS_BSY)
			i = ncr->respar_int;
	}
	if(atp->select != state_atribs[sg_oldstate].select) {
		if(atp->select)
			ncr->sel_ena = 1 << ourid;
		else
			ncr->sel_ena = 0;
	}
	DELAY;
	DELAY;
	if(atp->fcn) {
		TRACE(T_scsi, ("call func at 0x%x\n", atp->fcn));
		splx(s);
		(*atp->fcn)(inp);
	}
	else {
		if(((ncr->bus_status & SBS_IRQ) || scsifree) && atp->irq) {
			TRACE(T_scsi, ("scsisched: Early irq\n"));
			SPLINTR();
			scsiirq();
		}
		else if(sg_state == SG_WPHASE && !(ncr->bus_status & SBS_PHASE)
							&& (ncr->curr_stat & SCS_BSY)) {
			SPLINTR();
			scsiirq();
		}
		else {
#ifndef	STANDALONE
			TRACE(T_scsi, 
	("Nothing to call:bus_status 0x%x curr_status 0x%x entry mode = 0x%x\n",
			ncr->bus_status, ncr->curr_stat, ncr->mode));
#else	STANDALONE
			int	lim;

			SPL0();
			switch(sg_state) {
			case SG_WPHASE:
			case SG_WRECON:
			case SG_WBUS:
				TRACE(T_scsi, ("poll for interrupt\n"));
				if(sg_state == SG_WBUS)
					lim = 10000;	/* TUNE a few mill */
				else
					lim = 5000000;	/* TUNE a few sec */
				for(i = 0; i < lim; ++i) {
					if(ncr->bus_status & SBS_IRQ) {
						break;
					}
				}
				SPLINTR();
				if(!(ncr->bus_status & SBS_IRQ)) {
					if(sg_state == SG_WBUS
					&& (tasks[curtask].flags & SF_JOB))
						scsisched(SI_ABT);
					else if(sg_state == SG_WBUS)
						scsisched(SI_FREE);
					else
						scsisched(SI_RESET);
					splx(s);
					return;
				}
				scsiirq();
				break;
			case SG_WRESET:
				for(i = 0; i < 5000000; ++i) {
					;
				}
				finishreset();
				break;
			case SG_IDLE:
				break;
			default:
				sa_printf(
				"scsisched: waiting for unknown interrupt state %d\n",
				sg_state);
			}
#endif	STANDALONE
		}
	}
	splx(s);
}


/*	scsiselect -- select SCSI device.
 *	    Selects the given device prior to an I/O operation.  
 *	Before a device is selected the bus should be arbitrarted via 
 *	scsiget().
 *	Must be called at spl7();
 *	On entry SEL and BSY are both asserted.
 */

int
scsiselect(id, use_atn)

int	id;	/* device number (0 - 7) */
int	use_atn;	/* if true, ATN is held high */
{
	register int i;
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;

	ASSERT((spl7() & 0x700) == 0x700);
	ncr->targ_comm = 0;
	ncr->out_data = (1 << id) | (1 << ourid);
	ncr->sel_ena = 0;
	if(use_atn)
		ncr->init_comm = SIC_SEL | SIC_DB | SIC_ATN;
	else
		ncr->init_comm = SIC_SEL | SIC_DB;
	DELAY;
	DELAY;
	for(i = 0; !(ncr->curr_stat & SCS_BSY); ++i) {
		if(i > 300000) {	/* TUNE HERE < 250 millisec */
			TRACE(1, ("scsiselect: can't select id %d\n", id));
			ncr->init_comm &= ~SIC_DB;
			for(i = 0; i < 300; ++i) {	/* TUNE HERE 200 usec */
				DELAY;
			}
			if(ncr->curr_stat & SCS_BSY)
				break;
			ncr->init_comm = 0;
			ncr->sel_ena = 1 << ourid;
			return(SI_BSY);
		}
	}
	DELAY;
	DELAY;
	if(use_atn)
		ncr->init_comm = SIC_ATN;
	else
		ncr->init_comm = 0;
	TRACE(T_scsi, ("scsiselect: selected\n", id));
	ncr->sel_ena = 1 << ourid;
	return(SI_DONE);
}


/*	scsitask -- perform a task.
 *	     This routine moves a task through the various bus phases
 *	until the task is complete.  This routine is only called via
 *	the streams service queue.  It is never called directly.
 *	     On entry, the current task has already been determined.  
 *	An action code has been placed in taskinp.
 */

static
scsitask()

{
	register struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register struct tasks *stp;
	register i, newphase;
	int	laststate;
	int	atn = 0;

	ASSERT(checkconcur++ == 0);
#ifndef	STANDALONE
	ASSERT((spl0() & 0x700) == 0);
#endif	STANDALONE
	stp = &tasks[curtask];
again:
	TRACE(T_scsi, ("scsitask: taskinp = %s id = %d bus status 0x%x\n",
		eventnames[taskinp], curtask, ncr->bus_status));
	switch(taskinp) {
	case SI_NEWTASK:
		ASSERT((stp->flags & (SF_RUN | SF_JOB | SF_DISCON)) == 
			(SF_RUN | SF_JOB));
		SPL7();
		if((i = scsiget()) == SI_DONE) {
			if(scsiselect(curtask, DODIS(stp)) == SI_DONE) {
				laststate = stp->pstate = SP_SEL;
			}
			else {
				stp->req->ret = SST_SEL;
				stp->flags &= ~SF_RUN;
				SPLINTR();
				scsisched(SI_ABT);
				ASSERT(--checkconcur == 0);
				return;
			}
		}
		else {
			stp->flags &= ~SF_RUN;
			SPLINTR();
			scsisched(i);
			ASSERT(--checkconcur == 0);
			return;
		}
		SPL0();
		taskinp = SI_PHASE;
		goto again;
		break;
	case SI_PHASE:
	case SI_SEL:
		/* raise priority if it is a no-interrupt job */
		if ( (stp->flags & SF_NOINTR) )
			SPL7();
		laststate = stp->pstate;
		ASSERT((stp->flags & (SF_JOB | SF_DISCON)) == SF_JOB);
		ncr->mode &= ~SMD_DMA;
		i = 0;
		while((ncr->curr_stat & (SCS_REQ | SCS_BSY)) == SCS_BSY) {
			DELAY;
			/* This delay seems too long, but some drives
			 * (ST 225N) take a long time.
			 */
			if(++i > 125000) { 
				break;
			}
		}
		if(!(ncr->curr_stat & SCS_REQ)) {
			stp->req->ret = SST_PROT;
			stp->pstate = SP_ABT;
			TRACE(1, ("Req failed\n"));
			ncr->init_comm = 0;
			break;
		}
		newphase = (ncr->curr_stat >> 2) & 0x7;
		if(newphase <= SPH_STAT && 
		    !(stp->flags & SF_ABTSET)) {
			if(stp->pstate == SP_DATA && newphase > SPH_DIN) {
				/* If finishing data, check all requested
				 * bytes were transferred.
				 */
				if(stp->datasent != stp->datalen 
				   && !(stp->flags & SF_SENSE)) {
					TRACE(T_scsi, ("err = SST_MORE\n"));
					stp->pstate = SP_ABT;
					stp->req->ret = SST_MORE;
				}
			}
			ASSERT(stp->pstate != SP_ABT ||
					spstates[stp->pstate][newphase] == SP_ABT);
			TRACE(T_scsi, ("stp->pstate = %s newstate = %s\n", 
				pstatenames[stp->pstate],
				pstatenames[spstates[stp->pstate][newphase]]));
			stp->pstate = spstates[stp->pstate][newphase];
			if(stp->pstate == SP_DATA) {
				if(((stp->flags & SF_READ) 
				&& newphase != SPH_DIN) ||
				(!(stp->flags & SF_READ)
				&& newphase != SPH_DOUT)) {
					TRACE(T_scsi, ("data phase match err\n"));
					stp->pstate = SP_ABT;
					stp->req->ret = SST_CMD;
				}
			}
		}
		break;
	case SI_ABT:
		TRACE(T_scsi, (
		"begin abt databuf = 0x%x datalen = 0x%x datasent = 0x%x\n",
		stp->databuf, stp->datalen, stp->datasent));
		i = 0;
		while((ncr->curr_stat & (SCS_REQ | SCS_BSY)) == SCS_BSY) {
			DELAY;
			if(++i > 125000) {
				SPLINTR();
				scsisched(SI_RESET);
				ASSERT(--checkconcur == 0);
				return;
			}
		}
		atn = SIC_ATN;
		newphase = (ncr->curr_stat >> 2) & 0x7;
		break;
	default:
		printf("input code %d\n", taskinp);
		panic("unknown input to scsitask");
	}
	if((ncr->curr_stat & (SCS_BSY | SCS_RST)) != SCS_BSY) {
		if(!(stp->req->ret) && !(ncr->curr_stat & SCS_BSY))
			stp->req->ret = SST_BSY;
		stp->pstate = SP_ABT;
		stp->flags &= ~SF_ABTSET;
	}
	if(stp->pstate == SP_ABT && !(stp->flags & SF_ABTSET)) {
		TRACE(T_scsi, ("task first abort\n"));
		if(!stp->req->ret) {
			stp->req->ret = sp_errs[laststate];
			if(UNJAM)
				stp->req->ret = SST_TIMEOUT;
		}
		SPLINTR();
		scsisched(SI_ABT);
		ASSERT(--checkconcur == 0);
		return;
	}
	ncr->mode &= ~SMD_DMA;
	ncr->targ_comm = newphase;
	ncr->sel_ena = 0;
	i = ncr->respar_int;
	ncr->sel_ena = 1 << ourid;
	if((newphase & 1)) {	/* If input */
		ncr->init_comm = 0 | atn;
		if(newphase != SPH_MIN) {
			ncr->mode |= SMD_DMA;
			ncr->start_Ircv = 0;
		}
	}
	else {
		if(newphase != SPH_CMD) {
			ncr->init_comm = SIC_DB | atn;
			ncr->mode |= SMD_DMA;
		}
		ncr->start_xmt = 0;
	}
	ASSERT(ncr->bus_status & SBS_PHASE);
dophase:
	TRACE(T_scsi, ("scsitask: phase %d state = %s\n", 
		newphase, pstatenames[stp->pstate]));
	stp->curtime = 0;
	CLRJAM;
	switch(newphase) {
#ifdef	STANDALONE
#define	scsiin_dma scsiin_poll
#define	scsiout_dma scsiout_poll
#endif	STANDALONE
	case SPH_DIN:
		if(stp->datalen < 100 || (stp->devchar & SDC_RDPOLL) 
		    || (stp->datalen & 3)) {
			stp->datasent += scsiin_poll(&stp->databuf[stp->datasent],
			   stp->datalen - stp->datasent, 
			   (int)(stp->devchar & SDC_532));
		}
		else {
			stp->datasent += scsiin_dma(&stp->databuf[stp->datasent],
			   stp->datalen - stp->datasent, 
			   (int)(stp->devchar & SDC_532));
		}
		if(UNJAM)
			stp->req->ret = SST_TIMEOUT;
		if(berrflag) {
			berrflag = 0;
			stp->devchar |= SDC_RDPOLL;
			stp->req->ret = SST_AGAIN;
			stp->pstate = SP_ABT;
		}
		break;
	case SPH_DOUT:
		if(stp->flags & SF_ABTSET) {
			bzero(jnkbuf, sizeof(jnkbuf));
		}
		if(stp->datalen < 100 || (stp->devchar & SDC_WRPOLL) 
		    || (stp->datalen & 3)) {
			stp->datasent += scsiout_poll(&stp->databuf[stp->datasent],
			   stp->datalen - stp->datasent, 
			   (int)(stp->devchar & SDC_532));
		}
		else {
			stp->datasent += scsiout_dma(&stp->databuf[stp->datasent],
			   stp->datalen - stp->datasent, 
			   (int)(stp->devchar & SDC_532));
		}
		if(UNJAM)
			stp->req->ret = SST_TIMEOUT;
		if(berrflag) {
			berrflag = 0;
			stp->devchar |= SDC_WRPOLL;
			stp->req->ret = SST_AGAIN;
			stp->pstate = SP_ABT;
		}
		break;
	case SPH_CMD:
		if(stp->flags & SF_ABTSET) {
			bzero(jnkbuf, sizeof(struct scsig0cmd));
		}
		if(scsiout_slow(stp->cmdbuf, stp->cmdlen) != stp->cmdlen) {
			if(UNJAM)
				stp->req->ret = SST_TIMEOUT;
			else
				stp->req->ret = SST_CMD;
			stp->pstate = SP_ABT;
		}
		break;
	case SPH_STAT:
		if(scsiin_poll(&stp->stat, 1, 0) != 1) {
			stp->pstate = SP_ABT;
		}
		break;
	case SPH_MIN:
		do {
			ASSERT(ncr->mode == 0);
			ASSERT(ncr->curr_stat & SCS_REQ);
			ncr->init_comm = 0;
			stp->msgin = ncr->curr_data;
			ncr->init_comm = SIC_ACK;
			while(ncr->curr_stat & SCS_REQ)
				;
			ncr->init_comm = 0;
			if(stp->msgin & 0x80) {
				if(stp->lun == (stp->msgin & 0x7)) {
					if(!(stp->flags & (SF_SENSE | SF_ABTSET)))
						stp->datasent = stp->req->datasent;
				}
				else {
					TRACE(T_scsi, 
					("Lun does not match expected 0x%x new = 0x%x\n",
					stp->lun, stp->msgin));
					stp->pstate = SP_ABT;
					break;
				}
			}
			else {
				switch(stp->msgin) {
				case SMG_LNK:
				case SMG_LNKFLG:
				case SMG_COMP:
					TRACE(T_scsi, 
					("scsitask: complete message\n"));
					if(stp->pstate == SP_STAT 
					    || stp->pstate == SP_ABT) {
						if(stp->flags & SF_SENSE)
						    stp->req->sensesent = stp->datasent;
						else
						    stp->req->datasent = stp->datasent;
						if(stp->msgin == SMG_LNKFLG)
							stp->flags |= SF_LINKFLAG;
						else if(stp->msgin = SMG_LNK)
							stp->flags |= SF_LINK;
						SPLINTR();
						ncr->init_comm = 0;
						scsisched(SI_DONE);
						ASSERT(--checkconcur == 0);
						return;
					}
					else
						stp->pstate = SP_ABT;
					break;
				case SMG_SAVEP:
					TRACE(T_scsi, 
					("scsitask: save pointers message\n"));
					if(!(stp->flags & (SF_SENSE | SF_ABTSET))) {
						stp->req->datasent = stp->datasent;
					}
					break;
				case SMG_RESTP:
					TRACE(T_scsi, 
					("scsitask: restore pointers message\n"));
					if(!(stp->flags & (SF_SENSE | SF_ABTSET))) {
						stp->datasent = stp->req->datasent;
					}
					break;
				case SMG_DISC:
					TRACE(T_scsi, 
					("disconnect id %d\n", curtask));
					meter(numdisc);
					SPLINTR();
					stp->flags |= SF_DISCON;
					ncr->init_comm = 0;
					scsisched(SI_DONE);
					ASSERT(--checkconcur == 0);
					return;
				default:
					TRACE(T_scsi, 
					("scsitask: unrecognized message 0x%x\n",
						stp->msgin));
					stp->pstate = SP_ABT;
					break;
				}
			}
			/* Wait for phase or BSY to change or REQ to assert */
			while((ncr->curr_stat & (SCS_BSY | SCS_REQ | (7<<2)))
					  == (SCS_BSY | (SPH_MIN << 2)))
				DELAY;
		} while((ncr->curr_stat & (SCS_REQ | (7<<2)))
					== (SCS_REQ | (SPH_MIN << 2)));
		break;
	case SPH_MOUT:
		ncr->init_comm &= ~SIC_ATN;
		atn = 0;
		if(stp->pstate == SP_SEL) {
			TRACE(T_scsi, ("Send ident message lun = %d\n", stp->lun));
			stp->msgout = 0xC0 | stp->lun;
			stp->pstate = SP_IDENT;
		}
		else if(stp->pstate == SP_ABT) {
			TRACE(T_scsi, ("Send abort message\n"));
			stp->msgout = SMG_ABT;
		}
		else {
			TRACE(T_scsi, ("Unknown message out request\n"));
			stp->pstate = SP_ABT;
			break;
		}
		if(scsiout_poll(&stp->msgout, 1, 0) != 1) {
			TRACE(T_scsi, ("message did not send\n"));
			stp->pstate = SP_ABT;
		}
		if(stp->msgout == SMG_ABT) {
			for(i = 0; i < 1000; ++i)
				if(!(ncr->curr_stat & SCS_BSY))
					break;
		}
		break;
	default:
		printf("SCSI bus phase error\n");
		break;
	}
	if(stp->pstate == SP_ABT) {
		if(stp->pstate == SP_ABT && !(stp->flags & SF_ABTSET)) {
			if(!stp->req->ret)
				stp->req->ret = sp_errs[laststate];
			SPLINTR();
			ncr->init_comm = 0;
			scsisched(SI_ABT);
			ASSERT(--checkconcur == 0);
			return;
		}
		else {
			stp->datasent = 0;
			atn = SIC_ATN;
			if(!(ncr->curr_stat & SCS_BSY)) {
				SPLINTR();
				ncr->init_comm = 0;
				scsisched(SI_DONE);
				ASSERT(--checkconcur == 0);
				return;
			}
			else if(!(ncr->bus_status & SBS_PHASE)) {
				taskinp = SI_PHASE;
				goto again;
			}
			goto dophase;
		}
	}
	DELAY;
	DELAY;
	if(!(ncr->bus_status & SBS_PHASE)	/* Phase has already shifted */
		|| (stp->flags & SF_NOINTR) ) {	/* or no-interrupt job */
		taskinp = SI_PHASE;
		++jobstep;
		goto again;
	}
	else {
		SPLINTR();
		CLRJAM;
		ncr->init_comm = 0;
		stp->curtime = 0;
		scsisched(SI_WAITP);
		ASSERT(--checkconcur == 0);
		return;
	}
}


/*	watchdog -- abort an I/O that takes too long.
 *	     There is a two layer timeout.  We monitor specific tasks to see
 *	if they have been inactive greater than the longest permissible
 *	inactive time (set by the caller).  Once a task has gone over its
 *	time limit we start setting the unjam flag.  The unjam flag is
 *	cleared and tested by low level routines.  If it becomes clear, we
 *	assume something is still running.  If it stays set, then we begin
 *	eviction proceedings.  A similar process is used for disconnected
 *	jobs that aren't coming back.  When we set the stuckdisc flag, we
 *	stop bus activity.  If the job has not come back within another timeout
 *	interval, then we reset the bus.
 */

static
watchdog()

{
	struct ncr5380 *ncr = (struct ncr5380 *)SCSI_ADDR;
	register struct tasks *stp;
	register i;

#ifndef	STANDALONE
	timeout(watchdog, 0, TOUTTIME * v.v_hz);
	if((1 << sg_state) & ((1 << SG_WRESET) | (1 << SG_ABORTJ) 
	   | (1 << SG_RESET)))
		return;
	if(unjam) {
		if(unjam == 1) {
			++unjam;
			/* If the current task is jammed, don't
			 * count the time against the disc jobs.
			 */
			for(i = 0, stp = tasks; i < NSCSI; ++i, ++stp) {
				if(stp->flags & SF_DISCON)
					stp->curtime = 0;
			}
		}
		else if(unjam++ >= 2) {
			unjam = 0;
			SPLINTR();
			TRACE(1, ("scsi reset to unjam\n"));
			ncr->init_comm = SIC_RST;
			return;
		}
	}
	else if(stuckdisc >= 0) {
		if(laststuck != jobstep) {
			laststuck = jobstep;
		}
		else {
			SPLINTR();
			TRACE(1, ("scsi reset for stuck disc\n"));
			ncr->init_comm = SIC_RST;
			return;
		}
	}
	else {
		for(i = 0, stp = tasks; i < NSCSI; ++i, ++stp) {
			if((stp->flags & SF_RUN)
			   && stp->maxtime != SRT_NOTIME
			   && (stp->curtime += TOUTTIME) 
			   > stp->maxtime + TOUTTIME) {
				if(stp->flags & SF_DISCON) {
					stuckdisc = stp - tasks;
					laststuck = 0;
					TRACE(1, ("scsi stuck: %d\n",
						stp - tasks));
					break;
				}
				else {
					TRACE(1, ("scsi jammed: %d\n",
						stp - tasks));
					TRACE(1, ("global state %s phase %s\n",
						state_atribs[sg_state].name,
						pstatenames[stp->pstate]));
					unjam = 1;
					break;
				}
			}
		}
	}
#endif	STANDALONE
}


printscsi()
{
#if defined(DEBUG) || defined(HOWFAR)
	register struct ncr5380 *scsi;
	u_char	curr_data, init_comm, mode, targ_comm, curr_stat, bus_status,
		in_data;

	scsi = (struct ncr5380 *)SCSI_ADDR;
	curr_data = scsi->curr_data;
	init_comm = scsi->init_comm;
	mode = scsi->mode;
	targ_comm = scsi->targ_comm;
	curr_stat = scsi->curr_stat;
	bus_status = scsi->bus_status;
	in_data = scsi->in_data;

	TRACE(1, ("Current SCSI Data =          0x%x\n",curr_data));
	TRACE(1, ("Initiator Command Register = 0x%x\n",init_comm));
	TRACE(1, ("Mode Register =              0x%x\n",mode));
	TRACE(1, ("Target Command Register =    0x%x\n",targ_comm));
	TRACE(1, ("Current SCSI Bus Status =    0x%x\n",curr_stat));
	TRACE(1, ("Bus & Status Register =      0x%x\n",bus_status));
	TRACE(1, ("Input Data Register =        0x%x\n\n",in_data));
#endif defined(DEBUG) || defined(HOWFAR)
}
