#ifndef lint	/* .../appleprint/iw/daiw/dev_init.c */
#define _AC_NAME dev_init_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:43:27}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _dev_init_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of dev_init.c on 87/11/11 21:43:27";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"

static int read_troff_desc ();
static int read_device_desc ();
static int read_desc ();
static int load_desc ();
static int load_dev_desc ();

/*
 * dev_init (device)
 *
 * loads troff description file for typesetter 'name'
 */
void
dev_init (name)
    char	*name;
{
    if (read_troff_desc (name) == 0)
	open_device ();
    else
	harderr (216, "can't load %s description file", name);
}
    
static int
read_troff_desc (device)
    char	*device;
{
    char	 full_path[256];
    FILE	*fp;
    int		 status;

    sprintf (full_path, "%s/dev%s/DESC.out", TROFF_ROOT, device);
    if ((fp = fopen (full_path, "r")) == NULL) {
	softerr (217, "Can't open %s\n", full_path);
	return -1;
    } else {
	status = load_desc (fp);
	fclose (fp);
	return status;
    }
}
    
static int
load_desc (fp)
    FILE	*fp;
{
    register u_char	*p;
    register int	 i;
    register int	 n_widths;

    if (read (fileno (fp), &dev, sizeof (struct dev)) != sizeof (struct dev)) {
	softerr (218, "Can't read device header\n");
	return -1;
    } else if ((dev_data = (u_char *) malloc (dev.filesize)) == NULL) {
	softerr (219, "Can't allocate enough memory in load_desc\n");
	return -1;
    } else if (read (fileno (fp), dev_data, dev.filesize) != dev.filesize) {
	softerr (220, "Can't read in device description\n");
	return -1;
    } else {
	sizes = (u_short *) dev_data;
	ch_tab = sizes + dev.nsizes + 1;
	ch_name = (u_char *) (ch_tab + dev.nchtab);
	p = ch_name + dev.lchname;
	for (i = 1; i <= dev.nfonts; i++) {
	    fonts[i] = (struct font *) p; p += sizeof (struct font);
	    n_widths = (fonts[i]->nwfont & 0xff);
	    widths[i] = p;   p += n_widths;
	    kerning[i] = p;  p += n_widths;
	    codes[i] = p;    p += n_widths;
	    fitab[i] = p;    p += 96 + dev.nchtab;
	}

#ifdef DEBUG
	show_desc();
#endif	
	return 0;
    }
}

#ifdef DEBUG
show_desc()
{
    int		i;
    int		nw;
    int		j;

    fprintf (stderr, "res %d\n", dev.res);
    fprintf (stderr, "hor %d\n", dev.hor);
    fprintf (stderr, "vert %d\n", dev.vert);
    fprintf (stderr, "unitwidth %d\n", dev.unitwidth);
    fprintf (stderr, "nfonts %d\n", dev.nfonts);
    fprintf (stderr, "nsizes %d\n", dev.nsizes);
    fprintf (stderr, "sizescale %d\n", dev.sizescale);
    fprintf (stderr, "paperwidth %d\n", dev.paperwidth);
    fprintf (stderr, "paperlength %d\n", dev.paperlength);
    fprintf (stderr, "nchtab %d\n", dev.nchtab);
    fprintf (stderr, "lchname %d\n", dev.lchname);

    fprintf (stderr, "sizes ");
    for (i = 0; i < dev.nsizes; i++)
	fprintf (stderr, "%3d", sizes[i]);
    fprintf (stderr, "\n");
    
    fprintf (stderr, "special chars\n");
    for (i = 0; i < dev.nchtab; i++) {
	fprintf (stderr, "%-5s", &ch_name[ch_tab[i]]);
	if ((i + 1) % 16 == 0)
	    fprintf (stderr, "\n");
    }
    fprintf (stderr, "\n");

    for (i = 1; i <= dev.nfonts; i++) {
	fprintf (stderr, "\nfont %d\n", i);
	fprintf (stderr, "nwfont %d\n", fonts[i]->nwfont);
	fprintf (stderr, "specfont %d\n", fonts[i]->specfont);
	fprintf (stderr, "ligfont %d\n", fonts[i]->ligfont);
	fprintf (stderr, "namefont %10.10s\n", fonts[i]->namefont);
	fprintf (stderr, "intname %10.10s\n", fonts[i]->intname);
	nw = fonts[i]->nwfont;
	fprintf (stderr, "widths\n");
	for (j = 0; j < nw; j++) {
	    fprintf (stderr, "%5d", widths[i][j]);
	    if ((j + 1) % 16 == 0)
		fprintf (stderr, "\n");

	}
	fprintf (stderr, "\n");
	fprintf (stderr, "kerning\n");
	for (j = 0; j < nw; j++) {
	    fprintf (stderr, "%5d", kerning[i][j]);
	    if ((j + 1) % 16 == 0)
		fprintf (stderr, "\n");
	}
	fprintf (stderr, "\n");
	fprintf (stderr, "code\n");
	for (j = 0; j < nw; j++) {
	    fprintf (stderr, "%5d", codes[i][j]);
	    if ((j + 1) % 16 == 0)
		fprintf (stderr, "\n");
	}
	fprintf (stderr, "\n");
	fprintf (stderr, "index table\n");
	for (j = 0; j < 96 + dev.nchtab; j++) {
	    fprintf (stderr, "%5d", fitab[i][j]);
	    if ((j + 1) % 16 == 0)
		fprintf (stderr, "\n");
	}
	fprintf (stderr, "\n");
    }
}
#endif
