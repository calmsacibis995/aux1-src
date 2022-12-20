#ifndef lint	/* .../appletalk/at/util/at_nvereg.c */
#define _AC_NAME at_nvereg_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:03:11}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_nvereg.c on 87/11/11 21:03:11";
  static char *Version = "(C) Copyright 1985 UniSoft Corp. Version 4.2 @(#)at_nvereg.c	4.2 3/19/86";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                                                         |
  |                                                                         |
  |   ##     #####          #    #  #    #  ######  #####   ######   ####   |
  |  #  #      #            ##   #  #    #  #       #    #  #       #    #  |
  | #    #     #            # #  #  #    #  #####   #    #  #####   #       |
  | ######     #            #  # #  #    #  #       #####   #       #  ###  |
  | #    #     #            #   ##   #  #   #       #   #   #       #    #  |
  | #    #     #   #######  #    #    ##    ######  #    #  ######   ####   |
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
  |      At_nvereg is an Appletalk utility to register a NVE. The arguments
  |      are:  object  type  socket  seconds  [deregistration].
  |      The NVE with the given object and type name is registered at the
  |      given socket for the given amount of time. If the user specified
  |      the last argument, the NVE is deregistered immediately after that
  |      time expires. Otherwise it will remain registered untill it is
  |      removed by the RTMP daemon, or untill AppleTalk goes down.
  |
  |  SCCS:
  |      @(#)at_nvereg.c	4.2 3/19/86
  |
  |  Copyright:
  |      Copyright 1985 by UniSoft Systems Corporation.
  |
  |  History:
  |      24-Jun-85: Created by Philip K. Ronzone.
  |
  +*/

#include "appletalk.h"

#define _AC_MODS
static char *Version2= AT_APPLETALK_VERSION_GENERAL;

main(argc,argv)
        int     argc;
        char   *argv[];
        {
        char   *fn;
        int     status;
        int     nveno;
	extern	int	optind;
	extern	char   *optarg;
	int     opterr;
	int	option_letter;
	int	f_option;
	int     o_option;
	int     t_option;
	int     r_option;
	int     s_option;
	int     i_option;
	char   *object;
	char   *type;
	int     trys;
	int     seconds;
	int     socket;


        at_pgm = argv[0];
	fn = "main";

	/*-----------------------*/
	/* process the arguments */
	/*-----------------------*/
	while ((option_letter = getopt(argc,argv,"f:s:t:o:r:i:")) != EOF)
	{
	   switch(option_letter)
           {
                /*---------------------------------------------*/
                /* option '-f': use the filename following at  */
		/* the path to the socket		       */
                /*---------------------------------------------*/
                case 'f':
		    f_option++;
		    sprintf(at_network,"%s",optarg);
                    break;
		/*----------------------------------*/
		/* option '-o': object is specified */
		/*----------------------------------*/
		case 'o':
		    o_option++;
		    object = optarg;
		    break;
		/*----------------------------------*/
		/* option '-t': type   is specified */
		/*----------------------------------*/
		case 't':
		    t_option++;
		    type = optarg;
		    break;
		/*----------------------------------*/
		/* option '-s': socket is specified */
		/*----------------------------------*/
		case 's':
		    s_option++;
		    socket = (int) strtol(optarg,0,0);
		    break;
		/*----------------------------------*/
		/* option '-r': trys   is specified */
		/*----------------------------------*/
		case 'r':
		    r_option++;
		    trys = (int) strtol(optarg,0,0);
		    break;
		/*-----------------------------------*/
		/* option '-i': seconds is specified */
		/*-----------------------------------*/
		case 'i':
		    i_option++;
		    seconds = (int) strtol(optarg,0,0);
		    break;
		default:
		    printf("unknown argument\n");
		case '?':
		    opterr = 1;
		    break;
      	    }
	} /* end of while */

	/*-------------------------------------------------------------*/
	/* check whether there was a mistake in the args specification */
	/*-------------------------------------------------------------*/
	if (opterr || !s_option || !o_option || !t_option)
	    {
	    printf("at_nvereg -s socket -o object -t type [-r retries] [-s seconds]\n");
	    exit(1);
	    }
	if (!r_option)
	    trys = 5;
	if (!s_option)
	    seconds = 2;

       /*----------------------------------------*/
       /* spon of a child process to do the work */
       /*----------------------------------------*/
       if (fork())
           {
           return(0);
           }

       /*------------------*/
       /* register the NVE */
       /*------------------*/
       nveno = at_register_nve(object,0,type,0, socket, trys, seconds);

       /*------------------------------------------------------------*/
       /* check the return status: at_nve_lkup_reply_count indicates */
       /* whether any NVE's were found with the same name            */
       /*------------------------------------------------------------*/
       if (nveno == -1)
           {
           printf("%s: name already in use, not registered\n",at_pgm);
           return(1);
           }
       }
