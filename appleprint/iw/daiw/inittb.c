#ifndef lint	/* .../appleprint/iw/daiw/inittb.c */
#define _AC_NAME inittb_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:31}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _inittb_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of inittb.c on 87/11/11 21:44:31";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include <mac/types.h>
#include <mac/quickdraw.h>
#include <mac/fonts.h>

/* inittb
 *
 * initializes the Macintosh toolbox
 */

inittb ()
{
    InitGraf(&qd.thePort);
    InitFonts();
}
    
