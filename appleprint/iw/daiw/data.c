#ifndef lint	/* .../appleprint/iw/daiw/data.c */
#define _AC_NAME data_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:18}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _data_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of data.c on 87/11/11 21:43:18";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	(none)
 *
 *  Arguments:
 *	(none)
 *
 *  Description:
 *	This is a data only module. The include file local.h will
 *	`generate' the data externs here if the C preprocessor symbol
 *	DATA has been defined first.
 *
 *  Algorithm:
 *	(none)
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#define	DATA	1

#include "local.h"
