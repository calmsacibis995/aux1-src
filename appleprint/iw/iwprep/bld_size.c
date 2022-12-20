#ifndef lint	/* .../appleprint/iw/iwprep/bld_size.c */
#define _AC_NAME bld_size_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:42:01}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _bld_size_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of bld_size.c on 87/11/11 21:42:01";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
#include <stdio.h>
#include "troffprep.h"

#include <mac/types.h>
#include <mac/fonts.h>

short           sizes[NSIZES];
int             size_index = 0;

#define         MAXSIZE     72

void            add_sizes();
void            add_1size();
int             find_size();
void            sort_sizes();

build_size()
{
    struct font_desc    *fd;
    
    for (fd = prep_desc.pd_fontdesc; fd != NULL; fd = fd->fd_next) {
	add_sizes (fd);
    }
    sort_sizes();
}

static void
add_sizes (fd)
    struct font_desc    *fd;
{
    short                size;
    
    for (size = 1; size <= MAXSIZE; size++) {
	if (RealFont (fd->fd_mac_font, size) == true)
	    add_1size (size);
    }
}

static void
add_1size (size)
    short       size;
{
    if (find_size (size) == -1) {
	sizes[size_index++] = size;
    }
}

static int
find_size (size)
    short       size;
{
    int         i;
    
    for (i = 0; i < size_index; i++)
	if (sizes[i] == size)
	    return i;
    return -1;
}

static void
sort_sizes()
{
    int         incr;
    int         i;
    int         j;
    short       temp;
    
    incr = size_index / 2;
    while (incr > 0) {
	for (i = incr; i < size_index; i++) {
	    j = i - incr;
	    while (j >= 0) {
		if (sizes[j] > sizes[j+incr]) {
		    temp = sizes[j];
		    sizes[j] = sizes[j + incr];
		    sizes[j + incr] = temp;
		    j -= incr;
		} else {
		    break;
		}
	    }
	}
	incr /= 2;
    }
}

show_sizes()
{
    int         i;
    
    for (i = 0; i < size_index; i++) {
	printf ("%-3d", sizes[i]);
	if ((i + 1) % 25 == 0)
	    printf ("\n");
    }
    printf ("\n");
}
