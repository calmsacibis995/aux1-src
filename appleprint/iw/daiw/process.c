#ifndef lint	/* .../appleprint/iw/daiw/process.c */
#define _AC_NAME process_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.2 87/11/11 21:44:56}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char _process_c[] = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of process.c on 87/11/11 21:44:56";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#ifndef lint
#define _AC_MODS
#endif lint
/*
 *  Function:
 *	process(ifile,ofile)
 *
 *  Arguments:
 *	ifile	The input file that came from ditroff (ditroff output file).
 *	ofile	The output file that is going to go to the target output
 *		device. this function doesn't output to this file, but passes
 *		along the stream pointer to the subordinate functions that do.
 *
 *  Description:
 *	This function is the main processing loop for handling the ditroff
 *	output file. It reads the input file and decides what kind of
 *	ditroff output file command it read, and then calls the appropriate
 *	function to handle that command.
 *
 *  Algorithm:
 *	Get (`eat') a character at a time. The first character should be
 *	the ditroff output file command. The character is then parsed
 *	via a case statement to call the appropriate function to handle
 *	that command. Some commands have extra characters after them,
 *	and the function that handles that command will be responsible for
 *	reading and handling those character and to leave the input file
 *	such upon it's return the next character to be read by process()
 *	will be the next command character.
 *
 *  History:
 *	Coded October, 1984 by Philip K. Ronzone.
 *
 */

#include "local.h"

void process(ifile,ofile)
	FILE   *ifile;
	FILE   *ofile;
	{
	int	c;

	while ((c=eatc(ifile)) != EOF)
	    {
	    switch(c)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		    do_nnc(c,ifile);
		    break;
		case 'c':
		    do_cx(ifile);
		    break;
		case 'H':
		    do_Hn(ifile);
		    break;
		case 'V':
		    do_Vn(ifile);
		    break;
		case 'p':
		    do_pn(ifile,ofile);
		    break;
		case 'f':
		    do_fn(ifile);
		    break;
		case 'x':
		    do_x(ifile,ofile);
		    break;
		case '\n':
		    break;
		case 'h':
		    do_hn(ifile);
		    break;
		case 'v':
		    do_vn(ifile);
		    break;
		case 's':
		    do_sn(ifile);
		    break;
		case 'C':
		    do_Cxy(ifile);
		    break;
		case 'w':
		    break;
		case 'n':
		    do_nb_a(ifile);
		    break;
		case 'D':
		    do_D(ifile);
		    break;
		case ' ':
		case 0:
		    break;	/* occasional noise */
		default:
		    badinput(c,"process(): unknown command",ifile);
		}
	    }
	}
