#ifndef lint	/* .../psutil.c */
#define _AC_NAME psutil_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985 Adobe Systems Incorporated, All Rights Reserved.  {Apple version 1.2 87/11/11 21:48:43}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char ___src_psutil_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of psutil.c on 87/11/11 21:48:43";
  static char Notice[] = "Copyright (c) 1985 Adobe Systems Incorporated";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#ifndef lint
#define _AC_MODS
static char *RCSID="$Header: psutil.c,v 2.1 85/11/24 11:51:18 shore Rel $";
#endif
/* psutil.c
 *
 * Copyright (c) 1985 Adobe Systems Incorporated
 *
 * common utility subroutines
 *
 * RCSLOG:
 * $Log:	psutil.c,v $
 * Revision 2.1  85/11/24  11:51:18  shore
 * Product Release 2.0
 * 
 * Revision 1.3  85/11/20  00:57:02  shore
 * Support for System V
 * fixed bug in copyfile
 * added envget and Sys V gethostname
 * 
 * Revision 1.2  85/05/14  11:28:42  shore
 * 
 * 
 *
 */

#include <stdio.h>
#ifdef SYSV
#include <string.h>
#include <fcntl.h>
#else
#include <strings.h>
#include <sys/file.h>
#endif
#include "transcript.h"

extern char *getenv();

/* copy from file named fn to stream stm */
/* use read and write in hopes that it goes fast */
copyfile(fn, stm)
	char *fn;
	register FILE *stm;
{
    	int fd, fo;
	register int pcnt;
	char buf[BUFSIZ];

	VOIDC fflush(stm);
	fo = fileno(stm);
	if ((fd = open(fn, O_RDONLY, 0)) < 0) return(-1);
	while ((pcnt = read(fd, buf, sizeof buf)) > 0) {
	      if (write(fo, buf, (unsigned) pcnt) != pcnt) return(-2);
	}
	if (pcnt < 0) {perror("copyfile"); return (-1);}
	VOIDC close(fd);
	VOIDC fflush(stm);
	return(0);
}

/* exit with message and code */
VOID pexit(message, code)
char *message;
int code;
{
    perror(message);
    exit(code);
}
/* exit with message and code */
VOID pexit2(prog, message, code)
char *prog, *message;
int code;
{
    fprintf(stderr,"%s: ",prog);
    VOIDC perror(message);
    VOIDC exit(code);
}

/* concatenate s1 and s2 into s0 (of length l0), die if too long
 * returns a pointer to the null-terminated result
 */
char *mstrcat(s0,s1,s2,l0)
char *s0,*s1,*s2;
int	l0;
{
    if ((strlen(s1) + strlen(s2)) >= l0) {
	fprintf(stderr,"concatenate overflow %s%s\n",s1,s2);
	VOIDC exit(2);
    }
    return strcat(((s0 == s1) ? s0 : strcpy(s0,s1)),s2);
}

/* envget is a getenv
 * 	if the variable is not present in the environment or
 *	it has the null string as value envget returns NULL
 *	otherwise it returns the value from the environment
 */

char *envget(var)
char *var;
{
    register char *val;
    if (((val = getenv(var)) == NULL) || (*val == '\0'))
    	return ((char *) NULL);
    else return (val);
}


/* System V specific compatibility stuff */

#ifdef SYSV

#include <sys/types.h>
#include <sys/utsname.h>

#define SYSVNAMELIMIT 9

gethostname(name, namelen)
char *name;
int namelen;
{
    struct utsname uts;
    uname(&uts);
    VOIDC strncpy(name,uts.sysname,SYSVNAMELIMIT);
    return(0);
}

#endif
