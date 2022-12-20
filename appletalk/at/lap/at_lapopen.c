#ifndef lint	/* .../appletalk/at/lap/at_lapopen.c */
#define _AC_NAME at_lapopen_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:56:32}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_lapopen.c on 87/11/11 20:56:32";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)at_lapopen.c		UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	The lap module is multiplexing which means, there is one connection
 *	downstream (the network) and multiple connection upstream. The
 *	conections upstream are called demultiplexing queues and are viewed from
 *	the user interface as files in the lap directory for the network with
 *	the name "circuitX"; where X is a number < NCIRCUITx. There is no
 *	relationship between a connection and the type that is registered on
 *	it.
 *
 *	There are two parts to the lap module code. That which is common to
 *	all lap modules on the unix machine, no matter which network it is 
 *	connected to. And, that which is specific to a particular network
 *
 *	The network specific part is contained in this file. This file
 *	defines space needed and contains the open code for the module.
 *	To reconfigure the driver, just change the define:
 *		NCIRCUITx
 *		LAP_CHANNEL
 *		LAP_TIMER
 *			and/or in addition environmental specifics like
 *				scc
 *				via
 */


#include <sys/types.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <appletalk.h>
#include <via.h>
#include <scc.h>
#ifdef	FEP
#include <fwdicp.h>
#endif	FEP
#include "at_lap.h"

#ifndef NULL
#define NULL		0
#endif NULL

/************************************************************************/
/*	R E C O N F I G U R A T I O N  section				*/
/************************************************************************/

#define NCIRCUITx	4		/* number of virtual circuits allowed*/
#define LAP_CHANNEL	AT_CHANNEL	/* which of the lap_specifics to use
					 * for this network. This establishes
					 * the linkage to the interrupt level
					 * and is the scc channel
					 */
#define	LAP_TIMER	AT_TIMER	/* which via to use for this network */
/************************************************************************/

static int              lap_open();
extern int              lap_close();
extern int              lap_open();
extern int              lap_close();
extern int              lap_rsrvc();
extern int              lap_wputq();
extern int              lap_wsrvc();
extern                  nulldev();
extern mblk_t		*putqHI();

static struct module_info lap_info =
#ifndef	FEP	/*UNIX side*/
					{3942, "at_lap",
#else	FEP	/*FEP  side*/
					{FORWARDERMIN+3, "icpat_",
#endif	FEP
					 AT_LAP_HDR_SIZE, AT_LAP_SIZE,
					 3, 1024, NULL
					};
static struct qinit    	laprdata =	{putqHI, lap_rsrvc, lap_open, lap_close,
				 	 nulldev, &lap_info, NULL};
static struct qinit     lapwdata =	{lap_wputq, lap_wsrvc, nulldev, nulldev,
					 nulldev, &lap_info, NULL};
struct  streamtab       laptab = 	{&laprdata, &lapwdata, NULL, NULL};

/*
 *	The lap_specifics structure is external because the interrupt routine
 *	must have access to each one from each network. That way, the routine
 *	does not need to be recompiled and is common to all.
 */
extern lap_specifics_t lap_specifics[];

static lap_circuit_t lap_circuit[NCIRCUITx];


static int
lap_open(q, dev, flag, sflag, err)
queue_t *q;
int *err;

{
	register lap_circuit_t	*circuitp;

#ifdef DEBUG
        printf("lap_open: dev = %d\n", dev);
#endif
	if (sflag == CLONEOPEN)
	{
	    /* get the circuit number for this type of data packet */
	    for (   dev = 1, circuitp = lap_circuit;
		    (dev < NCIRCUITx) && (circuitp->upstreamq);
		    dev++, circuitp++
		)
		;
	}
	else
            dev = minor(dev);
        if (dev >= NCIRCUITx)
	{
            *err = ENXIO;
            return(OPENFAIL);
	}
        else
	/* initialize local data structures for the r&w q's */
	if (q->q_ptr != NULL)
	{
	    /* if already open, do not wipe out registered type and name */
	    return(dev);
	}
	q->q_ptr = OTHERQ(q)->q_ptr = (caddr_t) &lap_circuit[dev];
	lap_circuit[dev].type = 0;
	lap_circuit[dev].circuit = dev;
	lap_circuit[dev].name[0] = '\0';
	lap_circuit[dev].upstreamq = q;
	lap_circuit[dev].lap = &lap_specifics[LAP_CHANNEL];
	/*
	* initialize those things which have probably already have been
	* initialized, but auto initialization has not been implemented
	* yet, so we do it evey time so it will get done the first.
	*/
	lap_specifics[LAP_CHANNEL].ncircuits = NCIRCUITx;
	lap_specifics[LAP_CHANNEL].circuit0p = lap_circuit;
	lap_specifics[LAP_CHANNEL].scc = &sccs[LAP_CHANNEL];
	lap_specifics[LAP_CHANNEL].timer = LAP_TIMER;
	lap_specifics[LAP_CHANNEL].rwp = &at_rwp[(LAP_CHANNEL-1)/2];
	return(dev);
}
