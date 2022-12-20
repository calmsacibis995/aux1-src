#ifndef lint	/* .../appleprint/dwb/text/tbl.d/t3.c */
#define _AC_NAME t3_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:14:08}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of t3.c on 87/11/11 22:14:08";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
 /* t3.c: interpret commands affecting whole table */
# include "t..c"
struct optstr {char *optnam; int *optadd;} options [] = {
	"expand", &expflg,
	"EXPAND", &expflg,
	"center", &ctrflg,
	"CENTER", &ctrflg,
	"box", &boxflg,
	"BOX", &boxflg,
	"allbox", &allflg,
	"ALLBOX", &allflg,
	"doublebox", &dboxflg,
	"DOUBLEBOX", &dboxflg,
	"frame", &boxflg,
	"FRAME", &boxflg,
	"doubleframe", &dboxflg,
	"DOUBLEFRAME", &dboxflg,
	"tab", &tab,
	"TAB", &tab,
	"linesize", &linsize,
	"LINESIZE", &linsize,
	"delim", &delim1,
	"DELIM", &delim1,
	0,0};
getcomm()
{
char line[200], *cp, nb[25], *t;
struct optstr *lp;
int c, ci, found;
for(lp= options; lp->optnam; lp++)
	*(lp->optadd) = 0;
texname = texstr[texct=0];
tab = '\t';
printf(".nr %d \\n(.s\n", LSIZE);
gets1(line);
/* see if this is a command line */
if (strchr(line,';') == NULL)
	{
	backrest(line);
	return;
	}
for(cp=line; (c = *cp) != ';'; cp++)
	{
	if (!letter(c)) continue;
	found=0;
	for(lp= options; lp->optadd; lp++)
		{
		if (prefix(lp->optnam, cp))
			{
			*(lp->optadd) = 1;
			cp += strlen(lp->optnam);
			if (letter(*cp))
				error("Misspelled global option");
			while (*cp==' ')cp++;
			t=nb;
			if ( *cp == '(')
				while ((ci= *++cp) != ')')
					*t++ = ci;
			else cp--;
			*t++ = 0; *t=0;
			if (lp->optadd == &tab)
				{
				if (nb[0])
					*(lp->optadd) = nb[0];
				}
			if (lp->optadd == &linsize)
				printf(".nr %d %s\n", LSIZE, nb);
			if (lp->optadd == &delim1)
				{
				delim1 = nb[0];
				delim2 = nb[1];
				}
			found=1;
			break;
			}
		}
	if (!found)
		error("Illegal option");
	}
cp++;
backrest(cp);
return;
}
backrest(cp)
	char *cp;
{
char *s;
for(s=cp; *s; s++);
un1getc('\n');
while (s>cp)
	un1getc(*--s);
return;
}
