#ifndef lint	/* .../appletalk/fwd/fwdload.c */
#define _AC_NAME fwdload_c
#define _AC_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:10:24}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of fwdload.c on 87/11/11 21:10:24";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)fwdload.c	UniPlus VVV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems			*/
/*	All Rights Reserved					*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	This downloads Front End Processors (FEP) that have a "fwd" streamed 
 *	to them. The file to download is "ld"ed to the local environment of
 *	the board so a single application can be loaded into a multitude of
 *	environments. Also changing conditions on a single fep are not precluded
 *	in other words, additive loads are possible.
 */

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

#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>

char		buffer[MAXIOCBSZ];
char		check_buffer[MAXIOCBSZ];
LDFILE		*ldptr;
extern int	errno;
char		*dev_file = NULL;
int		dev_fd;
char		*in_file;
char		*mktemp();
char		*pgm_name;
char		*ptr;
extern char	*sys_errlist[];
extern int	sys_nerr;
char		*temp_file = "/tmp/downXXXXXX";
int		verbose = 0;
int		check_load = 0;




static	void
clean_up()

{
#ifdef NOTYET
    strcpy(buffer, "rm ");
    strcat(buffer, temp_file);
    if (system(buffer))
    {
	fprintf(stderr, "%s: failed to remove temporary file: %s",
							pgm_name, temp_file);
	perror(" because");
    }
#endif
    fflush(1);	/* what is going on */
    fflush(2);
}


struct key {
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
		fprintf(stderr, "%s: unknown error: %d during an %s command\n",
			pgm_name, errno, sys_errlist[errno], cmd_names[i].name);
	    else
		fprintf(stderr, "%s: error: %d, %s, during an %s command\n",
			pgm_name, errno, sys_errlist[errno], cmd_names[i].name);
	    clean_up();
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



static	void
send_section(sect_hdr, sect)
SCNHDR		*sect_hdr;
unsigned short	sect;

{
    char		*endbuf;
    static caddr_t	end_paddr;
    struct fwd_record 	*fwd_record;
    struct fwd_record 	*check_record;
    static caddr_t	last_size;
    static caddr_t	paddr;
    int			read_length;
    int			offset;

    /*------------------------------------------------------------------*/
    /* set up buffer, uses global buffer !!!!!!				*/
    /*------------------------------------------------------------------*/
    fwd_record = (struct fwd_record *) buffer;
    check_record = (struct fwd_record *) check_buffer;
    ptr = fwd_record->opt.data;
    endbuf = buffer + sizeof(buffer);
    paddr =  (caddr_t) sect_hdr->s_paddr;
    end_paddr = (char *) (sect_hdr->s_paddr + sect_hdr->s_size);
    if(sect_hdr->s_scnptr > 0)
    {
	/*----------------------------------------------------------------*/
	/* if there is data, not padding, to send, seek and ye shall find */
	/*----------------------------------------------------------------*/
	if(ldsseek(ldptr, sect) == FAILURE)
	{
	    fprintf(stderr, "%s: cannot read section %d in %s",
		    pgm_name, sect, in_file
		   );
	    perror(" because");
	    clean_up();
	    exit(1);
	}
	do
	{
	    fwd_record->begin = paddr;
	    if  (((endbuf - ptr) - (end_paddr - paddr)) >= 0)
	    {
		/* room in the buffer */
		read_length = (int) (end_paddr - paddr);
	    }
	    else
	    {
		/* no room in the buffer, so break section up */
		read_length = (int)(endbuf - ptr);
	    }
	    if (read_length != FREAD(ptr, sizeof(*ptr), read_length, ldptr))
		{
		    fprintf(stderr, "%s: FREAD returned low count\n", pgm_name);
		    clean_up();
		    exit(1);
		}
	    send_to_dev(I_FWD_DOWNLD, (caddr_t) fwd_record, 
				       sizeof(fwd_record->begin) + read_length);
	    if  (check_load)
	    {
		check_record->begin = fwd_record->begin;
		check_record->opt.ld_length = read_length;
		send_to_dev(I_FWD_UPLD, (caddr_t) check_record,
			    sizeof(check_record->begin)
			    + sizeof(check_record->opt.ld_length)
			   );
		if ((offset = 
		     diff(fwd_record->opt.data,check_record->opt.data,read_length)
		    ) != -1
		   )
		{
		    fprintf(stderr, "%s: check of load failed at %x\n",
						     pgm_name, paddr + offset);
		    clean_up();
		    exit(1);
		}
	    }   /* if  (check_load) */
	    paddr += read_length;
	}
	while (paddr < end_paddr);
    } /* if(sect_hdr->s_scnptr > 0) */
    /*----------------------------------------------------------------*/
    /* if there is padding to send, fill and ye shall be filled	      */
    /*----------------------------------------------------------------*/
    while(paddr < end_paddr)
    {
	fwd_record->begin = paddr;
	ptr = fwd_record->opt.data;
	while((ptr < endbuf) && (paddr < end_paddr))
	{
	    *ptr++ = 0;
	    paddr++;
	}
	read_length = ptr-fwd_record->opt.data;
	send_to_dev(I_FWD_DOWNLD, (caddr_t) fwd_record,
				   sizeof(fwd_record->begin) + read_length);
	if  (check_load)
	{
	    check_record->begin = fwd_record->begin;
	    check_record->opt.ld_length = read_length;
	    send_to_dev(I_FWD_UPLD, (caddr_t) check_record,
			    sizeof(check_record->begin)
			    + sizeof(check_record->opt.ld_length)
			   );
	    if ((offset = 
		 diff(fwd_record->opt.data,check_record->opt.data,read_length)
		) != -1
	       )
	    {
		fprintf(stderr, "%s: check of load failed at %x\n",
						 pgm_name, paddr + offset);
		clean_up();
		exit(1);
	    }
	}
    }
}

main(argc, argv)
int argc;
char *argv[];
{
    char		c;
    int			errflg = 0;
    int			i;
    FILHDR		*filehead;
    int			found = 0;
    fwd_entry_t 	fwd_entry;
    int			no_reset = 0; 
    extern int		optind;
    extern char		*optarg;
    unsigned short	sect;
    unsigned short	sect_cnt;
    SCNHDR		sect_hdr;
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
	    case 'a':
		no_reset++;
		break;
	    case 'c':
		check_load++;
		break;
	    case 'f':
		standard_out = 0;
		dev_file = optarg;
		break;
	    case 'n':
		strncpy(fwd_entry.name, optarg, 14);
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
	if((dev_fd = open(dev_file, O_NDELAY)) < 0)
	{
	    fprintf(stderr, "%s: error %d: Can't open %s",
						     pgm_name, errno, dev_file);
	    perror(" because");
	    exit(1);
	}
	if (verbose)
	    printf("%s: device opened\n", pgm_name);
    }

    /*------------------------------------------------------------------*/
    /* reset the device if not prohibited, i.e. loads are not additive	*/
    /*------------------------------------------------------------------*/
    if (!no_reset)
    {
	send_to_dev(I_FWD_RESET, NULL, 0); /* if there is an error, NO RETURN */
	if (verbose)
	    printf("%s: device reset\n", pgm_name);
    }

    /*------------------------------------------------------------------*/
    /* create the COFF file 						*/
    /*------------------------------------------------------------------*/
#ifdef  NOTYET
    strcpy(buffer, "ld -s -N -o ");
    (void) mktemp(temp_file);
    strcat(buffer, temp_file);
    strcat(buffer, " ");
    if (!standard_out)
    {   /*
         * now get the ld ifile in the dev specific directory which is path
	 * apended with ".d", watch out for names that are > 12 char
	*/
	ptr = strrchr(dev_file, '/') ? strrchr(dev_file, '/') + 1 : dev_file;
	strncat(buffer, dev_file, ptr-dev_file+13);
	strcat(buffer, ".d/ifile ");
    }
    strcat(buffer, in_file);
    strcat(buffer, " ");
    if (!standard_out)
    {   /*
         * now get the ld environ in the dev specific directory which is path
	 * apended with ".d", watch out for name that is > 12 char
	 */
	strncat(buffer, dev_file, ptr-dev_file+13);
	strcat(buffer, ".d/environ ");
    }
    if (verbose)
	printf("%s: ld invocation: %s\n", pgm_name, buffer);
    if (i = system(buffer))
    {
	fprintf(stderr, "%s: ld: failed", pgm_name);
	perror(" because");
	clean_up();
	exit(1);
    }
#endif

    /*------------------------------------------------------------------*/
    /* open the COFF file 						*/
    /*------------------------------------------------------------------*/
    ldptr = NULL;
/*   if((ldptr = ldopen(temp_file, ldptr)) == NULL) */
    if((ldptr = ldopen(in_file, ldptr)) == NULL)
    {
	    fprintf(stderr,"%s can't open %s for reading", pgm_name,in_file);
	    perror(" because");
	    clean_up();
	    exit(1);
    }
    if (verbose)
	printf("%s: COFF file opened\n", pgm_name);

    /*------------------------------------------------------------------*/
    /* check the magic number 						*/
    /*------------------------------------------------------------------*/
    if(HEADER(ldptr).f_magic != MC68MAGIC)
    {
	fprintf(stderr,
		"%s: %s is not in mc86 common object format\n",pgm_name, in_file
		);
	clean_up();
	exit(1);
    }

    /*------------------------------------------------------------------*/
    /* read sections 							*/
    /*------------------------------------------------------------------*/
    sect_cnt = HEADER(ldptr).f_nscns;
    for(sect = 1; sect <= sect_cnt; sect++)
    {
	if(ldshread(ldptr, sect, &sect_hdr) == FAILURE)
	{
	    fprintf(stderr,
		    "%s: cannot read section %d in %s",
		     pgm_name, sect, in_file
		    );
	    perror(" because");
	    clean_up();
	    exit(1);
	}
	if (verbose)
	    printf("%s:	section %d, %s start %x, length %x\n",
	 		pgm_name, sect, sect_hdr.s_name,
			sect_hdr.s_paddr, sect_hdr.s_size
		  );
	if (strcmp(sect_hdr.s_name, ".start") == 0)
	{
	    fwd_entry.start = (caddr_t) sect_hdr.s_paddr;
	    found = 1;
	}
	if (sect_hdr.s_flags&STYP_NOLOAD || sect_hdr.s_flags&STYP_DSECT)
	    continue;
	send_section(&sect_hdr, sect);
    } /* for(sect = 1; sect <= sect_cnt; sect++) */

    /*------------------------------------------------------------------*/
    /* close COFF file 							*/
    /*------------------------------------------------------------------*/
    ldclose(ldptr);
    if (verbose)
	printf("%s: COFF file closed\n", pgm_name);
    if(found == 0)
    {
	fprintf(stderr, "%s: error: No section name called .start\n", pgm_name);
	clean_up();
	exit(1);
    }
    if (fwd_entry.name[0] == '\0')
    {
	in_file = strrchr(in_file,'/') ? strrchr(in_file,'/') + 1 : in_file;
	strncat(fwd_entry.name, in_file, 14);
    }

    /*------------------------------------------------------------------*/
    /* jump to the location specified in section .start 		*/
    /*------------------------------------------------------------------*/
    send_to_dev(I_FWD_START, &fwd_entry, sizeof(fwd_entry));

    /*------------------------------------------------------------------*/
    /* close device 							*/
    /*------------------------------------------------------------------*/
    if(close(dev_fd) < 0) 
    {
	fprintf(stderr, "%s: couldn't close the driver", pgm_name);
	perror(" because");
	clean_up();
	exit(1);
    }
    clean_up();
}
