#ifndef lint	/* .../sys/PAGING/io/dump.c */
#define _AC_NAME dump_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:22:41}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of dump.c on 87/11/11 21:22:41";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)dump.c	UniPlus VVV.2.1.1	*/

/*
** Dump the memory to the disk
*/

#ifdef ORIG3B20		/* for now we don't use anything in this file */
#include <sys/types.h>
#ifndef ORIG3B20
#include "sys/mmu.h"
#include "sys/seg.h"
#endif ORIG3B20
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/dma.h>
#include <sys/dir.h>
#include <sys/vtoc.h>
#include <sys/page.h>
#include <sys/systm.h>
#include <sys/elog.h>
#include <sys/iobuf.h>
#include <sys/bic.h>
#include <sys/io.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/dfc.h>
#include <sys/ssr.h>
#ifdef ORIG3B20
#include <sys/seg.h>
#endif ORIG3B20

#define MAXDUMP 4		/* max times to try to dump */

int	dstack[1024];	/* Stack to be used while dump running */
struct dmaram dmaram;
union	ptbl	*dptblp;/* pointer to page table that contains sysgen data */
struct	dfcdsg	*dfcdsgp;/* pointer to sysgen data */
union	ptbl	*ddptblp;/* pointer to page table that contains data */
extern int dumplow, dumpcnt;
extern struct dsksecns dsksecns[];
extern struct dsksecns dsks675[];
int dumpoff;		/* offset from beginning of disk to dump secn */
extern int physmem;	/* number of pages equipped */
int dumpct;		/* number of times dump attempted */
int dfcgnrc;
int dumpip;		/* dump in progress flag */
/*int dumpdbg=1;*/


dump()
{
	register i, j;
	int err;		/* error flag */
	char s[16];		/* array for returning prm */
	int stat;
	uint command, msvaddr, stdskba, nwords;
	int blkaddr;
	int dfcn, dskn, secn;
	struct halftag *hashptr;
	ushort hashsum;
	int chan, dv;
	int startaddr;
	struct cmdword splcmd;
	int load;
	struct df2job curjob;
	int cmdtype;

	dumpip = 1;
/*	if(dumpdbg) printf("!dump START\n");*/
	clrssr(SSR_ISOD);
	clrssr(SSR_DISAB);
	setssr(SSR_BIN);
	reset();

	asm("	movw	&0xfffffbff,	%r0");
	asm("	wtsr	%r0,	!isc");
	asm("	movw	&0xfffffbff,	%r0");
	asm("	wtsr	%r0,	!im");

	dumpct = 0;
	while(1)  {
/*	if(dumpdbg) printf ("! loop %d/%d\n", dumpct, MAXDUMP);*/
		dfcn = dsk_daddr[funit(minor(dumpdev))].d_dfc;
		dskn = dsk_daddr[funit(minor(dumpdev))].d_dsk;
		secn = fsecn(minor(dumpdev));

		chan = dfc_devaddr[dfcn].v_chan;
		dv = dfc_devaddr[dfcn].v_dev;


		if(idmac(10, &stat) == -1) {
			dumphalt(3);		/* won't return on 3rd error */
			continue;
		}


		if(idsch(chan, &stat) == -1) {
			dumphalt(2);		/* won't return on 3rd error */
			continue;
		}

		reset();

		dfcdsgp->dfcxdma[0] = (uint) svirtophys(ddptblp) | DMAVAL;

		if(enabdma(10, 1, &stat) == -1) {
			dumphalt(5);		/* won't return on 3rd error */
			continue;
		}

		dmaram.wd[0] = (uint) svirtophys(ddptblp);
		dmaram.wd[1] = 0;
		dmaram.wd[2] = 0;
		dmaram.wd[3] = DMAVAL;
		if(wtdma(chan, dv, &dmaram, &stat) < 0) {
			dumphalt(6);		/* won't return on 3rd error */
			continue;
		}

		if(clrdbs(chan, dv) == -1) {
			dumphalt(7);		/* won't return on 3rd error */
			continue;
		}

		reset();

		if(sdcdev(chan,dv,BIC_CLR|BIC_CFIFO|BIC_SWDMD|PIC_RESET,&stat)<0) {
			dumphalt(8);		/* won't return on 3rd error */
			continue;
		}

		if(sdcdev(chan, dv, BIC_IFEN, &stat) < 0) {
			dumphalt(9);		/* won't return on 3rd error */
			continue;
		}

		if(enabdev(chan, dv, &stat) == -1) {
			dumphalt(10);		/* won't return on 3rd error */
			continue;
		}

		reset();

		dfcdsgp->dfcvers = 0;
/*	if(dumpdbg) printf ("! do generic\n");*/
		if(sdcdev (chan, dv, P2GIDPIO | (poff(&dfcdsgp->dfcvers) << 10), &stat) == -1)
			{dumphalt(11); goto contin;}
		if (dfcdelay (chan, dv, 0, 100) == -1) continue;
		dfcgnrc = dfcdsgp->dfcvers >> 28;
		if (dfcgnrc == 0) dfcgnrc = 1;
/*	if(dumpdbg) printf ("! gid=%d\n", dfcgnrc);*/
		switch (dfcgnrc)
		{
			case 1:
				dfcdsgp->dfcds.df1ds.df1ctrl.unload = 0;
				dfcdsgp->dfcds.df1ds.df1ctrl.load = 0;
				dfcdsgp->dfcds.df1ds.df1ctrl.qloc = poff(&dfcdsgp->dfcds.df1ds.dfcq[0]);
				dfcdsgp->dfcds.df1ds.df1ctrl.unused = 814;
				dfcdsgp->dfcds.df1ds.df1ctrl.option = 0;
				dfcdsgp->dfcds.df1ds.df1ctrl.qsize = 4;
				dfcdsgp->dfcds.df1ds.df1ctrl.version = 0;
				dfcdsgp->dfcds.df1ds.df1ctrl.verify = 0;
				msvaddr = poff(&dfcdsgp->dfcds.df1ds.df1ctrl.unload);
				command = SYSGEN | (msvaddr << sSGENVA) | (1 << sCURMODE);
				if(sdcdev(chan, dv, command, &stat) == -1) {
					/* failure #9: can't sysgen the DFC
						12-15	channel status		*/
					{dumphalt(12); goto contin;}
				}
/*	if(dumpdbg) printf ("! SYSGEN dfcdelay\n");*/
				if(dfcdelay(chan, dv, 0, 0))
					{dumphalt(12); goto contin;}
/*	if(dumpdbg) printf ("! SYSGEN done\n");*/
				command = DFCSERV | (1 << sCURMODE) | (2 << sJOBID);
				if(sdcdev(chan, dv, command, &stat) == -1) {
					/* failure #10: can't bring DFC in service
						12-15	channel status		*/
					{dumphalt(13); goto contin;}
				}
/*	if(dumpdbg) printf ("! DFCSERV dfcdelay\n");*/
				if(dfcdelay(chan, dv, 2, 0))
					{dumphalt(13); goto contin;}
/*	if(dumpdbg) printf ("! DFCSERV done\n");*/
				command = DSKSERV | (dskn << sDSK) | (1 << sCURMODE)
						| (3 << sJOBID);
				if(sdcdev(chan, dv, command, &stat) == -1) {
					/* failure #11: can't bring disk in service
						12-15	channel status		*/
					{dumphalt(14); goto contin;}
				}
/*	if(dumpdbg) printf ("! DSKSERV dfcdelay\n");*/
				if(dfcdelay(chan, dv, 3, 0))
					{dumphalt(14); goto contin;}
/*	if(dumpdbg) printf ("! DSKSERV done\n");*/
				dumpoff = dsksecns[secn].startaddr;
				break;
			case 2:
			case 3:
				/*
					initialize response queue to all empty.
				*/
				for (i=0;  i<BOOTRESP;  i++)  {
					dfcdsgp->dfcds.df2ds.respq[i].ccc.cc2word = EMPTY;
				}
				/*
					sysgen the dfc
				*/
				dfcdsgp->dfcds.df2ds.df2ctrl.baseq = poff(dfcdsgp->dfcds.df2ds.baseq);
				dfcdsgp->dfcds.df2ds.df2ctrl.basequnl = 0;
				dfcdsgp->dfcds.df2ds.df2ctrl.highq = poff(dfcdsgp->dfcds.df2ds.highq);
				dfcdsgp->dfcds.df2ds.df2ctrl.highqunl = 0;
				dfcdsgp->dfcds.df2ds.df2ctrl.specq = poff(dfcdsgp->dfcds.df2ds.specq);
				dfcdsgp->dfcds.df2ds.df2ctrl.specqunl = 0;
				dfcdsgp->dfcds.df2ds.df2ctrl.respq = poff(dfcdsgp->dfcds.df2ds.respq);
				dfcdsgp->dfcds.df2ds.df2ctrl.respqunl = 0;
				dfcdsgp->dfcds.df2ds.df2ctrl.retryq = poff(&dfcdsgp->dfcds.df2ds.retryq);
				dfcdsgp->dfcds.df2ds.df2ctrl.respload = 0;
				dfcdsgp->dfcds.df2ds.df2ctrl.specload = 0;
				dfcdsgp->dfcds.df2ds.df2ctrl.highload = 0;
				dfcdsgp->dfcds.df2ds.df2ctrl.baseload = 0;
				dfcdsgp->dfcds.df2ds.df2ctrl.respsize = BOOTRESP;
				dfcdsgp->dfcds.df2ds.df2ctrl.specsize = BOOTSPEC;
				dfcdsgp->dfcds.df2ds.df2ctrl.highsize = BOOTHIGH;
				dfcdsgp->dfcds.df2ds.df2ctrl.basesize = BOOTBASE;
				dfcdsgp->dfcds.df2ds.df2ctrl.sleep = 0;
				dfcdsgp->dfcds.df2ds.df2ctrl.options = OPTWORD;
				/*
					do the sysgen
				*/
				command = P2SYSGEN | (poff(&dfcdsgp->dfcds.df2ds.df2ctrl) << s2SGENVA);
				if (sdcdev(chan, dv, command, &stat) == -1)
					{dumphalt(12); goto contin;}
/*	if(dumpdbg) printf ("! SYSGEN dfcdelay\n");*/
				if (df2delay (chan, dv, 0) < 0)
					goto contin;
/*	if(dumpdbg) printf ("! SYSGEN done\n");*/
				splcmd.type = Q2DFCSRV;
				splcmd.newmode = 0;
				splcmd.jobid = 2;
				splcmd.did = 0;
				load = dfcdsgp->dfcds.df2ds.df2ctrl.specload;
				dfcdsgp->dfcds.df2ds.specq[load] = splcmd;
				load = (load + 1) % BOOTSPEC;
				dfcdsgp->dfcds.df2ds.df2ctrl.specload = load;
				if (dfcdsgp->dfcds.df2ds.df2ctrl.sleep)
				{
					dfcdsgp->dfcds.df2ds.df2ctrl.sleep = 0;
					if (sdcdev (chan, dv, P2WAKEUP, &stat) == -1)
						{dumphalt(13); goto contin;}
				}
/*	if(dumpdbg) printf ("! DFCSERV dfcdelay\n");*/
				if (df2delay (chan, dv, 2) < 0)
					goto contin;
/*	if(dumpdbg) printf ("! DFCSERV done\n");*/
				splcmd.type = Q2DSKSRV;
				splcmd.newmode = 0;
				splcmd.jobid = 3;
				splcmd.did = dskn;
				load = dfcdsgp->dfcds.df2ds.df2ctrl.specload;
				dfcdsgp->dfcds.df2ds.specq[load] = splcmd;
				load = (load + 1) % BOOTSPEC;
				dfcdsgp->dfcds.df2ds.df2ctrl.specload = load;
				if (dfcdsgp->dfcds.df2ds.df2ctrl.sleep)
				{
					dfcdsgp->dfcds.df2ds.df2ctrl.sleep = 0;
					if (sdcdev (chan, dv, P2WAKEUP, &stat) == -1)
						{dumphalt(14); goto contin;}
				}
/*	if(dumpdbg) printf ("! DSKSERV dfcdelay\n");*/
				if (df2delay (chan, dv, 3) < 0)
					goto contin;
/*	if(dumpdbg) printf ("! DSKSERV done\n");*/
				if (dfcdsgp->dfcds.df2ds.df2ctrl.numtrks > 20000)
					dumpoff = dsks675[secn].startaddr;
				else dumpoff = dsksecns[secn].startaddr;
				break;
			default:
				{dumphalt(12); goto contin;}
		}

		i = ptod(physmem);
		if(i > dumpcnt)
			i = dumpcnt;

		startaddr = 0;
		blkaddr = dumplow + dumpoff;

		err = 0;
		for( ; i > 0; i = i - 128) {
			reset();
			dfcdsgp->dfcxdma[1] = (uint) svirtophys(dptblp) | DMAVAL;
			for(j = 0; j < 32; j++) {
				dptblp->page[j] = (startaddr & PG_ADDR) | PG_DMAR;
				startaddr += ptob(1);
			}
			stdskba = blkaddr;
			blkaddr += (i >= 128) ? 128 : i;
			msvaddr = 0;
			j = (i >= 128) ? (128*512) : (i*512);
/*	if(dumpdbg) printf ("! write %d\n", stdskba);*/
			switch (dfcgnrc)
			{
				case 1:
					command = WRITE | (dskn << sDSK);
					command |= (1 << sJOBID);
					nwords = (j / 4) << sWORDS;
					dfcdsgp->dfcds.df1ds.dfcq[dfcdsgp->dfcds.df1ds.df1ctrl.load].j_cmd = command;
					dfcdsgp->dfcds.df1ds.dfcq[dfcdsgp->dfcds.df1ds.df1ctrl.load].j_daddr = stdskba;
					dfcdsgp->dfcds.df1ds.dfcq[dfcdsgp->dfcds.df1ds.df1ctrl.load].j_vaddr = msvaddr;
					dfcdsgp->dfcds.df1ds.dfcq[dfcdsgp->dfcds.df1ds.df1ctrl.load].j_hash = nwords;
					hashptr = (struct halftag *)(&dfcdsgp->dfcds.df1ds.dfcq[dfcdsgp->dfcds.df1ds.df1ctrl.load]);
					for(hashsum = j = 0; j < 3; j++, hashptr++)
						hashsum = hashsum ^ hashptr->lt16bits ^ hashptr->rt16bits;
					hashsum ^= hashptr->lt16bits;
					dfcdsgp->dfcds.df1ds.dfcq[dfcdsgp->dfcds.df1ds.df1ctrl.load].j_hash |= hashsum;
					load = dfcdsgp->dfcds.df1ds.df1ctrl.load;
/*	if(dumpdbg) printf ("!  j_cmd=0x%x\n", dfcdsgp->dfcds.df1ds.dfcq[load].j_cmd);*/
/*	if(dumpdbg) printf ("!  j_daddr=0x%x\n", dfcdsgp->dfcds.df1ds.dfcq[load].j_daddr);*/
/*	if(dumpdbg) printf ("!  j_vaddr=0x%x\n", dfcdsgp->dfcds.df1ds.dfcq[load].j_vaddr);*/
/*	if(dumpdbg) printf ("!  j_hash=0x%x\n", dfcdsgp->dfcds.df1ds.dfcq[load].j_hash);*/
					dfcdsgp->dfcds.df1ds.df1ctrl.load = (dfcdsgp->dfcds.df1ds.df1ctrl.load + 1) % 4;
					command = (ST_JPEND << sSTTYPE) | RDSTAT | JPEND;
					command |= (dfcdsgp->dfcds.df1ds.df1ctrl.load * 4) << sLOADP;
/*	if(dumpdbg) printf ("!  cmnd=0x%x\n", command);*/
					if(sdcdev(chan, dv, command, &stat) == -1) {
						/* failure #12: can't read DFC status
							12-15	channel status		*/
						dumphalt(21);
						err = -1;
						break;
					}
/*	if(dumpdbg) printf ("!  write dfcdelay\n");*/
					if (dfcdelay(chan, dv, 1, 0) == -1)
					{
						err = -1;
						break;
					}
					break;
				case 2:
				case 3:
					cmdtype = Q2WRITE;
					curjob.cmdword.type = cmdtype;
					curjob.cmdword.did = dskn;
					curjob.cmdword.jobid = 1;
					curjob.daddr = stdskba;
					curjob.vaddr = msvaddr;
					curjob.numwords = j >> 2;
					load = dfcdsgp->dfcds.df2ds.df2ctrl.baseload;
					dfcdsgp->dfcds.df2ds.baseq[load] = curjob;
					dfcdsgp->dfcds.df2ds.df2ctrl.baseload =
						(load + 1) % BOOTBASE;
					if (dfcdsgp->dfcds.df2ds.df2ctrl.sleep)
					{
						dfcdsgp->dfcds.df2ds.df2ctrl.sleep = 0;
/*	if(dumpdbg) printf ("!  wakeup\n");*/
						if (sdcdev (chan, dv, P2WAKEUP, &stat) == -1)
						{
							dumphalt(21);
							err = -1;
							break;
						}
					}
/*	if(dumpdbg) printf ("!  write dfcdelay\n");*/
					if (df2delay (chan, dv, 1) < 0)
					{
						err = -1;
						break;
					}
					break;
				default:
					dumphalt (21);
					err = -1;
			}
			if (err != 0) break;
		}
		if(err != 0)
contin		:
			continue;	/* try again if error occured */
		for (i=0; i<16; i++) 
			s[i] = '0';
		s[0] = 'E';
		s[10] = 7;
		s[15]=dumpct + '0';
		prm(s);
		for(i = 0  ;  i < 50  ;  i++)
			reset();
		return;
	}
}



dfcdelay(chan, dv, jobid, tocnt)
int chan, dv, tocnt;
{
	int rtn1, rtn2;
	int stat;

again1	:
	if (tocnt > 0)
	{
		if (tintrsvc (chan, dv, tocnt) == -1) return (-2);
	}
	else if (tintrsvc (chan, dv, 200) == -1)
	{
		dumphalt (22);
		return (-1);
	}
	if(sdcdev(chan, dv, BIC_RSINT, &stat) < 0) {
		/* failure #13: can't reset BIC interrupt
			12-15	channel status		*/
		dumphalt(15);
		return (-1);
	}
	ssdev(chan, dv, &stat);
	if(sdcdev(chan,dv,RDSTAT|(ST_CCOM << sSTTYPE),&stat) < 0) {
		/* failure #14: can't send RDSTAT cmd to DFC
			12-15	channel status		*/
		dumphalt(16);
		return (-1);
	}
	ssdev(chan, dv, &stat);
	if(rdbic(chan,dv,&rtn1,&stat) < 0) {
		/* failure #15: can't read 1st DFC status word
			12-15	channel status		*/
		dumphalt(17);
		return (-1);
	}
	if ((rtn1 & 0xff) != jobid) goto again1;
	if((rtn1 >> 25) >= CC_FAIL) {
		if(rdbic(chan, dv, &rtn2, &stat) == -1) {
			/* failure #16: can't read 2nd DFC status word
				12-15	channel status		*/
			dumphalt(18);
			return (-1);
		}
		/* failure #17: bad DFC status
			6-7	DFC completion code
			8-15	2nd DFC status word
		 */
		dumphalt(19);
		return (-1);
	}
	if(enabdev(chan, dv, &stat) == -1) {
		/* failure #18: can't enable BIC interrupts
			12-15	channel status		*/
		dumphalt(20);
		return (-1);
	}
	return (0);
}

df2delay (chan, dv, jobid)
 int chan, dv, jobid;
{
 int noresp, once, rval, unl, nunl;
 struct df2resp curresp;
 int stat;

/*	if (dumpdbg) printf ("!df2delay: jid=%d\n", jobid);*/
	noresp = 1;
	do
	{
		if (tintrsvc (chan, dv, 200) == -1)
		{
			dumphalt (22);
			return (-1);
		}
/*	if(dumpdbg) printf ("!  intrsvc done\n");*/
		once = 1;
theloop		:
		unl = nunl = dfcdsgp->dfcds.df2ds.df2ctrl.respqunl;
		while (unl = nunl, curresp = dfcdsgp->dfcds.df2ds.respq[unl],
			curresp.ccc.cc2word != EMPTY)
		{
			nunl = (unl + 1) % BOOTRESP;
			dfcdsgp->dfcds.df2ds.respq[unl].ccc.cc2word = EMPTY;
			dfcdsgp->dfcds.df2ds.df2ctrl.respqunl = nunl;
/*	if (dumpdbg) printf ("!   ccw=0x%x\n", curresp.ccc.cc2word);*/
			if (curresp.ccc.cc2flds.jobid != jobid) continue;
			noresp = 0;
			rval = curresp.ccc.cc2word;
		}
		if (once)
		{
			if (sdcdev (chan, dv, BIC_RSINT, &stat) < 0)
			{
				dumphalt(15);
				return (-1);
			}
			once = 0;
			goto theloop;
		}
		if (noresp && dfcdsgp->dfcds.df2ds.df2ctrl.sleep)
		{
			dfcdsgp->dfcds.df2ds.df2ctrl.sleep = 0;
			if (sdcdev (chan, dv, P2WAKEUP, &stat) == -1)
			{
				dumphalt (16);
				return (-1);
			}
		}
	} while (noresp);
	if (rval < 0) dumphalt (19);
	return (rval);
}

reset()
{
	asm("	movw	&0,	%r0");
	asm("	wtsr	%r0,	!timers");
}

dumphalt(dmsg)
{
	int i, j;
	char s[16];
	char c;

/*	if(dumpdbg) printf ("!DUMPHALT %d\n", dmsg);*/
	i = 0;
	do {
		s[i++] = dmsg % 10 + '0';
	} while((dmsg /= 10) > 0);
	for( ; i < 16; i++)
		s[i] = '0';
	j = i - 1;
	for(i = 0; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
	s[11] = 'D';
	s[10] = 'D';
	s[9] = 'D';
	s[8] = 'D';
	s[7] = (++dumpct % 10) + '0';
	prm(s);
/*	if (dumpdbg) printf ("!dumphalt prm done\n");*/
	for (j = 0; j < 50; j++)
		reset();
	if(dumpct<MAXDUMP)
	{
/*	if (dumpdbg) printf ("!dumphalt return\n"); */
		return;	/* try dumping MAXDUMP times */
	}
/*	if (dumpdbg) printf ("!dumphalt stop\n");*/
	for ( ; ; ) reset ();
}

dumpinit()
{
			/* empty page table for use with dump */
	if ((dptblp = getptbl()) == -1)
		printf("dumpinit: cannot get ptbl\n"); 
	if((dfcdsgp=(struct dfcdsg *)kseg(SEG_RW,btop(sizeof(struct dfcdsg))))==0) {
		printf("dfc dump seg not initialzed (enomem)\n");
	}
#ifndef ORIG3B20
	ddptblp = segvptbl(&kstbl[snum(dfcdsgp)]);
#else ORIG3B20
	ddptblp = segvptbl(kstbl[snum(dfcdsgp)]);
#endif ORIG3B20
	if(ptod(physmem) > dumpcnt){
		printf("%s", "\007\n\007\n\007");
		printf("WARNING: Physical memory (%d blocks)\n",
			ptod(physmem));
		printf("is greater than dump file (%d blocks).\n",
			dumpcnt);
		printf("A proper dump cannot be taken on this system\n");
		printf("%s", "\007\n\007\n\007");
	}
}
#endif ORIG3B20


/* <@(#)dump.c	6.4> */
