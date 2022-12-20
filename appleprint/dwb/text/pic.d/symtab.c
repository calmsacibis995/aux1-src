#ifndef lint	/* .../appleprint/dwb/text/pic.d/symtab.c */
#define _AC_NAME symtab_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:57:42}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of symtab.c on 87/11/11 21:57:42";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#include	<stdio.h>
#include	<ctype.h>
#include	"pic.h"
#include	"y.tab.h"

YYSTYPE getvar(s)	/* return value of variable s (usually pointer) */
	char *s;
{
	struct symtab *p;
	static YYSTYPE bug;

	p = lookup(s);
	if (p == NULL) {
		if (islower(s[0]))
			yyerror("no such variable as %s", s);
		else
			yyerror("no such place as %s", s);
		return(bug);
	}
	return(p->s_val);
}

float getfval(s)	/* return float value of variable s */
	char *s;
{
	YYSTYPE y;

	y = getvar(s);
	return y.f;
}

setfval(s, f)	/* set variable s to f */
	char *s;
	float f;
{
	struct symtab *p;

	if ((p = lookup(s)) != NULL)
		p->s_val.f = f;
}

struct symtab *makevar(s, t, v)	/* make variable named s in table */
	char *s;		/* assumes s is static or from tostring */
	int t;
	YYSTYPE v;
{
	int i;
	struct symtab *p;

	for (p = stack[nstack].p_symtab; p != NULL; p = p->s_next)
		if (strcmp(s, p->s_name) == 0)
			break;
	if (p == NULL) {	/* it's a new one */
		p = (struct symtab *) malloc(sizeof(struct symtab));
		if (p == NULL) {
			yyerror("out of symtab space with %s", s);
			exit(1);
		}
		p->s_next = stack[nstack].p_symtab;
		stack[nstack].p_symtab = p;	/* stick it at front */
	}
	p->s_name = s;
	p->s_type = t;
	p->s_val = v;
	return(p);
}

struct symtab *lookup(s)	/* find s in symtab */
	char *s;
{
	int i;
	struct symtab *p;

	for (i = nstack; i >= 0; i--)	/* look in each active symtab */
		for (p = stack[i].p_symtab; p != NULL; p = p->s_next)
			if (strcmp(s, p->s_name) == 0)
				return(p);
	return(NULL);
}

freesymtab(p)	/* free space used by symtab at p */
	struct symtab *p;
{
	for ( ; p != NULL; p = p->s_next) {
		free(p->s_name);	/* assumes done with tostring */
		free(p);
	}
}
