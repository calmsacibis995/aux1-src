#ifndef lint	/* .../appletalk/cmd/looptest/looptest.c */
#define _AC_NAME looptest_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:10:50}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of looptest.c on 87/11/11 21:10:50";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)looptest.c	UniPlus VVV.2.1.1	*/

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
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/mmu.h>
#include <sys/page.h>
#include <sys/seg.h>
#include <sys/region.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <fwd.h>

unsigned char	buffer[4000];
unsigned char	check_buffer[4000];
extern int	errno;
extern char	*sys_errlist[];
extern int	sys_nerr;
char		*dev_file = NULL;
int		in_fd;
int		out_fd;
char		*in_file;
char		*pgm_name;
int		verbose = 0;



static  int
diff(p1, p2, length)
caddr_t		p1;
caddr_t		p2;

{
	int 	i = length;

	for  (; i && *p1++ == *p2++; i--)
	    ;
	return((i <= 0) ? -1 : length - i);
}



main(argc, argv)
int argc;
char *argv[];
{
    char		c;
    int			bytes_written = 0;
    int			bytes_read;
    int			bytes_read_offset;
    int			bytes_to_read;
    int			diff_offset;
    int			errflg = 0;
    int			fixed_size;
    int			i;
    int			iterations = 10000;
    int			j = 0;
    int			k;
    int			length_this_time;
    extern int		optind;
    extern char		*optarg;
    int			standard_out = 1;
    int			zero_buffer = 0;

    
    /*------------------------------------------------------------------*/
    /* find out what this program is called 				*/
    /*------------------------------------------------------------------*/
    pgm_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];
    /*------------------------------------------------------------------*/
    /* load in all the arguments					*/
    /*------------------------------------------------------------------*/
    while ((c = getopt(argc, argv, "f:i:s:vz")) != EOF)
	switch (c)
	    {
	    case 'f':
		standard_out = 0;
		dev_file = optarg;
		break;
	    case 'i':
		iterations = atoi(optarg);
		break;
	    case 's':
		fixed_size = atoi(optarg);
		break;
	    case 'v':
		verbose++;
		break;
	    case 'z':
		zero_buffer++;
		break;
	    case '?':
	    default:
		errflg++;
		break;
	    }
    if (argc < 2 || errflg)
    {
	fprintf(stderr,"Usage: %s [-f devfile] [-i] [-s] [-v] [-z]\n", pgm_name);
	exit(1);
    }
    in_file = argv[optind];
    if (standard_out && verbose)
    {
	fprintf(stderr,"%s: verbose and looptest to standard out mutually exclusive\n", pgm_name);
	exit(1);
    }

    /*-------------------------------------------------------------------*/
    /* if not standard out, i.e. if not open already, open the device    */
    /*-------------------------------------------------------------------*/
    if (!standard_out)
    {
	if((in_fd = out_fd = open(dev_file, O_RDWR)) < 0)
	{
	    fprintf(stderr, "%s: error %d: Can't open %s\n",
						     pgm_name, errno, dev_file);
	    exit(1);
	}
	if (verbose)
	    printf("%s: device opened\n", pgm_name);
    }

    /*------------------------------------------------------------------*/
    /* 	initialize the write buffer					*/
    /*------------------------------------------------------------------*/
    for (i=0; i<4000; i++)
	buffer[i] = i;

    /*------------------------------------------------------------------*/
    /* 	write/read loop							*/
    /*------------------------------------------------------------------*/
    while (j<iterations)
    {
	for(i=1; i<=1000 && j<iterations; i++, j++)
	{
	    if  (fixed_size)
		length_this_time = fixed_size;
	    else
		length_this_time = sizeof(int) * i;
	    if (verbose)
		printf("iterations %d, bytes_written %d\n",j ,bytes_written);
	    if(write(out_fd, buffer, length_this_time) != length_this_time)
	    {
		if (errno >= sys_nerr)
		    fprintf(stderr,"%s: write failed on iteration %d with an unknown error %d\n",
							 pgm_name, j, errno);
		else
		    fprintf(stderr,"%s: write failed on iteration %d with a %s\n",
					     pgm_name, j, sys_errlist[errno]);
		pause();
		exit(1);
	    }
	    bytes_written += length_this_time;
	    bytes_to_read = length_this_time;
	    bytes_read = 0;
	    bytes_read_offset = 0;
	    while ((bytes_read = read(	in_fd,
				      	(caddr_t)((int)check_buffer+bytes_read_offset),
					bytes_to_read
				     )
		    ) != bytes_to_read
		  )
	    {
		if (bytes_read >= 0)
		{
	    	    bytes_read_offset += bytes_read;
		    bytes_to_read -= bytes_read;
#ifdef NOTDEF
		    fprintf(stderr,"%s: read only %d bytes, trying for more\n",
			     pgm_name, bytes_read
			   );
#endif NOTDEF
		}
		else
		{
		    if (errno >= sys_nerr)
			fprintf(stderr,"%s: read failed on iteration %d with an unknown error %d\n",
							 pgm_name, j, errno);
		    else
			fprintf(stderr,"%s: read failed on iteration %d with %d, %s\n",
				     pgm_name, j, errno, sys_errlist[errno]);
		    pause();
		    exit(1);
		}
	    }
	    if ((diff_offset = diff(buffer, check_buffer, length_this_time))
									 != -1)
	    {
		fprintf(stderr, "%s: read != write on iteration %d\n",
								 pgm_name, j);
		fprintf(stderr, "\tat offset %d 0x%x 0x%x\n",
		   diff_offset, buffer[diff_offset], check_buffer[diff_offset]);
		exit(1);
	    }
	    if  (zero_buffer)
		for (k=0; k<4000; k++)
		    check_buffer[i] = 0;

	}
	if (!standard_out)
	    printf("iterations %d, bytes_written %d\n",j ,bytes_written);
    }
    /*------------------------------------------------------------------*/
    /* close device 							*/
    /*------------------------------------------------------------------*/
    if (!standard_out)
	if(close(in_fd) < 0)
	{
	    fprintf(stderr, "%s: couldn't close the driver\n", pgm_name);
	    exit(1);
	}
}
