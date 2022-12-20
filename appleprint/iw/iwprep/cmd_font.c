#ifndef lint	/* .../appleprint/iw/iwprep/cmd_font.c */
#define _AC_NAME cmd_font_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:04}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _cmd_font_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of cmd_font.c on 87/11/11 21:42:04";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include "util.h"

#include <mac/types.h>
#include <mac/fonts.h>

short            font_type_of ();
short            font_style_of ();
short            font_num_of ();
struct map_desc *load_map();
struct map_desc *find_map();
void             add_map();
void             load_1map();
void             add_map_entry();
void             map_error();
void             add_font_desc ();
int              check_errors ();
void		 load_res_file ();
int		 open_res_file ();

#define          INT_ERROR_VAL      (-1)
#define          STRING_ERROR_VAL   0

#define          BOLD               1
#define          ITALIC             2

struct map_desc *map_desc_list = NULL;
static char            *map_filename;
static int              map_line;

struct open_res *open_res_list = NULL;

cmd_font (argc, argv)
    char                *argv[];
{
    struct font_desc    *fd;
    static int           index = 0;
    
    if (argc != 7) {
	error ("font: invalid specification");
	return 1;
    } else {
	fd = NEW(struct font_desc);
	load_res_file (argv[5]);
	fd->fd_index        = index;
	fd->fd_troff_name   = strsave (argv[1]);
	fd->fd_type         = font_type_of (argv[2]);
	fd->fd_mac_name     = strsave (argv[3]);
	fd->fd_mac_style    = font_style_of (argv[4]);
	fd->fd_mac_font     = font_num_of (argv[3]);
	fd->fd_mac_file     = strsave (argv[5]);
	fd->fd_map          = load_map (argv[6]);
	fd->fd_next         = NULL;
	index++;
	add_font_desc (fd);
	if (check_errors (fd) != 0)
	    return 1;
	else
	    return 0;
    }
}

static short    
font_type_of (string)
    char        *string;
{
    short        type;
    
    if (strcmp (string, "regular") == 0)
	type = FONT_REGULAR;
    else if (strcmp (string, "special") == 0)
	type = FONT_SPECIAL;
    else {
	error("font type must be 'regular' or 'special'");
	type = INT_ERROR_VAL;
    }
    return type;
}

static short    
font_style_of (string)
    char        *string;
{
    short        style;
    char         buf[128];
    int          argc;
    char        *argv[32];
    int          i;
    
    style = 0;
    strcpy (buf, string);
    argc = line2av (buf, argv);
    for (i = 0; i < argc; i++) {
	if (strcmp (argv[i], "plain") == 0)
	    style += 0;
	else if (strcmp (argv[i], "bold") == 0)
	    style += BOLD;
	else if (strcmp (argv[i], "italic") == 0)
	    style += ITALIC;
	else {
	    error("font:font style must be 'bold' or 'italic'");
	    style = INT_ERROR_VAL;
	    break;
	}
    }
    return style;
}

static short    
font_num_of (string)
    char        *string;
{
    char         font_name[255];
    short        font_num;
    
    strcpy (font_name, string);
/*  CtoPstr (font_name); */
    GetFNum (font_name, &font_num);
    if (font_num == 0) {
	error("font_name %s doesn't exist", string);
	font_num = INT_ERROR_VAL;
    }
    return font_num;
}

static struct map_desc *
load_map(string)
    char        *string;
{
    struct map_desc     *md;
    FILE                *fp;
    char                 buf[256];
    int                  argc;
    char                *argv[32];
    int                  i;
    
    if ((md = find_map (string)) == NULL) {
	md = NEW(struct map_desc);
	if ((fp = fopen (string, "r")) == NULL) {
	    error("map file %s doesn't exist", string);
	    md = NULL;
	} else {
	    md = NEW(struct map_desc);
	    for (i = 0; i < NMAPENTRY; i++)
		md->md_entry[i].me_code = -1;
	    md->md_name = strsave (string);
	    map_filename = string;
	    map_line = 0;
	    while (getline (fp, buf) != EOF) {
		map_line++;
		argc = line2av (buf, argv);
		if (argc > 0 && strcmp (argv[0], "#") != 0) /* skip comments */
		    load_1map (md, argc, argv);
	    }
	    fclose (fp);
	    add_map (md);
	}
    }
    return md;
}
static struct map_desc *
find_map (string)
    char            *string;
{
    struct map_desc *md;
    
    for (md = map_desc_list; md != NULL; md = md->md_next)
	if (strcmp (string, md->md_name) == 0)
	    return md;
    return NULL;
}

static void
add_map (md)
    struct map_desc *md;
{
    md->md_next = map_desc_list;
    map_desc_list = md;
}

static void
load_1map (md, argc, argv)
    struct map_desc     *md;
    char                *argv[];
{
    int                  code;
    int                  i;
    
    if (argc < 2) {
	map_error ("map entry requires a code and a troff name");
    } else {
	code = eatoi (argv[0]);
	if (code == INT_ERROR_VAL) 
	    map_error ("map:code must be non-negative");
	else {
	    for (i = 1; i < argc; i++)
		add_map_entry (md, code, argv[i]);
	}
    }
}
	
static void 
add_map_entry (md, cd, string)
    struct map_desc     *md;
    char                *string;
{
    struct map_entry    *me;
    struct char_alias   *ca;
    struct char_alias   *ap;
    
    if (cd < 0 || cd > NMAPENTRY) {
	map_error ("map:code must be between 0 and %d inclusive",
		    NMAPENTRY);
    } else {
	me = &md->md_entry[cd];
	if (me->me_code == -1) {            /* first definition */
	    me->me_code = cd;
	    me->me_name = strsave (string);
	    me->me_alias = NULL;
	} else {
	    ca = NEW(struct char_alias);
	    ca->ca_name = strsave (string);
	    ca->ca_next = NULL;
	    if (me->me_alias == NULL)
		me->me_alias = ca;
	    else {
		for (ap = me->me_alias; ap->ca_next != NULL; ap = ap->ca_next)
		    ;
		ap->ca_next = ca;
	    }
	}
    }
}

static void
add_font_desc (fd)
    struct font_desc    *fd;
{
    struct font_desc    *dp;
    
    if (prep_desc.pd_fontdesc == NULL)
	prep_desc.pd_fontdesc = fd;
    else {
	for (dp = prep_desc.pd_fontdesc; dp->fd_next != NULL; dp = dp->fd_next)
	    ;
	dp->fd_next = fd;
    }
}

static int
check_errors (fd)
    struct font_desc    *fd;
{
    return 0;
}

/*VARARGS*/
static void
map_error (fmt, a1, a2, a3, a4, a5)
    char        *fmt;
{
    fprintf (stderr,"%s:", map_filename);
    fprintf (stderr, fmt, a1, a2, a3, a4, a5);
    fprintf (stderr, "\n");
}


static void
load_res_file (name)
    char		*name;
{
    struct open_res	*or;

    for (or = open_res_list; or != NULL; or = or->res_next) {
	if (strcmp (or->res_name, name) == 0)
	    return;
    }
    /* not in the list, lets add it */
    if (open_res_file (name) == -1)
	fprintf (stderr, "Can't open font resource file %s\n", name);
    else {
	or = NEW (struct open_res);
	or->res_name = strsave (name);
	or->res_next = open_res_list;
	open_res_list = or;
    }
}

static int
open_res_file (name)
    char		*name;
{
    char		 fullname[256];
    short		 refnum;

    if ((refnum = OpenResFile (name)) == -1) {
	sprintf (fullname, "%s/dev%s/%s", LIBDIR, prep_desc.pd_device, name);
	if ((refnum = OpenResFile (fullname)) == -1) {
	    sprintf (fullname, "%s/dev%s/fonts/%s", LIBDIR, prep_desc.pd_device, name);
	    refnum = OpenResFile (fullname);
	}
    }
    return refnum;
}
