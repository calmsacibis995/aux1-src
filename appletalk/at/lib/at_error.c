#ifndef lint	/* .../appletalk/at/lib/at_error.c */
#define _AC_NAME at_error_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:50}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_error.c on 87/11/11 20:58:50";
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

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();


/*--------------------------------------------------------------*/
/* NOTE!! - Leave the spaces between at_error an (number,...    */
/* so as to prevent renem from renumbering the function itself! */
/*--------------------------------------------------------------*/
/* VARARGS3 */
void at_error  (number,fn,format,p1,p2,p3,p4,p5,p6,p7,p8)
        int     number;
        char   *fn;
        char   *format;
        int     p1,p2,p3,p4,p5,p6,p7,p8;
        {
        auto    int     olderrno;
        extern  int     errno;
        extern  int     sys_nerr;
        extern  char   *sys_errlist[];

        olderrno = errno;
        /*****
        (void) fprintf(stderr,"\n");
        if (at_pid) (void) fprintf(stderr,"*** process  %d\n",at_pid);
	if (at_pgm) (void) fprintf(stderr,"*** program  %s\n",at_pgm);
        (void) fprintf(stderr,"*** function %s()\n",fn);
        (void) fprintf(stderr,"*** error %d has occured\n",number);
        *****/
	if (at_error_status)
   	    {
            (void) fprintf(stderr,"\n*** ");
            (void) fprintf(stderr,
                       "process %d, program %s, function %s(), error %d\n",
                        at_pid,
                        at_pgm != (char *)NULL ? at_pgm : "???",fn,number);
            (void) fprintf(stderr,"*** ");
            (void) fprintf(stderr,format,p1,p2,p3,p4,p5,p6,p7,p8);
            (void) fprintf(stderr,"\n");
            if (0 < olderrno  &&  olderrno < sys_nerr)
                {
                (void) fprintf(stderr,"*** system error %d: \"%s\"\n",
                               olderrno,sys_errlist[olderrno]);
                }
            (void) fflush(stderr);
            errno = 0;
            at_errno = number;
            }
	}
