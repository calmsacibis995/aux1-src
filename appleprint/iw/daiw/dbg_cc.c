#ifndef lint	/* .../appleprint/iw/daiw/dbg_cc.c */
#define _AC_NAME dbg_cc_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:21}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _dbg_cc_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of dbg_cc.c on 87/11/11 21:43:21";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	dbg_cc(cc)
 *
 *  Arguments:
 *	cc	A pointer to a CC structure that is part of a linked list of
 *		CC structures.
 *
 *  Description:
 *	This is a debug function. It takes a pointer to a CC structure,
 *	and prints out it's contents, then advances to the next CC
 *	structure via the next pointer until the next pointer is null.
 *
 *  Algorithm:
 *	For while the CC pointer is not null, advancing to the next
 *	linked list member after each loop,
 *	    Call dbg_cc1() to print out the CC structure contents.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void dbg_cc(cc)
	register CC    *cc;
	{

	for (;  cc;  cc = cc->cc_next)
	    {
	    dbg_cc1(cc);
	    }
	}
