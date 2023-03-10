#ifndef lint	/* .../appleprint/dwb/text/tbl.d/tc.c */
#define _AC_NAME tc_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 22:14:38}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987	Version 1.2 of tc.c on 87/11/11 22:14:38";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
 /* tc.c: find character not in table to delimit fields */
# include "t..c"
choochar()
{
/* choose funny characters to delimit fields */
int had[128], ilin,icol, k;
char *s;
for(icol=0; icol<128; icol++)
	had[icol]=0;
F1 = F2 = 0;
for(ilin=0;ilin<nlin;ilin++)
	{
	if (instead[ilin]) continue;
	if (fullbot[ilin]) continue;
	for(icol=0; icol<ncol; icol++)
		{
		k = ctype(ilin, icol);
		if (k==0 || k == '-' || k == '=')
			continue;
		s = table[ilin][icol].col;
		if (point(s))
		while (*s)
			had[*s++]=1;
		s=table[ilin][icol].rcol;
		if (point(s))
		while (*s)
			had[*s++]=1;
		}
	}
/* choose first funny character */
for(
	s="\002\003\005\006\007!%&#/?,:;<=>@`^~_{}+-*ABCDEFGHIJKMNOPQRSTUVWXYZabcdefgjkoqrstwxyz";
		*s; s++)
	{
	if (had[*s]==0)
		{
		F1= *s;
		had[F1]=1;
		break;
		}
	}
/* choose second funny character */
for(
	s="\002\003\005\006\007:_~^`@;,<=>#%&!/?{}+-*ABCDEFGHIJKMNOPQRSTUVWXZabcdefgjkoqrstuwxyz";
		*s; s++)
	{
	if (had[*s]==0)
		{
		F2= *s;
		break;
		}
	}
if (F1==0 || F2==0)
	error("couldn't find characters to use for delimiters");
return;
}
point(s)
{
return(s>= 128 || s<0);
}
