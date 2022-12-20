#ifndef lint	/* .../appletalk/at/lib/at_lk_en.c */
#define _AC_NAME at_lk_en_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:10}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_lk_en.c on 87/11/11 20:59:10";
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


int at_lookup_en(en,enlen,trys,trywait)
        char   *en;
        int     enlen;
        int     trys;
        int     trywait;
        {
        char   *fn;
        int     status;
        char    object[AT_ENF_SIZE];
        char    type[AT_ENF_SIZE];
        char    zone[AT_ENF_SIZE];
        int     olen;
        int     tlen;
        int     zlen;

        fn = "at_lookup_en";
        status = at_decompose_en(en,enlen,object,&olen,type,&tlen,zone,&zlen);
        if (status == -1)
            {
            at_error(2350,fn,"bad return from at_decompose_en()");
            return(-1);
            }
        status = at_lookup_nve(object,olen,type,tlen,zone,zlen,trys,trywait);
        if (status == -1)
            {
            at_error(2351,fn,"bad return from at_lookup_nve()");
            return(-1);
            }
        return(status);
        }
