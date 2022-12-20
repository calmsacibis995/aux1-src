#ifndef lint	/* .../appleprint/iw/iwprep/troffprep.h */
#define _AC_NAME troffprep_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:47}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _troffprep_h[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of troffprep.h on 87/11/11 21:42:47";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/* globals definitions for use by troffprep */

struct command {
    char        *c_name;
    int         (*c_func)();
};
extern struct command command_table[];

#define FONT_REGULAR    1
#define FONT_SPECIAL    2

struct font_desc {
    struct font_desc    *fd_next;           /* next font description */
    short                fd_index;          /* index of this description */
    char                *fd_troff_name;     /* troff name of font */
    short                fd_type;           /* SPECIAL or REGULAR */
    char                *fd_mac_name;       /* Macintosh name of font */
    short                fd_mac_style;      /* Macintosh style code */
    short                fd_mac_font;       /* Macintosh font number */
    char                *fd_mac_file;       /* Macintosh resource file */
    struct map_desc     *fd_map;            /* this fonts map */
};

struct open_res {
    char		*res_name;
    struct open_res	*res_next;
};

extern struct open_res  *open_res_list;

struct map_entry {
    short                me_code;           /* character index */
    char                *me_name;           /* troff character name */
    struct char_alias   *me_alias;          /* list of aliases */
};

#define NMAPENTRY       256
struct map_desc {
    struct map_desc     *md_next;               /* next map description */
    char                *md_name;               /* map name */
    struct map_entry     md_entry[NMAPENTRY];   /* the maps contents */
};
extern struct map_desc  *map_desc_list;

struct char_alias {
    struct char_alias   *ca_next;           /* next in chain */
    char                *ca_name;           /* troff character name */
};

struct prep_desc {
    char                *pd_device;         /* device name */
    short                pd_res;            /* resolution */
    short                pd_hor;            /* horizontal positioning */
    short                pd_vert;           /* vertical positioning */
    short                pd_unitwidth;      /* unitwidth */
    short                pd_paperlength;    /* paper length */
    short                pd_paperwidth;     /* paper width */
    struct font_desc    *pd_fontdesc;       /* list of font descriptions */
};
extern struct prep_desc  prep_desc;

extern int               line_no;
extern int               errors;

extern short             sizes[];           /* sizes available */
extern int               size_index;        /* number of sizes */
extern char             *specials[];        /* special characters */
extern int               sp_index;          /* number of specials */
#define NSIZES           100                /* limited by makedev */
#define NSPECIALS        256                /* limited by makedev */

#define LIBDIR		"/usr/lib/font"
