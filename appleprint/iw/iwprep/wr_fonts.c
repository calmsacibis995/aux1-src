#ifndef lint	/* .../appleprint/iw/iwprep/wr_fonts.c */
#define _AC_NAME wr_fonts_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:43}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _wr_fonts_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of wr_fonts.c on 87/11/11 21:42:43";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include <stdio.h>

#include <mac/types.h>
#include <mac/fonts.h>

void        put_1font();
void        put_charset();
void        put_entry();
void        put_1entry();
void        put_1line();
void        put_specials();
void	    put_ligatures();
int	    ligature();
short       width_of();
short       ascender_of();
void        get_image_height();

static char *lig_table[] = { "ff", "fi", "fl", "ffi", "ffl", };
#define NUM_LIGATURES (sizeof lig_table / sizeof lig_table[0])

write_fonts()
{
    struct font_desc    *fd;
    
    for (fd = prep_desc.pd_fontdesc; fd != NULL; fd = fd->fd_next)
	put_1font (fd);
}

static void
put_1font (fd)
    struct font_desc    *fd;
{
    FILE                *fp;
    
    if ((fp = fopen (fd->fd_troff_name, "w")) == NULL)
	fprintf (stderr, "Can't create font file %s\n", fd->fd_troff_name);
    else {
	fprintf (fp, "name %s\n", fd->fd_troff_name);
	fprintf (fp, "internalname %d,%d\n",
		    fd->fd_mac_font, fd->fd_mac_style);
	if (fd->fd_type == FONT_SPECIAL)
	    fprintf (fp, "special\n");
	put_ligatures (fp, fd);
	put_charset (fp, fd);
	fclose (fp);
    }
}

static void
put_charset (fp, fd)
    FILE                *fp;
    struct font_desc    *fd;
{
    struct map_desc     *md;
    int                 i;
    
    fprintf (fp, "charset\n");
    put_specials (fp, fd);
    md = fd->fd_map;
    for (i = 0; i < NMAPENTRY; i++) {
	put_entry (fp, fd, &md->md_entry[i]);
    }
}

static void
put_entry (fp, fd, me)
    FILE                *fp;
    struct font_desc    *fd;
    struct map_entry    *me;
{
    struct char_alias   *ca;
    
    if (me->me_code != -1) {
	put_1entry (fp, fd, me->me_code, me->me_name);
	for (ca = me->me_alias; ca != NULL; ca = ca->ca_next)
	    put_1entry (fp, fd, me->me_code, ca->ca_name);
    }
}

static void
put_1entry (fp, fd, cd, name)
    FILE                *fp;
    struct font_desc    *fd;
    char                *name;
{
    short                width;
    short                ascender;
    
    width = width_of (fd, cd);
    ascender = ascender_of (fd, cd);
    put_1line (fp, name, width, ascender, cd);
}

static void
put_1line (fp, name, width, ascender, cd)
    FILE        *fp;
    char        *name;
    short        width;
    short        ascender;
{
    fprintf (fp, "%s\t%d\t%d\t0%o\n", name, width, ascender, cd);
}

static short
width_of (fd, cd)
    struct font_desc    *fd;
{
    short                width;
    
    TextFont (fd->fd_mac_font);
    TextFace (fd->fd_mac_style);
    if (prep_desc.pd_res == 144)
	TextSize (prep_desc.pd_unitwidth * 2);
    else
	TextSize (prep_desc.pd_unitwidth);
    width = CharWidth (cd);

#ifdef notdef
    TextFont (monaco);
    TextFace (0);
    TextSize (9);
#endif    
    return width;
}

static short
ascender_of (fd, cd)
    struct font_desc    *fd;
{
    short               a_start;
    short               a_length;
    short               cd_start;
    short               cd_length;
    short               ascender;
    
    get_image_height (fd, 'a', &a_start, &a_length);
    get_image_height (fd, cd, &cd_start, &cd_length);
    ascender = 0;
    if (cd_start < a_start)     /* we have an ascender */
	ascender += 2;
    if (cd_start + cd_length > a_start + a_length) /* is this a descender */
	ascender += 1;
    return ascender;
}

static void
get_image_height (fd, cd, start, length)
    struct font_desc    *fd;
    short               *start;
    short               *length;
{
    static struct font_desc *last_fd = NULL;
    static short            *ih = NULL;
    static FontRec          *font;
    FMInput                  the_input;
    FMOutPtr                 the_output;
    register short           ih_off;
    register short           incr;
    char                    *imh;
    short                    image_height;
    
    if (fd != last_fd) {
	the_input.family = fd->fd_mac_font;
	the_input.size  = prep_desc.pd_unitwidth;
	the_input.face = fd->fd_mac_style;
	the_input.needBits = true;
	the_input.device = 0;
	the_input.numer.h = 1;
	the_input.numer.v = 1;
	the_input.denom.h = 1;
	the_input.denom.v = 1;
	the_output = FMSwapFont (&the_input);
	font = (FontRec *) (*the_output->fontHandle);
	if (font->fontType & 0x1) {         /* has height table? */
	    ih_off = sizeof (FontRec);      /* offset to bitimage */
	    incr = (2 * font->rowWords  * font->fRectHeight);
	    ih_off += incr;                 /* offset to loctable */
	    incr = (2 * (font->lastChar - font->firstChar + 3));
	    ih_off += incr;                 /* offset to owtable */
	    ih_off += incr;                 /* offset to width table(if there)*/
	    if (font->fontType & 0x2)       /* has a width table ? */
		ih_off += incr;             /* skip width table */
	    imh = &((char *)font)[ih_off];
	    ih = (short *)imh;
	    last_fd = fd;
	}
    }
    image_height = ih[cd - font->firstChar];
    *start = ((image_height >> 8) & 0xff);
    *length = (image_height & 0xff);
}

static void
put_specials(fp, fd)
    FILE                *fp;
    struct font_desc    *fd;
{
    short               em_width;
    
    em_width = width_of (fd, 'm');
    put_1line (fp, "\\|", em_width / 6, 0, 0);
    put_1line (fp, "\\^", em_width / 12, 0, 0);
}

static void
put_ligatures (fp, fd)
    FILE		*fp;
    struct font_desc	*fd;
{
    struct map_desc     *md;
    struct map_entry	*me;
    struct char_alias	*ca;
    int                 i;
    int			label_written = 0;
    
    md = fd->fd_map;
    for (i = 0; i < NMAPENTRY; i++) {
	me = &md->md_entry[i];
	if (me->me_code != -1) {
	    if (ligature (me->me_name)) {
		if (label_written == 0) {
		    fprintf (fp, "ligature");
		    label_written = 1;
		}
		fprintf (fp, " %s", me->me_name);
	    }
	    for (ca = me->me_alias; ca != NULL; ca = ca->ca_next) {
		if (ligature (ca->ca_name)) {
		    if (label_written == 0) {
			fprintf (fp, "ligature");
			label_written = 1;
		    }
		    fprintf (fp, " %s", ca->ca_name);
		}
	    }
	}
    }
    if (label_written == 1)
	fprintf (fp, " 0\n");
}

static int
ligature (string)
    char	*string;
{
    int		 i;

    for (i = 0; i < NUM_LIGATURES; i++)
	if (strcmp (lig_table[i], string) == 0)
	    return 1;
    return 0;
}
