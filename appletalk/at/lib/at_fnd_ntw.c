#ifndef lint	/* .../appletalk/at/lib/at_fnd_ntw.c */
#define _AC_NAME at_fnd_ntw_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:57}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_fnd_ntw.c on 87/11/11 20:58:57";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include "appletalk.h"
#include <fcntl.h>

char *getenv();

int at_find_dflt_netw()
	{
	char           *s;
        char	       *fn;
	char		at_ctl_name[MAXNAMLEN+1];
	int		netw_name_len;
	int		at_ctl_fd;
	int       	status;
	
	fn = "at_find_dflt_netw";
	if (s=getenv("ATNETWORK"))
	    {
	    sprintf(at_network,"%s",s);
	    }
	else
	    {
	    if (s=getenv("ATDIR"))
	        sprintf(at_ctl_name,"%s/.appletalkrc",s);
	    else
		{
	        if (s=getenv("HOME"))
	            sprintf(at_ctl_name,"%s/.appletalkrc",s);
		else
	            sprintf(at_ctl_name,"/.appletalkrc");
		}
	    if ((at_ctl_fd = open(at_ctl_name, O_RDONLY)) == -1)
	        {
		sprintf(at_network,"/dev/appletalk");
		}
	    else
		{
	        if ((netw_name_len = read (at_ctl_fd, at_network, MAXNAMLEN+1)) == -1)
	            {
		    at_error(2150,fn,"bad read on %s",at_ctl_name);
		    exit(1);
		    /* NOTREACHED */
		    }
	        at_network[netw_name_len-1] = NULL;
		}
	    }
	return(0);
	}

