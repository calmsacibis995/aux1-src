#ifndef lint	/* .../appleprint/dwb/text/grap.d/for.c */
#define _AC_NAME for_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:52:56}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of for.c on 87/11/11 21:52:56";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS

#include <stdio.h>
#include "grap.h"
#include "y.tab.h"

typedef struct {
	Obj	*var;	/* index variable */
	double	to;	/* limit */
	double	by;
	int	op;	/* operator */
	char	*str;	/* string to push back */
} For;

#define	MAXFOR	10

For	forstk[MAXFOR];	/* stack of for loops */
For	*forp = forstk;	/* pointer to current top */

forloop(var, from, to, op, by, str)	/* set up a for loop */
	Obj *var;
	double from, to, by;
	int op;
	char *str;
{
	fprintf(tfd, "# for %s from %g to %g by %c %g \n",
		var->name, from, to, op, by);
	if (++forp >= forstk+MAXFOR)
		fatal("for loop nested too deep");
	forp->var = var;
	forp->to = to;
	forp->op = op;
	forp->by = by;
	forp->str = str;
	setvar(var, from);
	nextfor();
	unput('\n');
}

nextfor()	/* do one iteration of a for loop */
{
	/* BUG:  this should depend on op and direction */
	if (forp->var->fval > SLOP * forp->to) {	/* loop is done */
		free(forp->str);
		if (--forp < forstk)
			fatal("forstk popped too far");
	} else {		/* another iteration */
		pushsrc(String, "\nEndfor\n");
		pushsrc(String, forp->str);
	}
}

endfor()	/* end one iteration of for loop */
{
	switch (forp->op) {
	case '+':
	case ' ':
		forp->var->fval += forp->by;
		break;
	case '-':
		forp->var->fval -= forp->by;
		break;
	case '*':
		forp->var->fval *= forp->by;
		break;
	case '/':
		forp->var->fval /= forp->by;
		break;
	}
	nextfor();
}

char *ifstat(expr, thenpart, elsepart)
	double expr;
	char *thenpart, *elsepart;
{
	dprintf("if %g then <%s> else <%s>\n", expr, thenpart, elsepart? elsepart : "");
	if (expr) {
		unput('\n');
		pushsrc(Free, thenpart);
		pushsrc(String, thenpart);
		unput('\n');
  		if (elsepart)
			free(elsepart);
		return thenpart;	/* to be freed later */
	} else {
		free(thenpart);
		if (elsepart) {
			unput('\n');
			pushsrc(Free, elsepart);
			pushsrc(String, elsepart);
			unput('\n');
		}
		return elsepart;
	}
}
