#ifndef lint	/* .../appletalk/at/lap/at_laplkup.c */
#define _AC_NAME at_laplkup_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:56:25}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_laplkup.c on 87/11/11 20:56:25";
  static char *Version = "(C) Copyright 1986 UniSoft Corp. Version 1.1 @(#)at_laplkup.c	1.1 4/12/87";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                                                         |
  |                                                                         |
  |	 #         ##    #####           #       #    #  #    #  #####	    |
  |	 #        #  #   #    #          #       #   #   #    #  #    #	    |
  |	 #       #    #  #    #          #       ####    #    #  #    #	    |
  |	 #       ######  #####           #       #  #    #    #  #####	    |
  |	 #       #    #  #               #       #   #   #    #  #	    |
  |	 ######  #    #  #      #######  ######  #    #   ####   #	    |
  |                                                                         |
  |                                                                         |
  |                                   ####                                  |
  |                                  #    #                                 |
  |                                  #                                      |
  |                           ###    #                                      |
  |                           ###    #    #                                 |
  |                           ###     ####                                  |
  |                                                                         |
  |                                                                         |
  |                                                                         |
  |                                                                         |
  +-------------------------------------------------------------------------+
  |
  |  Description:
  |	This utility is to interigate a lap module and find out what TYPEs are
  |	registered on what CIRCUITx's by which applications.
  |
  |  SCCS:
  |      @(#)at_laplkup.c	1.1 4/12/87
  |
  |  Copyright:
  |	Copyright (c) 1986 UniSoft Systems 
  |	All Rights Reserved
  |	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft
  |	The copyright notice above does not evidence any
  |	actual or intended publication of such source code.
  |
  |  History:
  |      08-Sep-86: Kip, change to stream.
  |
  +*/

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <appletalk.h>

#define _AC_MODS

caddr_t			pgm_name;
int			verbose;

main(argc, argv)
int argc;
char *argv[];
{
    char		lap_name[MAXNAMLEN];
    char		buffer[MAXIOCBSZ];
    char		c;
    int			errflg = 0;
    char		*fn;
    int			size;
    int			args;
    int			lap_fd = 0;	/* default to stdin */
    at_lap_cfg_t 	*cfgp;
    at_lap_entry_t 	*entryp;
    extern int		optind;
    extern char		*optarg;
    int			standard_in;
    int			f_option;

    
    
    /*------------------------------------------------------------------*/
    /* find out what this program is called 				*/
    /*------------------------------------------------------------------*/
    pgm_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];
    fn = "main";
    at_pid = getpid();

    /*------------------------------------------------------------------*/
    /* load in all the arguments					*/
    /*------------------------------------------------------------------*/
    while ((c = getopt(argc, argv, "f:v")) != EOF)
    {
	args++;
	switch (c)
	    {
	    case 'f':
		f_option = 1;
		if (*optarg == '-')
		    standard_in = 1;
		else
		{
		    sprintf(lap_name,"%s/lap/control",optarg);
		    sprintf(at_network,"%s",optarg);
		}
		break;
	    default:
		errflg++;
		break;
	    }
    }
    if (errflg)
    {
	fprintf(stderr,"Usage: %s [-f devfile] [-v]\n", pgm_name);
	exit(1);
    }

    
    /*------------------------------------------------------*/
    /* set the default network if the f option was not used */
    /*------------------------------------------------------*/
    if (!f_option)
	{
	if (at_find_dflt_netw() == -1)
	    {
	    at_error(9999,fn,"could not select default network\n");
	    exit(1);
	    /* NOTREACHED */
	    }
	sprintf(lap_name,"%s/lap/control",at_network);
	}

    /*-------------------------------------------------------------------*/
    /* if not standard in, i.e. if not open already, open the device    */
    /*-------------------------------------------------------------------*/
    if (!standard_in)
    {
	if((lap_fd = open(lap_name, 0)) < 0)
	{
	    at_error(9999, fn, "an error occured during the open\n");
	    exit(1);
	}
    }

    /*------------------------------------------------------------------*/
    /* send the ioctl to look up the table and print it out		*/
    /*------------------------------------------------------------------*/
    size = 0;
    entryp = (at_lap_entry_t *) buffer;
    if (at_send_to_dev(lap_fd,AT_LAP_GET_CFG, entryp, &size) == -1)
	{
	at_error(6001,fn,"bad ioctl() AT_LAP_GET_CFG");
	exit(1);
	/* NOTREACHED */
	}

    printf("\nThe following types are registered on the circuitXs:\n");
    printf("%-20s%-10s%-15s\n","type","circuitx","name");
    for (;(caddr_t) entryp < (caddr_t) buffer + size; entryp++)
    {
	printf("%-20x%-10x%-15s\n",entryp->type,entryp->circuitx,entryp->name);
    }


    /*------------------------------------------------------------------*/
    /* close device 							*/
    /*------------------------------------------------------------------*/
    if(!standard_in && close(lap_fd) < 0) 
    {
	at_error(9999, fn, "an error occured during the close\n");
	perror("");
	exit(1);
    }
}
