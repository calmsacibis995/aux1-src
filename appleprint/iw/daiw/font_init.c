#ifndef lint	/* .../appleprint/iw/daiw/font_init.c */
#define _AC_NAME font_init_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:25}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _font_init_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of font_init.c on 87/11/11 21:44:25";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "local.h"

static int load_mac_fonts ();
static int load_res_file ();
static int open_res_file ();

struct open_res {
    char		*res_name;
    struct open_res	*res_next;
};

static struct open_res  *open_res_list;

void
font_init (name)
    char	*name;
{
    if (load_mac_fonts (name) != 0)
	harderr (211, "can't load Macintosh fonts");
}


static int
load_mac_fonts (device)
    char	*device;
{
    char	 full_path[256];
    char	 buf[512];
    FILE	*fp;
    int		 ac;
    char	*av[32];

    sprintf (full_path, "%s/dev%s/%s.desc", TROFF_ROOT, device, device);
    if ((fp = fopen (full_path, "r")) == NULL) {
	softerr (212, "Can't open %s\n", full_path);
	return -1;
    } else {
	while (fgets (buf, sizeof buf, fp) != NULL) {
	    buf[strlen(buf)] = '\0';
	    ac = line2av (buf, av);
	    if (ac > 0 && strcmp (av[0], "font") == 0)
		if (load_res_file (av[5]) != 0)
		    return -1;
	}
	fclose (fp);
    }
    return 0;
}

static int
load_res_file (name)
    char		*name;
{
    struct open_res	*or;

    for (or = open_res_list; or != NULL; or = or->res_next) {
	if (strcmp (or->res_name, name) == 0)
	    return 0;
    }
    /* not in the list, lets add it */
    if (open_res_file (name) == -1) {
	fprintf (stderr, "Can't open font resource file %s\n", name);
	return -1;
    } else {
	or = NEW (struct open_res);
	or->res_name = strsave (name);
	or->res_next = open_res_list;
	open_res_list = or;
    }
    return 0;
}

static int
open_res_file (name)
    char		*name;
{
    char		 fullname[256];
    short		 refnum;

    if ((refnum = OpenResFile (name)) == -1) {
	sprintf (fullname, "%s/dev%s/fonts/%s", TROFF_ROOT, x_T_s, name);
	if ((refnum = OpenResFile (fullname)) == -1) {
	    sprintf (fullname, "%s/dev%s/%s", TROFF_ROOT, x_T_s, name);
	    refnum = OpenResFile (fullname);
	}
    }
    return refnum;
}
    
