#ifndef lint	/* .../appleprint/iw/daiw/load_font.c */
#define _AC_NAME load_font_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:43}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _load_font_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of load_font.c on 87/11/11 21:44:43";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"

static struct font	*find_font();

void
load_font (num, name)
    char	*name;
{
    if (num > NFONT)
	harderr (207, "font number %d is greater than %d", num, NFONT);
    else if (font_pos[num].fp_name == 0 ||
	     strcmp (font_pos[num].fp_name, name) != 0) {
	font_pos[num].fp_index = num;
	font_pos[num].fp_name = strsave (name);
	font_pos[num].fp_font = find_font (name);
    }
}

static struct font *
find_font (name)
    char	*name;
{
    int		 i;

    for (i = 1; i <= NFONT; i++)
	if (strcmp (fonts[i]->namefont, name) == 0)
	    return fonts[i];
    harderr(208, "Can't find font %s", name);
    /*NORETURN*/
}
