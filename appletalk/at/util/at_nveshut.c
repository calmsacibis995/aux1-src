#ifndef lint	/* .../appletalk/at/util/at_nveshut.c */
#define _AC_NAME at_nveshut_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:03:15}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_nveshut.c on 87/11/11 21:03:15";
  static char *Version = "(C) Copyright 1987 UniSoft Corp. Version @(#)at_shutdown.c	4.2 3/19/86";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS

/*+----------------------------------------------------------------------------+
  |
  |  Description:
  |      At_nveshut is an AppleTalk utility which shuts down the NBP daemon.
  |      There are no arguments to this utility.
  |
  |  SCCS:
  |      @(#)at_nveshut.c	?.????
  |
  |  Copyright:
  |      Copyright 1987 by UniSoft Systems Corporation.
  |
  |  History:
  |      28-Feb-87: Created by Paul
  |
  +*/

#include "appletalk.h"
#include <time.h>
#include <memory.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>


#define _AC_MODS
static char *Version2= AT_APPLETALK_VERSION_GENERAL;

int
main(argc,argv)
int     argc;
char   *argv[];
{
        at_pgm = argv[0];
	at_nbp_shutdown();
}
