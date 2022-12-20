#ifndef lint	/* .../appleprint/iw/daiw/do_x.c */
#define _AC_NAME do_x_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:07}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _do_x_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of do_x.c on 87/11/11 21:44:07";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	do_x(ifile,ofile)
 *
 *  Arguments:
 *	ifile	The stream I/O pointer to a FILE structure. The input file
 *		contains the input produced by ditroff.
 *	ofile	The stream I/O pointer to a FILE structure. The output file
 *		that is being produced for the typesetting device by this
 *		program.
 *
 *  Returns:
 *	(nothing)
 *
 *  Description:
 *	This functions handles the ditroff output language `x' command,
 *	which is a family of device control commands.
 *
 *  Algorithm:
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void do_x(ifile,ofile)
	FILE   *ifile;
	FILE   *ofile;
	{
	char	word1[128];

	eatword(ifile,word1);
	switch(word1[0])
	    {
	    /* Initialization. */
	    case 'i':
		dev_init (x_T_s);
		font_init (x_T_s);
		break;


	    /* Typesetter name. */
	    case 'T':
		eatword(ifile,x_T_s);
		if (sw_debug[0])
		    {
		    (void) fprintf(stderr,"typesetter name is `%s'\n",x_T_s);
		    }
		break;


	    /* Resolution. */
	    case 'r':
		x_r_n = eatnnn(ifile);
		x_r_h = eatnnn(ifile);
		x_r_v = eatnnn(ifile);
		if (sw_debug[0])
		    {
		    (void) fprintf(stderr,
                                   "resolution = %d, hinc = %d, vinc = %d\n",
			           x_r_n,x_r_h,x_r_v);
		    }
		break;


	    /* Pause. */
	    case 'p':
		break;


	    /* Stop. */
	    case 's':
		pageflush(ofile);
		break;


	    /* Generate trailer. */
	    case 't':
		break;


	    /* Font position. */
	    case 'f':
		x_f_n = eatnnn(ifile);
		eatword(ifile,x_f_s[x_f_n]);
		load_font (x_f_n, x_f_s[x_f_n]);
		if (sw_debug[0])
		    {
		    (void) fprintf(stderr,
                                   "font `%s' on %d\n",x_f_s[x_f_n],x_f_n);
		    }
		break;


	    /* Character height. */
	    case 'H':
		x_H_n = eatnnn(ifile);
		break;


	    /* Set slant. */
	    case 'S':
		x_S_n = eatnnn(ifile);
		break;


	    default:
		softerr (213,"do_x(): unknown x subcommand `%s'",word1);
	    }
	}
