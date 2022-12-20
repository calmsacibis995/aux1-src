#ifndef lint	/* .../appleprint/iw/iwprep/wr_desc.c */
#define _AC_NAME wr_desc_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:40}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _wr_desc_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of wr_desc.c on 87/11/11 21:42:40";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include <stdio.h>

void        write_fonts();
void        write_sizes();
void        write_charset();

write_DESC()
{
    FILE        *fp;
    
    if ((fp = fopen ("DESC", "w")) == NULL) {
	fprintf (stderr, "Can't write DESC file\n");
    } else {
	fprintf (fp, "#\n");
	fprintf (fp, "# ditroff DESC file for %s\n", prep_desc.pd_device);
	fprintf (fp, "#\n");
	write_fonts (fp);
	write_sizes (fp);
	fprintf (fp, "res %d\n", prep_desc.pd_res);
	fprintf (fp, "hor %d\n", prep_desc.pd_hor);
	fprintf (fp, "vert %d\n", prep_desc.pd_vert);
	fprintf (fp, "unitwidth %d\n", prep_desc.pd_unitwidth);
	fprintf (fp, "paperlength %d\n", prep_desc.pd_paperlength);
	fprintf (fp, "paperwidth %d\n", prep_desc.pd_paperwidth);
	write_charset (fp);
	fclose (fp);
    }
}

static void
write_fonts (fp)
    FILE        *fp;
{
    struct font_desc *fd;
    int               count;
    
    count = 0;
    for (fd = prep_desc.pd_fontdesc; fd != NULL; fd = fd->fd_next)
	count++;
    fprintf (fp, "fonts %d", count);
    for (fd = prep_desc.pd_fontdesc; fd != NULL; fd = fd->fd_next)
	fprintf (fp, " %s", fd->fd_troff_name);
    fprintf (fp, "\n");
}

static void
write_sizes (fp)
    FILE        *fp;
{
    int          i;
    
    fprintf (fp, "sizes");
    for (i = 0; i < size_index; i++)
	fprintf (fp, " %d", sizes[i]);
    fprintf (fp, " 0\n");
}

static void
write_charset (fp)
    FILE        *fp;
{
    int         i;

    fprintf (fp, "charset\n");
    for (i = 0; i < sp_index; i++) {
	fprintf (fp, "%-3s", specials[i]);
	if ((i + 1) % 25 == 0)
	    fprintf (fp, "\n");
    }
    fprintf (fp, "\n");
}
