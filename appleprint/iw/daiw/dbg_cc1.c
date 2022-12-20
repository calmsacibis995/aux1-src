#ifndef lint	/* .../appleprint/iw/daiw/dbg_cc1.c */
#define _AC_NAME dbg_cc1_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:24}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _dbg_cc1_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of dbg_cc1.c on 87/11/11 21:43:24";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	dbg_cc1(cc)
 *
 *  Arguments:
 *	cc	A pointer to a CC structure.
 *
 *  Description:
 *	This function prints out the contents of a CC structure. It is
 *	used by the built-in debugging code.
 *
 *  Algorithm:
 *	Print it.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void dbg_cc1(cc)
	register CC    *cc;
	{

	(void) fprintf(stderr,
                       "%08x -- p:%08x n:%08x f:%02d pt:%03d v:%05d h:%05d c:%s\n",
	               cc,
                       cc->cc_prev,cc->cc_next,cc->cc_font,cc->cc_ptsz,
                       cc->cc_vert,cc->cc_horz,cc->cc_char);
	}
