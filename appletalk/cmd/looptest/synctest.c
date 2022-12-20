#ifndef lint	/* .../appletalk/cmd/looptest/synctest.c */
#define _AC_NAME synctest_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:10:57}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of synctest.c on 87/11/11 21:10:57";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)download.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems			*/
/*	All Rights Reserved					*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	This tests the Front End Processors (FEP) that have a "fwd" streamed 
 *	to them. It does a write and then a read N times.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/sysmacros.h>
#include <errno.h>
#include <fwd.h>

char		buffer[MAXIOCBSZ];
char		check_buffer[MAXIOCBSZ];
extern int	errno;
extern char	*sys_errlist[];
extern int	sys_nerr;
char		*dev_file = NULL;
int		dev_fd;
char		*in_file;
char		*pgm_name;
int		verbose = 0;



struct key {
	int  ic_cmd;
	char *name;
} cmd_names[] = { 
	I_FWD_RESET,"I_FWD_RESET", 
	I_FWD_DOWNLD,"I_FWD_DOWNLD", 
	I_FWD_START,"I_FWD_START", 
	I_FWD_LOOKUP,"I_FWD_LOOKUP", 
	0,"UNKNOWN"
};


static void
send_to_dev(cmd, dp, length)
int cmd;
char *dp;
int length;
{
	char		cmd_name;
	extern int 	dev_fd;
	int		i;
	struct strioctl i_str;



	i_str.ic_cmd = cmd;
	i_str.ic_timout = (cmd == I_FWD_RESET || cmd == I_FWD_START) ? 30 : 4;
	i_str.ic_len = length;
	i_str.ic_dp = dp;

        /* find the name of the command */
        for (i=0; cmd_names[i].ic_cmd!=0; i++)
	    if (cmd == cmd_names[i].ic_cmd)
	        break;
	if(ioctl(dev_fd, I_STR, &i_str) < 0)
	{
	    /* uh oh, error time */
	    /* find the name of the command */
	    for (i=0; cmd_names[i].ic_cmd!=0; i++)
		if (cmd == cmd_names[i].ic_cmd)
		    break;
	    if (errno >= sys_nerr)
		fprintf(stderr, "%s: during an %s command there was an unknow error %d\n",
			pgm_name, errno);
	    else
		fprintf(stderr, "%s: during an %s command there was a %s\n",
			pgm_name, cmd_names[i].name, sys_errlist[errno]);
	    exit(1);
	}
}

static  int
diff(p1, p2, length)
caddr_t		p1;
caddr_t		p2;

{
	int 	i = length;

	for  (; *p1++ == *p2++  && i--;)
	    ;
	return((i <= 0) ? -1 : length - i);
}



main(argc, argv)
int argc;
char *argv[];
{
    char		c;
    int			errflg = 0;
    int			iterations = 1000;
    int			i;
    int			j = 0;
    int			readback;
    fwd_entry_t 	fwd_entry;
    extern int		optind;
    extern char		*optarg;
    int			standard_out = 1;

    
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    /*------------------------------------------------------------------*/
    /* find out what this program is called 				*/
    /*------------------------------------------------------------------*/
    pgm_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];
    /*------------------------------------------------------------------*/
    /* load in all the arguments					*/
    /*------------------------------------------------------------------*/
    while ((c = getopt(argc, argv, "af:n:cv")) != EOF)
	switch (c)
	    {
	    case 'f':
		standard_out = 0;
		dev_file = optarg;
		break;
	    case 'i':
		iterations = atoi(optarg);
		break;
	    case 'v':
		verbose++;
		break;
	    case '?':
	    default:
		errflg++;
		break;
	    }
    if (argc < 2 || errflg)
    {
	fprintf(stderr,"Usage: %s [-a] [-f devfile] [-n name] [-c] [-v] loadfile\n", pgm_name);
	exit(1);
    }
    in_file = argv[optind];
    if (standard_out && verbose)
    {
	fprintf(stderr,"%s: verbose and download to standard out mutually exclusive\n", pgm_name);
	exit(1);
    }

    /*-------------------------------------------------------------------*/
    /* if not standard out, i.e. if not open already, open the device    */
    /*-------------------------------------------------------------------*/
    if (!standard_out)
    {
	if((dev_fd = open(dev_file, O_RDWR)) < 0)
	{
	    fprintf(stderr, "%s: error %d: Can't open %s\n",
						     pgm_name, errno, dev_file);
	    exit(1);
	}
	if (verbose)
	    printf("%s: device opened\n", pgm_name);
    }

    /*------------------------------------------------------------------*/
    /* 	write/read loop							*/
    /*------------------------------------------------------------------*/
    for(; iterations; iterations--)
    {
	for(i=1000; i; i--, j++)
	{
	    if(write(dev_fd, &iterations, sizeof(iterations)) != sizeof(iterations))
	    {
		if (errno >= sys_nerr)
		    fprintf(stderr,"%s: write failed on -interation %d with an unknown error %d\n",
						     pgm_name, iterations, errno);
		else
		    fprintf(stderr,"%s: write failed on -interation %d with a %s\n",
					 pgm_name, iterations, sys_errlist[errno]);
		exit(1);
	    }
	    if(read(dev_fd, &readback, sizeof(readback)) != sizeof(readback))
	    {
		if (errno >= sys_nerr)
		    fprintf(stderr,"%s: read failed on -interation %d with an unknown error %d\n",
						     pgm_name, iterations, errno);
		else
    /*
		    fprintf(stderr,"%s: read failed on -interation %d with %d,%s\n",
				 pgm_name, iterations, errno, sys_errlist[errno]);
    */
		    fprintf(stderr,"%s: read failed on -interation %d with %d\n",
				 pgm_name, iterations, errno);
		exit(1);
	    }
	    if (readback != iterations)
	    {
		fprintf(stderr, "%s: read != write -interation %d readback %d\n",
						 pgm_name, iterations, readback);
		exit(1);
	    }
	}
	printf("iterations %d\n", j);
    }

    /*------------------------------------------------------------------*/
    /* close device 							*/
    /*------------------------------------------------------------------*/
    if(close(dev_fd) < 0) 
    {
	fprintf(stderr, "%s: couldn't close the driver\n", pgm_name);
	exit(1);
    }
}
