#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/icpaccess.c */
#define _AC_NAME icpaccess_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:07:01}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of icpaccess.c on 87/11/11 21:07:01";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

/*	@(#)icpaccess.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 *	This file defines the access structure and the command queues for
 *	communication with AST's communication processor.
 */



#include <sys/types.h>
#include <sys/stream.h>
#include <sys/param.h>
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
#include <fwdicp.h>

/*
 *	note that these are initialized to get them into the data section
 *	so we can get by the ld bug!
 */
access_t		access = {1};
ipc_t			fep_exp_ipc_q[SIZE_FEPQ] = {1};
ipc_t			fep_cmd_ipc_q[SIZE_FEPQ] = {1};
ipc_t			fep_rep_ipc_q[2*SIZE_UNIXQ] = {1};
ipc_t			unix_exp_ipc_q[SIZE_UNIXQ] = {1};
ipc_t			unix_cmd_ipc_q[SIZE_UNIXQ] = {1};
ipc_t			unix_rep_ipc_q[2*SIZE_FEPQ] = {1};


extern icp_info_t	icp_info;


init_access()

{
	ipc_t	*ipcp;
	int	i;

	access.status = FEP_NOTREADY;

	access.exp2fep.rp = access.exp2fep.wp = access.exp2fep.start = fep_exp_ipc_q;
	access.exp2fep.end = fep_exp_ipc_q + SIZE_FEPQ;
	for (i=SIZE_FEPQ, ipcp = fep_exp_ipc_q; i; i--)
		ipcp++->cmd = 0;

	access.cmd2fep.rp = access.cmd2fep.wp = access.cmd2fep.start = fep_cmd_ipc_q;
	access.cmd2fep.end = fep_cmd_ipc_q + SIZE_FEPQ;
	for (i=SIZE_FEPQ, ipcp = fep_cmd_ipc_q; i; i--)
		ipcp++->cmd = 0;

	access.rep2fep.rp = access.rep2fep.wp = access.rep2fep.start = fep_rep_ipc_q;
	access.rep2fep.end = fep_rep_ipc_q + 2*SIZE_UNIXQ;
	for (i=2*SIZE_UNIXQ, ipcp = fep_rep_ipc_q; i; i--)
		ipcp++->cmd = 0;

	access.exp2unix.rp = access.exp2unix.wp = access.exp2unix.start = unix_exp_ipc_q;
	access.exp2unix.end = unix_exp_ipc_q + SIZE_UNIXQ;
	for (i=SIZE_UNIXQ, ipcp = unix_exp_ipc_q; i; i--)
		ipcp++->cmd = 0;

	access.cmd2unix.rp = access.cmd2unix.wp = access.cmd2unix.start = unix_cmd_ipc_q;
	access.cmd2unix.end = unix_cmd_ipc_q + SIZE_UNIXQ;
	for (i=SIZE_UNIXQ, ipcp = unix_cmd_ipc_q; i; i--)
		ipcp++->cmd = 0;

	access.rep2unix.rp = access.rep2unix.wp = access.rep2unix.start = unix_rep_ipc_q;
	access.rep2unix.end = unix_rep_ipc_q + 2*SIZE_FEPQ;
	for (i=2*SIZE_FEPQ, ipcp = unix_rep_ipc_q; i; i--)
		ipcp++->cmd = 0;

	access.cmds_to_fep = 0;
	access.fep_q_size = SIZE_FEPQ;
	access.bytes_to_fep = 0;
	access.cmds_to_unix = 0;
	access.unix_q_size = SIZE_UNIXQ;
	access.bytes_to_unix = 0;
	access.next_id = 0;
	access.status = FEP_READY;
	icp_info.count = SIZE_UNIXQ;
}
