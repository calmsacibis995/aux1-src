#ifndef lint	/* .../psbanner.c */
#define _AC_NAME psbanner_c
#define _AC_MAIN "@(#) Copyright (c) 1985 Adobe Systems Incorporated, All Rights Reserved.  {Apple version 1.2 87/11/11 21:47:01}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char ___src_psbanner_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of psbanner.c on 87/11/11 21:47:01";
  static char Notice[]="Copyright (C) 1985 Adobe Systems Incorporated";
  char *_Version_ = "A/UX Release 1.0";
  char *_Origin_ = "Transcript release 2.0";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#ifndef lint
#define _AC_MODS
static char *RCSID="$Header: psbanner.sysv,v 2.1 85/11/24 11:49:54 shore Rel $";
#endif
/* psbanner.c
 *
 * Copyright (c) 1985 Adobe Systems Incorporated
 *
 * System V banner/breakpage program 
 *
 * RCSLOG:
 * $Log:	psbanner.sysv,v $
 * Revision 2.1  85/11/24  11:49:54  shore
 * Product Release 2.0
 * 
 * Revision 1.1  85/11/20  00:25:33  shore
 * Initial revision
 * 
 *
 */

#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include <ctype.h>
#include "transcript.h"

#define _AC_MODS
#define _AC_MODS

struct passwd *getpwnam();
VOID quote();

/* psbanner
 * 	gets called with argv:
 *	printer seqid user title date
 */
main(argc, argv)
int argc;
char **argv;
{
    struct passwd *pswd;
    char *program, *bannerpro, *fulluser;
    char *printer, *seqid, *user, *title, *date;
    char host[100];

    program = strrchr(*argv,'/');
    if (program) program++;
    else program = *argv;
    argv++;

    printer = *argv++;
    seqid = *argv++;
    user = *argv++;
    title = *argv++;
    date = *argv++;

    if ((pswd = getpwnam(user)) == NULL) fulluser = "";
    else fulluser = pswd->pw_gecos;
    gethostname(host,100);

    bannerpro = envget("BANNERPRO");
    copyfile(bannerpro,stdout);

    quote(user);
    quote(fulluser);
    quote(host);
    quote(seqid);
    quote(title);
    quote(printer);
    quote(date);
    printf("Banner\n");
    return(0);

}

/* send PostScript delimited/quoted string to stdout */
VOID quote(str)
char *str;
{
    int c;
    putchar('(');
    while ((c = ((*str++) & 0377)) != 0) {
	if (isascii(c) && (isprint(c) || isspace(c))) {
	    if ((c == '(') || (c == ')') || (c =='\\')) {
		putchar('\\');
	    }
	    putchar(c);
	}
	else {
	    putchar('\\');
	    putchar(((c>>6)&03)+'0');
	    putchar(((c>>3)&07)+'0');
	    putchar((c&07)+'0');
	}
    }
    putchar(')');
    putchar('\n');
}
