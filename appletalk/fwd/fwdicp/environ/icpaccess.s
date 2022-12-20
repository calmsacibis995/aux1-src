#	@(#)Copyright Apple Computer 1987	Version 1.2 of icpaccess.s on 87/11/11 21:07:46

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
ipc_t			fep_ipc_q[SIZE_FEPQ] = {1};
ipc_t			unix_ipc_q[SIZE_UNIXQ] = {1};

extern icp_info_t	icp_info;


init_access()

{
	ipc_t	*ipcp;
	int	i;

	access.status = FEP_NOTREADY;
	access.cmd2fep.rp = access.cmd2fep.wp = access.cmd2fep.start =
								    fep_ipc_q;
	access.cmd2fep.end = fep_ipc_q + SIZE_FEPQ;
	for (i=SIZE_FEPQ, ipcp = fep_ipc_q; i; i--)
		ipcp++->cmd = 0;
	access.cmd2unix.rp = access.cmd2unix.wp = access.cmd2unix.start =
								    unix_ipc_q;
	access.cmd2unix.end = unix_ipc_q + SIZE_UNIXQ;
	for (i=SIZE_UNIXQ, ipcp = unix_ipc_q; i; i--)
		ipcp++->cmd = 0;

	access.cmds_to_fep = 0;
	access.fep_q_size = SIZE_FEPQ;
	access.bytes_to_fep = 0;
	access.cmds_to_unix = 0;
	access.unix_q_size = SIZE_UNIXQ;
	access.bytes_to_unix = 0;
	access.next_id = 0;
	access.status = FEP_READY;
	icp_info.count = SIZE_UNIXQ/2;
}
