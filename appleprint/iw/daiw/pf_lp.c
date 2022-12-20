#ifndef lint	/* .../appleprint/iw/daiw/pf_lp.c */
#define _AC_NAME pf_lp_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:52}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _pf_lp_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of pf_lp.c on 87/11/11 21:44:52";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"

static  void	 change_font ();
static  void	 change_size ();
static  void	 out_char ();
static  int	 is_special ();
static  int	 lookup_special ();
static  int	 lookup_regular ();
static  int	 lookup_char ();

void
pf_lp (ofile,cc)
    register FILE  *ofile;
    register CC    *cc;
{
    register int    font = -1;
    register int    size = -1;

    while (cc) {
    	if (sw_debug[4]) {
	    dbg_cc1(cc);
	}
	MoveTo (cc->cc_horz, cc->cc_vert);
	if (font != cc->cc_font) {
	    change_font (cc->cc_font);
	    font = cc->cc_font;
	}
	if (size != cc->cc_ptsz) {
	    change_size (cc->cc_ptsz);
	    size = cc->cc_ptsz;
	}
	
	out_char (cc->cc_font, cc->cc_char);
	
	cc = cc->cc_next;
    }

    print_page (iw2, gp);
    clear_page (iw2, gp);
    
}

static  void
change_font (font)
{
    int		font_num;
    int		font_style;

    sscanf (font_pos[font].fp_font->intname, "%d,%d", &font_num, &font_style);
#ifdef notdef
fprintf(stderr,"pf_lp:intname=%s\n", font_pos[font].fp_font->intname);
fprintf(stderr, "pf_lp:change_font %d style %d\n", font_num, font_style);    
#endif
    TextFont (font_num);
    TextFace (font_style);
}

static  void
change_size (size)
{
    if (parm_res == 144)
	TextSize (size * 2);
    else
	TextSize (size);
}    

static  void
out_char (current_font, ch)
    char	*ch;
{
    int		 index;
    int		 font;
    
    if (*ch == 32)
	return;
    if (is_special (ch))
	index = lookup_special (&font, current_font, ch);
    else
	index = lookup_regular (&font, current_font, ch);
    if (index == 0) {
	if (is_special (ch))
	    softerr (200, "%s undefined in font %d", ch, current_font);
	else
	    softerr (201, "%c(%x) undefined in font %d", *ch, *ch, current_font);
    } else {
	if (codes[font][index] != 0) {
	    if (current_font != font)
		change_font (font);
	    DrawChar (codes[font][index]);
	    if (current_font != font)
		change_font (current_font);
	}
    }
}

static int
is_special (char_name)
    char	*char_name;
{
    return strlen (char_name) > 1;
}

static int
lookup_special (found_font, current_font, char_name)
    int		*found_font;
    char	*char_name;
{
    int		 ix;

    for (ix = 0; ix < dev.nchtab; ix++) {
	if (strcmp (char_name, &ch_name[ch_tab[ix]]) == 0)
		break;
    }
    if (ix == dev.nchtab)
	return 0;
    return lookup_char (found_font, current_font, char_name, ix + 128);
}

static int
lookup_regular (found_font, current_font, char_name)
    int		*found_font;
    char	*char_name;
{
    return lookup_char (found_font, current_font, char_name, *char_name);
}

static int
lookup_char (found_font, current_font, char_name, code)
    int		*found_font;
    char	*char_name;
{
    int		 ch;
    int		 c;
    int		 ix;

    *found_font = current_font;
    ch = code - 32;
    if (ch < 0)
	return 0;
    /*
     * does this character exist on the current font?
     */
    if ((c = fitab[current_font][ch]) != 0)
	return c;
    /*
     * nope, look through the special fonts
     */
    for (ix = 1; ix <= dev.nfonts; ix++) {
	if (fonts[ix]->specfont) {
	    c = fitab[ix][ch];
	    if (c != 0) {
		*found_font = ix;
		return c;
	    }
	}
    }
    /* not on any font */
    return 0;
}
