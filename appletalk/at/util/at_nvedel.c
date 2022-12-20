#ifndef lint	/* .../appletalk/at/util/at_nvedel.c */
#define _AC_NAME at_nvedel_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:02:39}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_nvedel.c on 87/11/11 21:02:39";
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
	extern	int	optind;
	extern	char   *optarg;
	int     opterr;
	int	option_letter;
	int	f_option;
	int     o_option;
	int     t_option;
	char   *object;
	char   *type;
	static  char  new_env[MAXNAMLEN+11];

        at_pgm = argv[0];
	fn = "main";


	/*-----------------------*/
	/* process the arguments */
	/*-----------------------*/
	while ((option_letter = getopt(argc,argv,"f:t:o:")) != EOF)
	{
	   switch(option_letter)
           {
                /*---------------------------------------------*/
                /* option '-f': use the filename following at  */
		/* the path to the network		       */
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
	if (opterr || !o_option)
	    {
	    printf("at_nvedel [-f network] -o object [-t type]\n");
	    exit(1);
	    }

        /*------------------------------------------------------*/
        /* set the default network if the f option was not used */
        /*------------------------------------------------------*/
 	if (!f_option)
            {
	    if ((status = at_find_dflt_netw()) == -1)
	        {
	        at_error(5500,fn,"could not select default network\n");
	        exit(1);
	        /* NOTREACHED */
	        }
	    }

       /*------------------*/
       /* delete the NVE   */
       /*------------------*/
       if ((status = at_deregister_name_nve(object,0,type,0)) == -1)
           {
           printf("%s: deletion failed\n",at_pgm);
           return(1);
           }
        }
