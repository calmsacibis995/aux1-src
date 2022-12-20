#ifndef lint	/* .../appleprint/iw/iwprep/prep.c */
#define _AC_NAME prep_c
#define _AC_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.5 87/11/11 21:42:34}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _prep_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.5 of prep.c on 87/11/11 21:42:34";
  char *_Version_ = "A/UX Release 1.0";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include <stdio.h>
#include <mac/types.h>
#include <mac/quickdraw.h>
#include "troffprep.h"

#define _AC_MODS

int                 line_no = 0;
int                 errors = 0;
struct prep_desc    prep_desc;

int		    noEvents = 1;
int		    noCD = 1;

main(argc, argv)
    char	*argv[];
{
    FILE        *fp;
    int          errors;
    GrafPort	 gp;
    
    InitGraf(&qd.thePort);
    InitFonts();
    OpenPort (&gp);
    if (argc != 2)
	fprintf (stderr, "usage:%s description_file\n", argv[0]);
    else if ((fp = fopen (argv[1], "r")) == NULL)
	fprintf (stderr, "Can't open %s\n", argv[1]);
    else {
	if (load_desc (fp) == 0)
	    process_desc ();
	fclose (fp);
    }
    exit (0);
}

load_desc (fp)
    FILE        *fp;
{
    char         buf[256];
    char        *argv[32];
    int          argc;
    int          errors = 0;
    
    while (getline (fp, buf) != EOF) {
	line_no++;
	argc = line2av (buf, argv);
	if (argc != 0) {
	    if (command (argc, argv) != 0) {
		errors++;
	    }
	}
    }
    return errors;
}

process_desc ()
{
    build_size ();
    build_charset ();
    write_DESC ();
    write_fonts ();
}

show_desc()
{
    int                  junk;
    struct font_desc    *fd;
    
    printf ("device      %s\n", prep_desc.pd_device);
    printf ("res         %d\n", prep_desc.pd_res);
    printf ("hor         %d\n", prep_desc.pd_hor);
    printf ("vert        %d\n", prep_desc.pd_vert);
    printf ("unitwidth   %d\n", prep_desc.pd_unitwidth);
    printf ("paperlength %d\n", prep_desc.pd_paperlength);
    printf ("paperwidth  %d\n", prep_desc.pd_paperwidth);
    printf ("hit carriage return to continue");
    junk = getchar();
    for (fd = prep_desc.pd_fontdesc; fd; fd = fd->fd_next) {
	show_font_desc(fd);
	printf ("hit carriage return to continue");
	junk = getchar();
    }
}

show_font_desc (fd)
    struct font_desc    *fd;
{
    printf ("index      %d\n", fd->fd_index);
    printf ("troff name %s\n", fd->fd_troff_name);
    printf ("type       %d\n", fd->fd_type);
    printf ("mac name   %s\n", fd->fd_mac_name);
    printf ("mac style  %d\n", fd->fd_mac_style);
    printf ("mac font   %d\n", fd->fd_mac_font);
    printf ("mac file   %s\n", fd->fd_mac_file);
    show_map_desc (fd->fd_map);
}

show_map_desc (md)
    struct map_desc     *md;
{
    int                  i;
    int                  junk;
    struct char_alias   *ap;
    
    printf ("map name %s\n", md->md_name);
    for (i = 0; i < NMAPENTRY; i++) {
	if (md->md_entry[i].me_code != -1) {
	    printf ("%4x    %4s", md->md_entry[i].me_code,
		    md->md_entry[i].me_name);
	    for (ap = md->md_entry[i].me_alias; ap; ap = ap->ca_next)
		printf ("%4s", ap->ca_name);
	    printf ("\n");
	}
	if (i % 16 == 0) {
	    printf ("hit cr to continue");
	    junk = getchar();
	}
    }
}

