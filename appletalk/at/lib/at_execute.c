#ifndef lint	/* .../appletalk/at/lib/at_execute.c */
#define _AC_NAME at_execute_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:53}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_execute.c on 87/11/11 20:58:53";
  static char *Version1 = "@(#)at_lib.c	5.2 10/13/86 (at_lib.c version)";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                                                         |
  |                                                                         |
  |	   #    #######         #         ###   ######           #####      |
  |	  # #      #            #          #    #     #         #     #     |
  |	 #   #     #            #          #    #     #         #           |
  |	#     #    #            #          #    ######          #           |
  |	#######    #            #          #    #     #   ###   #           |
  |	#     #    #            #          #    #     #   ###   #     #     |
  |	#     #    #    ####### #######   ###   ######    ###    #####      |
  |                                                                         |
  |                                                                         |
  |                                                                         |
  +-------------------------------------------------------------------------+
  |
  |  Description:
  |
  |  SCCS:
  |      @(#)at_lib.c	5.2 10/13/86
  |
  |  Copyright:
  |      Copyright 1986 by UniSoft Systems Corporation.
  |
  |  History:
  |      24-Jun-85: Created by Philip K. Ronzone.
  |
  +*/

#include "appletalk.h"
#include <sys/types.h>
#include <sys/stat.h>

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();


/* VARARGS3 */
/* ARGSUSED */
int at_execute(pathnames,pgm,args)
        char   *pathnames[];
        char   *pgm;
	int	args;
        {
        char        *fn;
        int          i;
        int          length;
        char        *sep;
        char         buffer[1024];
        int          status;
        struct stat  file_stats;

        fn = "at_execute";
        for (i = 0;  pathnames[i] != (char *)NULL;  i++)
            {
            sep = "/";
            length = strlen(pathnames[i]);
            if (pathnames[i][length-1] == '/')
                {
                sep = "";
                }
            (void) sprintf(buffer,"%s%s%s",pathnames[i],sep,pgm);
            status = stat(buffer,&file_stats);
            if (status == -1)
                {
                continue;
                }
            else
                {
                if (!fork())
                    {
                    pgm = buffer;
                    status = execv(pgm,&pgm);
                    if (status == -1)
                        {
                        at_error(2100,fn,"exec error for <%s>",pgm);
                        }
                    exit(0);
                    /* NOTREACHED */
                    }
                else
                    {
                    return(0);
                    }
                }
            }
        if (!fork())
            {
            status = execvp(pgm,&pgm);
            if (status == -1)
                {
                at_error(2101,fn,"exec error for <%s>",pgm);
                }
            exit(0);
            /* NOTREACHED */
            }
        else
            {
            return(0);
            }
        }
