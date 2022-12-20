#ifndef lint	/* .../appletalk/fwd/fwdicp/fwdicpopen.c */
#define _AC_NAME fwdicpopen_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:09:42}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of fwdicpopen.c on 87/11/11 21:09:42";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)fwdicpopen.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *      Forwarder driver for communication with a Front End Processor.
 *	This forwarder is for communications with a peer to peer, or otherwise
 *	master to master relationship BUT with no read modified write (TAS).
 *      This is the unix side, there is an identical pair for the
 *      FEP side. Each side is divided up into two parts, a generic which is
 *      common to all FEPs and a component which is specific to a particular
 *      type of FEP. The specific component described here is for an AST
 *	4 port serical card and is further devided into two parts; that 
 *	which can talk to any ACP in the system (follows below), and the part
 *	which can onlytalk to a particular ACP. There must be one of the 
 *	latter for every card installed. The routines below can talk to any
 *	card, because they have been written so the board information 
 *	structure, acp_info is passed to these functions via the q local 
 *	pointer q->q_ptr->fep->ginfop.
 *
 *****************************************************************************
 *	To reconfigure the driver, just change the three (3) defines:
 *		NCIRCUITS, NSLOTS, NENTRIES
 *****************************************************************************
 */

#ifndef FEP
#define NSLOTS		4
#endif FEP

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/signal.h>
#include <sys/mmu.h>
#ifdef PAGING
#include <sys/page.h>
#endif PAGING
#include <sys/seg.h>
#ifdef PAGING
#include <sys/region.h>
#endif PAGING
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <fwd.h>
#include <via.h>
#include <fwdicp.h>

#ifndef NULL
#define NULL		0
#endif NULL



static int              fwdicpbdopen();
extern int              fwd_close();
extern int              fwd_wsrvc();
extern int              fwd_rsrvc();
extern int              icpreset();
extern int              icpdownld();
extern int              icpupld();
extern int              icpstart();
extern int              icpputq();
extern int              icpflow();
extern int              fwd_wputq();
extern void             fwdicpirupt();
extern                  nulldev();

static
struct module_info	fwdmodinfo =	{FORWARDERMIN, "fwd_icp", 0, INFPSZ, 1, 1,
					 NULL};

#ifndef	FEP		/* UNIX side */
static
struct qinit    	fwdrdata =	{putq, fwd_rsrvc, fwdicpbdopen, fwd_close,
				 	 nulldev, &fwdmodinfo, NULL};
static
struct qinit     	fwdwdata =	{fwd_wputq, fwd_wsrvc, nulldev, nulldev,
			     		 nulldev, &fwdmodinfo, NULL};
#else	FEP		/* FEP  side */
static
struct qinit    	fwdrdata =	{fwd_wputq, fwd_rsrvc, nulldev, nulldev,
				 	 nulldev, &fwdmodinfo, NULL};
static
struct qinit     	fwdwdata =	{putq, fwd_wsrvc, nulldev, nulldev,
			     		 nulldev, &fwdmodinfo, NULL};
#endif	FEP


#ifndef	FEP		/* UNIX side */
struct  streamtab       fwdicp_info = 	{&fwdrdata, &fwdwdata, NULL, NULL};
#else	FEP		/* FEP  side */
struct  streamtab       fwd_info = 	{&fwdrdata, &fwdwdata, NULL, NULL};
#endif	FEP




 

#ifndef	FEP		/* UNIX side */
fep_specifics_t		*icp_boards[16] = {NULL, NULL, NULL, NULL,
					   NULL, NULL, NULL, NULL,
					   NULL, NULL, NULL, NULL,
					   NULL, NULL, NULL, NULL};
static
icp_info_t		icp_info[NSLOTS] = {
						{
							0,
					 		(access_t *) 0,
					 		(caddr_t) 0,
							0,
					   		0,
							0,
						},
					   };

static fwd_circuit_t	fwd_circuit[NSLOTS*NCIRCUITS];
struct fep_specifics	icp_specifics[NSLOTS] = {
					{NCIRCUITS, (caddr_t) FEP_RAM_START,
					 (caddr_t) FEP_RAM_END,
					 icpreset, icpdownld, icpupld, icpstart,
					 icpputq, icpflow, fwd_circuit, (caddr_t) NULL}
					   };
extern int fwdicp_cnt;
extern int fwdicp_addr[];

fwdicp_init()
{
	register int i;

	if (fwdicp_cnt > NSLOTS)
		fwdicp_cnt = NSLOTS;
	for (i = 1; i < fwdicp_cnt; i++) {
		icp_info[i] = icp_info[0];
		icp_specifics[i] = icp_specifics[0];
	}
	for (i = 0; i < fwdicp_cnt; i++) {
		icp_info[i].ramp = (caddr_t)(((fwdicp_addr[i]&0xf) << 24) | 0xf0000000);
		icp_info[i].accessp = (access_t *)(icp_info[i].ramp + ACCESS_OFFSET);
		icp_info[i].count = 0;
		icp_info[i].sleep = 0;
		icp_info[i].rcvtimer = 0;
		icp_boards[fwdicp_addr[i]] = &icp_specifics[i];
		icp_specifics[i].ginfop = (caddr_t)&icp_info[i];
		icp_specifics[i].fwd_circuit = &fwd_circuit[i*NCIRCUITS];
		icp_specifics[i].state = FEP_CLOSED;
	}
	for (i = 0; i < NCIRCUITS*NSLOTS; i++) {
		fwd_circuit[i].state = CIRC_CLOSED;
	}
}

#else	FEP		/* FEP  side */
#define RAM		0
icp_info_t		icp_info =	{
						0,
					 	(access_t *) (RAM+ACCESS_OFFSET),
					 	(caddr_t) RAM,
						0,
						0,
						0,
					};

static fwd_circuit_t	fwd_circuit[NCIRCUITS];
struct fep_specifics	icp_specifics =  {NCIRCUITS, (caddr_t) FEP_RAM_START,
					 (caddr_t) FEP_RAM_END,
					NULL, NULL, NULL, NULL,
					icpputq, icpflow, fwd_circuit,
					(caddr_t) &icp_info};
#endif	FEP

#ifndef	FEP		/* UNIX side */
/*
 * open routine for the board. This should only be access by download, or the
 * system administrator. All other opens should go through fwdicpopen.
 */

static int
fwdicpbdopen(q, dev, flag, sflag, err)
queue_t *q;
int *err;
{
	struct stroptions	*stroptp;
	struct msgb		*mp;
	fwd_circuit_t		*circuit;
	

        dev = minor(dev);
        if (dev < 0 || dev >= fwdicp_cnt || sflag != DEVOPEN)
	{
            *err = ENXIO;
            return(OPENFAIL);
	}
        if (!suser()) {			/* only root can download */
            *err = EPERM;
            return(OPENFAIL);
	}
        if (q->q_ptr) {			/* only one download at a time */
            *err = EBUSY;
            return(OPENFAIL);
	}
        /* initialize local data structures for the r&w q's */
	circuit = icp_specifics[dev].fwd_circuit;
        q->q_ptr = OTHERQ(q)->q_ptr = (caddr_t) circuit;
        circuit->circuit = 0;
        circuit->upstreamq = q;
        circuit->fep = &icp_specifics[dev];
        circuit->state = CIRC_OPEN;
        circuit->open = 1;
	icp_specifics[dev].open++;
	return(0);
}
 
#endif	FEP
