#ifndef lint	/* .../appleprint/iw/iwprep/bld_chset.c */
#define _AC_NAME bld_chset_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:41:56}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _bld_chset_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of bld_chset.c on 87/11/11 21:41:56";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include "troffprep.h"
#include "util.h"

char                *specials[NSPECIALS];
int                  sp_index = 0;

void                add_special();
void                add_1special();
int                 find_special();
void                sort_special();

build_charset()
{
    struct map_desc *md;
    int              i;
    
    for (md = map_desc_list; md != NULL; md = md->md_next) {
	for (i = 0; i < NMAPENTRY; i++) { 
	    add_special (&md->md_entry[i]);
	}
    }
    add_1special ("\\|");   /* these HAVE to exist */
    add_1special ("\\^");
    sort_special();
}

static void
add_special (me)
    struct map_entry    *me;
{
    struct char_alias   *ca;
    
    if (me->me_code != -1) {
	add_1special (me->me_name);
	for (ca = me->me_alias; ca != NULL; ca = ca->ca_next)
	    add_1special (ca->ca_name);
    }
}

static void
add_1special (name)
    char            *name;
{
    if (strlen (name) >= 2) {
	if (find_special (name) == -1)
	    specials[sp_index++] = strsave (name);
    }
}

static int
find_special (name)
    char        *name;
{
    int          i;
    
    for (i = 0; i < sp_index; i++)
	if (strcmp (name, specials[i]) == 0)
	    return i;
    return -1;
}

static void
sort_special ()
{
    int         incr;
    int         i;
    int         j;
    char       *temp;
    
    incr = sp_index / 2;
    while (incr > 0) {
	for (i = incr; i < sp_index; i++) {
	    j = i - incr;
	    while (j >= 0) {
		if (strcmp (specials[j], specials[j+incr]) > 0) {
		    temp = specials[j];
		    specials[j] = specials[j + incr];
		    specials[j + incr] = temp;
		    j -= incr;
		} else {
		    break;
		}
	    }
	}
	incr /= 2;
    }
}

show_special()
{
    int         i;

    for (i = 0; i < sp_index; i++) {
	printf ("%-3s", specials[i]);
	if ((i + 1) % 25 == 0)
	    printf ("\n");
    }
    printf ("\n");
}
