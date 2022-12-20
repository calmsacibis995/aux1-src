#ifndef lint	/* .../map.c */
#define _AC_NAME map_c
#define _AC_MAIN "@(#) Copyright (c) 1985 Adobe Systems Incorporated, All Rights Reserved.  {Apple version 1.2 87/11/11 21:46:21}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char ___src_map_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of map.c on 87/11/11 21:46:21";
  static char Notice[] = "Copyright (c) 1985 Adobe Systems Incorporated";
  char *_Version_ = "A/UX Release 1.0";
  char *_Origin_ = "Transcript release 2.0";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#ifndef lint
#define _AC_MODS
static char *RCSID="$Header: map.c,v 2.1 85/11/24 11:49:13 shore Rel $";
#endif
/* map.c
 *
 * Copyright (c) 1985 Adobe Systems Incorporated
 *
 * front end to mapname -- font mapping for users
 *
 * for non-4.2bsd systems (e.g., System V) which do not
 * allow long Unix file names
 *
 * RCSLOG:
 * $Log:	map.c,v $
 * Revision 2.1  85/11/24  11:49:13  shore
 * Product Release 2.0
 * 
 * Revision 1.1  85/11/20  00:14:39  shore
 * Initial revision
 * 
 *
 */

#include <stdio.h>
#include "transcript.h"

#define _AC_MODS
#define _AC_MODS

main(argc,argv)
int argc;
char **argv;
{
    char result[128];

    if (argc != 2) exit(1);
    if (mapname(argv[1],result) == NULL) exit(1);

    printf("%s\n",result);
    exit(0);
}
