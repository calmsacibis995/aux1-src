#ifndef lint	/* .../appleprint/dwb/text/tbl.d/te.c */
#define _AC_NAME te_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:14:41}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of te.c on 87/11/11 22:14:41";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
 /* te.c: error message control, input line count */
# include "t..c"
error(s)
	char *s;
{
fprintf(stderr, "\n%s: line %d: %s\n", ifile, iline, s);
# ifdef unix
fprintf(stderr, "tbl quits\n");
exit(1);
# endif
# ifdef gcos
fprintf(stderr, "run terminated due to error condition detected by tbl preprocessor\n");
exit(0);
# endif
}
char *gets1(s)
	char *s;
{
char *p;
int nbl = 0;
iline++;
p=fgets(s,512,tabin);
while (p==0)
	{
	if (swapin()==0)
		return(0);
	p = fgets(s,512,tabin);
	}

while (*s) s++;
s--;
if (*s == '\n') *s-- =0;
for(nbl=0; *s == '\\' && s>p; s--)
	nbl++;
if (linstart && nbl % 2) /* fold escaped nl if in table */
	gets1(s+1);

return(p);
}
# define BACKMAX 500
char backup[BACKMAX];
char *backp = backup;
un1getc(c)
{
if (c=='\n')
	iline--;
*backp++ = c;
if (backp >= backup+BACKMAX)
	error("too much backup");
}
get1char()
{
int c;
if (backp>backup)
	c = *--backp;
else
	c=getc(tabin);
if (c== EOF) /* EOF */
	{
	if (swapin() ==0)
		error("unexpected EOF");
	c = getc(tabin);
	}
if (c== '\n')
	iline++;
return(c);
}
