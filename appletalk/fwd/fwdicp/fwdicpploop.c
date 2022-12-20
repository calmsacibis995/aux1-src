#ifndef lint	/* .../appletalk/fwd/fwdicp/fwdicpploop.c */
#define _AC_NAME fwdicpploop_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.3 87/11/13 10:19:23}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.1 of fwdicpploop.c on 87/05/03 14:57:17";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)fwdicpploop.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *      Forwarder driver for communication with a Front End Processor (FEP).
 *	This forwarder is for communications with a peer to peer, or otherwise
 *	master to master relationship, BUT with no read modified write (TAS).
 *      This is the unix side, there is an analogous pair for the
 *      FEP side. Each side is divided up into two parts, a generic which is
 *      common to all FEPs and a component which is specific to a particular
 *      type of FEP. The specific component described here is for an AST ICP,
 *	a 4 port serical card, and is further divided into two parts; that 
 *	which can talk to any ICP in the system and the part
 *	which can onlytalk to a particular ICP (below). The only differences
 *	between different applications talking to the same card, or the same
 *	application talking to different cards are:
 *		what is the name of the application,
 *		what board is it on.
 *	The name is located in the module info structure, and the board is
 *	referenced by the fwd_specific structure. Both of these details only
 *	need to be know by the OPEN routine!!!!!!!!!!!. For every board, and
 *	every application, you only need to have versions of the following.
 *
 *****************************************************************************
 *	To reconfigure the driver, just change the three (3) defines:
 *		FEP_SPECIFICS
 *		MODULE_NAME
 *		TAB_NAME
 *****************************************************************************
 *
 *	To then integrate this into the kernel
 *		edit the *conf.c file for the major number (unless auto config)
 *		mknod for the new major number
 *		update the makefile (for ASTICP, the makefile in FWDICP/*mk)
 *	To integrate into the load for the board
 *		edit the conf.c for the board
 *		update the makefile (for ASTICP, makefile in FWDICP/environ/*mk)
 *
 */
#define	MODULE_NAME	"fwd_ploop"
#define	MODULE_NUM	(FORWARDERMIN+2)
#define	TAB_NAME	fwdploop_info


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

static int              fwdthisopen();
extern int              fwd_open();
extern int              fwd_close();
extern int              fwd_wsrvc();
extern int              fwd_wputq();
extern int              fwd_rsrvc();
extern                  nulldev();

static
struct module_info	ploop_info =	{MODULE_NUM, MODULE_NAME, 0, INFPSZ, 1,
					 1, NULL};

static
struct qinit    	fwdrdata =	{putq, fwd_rsrvc, fwdthisopen,
					 fwd_close, nulldev, &ploop_info, NULL};

static
struct qinit     	fwdwdata =	{fwd_wputq, fwd_wsrvc, nulldev,
					 nulldev, nulldev, &ploop_info, NULL};

struct  streamtab       TAB_NAME	= {&fwdrdata, &fwdwdata, NULL, NULL};

extern fep_specifics_t	icp_specifics[];


static int
fwdthisopen(q, dev, flag, sflag, err, devp)
queue_t *q;
int *err;
dev_t dev, *devp;
{
	return(fwd_open(q, dev, flag, sflag, err, devp, &icp_specifics[(minor(dev)>>6)&0x3]));
}