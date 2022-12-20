#ifndef lint	/* .../appleprint/iw/iwprep/util.c */
#define _AC_NAME util_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:37}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _util_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of util.c on 87/11/11 21:42:37";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include <stdio.h>
#include "util.h"

/* util.c - miscellaneous utility routines
 *
 * getline(fp, buf) - read a line terminated by '\n' into buf
 *                    return EOF at end of file
 *
 * argc = line2av(buf, argv) - split buf into white space separated words
 *                             Strings are recognized by a beginning " or '
 *                             to allow embedded spaces.
 *                             returns number of words found
 * char *strsave (string) - saves a string in malloc'ed storage
 *
 * int = eatoi (string) - like atoi but knows octal and hexadecimal 
 */


getline (fp, buf)
    register FILE       *fp;
    char                *buf;
{
    register int         c;
    register char       *p = buf;
    
    while ((c = getc (fp)) != EOF) {
	if (c == '\n') {
	    *p++ = '\0';
	    return 0;
	}
	*p++ = c;
    }
    return EOF;
}

line2av (buf, argv)
    char                *buf;
    char                *argv[];
{
    register char       *p;
    register char       **av;
    int                  quote;
    
    p = buf;
    av = argv;
    while (*p) {
	while (*p == ' ' || *p == '\t')
	    p++;
	if (*p == '"' || *p == '\'') {
	    quote = *p++;
	    *av++ = p;
	    do
		p++;
	    while (*p && *p != quote);
	} else if (*p != '\0') {
	    *av++ = p;
	    while (*p && *p != ' ' && *p != '\t')
		p++;
	}
	if (*p == '\0')
	    return av - argv;
	*p++ = '\0';
    }
    return av - argv;
}

char *
strsave (string)
    char        *string;
{
    char        *retval;
    
    if ((retval = malloc (strlen (string) + 1)) == NULL)
	return NULL;
    strcpy (retval, string);
    return retval;
}

eatoi (string)
    char        *string;
{
    register int             value = 0;
    register int             c;
    register int             base;
    register char           *p;
	     int             sign = 1;
	     
    p = string;
    if (*p == '-') {
	sign = -1;
	p++;
    }
    if (*p == '0')
	if (p[1] == 'x') {
	    base = 16;
	    p += 2;                 /* skip the 0x */
	} else
	    base = 8;
    else
	base = 10;
    while (c = *p++) {
	if (base == 8) 
	    if ('0' <= c && c <= '7')
		value = value * base + c - '0';
	    else
		break;
	else if (base == 10)
	    if ('0' <= c && c <= '9')
		value = value * base + c - '0';
	    else
		break;
	else if (base == 16) {
	    if ('0' <= c && c <= '9')
		value = value * base + c - '0';
	    else {
		c = tolower (c);
		if ('a' <= c && c <= 'f')
		    value = value * base + 10 + c - 'a';
		else
		    break;
	    }
	}
    }
    if (c == 0)
	return value * sign;
    else
	return -1;
}

