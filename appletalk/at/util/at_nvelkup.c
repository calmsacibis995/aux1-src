#ifndef lint	/* .../appletalk/at/util/at_nvelkup.c */
#define _AC_NAME at_nvelkup_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:02:46}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_nvelkup.c on 87/11/11 21:02:46";
  static char *Version = "(C) Copyright 1985 UniSoft Corp. Version 4.3 @(#)at_nvelkup.c.c	4.3 9/15/86";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*+----------------------------------------------------------------------------+
  |                                                                            |
  |                                                                            |
  |                                                                            |
  |  ##     #####        #    #  #    #  ######  #       #    #  #    #  ##### |
  | #  #      #          ##   #  #    #  #       #       #   #   #    #  #    #|
  |#    #     #          # #  #  #    #  #####   #       ####    #    #  #    #|
  |######     #          #  # #  #    #  #       #       #  #    #    #  ##### |
  |#    #     #          #   ##   #  #   #       #       #   #   #    #  #     |
  |#    #     #   #####  #    #    ##    ######  ######  #    #   ####   #     |
  |                                                                            |
  |                                                                            |
  |                                   ####                                     |
  |                                  #    #                                    |
  |                                  #                                         |
  |                           ###    #                                         |
  |                           ###    #    #                                    |
  |                           ###     ####                                     |
  |                                                                            |
  |                                                                            |
  |                                                                            |
  |                                                                            |
  +----------------------------------------------------------------------------+
  |
  |  Description:
  |      At_nvelkup is an AppleTalk utility for looking up NVE's matching the
  |      object, type and zone specified by the user. The arguments are:
  |              object  type  zone  tries  seconds
  |      The utility performs the lookup a certain number of times, as specified
  |      in the 4th argument. The 5th argument determines how far apart the
  |      tries occur.
  |      The utility prints a table of the NVE's found, and displays the object,
  |      type, zone, net, node and socket for each one of them.
  |
  |  SCCS:
  |      @(#)at_nvelkup.c.c	4.3 9/15/86
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
	extern	int	optind;
	extern	char   *optarg;
        int     count;
        int     maxolen;
        int     maxtlen;
        int     maxzlen;
        char   *object_header;
        char   *type_header;
        char   *zone_header;
        at_nve *nve;
        at_nve *nve_head;
	int     opterr;
	int	option_letter;
	int	l_option;
	int	f_option;
	int     o_option;
	int     t_option;
	int     z_option;
	char   *object;
	char   *type;
	char   *zone;
	int	status;


        fn = "main";
        at_pgm = argv[0];


	/*-----------------------*/
	/* process the arguments */
	/*-----------------------*/
	while ((option_letter = getopt(argc,argv,"lf:t:z:o:")) != EOF)
	{
	   switch(option_letter)
           {
		/*----------------------------------------------*/
		/* option '-l': show entities on local net only */
		/*----------------------------------------------*/
		case 'l':
		    l_option++;
		    break;
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
		/*----------------------------------*/
		/* option '-z': zone is specified   */
		/*----------------------------------*/
		case 'z':
		    z_option++;
		    zone = optarg;
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
	/* set up defaults for unspecified options		       */
	/*-------------------------------------------------------------*/
	if (opterr)
	    exit(1);
	if (!o_option)
	    {
	    object = "=";
	    }
	if (!t_option)
	    {
	    type = "=";
	    }
	if (!z_option)
	    {
	    zone = "*";
	    }

        /*----------------*/
        /* lookup the NVE */
        /*----------------*/
	if (l_option)
	    {
            if ((status = at_lookup_or_confirm_nve(object,0,
						   type,0,
						   zone,0,
                                                   1,1,0,0,
            				           AT_NBP_LOOK_LOCAL)) == -1)
                {
                at_error(6500,fn,"bad return from at_lookup_nve()");
                return(1);
                }
	    }
	else
	    {
	    if ((count = at_lookup_nve(object,0,
                                       type,0,
                                       zone,0,
				       1,1)) == -1)
                 {
                 at_error(6501,fn,"bad return from at_lookup_nve()");
                 return(1);
                 }
	    }

        /*---------------------------------------------------*/
        /* initialize the headers of the table to be printed */
        /*---------------------------------------------------*/
        object_header = "OBJECT";
        type_header = "TYPE";
        zone_header = "ZONE";
        maxolen = strlen(object_header);
        maxtlen = strlen(type_header);
        maxzlen = strlen(zone_header);

        /*---------------------------------------------------*/
        /* scan the linked list of NVE's found and determine */
        /* the column width for each NVE component           */
        /*---------------------------------------------------*/
        nve_head = at_nve_lkup_reply_head;
        for (nve = nve_head; nve != (at_nve *)NULL; nve = nve->at_nve_next)
            {
            if (nve->at_nve_object_length > maxolen)
                {
                maxolen = nve->at_nve_object_length;
                }
            if (nve->at_nve_type_length > maxtlen)
                {
                maxtlen = nve->at_nve_type_length;
                }
            if (nve->at_nve_zone_length > maxzlen)
                {
                maxzlen = nve->at_nve_zone_length;
                }
            }

        /*--------------------------------*/
        /* print the headers of the table */
        /*--------------------------------*/
        printf("%-*s ",maxolen,object_header);
        printf("%-*s ",maxtlen,type_header);
        printf("%-*s ",maxzlen,zone_header);
        printf("NET  ND SK\n");

        /*--------------------------------------------------------*/
        /* scan the linked list of NVE's and print for each entry */
        /* the object, type, zone, net, node and socket           */
        /*--------------------------------------------------------*/
        for (nve = nve_head;  nve != (at_nve *)NULL;  nve = nve->at_nve_next)
            {
            printf("%-*s ",maxolen,nve->at_nve_object);
            printf("%-*s ",maxtlen,nve->at_nve_type);
            printf("%-*s ",maxzlen,nve->at_nve_zone);
            printf("%.4x ",nve->at_nve_net);
            printf("%.2x ",nve->at_nve_node);
            printf("%.2x ",nve->at_nve_socket);
            printf("\n");
            }
        return(0);
        }
