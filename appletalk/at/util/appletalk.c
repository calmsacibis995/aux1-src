#ifndef lint	/* .../appletalk/at/util/appletalk.c */
#define _AC_NAME appletalk_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:02:17}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of appletalk.c on 87/11/11 21:02:17";
  static char *Version = "(C) Copyright 1986 UniSoft Corp. Version 3.6 @(#)appletalk.c	3.6 1/17/86";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                                                         |
  |                                                                         |
  |    ##    #####   #####   #       ######   #####    ##    #       #    # |
  |   #  #   #    #  #    #  #       #          #     #  #   #       #   #  |
  |  #    #  #    #  #    #  #       #####      #    #    #  #       ####   |
  |  ######  #####   #####   #       #          #    ######  #       #  #   |
  |  #    #  #       #       #       #          #    #    #  #       #   #  |
  |  #    #  #       #       ######  ######     #    #    #  ######  #    # |
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
  |      AppleTalk is a general purpose utility for starting up and shutting
  |      down the AppleTalk network, and for miscellaneous control and 
  |      information reporting functions dealing with AppleTalk. A whole bunch
  |      of options are supported which are documented below.
  |
  |  SCCS:
  |      @(#)appletalk.c	3.6 1/17/86
  |
  |  Copyright:
  |      Copyright 1985 by UniSoft Systems Corporation.
  |
  |  History:
  |      24-Jun-85: Created by Philip K. Ronzone.
  |      08-Sep-86: Kip, change to stream.
  |
  +*/

#include <string.h>
#include <fcntl.h>
#include "appletalk.h"
#include <signal.h>
#include <sys/errno.h>

#define _AC_MODS
static char *Version2= AT_APPLETALK_VERSION_GENERAL;

static  char   *bin_pathnames[] =       { "/bin",
                                          "/usr/bin",
                                          NULL};

static  char   *appletalk_pathnames[] = { "/usr/lib/appletalk",
                                          "/lib/appletalk",
                                          ".",
                                          NULL};

int
main(argc,argv)
	int     argc;
	char   *argv[];

        {
        char            *fn;
        extern  int     optind;
        extern  char    *optarg;
	extern  int	errno;
        int 	        opterr;
        int             option_letter;
	int		a_option;
	int		d_option;
	int		e_option;
	int		f_option;
	int		i_option;
	int		n_option;
	int		p_option;
	int		r_option;
	int		u_option;
	char	        ddp_name[MAXNAMLEN];
	char            lap_name[MAXNAMLEN];
	char            at_node_name[MAXNAMLEN];
	char		new_node[9];
        int             lap_control;
        int             ddp_control;
	int		at_node_fd;
        int             status;
        int             size;
        at_lap_cfg_t    lap_cfg;
        at_ddp_cfg_t    ddp_cfg;
        int         	new_node_address;
        int         	new_rts_attempts;
	char	       *blanks = "    ";
	char           *at_kill;

	/*------------------------------------------------------------------*/
	/* find out what this program is called and other miscelanious stuff*/
	/*------------------------------------------------------------------*/
	at_pgm = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];
        fn = "main";
        at_pid = getpid();

        /*-----------------------*/
        /* process the arguments */
        /*-----------------------*/
        while ((option_letter = getopt(argc,argv,"adef:in:pr:u")) != EOF)
	{
            switch (option_letter)
	    {
                /*---------------------------------*/
                /* option '-a': print node address */
                /*---------------------------------*/
                case 'a':
		    a_option++;
                    break;

                /*------------------------------------*/
                /* option '-d': shut the network down */
                /*------------------------------------*/
                case 'd':
		    d_option++;
                    break;

                /*------------------------------------------------*/
                /* option '-e': print statistics and error counts */
                /*------------------------------------------------*/
                case 'e':
		    e_option++;
                    break;

                /*---------------------------------------------*/
                /* option '-f': use the filename following at  */
		/* the path to the network		       */
                /*---------------------------------------------*/
                case 'f':
		    f_option++;
		    sprintf(lap_name,"%s/lap/control",optarg);
		    sprintf(ddp_name,"%s/socket",optarg);
		    sprintf(at_network,"%s",optarg);
		    sprintf(at_node_name,"%s/.atnode",optarg);
                    break;

                /*---------------------------------------------*/
                /* option '-i': print the initial node address */
                /*---------------------------------------------*/
                case 'i':
		    i_option++;
                    break;

                /*-------------------------------------------*/
                /* option '-n': set the initial node address */
                /*-------------------------------------------*/
                case 'n':
		    n_option++;
                    new_node_address = (int) strtol(optarg,(char **)NULL,0);
		    sprintf(new_node,"%s",optarg);
                    break;

                /*-------------------------------------------------------*/
                /* option '-p': print the number of RTS attempts to make */
                /*-------------------------------------------------------*/
                case 'p':
		    p_option++;
                    break;

                /*-----------------------------------------------------*/
                /* option '-r': set the number of RTS attempts to make */
                /*-----------------------------------------------------*/
                case 'r':
		    r_option++;
                    new_rts_attempts = (int) strtol(optarg,(char **)NULL,0);
                    break;

                /*-----------------------------------*/
                /* option '-u': start the network up */
                /*-----------------------------------*/
                case 'u':
		    u_option++;
                    break;

		case '?':
		    opterr++;
		    break;
	    }

	} /* of while */


        /*--------------------------------------------------*/
        /* check whether at least one argument was provided */
        /*--------------------------------------------------*/
	if (argc < 2 || opterr)
            {
            printf("-a       prints node address\n");
            printf("-d       shut network down\n");
            printf("-e       print statistics & error counts\n");
            printf("-f file  is the network to access\n");
            printf("-i       print the initial node address\n");
            printf("-n xx    sets initial node address to xx\n");
            printf("-p       print number of RTS attempts to make\n");
            printf("-r xx    set number of RTS attempts to xx\n");
            printf("-u       start network up\n");
            exit(1);
            }
	
        /*------------------------------------------------------*/
        /* set the default network if the f option was not used */
        /*------------------------------------------------------*/
 	if (!f_option)
            {
	    if ((status = at_find_dflt_netw()) == -1)
	        {
	        at_error(9000,fn,"could not select default network\n");
	        exit(1);
	        /* NOTREACHED */
	        }
	    sprintf(lap_name,"%s/lap/control",at_network);
	    sprintf(ddp_name,"%s/socket",at_network);
	    sprintf(at_node_name,"%s/.atnode",at_network);
	    }

        /*-----------------------------------------------------*/
        /* N O W  D O  R E A L  W O R K  asked above	       */
        /*-----------------------------------------------------*/

        /*------------------------------------------------------*/
        /* open the files for communication with the I/O driver */
        /*------------------------------------------------------*/
        lap_control = open(lap_name,O_RDWR);
        if (lap_control == -1)
            {
            at_error(9001,fn,"bad return from open");
            exit(1);
            /* NOTREACHED */
            }
            

	/*-----------------------------------------------------------------*/
	/* obtain the actual node status at this time			   */
	/*-----------------------------------------------------------------*/
	size = 0;
	status = at_send_to_dev(lap_control,AT_LAP_GET_CFG, &lap_cfg, &size);
	if (status == -1)
	    {
	    at_error(9002,fn,"bad ioctl() AT_LAP_GET_CFG");
	    exit(1);
	    /* NOTREACHED */
	    }

	/*---------------------------------*/
	/* option '-a': print node address */
	/*---------------------------------*/
	if (a_option)
  	    {
	    size = 0;
	    status = at_send_to_dev(lap_control,AT_LAP_GET_CFG, &lap_cfg, &size);
	    if (status == -1)
	        {
	        at_error(9003,fn,"bad ioctl() AT_LAP_GET_CFG");
	        exit(1);
	        /* NOTREACHED */
	        }
	    printf("node address 0x%.2x\n", lap_cfg.node);
	    }

	/*-------------------------------------------*/
	/* option '-n': set the initial node address */
	/*-------------------------------------------*/
	if (n_option)
	    {
	    lap_cfg.initial_node = new_node_address;
	    if ((at_node_fd = open(at_node_name, O_WRONLY|O_CREAT)) == -1)
		{
		at_error(9004,fn,"could not open %s file",at_node_name);
		exit(1);
		/* NOTREACHED */
		}
	    if ((status = write(at_node_fd, new_node, strlen(new_node))) <= 0)
		{
		at_error(9005,fn,"could not write to %s file",at_node_name);
		exit(1);
		/* NOTREACHED */
		}
	    if ((status = write(at_node_fd, blanks, 4)) <= 0)
		{
		at_error(9006,fn,"could not write to %s file",at_node_name);
		exit(1);
		/* NOTREACHED */
		}
	    if ((status = close(at_node_fd)) == -1)
		{
		at_error(9007,fn,"could not close %s file",at_node_name);
		exit(1);
		/* NOTREACHED */
		}
	    }

	/*---------------------------------------------*/
	/* option '-i': print the initial node address */
	/*---------------------------------------------*/
	if (i_option)
	    {
	    size = 0;
	    status = at_send_to_dev(lap_control,AT_LAP_GET_CFG, &lap_cfg, &size);
	    if (status == -1)
	        {
	        at_error(9008,fn,"bad ioctl() AT_LAP_GET_CFG");
	        exit(1);
	        /* NOTREACHED */
	        }
	    printf("0x%x\n",lap_cfg.initial_node);
	    }

	/*-----------------------------------------------------*/
	/* option '-r': set the number of RTS attempts to make */
	/*-----------------------------------------------------*/
	if (r_option)
	    lap_cfg.rts_attempts = new_rts_attempts;

	/*-------------------------------------------------------*/
	/* option '-p': print the number of RTS attempts to make */
	/*-------------------------------------------------------*/
	if (p_option)
	    {
	    size = 0;
	    status = at_send_to_dev(lap_control,AT_LAP_GET_CFG, &lap_cfg, &size);
	    if (status == -1)
	        {
	        at_error(9009,fn,"bad ioctl() AT_LAP_GET_CFG");
	        exit(1);
	        /* NOTREACHED */
	        }
	    printf("RTS attempts = %d\n",lap_cfg.rts_attempts);
	    }

	/*---------------------------------------------------------------*/
	/* option '-d': shut the network down and kill the child process */
	/*---------------------------------------------------------------*/
	if (d_option)
	    {
	    if (!fork())
 		{
	        /*---------------------*/
	        /* kill the NBP daemon */
	        /*---------------------*/
		at_kill = "/etc/at_nveshut";
	        status = execv(at_kill, &at_kill);
	        if (status == -1)
		    {
		    at_error(9010,fn,"bad return from exec at_nveshut");
		    exit(1);
		    /* NOTREACHED */
		    }
	        }
	    else
		{
		wait();

	        /*------------------------*/
	        /* do the actual shutdown */
	        /*------------------------*/
	        status = at_send_to_dev(lap_control, AT_LAP_OFFLINE, NULL, NULL);
	        if (status == -1)
		    {
		    at_error(9011,fn,"bad ioctl() AT_LAP_OFFLINE");
		    exit(1);
		    /* NOTREACHED */
		    }
		}
	    }

	/*------------------------------------------------*/
	/* option '-e': print statistics and error counts */
	/*------------------------------------------------*/
	if (e_option)
	    {
	    size = 0;
	    status = at_send_to_dev(lap_control,AT_LAP_GET_CFG, &lap_cfg, &size);
	    if (status == -1)
	        {
	        at_error(9012,fn,"bad ioctl() AT_LAP_GET_CFG");
	        exit(1);
	        /* NOTREACHED */
	        }
	    printf("LAP Node address %d\n",
	      lap_cfg.node);
	    printf("\tTotal bytes transmitted ......... %d\n",
	      lap_cfg.stats.xmit_bytes);
	    printf("\tTotal packets transmitted ....... %d\n",
	      lap_cfg.stats.xmit_packets);
	    printf("\tTransmitter underrrun errors .... %d\n",
	      lap_cfg.stats.underrun_errors);

	    printf("\n");
	    printf("\tTotal bytes received ............ %d\n",
	      lap_cfg.stats.rcv_bytes);
	    printf("\tTotal packets received .......... %d\n",
	      lap_cfg.stats.rcv_packets);
	    printf("\tPacket too short errors ......... %d\n",
	      lap_cfg.stats.too_short_errors);
	    printf("\tPacket too long errors .......... %d\n",
	      lap_cfg.stats.too_long_errors);
	    printf("\tReceiver overrun errors ......... %d\n",
	      lap_cfg.stats.overrun_errors);
	    printf("\tCRC/frame errors ................ %d\n",
	      lap_cfg.stats.crc_errors);
	    printf("\tAbort errors .................... %d\n",
	      lap_cfg.stats.abort_errors);
	    printf("\tPackets with unregistered type .. %d\n",
	      lap_cfg.stats.type_unregistered);

	    printf("\n");
	    printf("\tProtocol timeouts................ %d\n",
	      lap_cfg.stats.timeouts);
	    printf("\tMissing EOF sync interrupt ...... %d\n",
	      lap_cfg.stats.missing_sync_irupt);
	    printf("\tNonproductive interrupts ........ %d\n",
	      lap_cfg.stats.unknown_irupts);
	    printf("\tIOCTLs unfinished at socket close %d\n",
	      lap_cfg.stats.ioc_unregistered);
	    printf("\tUnknown stream message types .... %d\n",
	      lap_cfg.stats.unknown_mblks);

	    printf("\n");
	    printf("\tNumber of collisions............. %d\n",
	      lap_cfg.stats.collisions);
	    printf("\tNumber of defers................. %d\n",
	      lap_cfg.stats.defers);

	    if (lap_cfg.network_up)
		{
                ddp_control = open(ddp_name,O_RDWR);
                if (ddp_control == -1)
                    {
                    at_error(9013,fn,"bad return from open");
                    exit(1);
                    /* NOTREACHED */
                    }
	        size = 0;
	        status = at_send_to_dev(ddp_control,AT_DDP_GET_CFG, &ddp_cfg, &size);
	        if (status == -1)
	            {
	            at_error(9014,fn,"bad ioctl() AT_DDP_GET_CFG");
	            exit(1);
	            /* NOTREACHED */
	            }
    
	        printf("\n");
	        printf("\tNetwork number .................. %d\n",
	          ddp_cfg.this_net);
	        printf("\tBridge number ................... %d\n",
	          ddp_cfg.a_bridge);
	        printf("\tPackets for unregistered socket...%d\n",
	          ddp_cfg.stats.socket_unregistered);
	        printf("\tPackets for out of range socket ..%d\n",
	          ddp_cfg.stats.rcv_socket_outrange);
	        printf("\tLength errors ................... %d\n",
	          ddp_cfg.stats.rcv_length_errors);
	        printf("\tChecksum errors ................. %d\n",
	          ddp_cfg.stats.rcv_checksum_errors);
	        printf("\tTransmit errors ................. %d\n",
	          ddp_cfg.stats.tag_room_errors);
		}
	}

	/*----------------------------------------*/
	/* are we to set the driver configuration?*/
	/*----------------------------------------*/
	if (n_option || r_option)
	    {
		size = sizeof(at_lap_cfg_t);
		status = at_send_to_dev(lap_control, AT_LAP_SET_CFG, &lap_cfg, &size);
		if (status == -1)
		    {
		    at_error(9015,fn,"bad return from ioctl(), AT_LAP_SET_CFG");
		    (void) exit(1);
		    /* NOTREACHED */
		    }
	    }
	
	/*------------------------------------------------------------------*/
	/* do the actual startup; 					    */
	/*------------------------------------------------------------------*/
	if (u_option)
	    {
	    /*---------------------------------------------------------*/
            /* if a node address exists, read it from the .atnode file */
	    /* and set the configuration with it                       */
	    /*---------------------------------------------------------*/
	    new_node_address = 0;
	    if (!n_option)
	        {
	        if (((at_node_fd = open(at_node_name, O_RDONLY)) == -1) && 
		    	(errno != ENOENT))
		    {
		    at_error(9016,fn,"could not open %s file",at_node_name);
		    exit(1);
		    /* NOTREACHED */
		    }
	        if (at_node_fd > 0)
		    {
	            if ((status = read(at_node_fd, new_node, 8)) <= 0)
		        {
		        at_error(9017,fn,"could not read from %s file",at_node_name);
		        exit(1);
		        /* NOTREACHED */
		        }
	            if ((status = close(at_node_fd)) == -1)
		        {
		        at_error(9018,fn,"could not close %s file",at_node_name);
		        exit(1);
		        /* NOTREACHED */
		        }
	            lap_cfg.initial_node = (int) strtol(new_node,(char **)NULL,0);
		    new_node_address = lap_cfg.initial_node;
		    size = sizeof(at_lap_cfg_t);
	            status = at_send_to_dev(lap_control, AT_LAP_SET_CFG, &lap_cfg, &size);
	            if (status == -1)
	                {
	                at_error(9019,fn,"bad return from ioctl(), AT_LAP_SET_CFG");
	                (void) exit(1);
	                /* NOTREACHED */
	                }
	            }
		}

    	    size = 0;
	    status = at_send_to_dev(lap_control, AT_LAP_ONLINE, NULL, &size);
	    if (status == -1)
		{
		at_error(9020,fn,"bad return from ioctl(), AT_LAP_ONLINE");
		exit(1);
		/* NOTREACHED */
		}

	    /*-----------------------------------------------------------------*/
            /* obtain the actual node address and store it in the .atnode file */
	    /*-----------------------------------------------------------------*/
	    size = 0;
	    status = at_send_to_dev(lap_control,AT_LAP_GET_CFG, &lap_cfg, &size);
	    if (status == -1)
	        {
	        at_error(9021,fn,"bad ioctl() AT_LAP_GET_CFG");
	        exit(1);
	        /* NOTREACHED */
	        }
	    if (lap_cfg.node != new_node_address)
	        {
	        if ((at_node_fd = open(at_node_name, O_WRONLY|O_CREAT)) == -1)
		    {
		    at_error(9022,fn,"could not open %s file",at_node_name);
		    exit(1);
		    /* NOTREACHED */
		    }
	        sprintf(new_node,"%d", lap_cfg.node);
	        if ((status = write(at_node_fd, new_node, strlen(new_node))) <= 0)
		    {
		    at_error(9023,fn,"could not write to %s file",at_node_name);
		    exit(1);
		    /* NOTREACHED */
		    }
	    	if ((status = write(at_node_fd, blanks, 4)) <= 0)
		    {
		    at_error(9024,fn,"could not write to %s file",at_node_name);
		    exit(1);
		    /* NOTREACHED */
		    }
	        if ((status = close(at_node_fd)) == -1)
		    {
		    at_error(9025,fn,"could not close %s file",at_node_name);
		    exit(1);
		    /* NOTREACHED */
		    }
		}

	    /*-----------------------------------------------------*/
	    /* set the error printing status to the default        */
	    /*-----------------------------------------------------*/
	    at_error_status = AT_ERROR_ON;

	    /*-----------------------------------------------------*/
	    /* let a child process start the Appletalk daemons so  */
	    /* that certain signals will be ignored by the daemons */
	    /*-----------------------------------------------------*/
	    if (!fork())
		{
		status = (int) signal (SIGINT, SIG_IGN);
		status = (int) signal (SIGQUIT, SIG_IGN);
		/*----------------------*/
		/* start the NBP daemon */
		/*----------------------*/
		printf("%s: starting NBP daemon ...\n",at_pgm);
		status = at_execute(appletalk_pathnames,"at_nbpd",0);
		if (status == -1)
		    {
		    at_error(9026,fn,"bad return from at_execute(), at_nbpd");
		    exit(1);
		    /* NOTREACHED */
		    }
		sleep(3);

		} /* of forked process */

	    }
	exit (0);
	}

