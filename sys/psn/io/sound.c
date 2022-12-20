#ifndef lint	/* .../sys/psn/io/sound.c */
#define _AC_NAME sound_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/19 18:04:20}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of sound.c on 87/11/19 18:04:20";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*
 * sound.c -- make noise
 *
 * August 1987, Greg Satz
 *
 */

#define _AC_MODS

#include "sys/types.h"

#define DISP_CONSOLE 0

struct chorddata {
    short volume;
    long spinloop;
    long notespeed;
    long countdown;
    short nnotes;
    long note[4];
};

struct chorddata chorddata = {
	0x36,		/* volume */
	50L,		/* spin loop speed */
	1000L,		/* next note speed */
	4000L,		/* count down till done */
	4,		/* number of notes */
	0x000143EF,
	0x00019821,
	0x0001E55A,
	0x000287DE
};

/*
 * sound
 * Makes noise for given amount of time. If volume is zero, we flash
 * the menu bar. If duration is zero we may b called from interrupt level
 * so we use a fixed duration via a busy loop.
 */

sound(duration)
    long duration;
{
    int vol;
    long len;

    vol = nvram_volume();
    if (vol == 0) {
	vt100_flashmenu(DISP_CONSOLE);
	return;
    }
    chorddata.volume = (vol << 3) | vol;
    len = chorddata.countdown;
    if (duration != 0L)
	chorddata.countdown = duration * 1000;
    chord(&chorddata);
    chorddata.countdown = len;
}
