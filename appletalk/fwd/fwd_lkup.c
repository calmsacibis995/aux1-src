#ifndef lint	/* .../appletalk/fwd/fwd_lkup.c */
#define _AC_NAME fwd_lkup_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:10:16}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of fwd_lkup.c on 87/11/11 21:10:16";
  static char *Version = "(C) Copyright 1986 UniSoft Corp. Version 1.2 @(#)fwd_lkup.c	1.2 4/14/87";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                                                         |
  |                                                                         |
  |	 ######  #    #  #####           #       #    #  #    #  #####	    |
  |	 #       #    #  #    #          #       #   #   #    #  #    #	    |
  |	 #####   #    #  #    #          #       ####    #    #  #    #	    |
  |	 #       # ## #  #    #          #       #  #    #    #  #####	    |
  |	 #       ##  ##  #    #          #       #   #   #    #  #	    |
  |	 #       #    #  #####  #######  ######  #    #   ####   #	    |
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
  |	This utility is to interigate a ForWarDer and see what applications are
  |	loaded and how much space they take up.
  |
  |  SCCS:
  |      @(#)fwd_lkup.c	1.2 4/14/87
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
#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <sys/mmu.h>
#include <sys/page.h>
#include <sys/seg.h>
#include <sys/region.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <fcntl.h>
#include "fwd.h"

#define _AC_MODS

int			dev_fd = 0;	/* default to stdin */
caddr_t			pgm_name;
int			verbose;

main(argc, argv)
int argc;
char *argv[];
{
    char		c;
    char		*dev_file = NULL;
    int			errflg = 0;
    int			size = 0;
    int			args;
    extern int 		dev_fd;
    fwd_entry_t 	fwd_entry[1024/sizeof(fwd_entry_t)];
    fwd_entry_t 	*entryp;
    extern int		optind;
    extern char		*optarg;
    int			standard_in = 1;

    
    
    /*------------------------------------------------------------------*/
    /* find out what this program is called 				*/
    /*------------------------------------------------------------------*/
    pgm_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];

    /*------------------------------------------------------------------*/
    /* load in all the arguments					*/
    /*------------------------------------------------------------------*/
    while ((c = getopt(argc, argv, "f:v")) != EOF)
    {
	args++;
	switch (c)
	    {
	    case 'f':
		standard_in = 0;
		dev_file = optarg;
		args++;			/* count the -f parameter */
		break;
	    case 'v':
		verbose++;
		break;
	    case '?':
		errflg++;
		break;
	    }
    }
    if (errflg || argc-1 > args)
    {
	fprintf(stderr,"Usage: %s [-f devfile] [-v] [< devfile]\n", pgm_name);
	exit(1);
    }

    /*-------------------------------------------------------------------*/
    /* if not standard in, i.e. if not open already, open the device    */
    /*-------------------------------------------------------------------*/
    if (!standard_in)
    {
	if((dev_fd = open(dev_file, 0)) < 0)
	{
	    fprintf(stderr, "Error %d: Can't open %s\n", errno, dev_file);
	    exit(1);
	}
	if (verbose)
	    printf("device opened\n");
    }

    /*------------------------------------------------------------------*/
    /* send the ioctl to look up the table and print it out		*/
    /*------------------------------------------------------------------*/
    send_to_dev(I_FWD_LOOKUP, (caddr_t) fwd_entry, &size);
    printf("%8s %8s   %-15s\n","begin","start","name");
    for (entryp = fwd_entry; size > 0; size -= sizeof(fwd_entry_t), entryp++)
    {
	printf("%8x %8x   %-15s\n",entryp->begin,entryp->start,entryp->name);
    }

    /*------------------------------------------------------------------*/
    /* close device 							*/
    /*------------------------------------------------------------------*/
    if(!standard_in && close(dev_fd) < 0) 
    {
	fprintf(stderr, "Couldn't close the driver\n");
	exit(1);
    }
}

struct {
	int  ic_cmd;
	char *name;
} cmd_names[] = { 
	I_FWD_RESET,"I_FWD_RESET", 
	I_FWD_DOWNLD,"I_FWD_DOWNLD", 
	I_FWD_UPLD,"I_FWD_UPLD", 
	I_FWD_START,"I_FWD_START", 
	I_FWD_LOOKUP,"I_FWD_LOOKUP", 
	0,"UNKNOWN"
};


send_to_dev(cmd, dp, length)
int	cmd;
char	*dp;
int	*length;
{
	extern int 	dev_fd;
	int		i;
	struct strioctl i_str;


	/* set up the iocblk for the I_STR call */
	i_str.ic_cmd = cmd;
	i_str.ic_timout = 4;
	i_str.ic_len = *length;
	i_str.ic_dp = dp;

	if (verbose)
	{
	    /* find the name of the command */
	    for (i=0; cmd_names[i].ic_cmd!=0; i++)
		if (cmd == cmd_names[i].ic_cmd)
		    break;
	    printf("about to send %s command\n", cmd_names[i].name);
	}

	if(ioctl(dev_fd, I_STR, &i_str) < 0)
	{
	    /* UH OH, error time, find the name of the command and print error*/
	    for (i=0; cmd_names[i].ic_cmd!=0; i++)
		if (cmd == cmd_names[i].ic_cmd)
		    break;
	    fprintf(stderr, "%s: ioctl error: %d during an %s command\n",
		    pgm_name, errno, cmd_names[i].name);
	    exit(1);
	}
	*length = i_str.ic_len;
	return(0);
}
