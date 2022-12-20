#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/fep_var.c */
#define _AC_NAME fep_var_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:06:55}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of fep_var.c on 87/11/11 21:06:55";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)fep_var.h	UniPlus VVV.2.1.1			*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*      This is the header file for accesses to the local	*/
/*	environment on a ast icp serial card			*/

/* UniSoft's var structure for FEP streams */

#include <sys/types.h>
#include  "sys/stream.h"
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
#include  "fwd.h"
#include  "fwdicp.h"


struct var v = {
	NSTREAM,			/* number of stream heads */
	NQUEUE,				/* number of queue heads */
	NBLK4096,			/* number of 4k stream blocks */
	NBLK2048,			/* number of 2k stream blocks */
	NBLK1024,			/* number of 1k stream blocks */
	NBLK512,			/* number of 512 byte stream blocks */
	NBLK256,			/* number of 256 byte stream blocks */
	NBLK128,			/* number of 128 byte stream blocks */
	NBLK64,				/* number of 64 byte stream blocks */
	NBLK16,				/* number of 16 byte stream blocks */
	NBLK4,				/* number of 4 byte stream blocks */
} ;


int	str_buffers
	[
	    (
		sizeof(dblk_t) * (NBLK4096 + NBLK2048 + NBLK1024 + NBLK512
		       + NBLK256 + NBLK128 + NBLK64 + NBLK16 + NBLK4)
		+ 2 * sizeof(mblk_t) * (NBLK4096 + NBLK2048 + NBLK1024 + NBLK512
		       + NBLK256 + NBLK128 + NBLK64 + NBLK16 + NBLK4)
		+ NSTREAM	 * sizeof(queue_t)
		+ NQUEUE	 * sizeof(struct stdata)
		+ NBLK4096	 * 4096
		+ NBLK2048	 * 2048
		+ NBLK1024	 * 1024
		+ NBLK512	 * 512
		+ NBLK256	 * 256
		+ NBLK128	 * 128
		+ NBLK64	 * 64
		+ NBLK16	 * 16
		+ NBLK4	 * 14
	     ) / sizeof(int) /* because the array is ints */
	];
int	sizeof_str_buffers = sizeof(str_buffers);


char	qrunflag;
char	queueflag;
