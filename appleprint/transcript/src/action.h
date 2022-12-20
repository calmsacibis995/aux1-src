#ifndef lint	/* .../action.h */
#define _AC_NAME action_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985 Adobe Systems Incorporated, All Rights Reserved.  {Apple version 1.2 87/11/11 21:45:52}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char ___src_action_h[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of action.h on 87/11/11 21:45:52";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#
/* action.h
 *
 * Copyright (c) 1985 Adobe Systems Incorporated
 *
 * action table types for pscat/pscatmap/ptroff character mapping
 *
 * RCSID: $Header: action.h,v 2.1 85/11/24 12:07:51 shore Rel $
 *
 * Edit History:
 * Andrew Shore: Tue May 14 10:09:10 1985
 * End Edit History.
 *
 * RCSLOG:
 * $Log:	action.h,v $
 * Revision 2.1  85/11/24  12:07:51  shore
 * Product Release 2.0
 * 
 * Revision 1.2  85/05/14  11:21:08  shore
 * 
 * 
 *
 */

#define PFONT 1
#define PLIG 2
#define PPROC 3
#define PNONE 4

struct chAction {
    unsigned char	actCode;
    char *		actName;
};


struct map {
	int	wid;
	int	x,y;
	int	font;
	int	pschar;
	int	action;
	int	pswidth;
};
