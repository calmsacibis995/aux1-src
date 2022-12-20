#ifndef lint	/* .../appleprint/iw/daiw/initme.c */
#define _AC_NAME initme_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:28}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _initme_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of initme.c on 87/11/11 21:44:28";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	initme()
 *
 *  Arguments:
 *	(none)
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	All the initialization cod eis placed here.
 *
 *	This function probes a big stack and breaks a large chunk of
 *	memory (which is promptly freed) for use  by later malloc calls.
 *	Essentially, we are paying for a large stack probe and break all
 *	at once in the beginning rather than bit by bit later.
 *
 *  Algorithm:
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void initme()
	{
	int	i[6000];

	i[sizeof(i)/sizeof(i[0])-1] = 0;
	init_mz = 40000;
	free(malloc(init_mz));
	linecnt = 1;
	}
