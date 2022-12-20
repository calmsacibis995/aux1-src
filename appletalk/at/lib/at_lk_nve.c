#ifndef lint	/* .../appletalk/at/lib/at_lk_nve.c */
#define _AC_NAME at_lk_nve_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:14}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_lk_nve.c on 87/11/11 20:59:14";
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


int at_lookup_nve(object,object_len,type,type_len,zone,zone_len,trys,trywait)
        char    *object;
        int      object_len;
        char    *type;
        int      type_len;
        char    *zone;
        int      zone_len;
        int      trys;
        int      trywait;
        {
        char   *fn;
        int     status;

        fn = "at_lookup_nve";
        status = at_lookup_or_confirm_nve(object,object_len,
                                          type,type_len,
                                          zone,zone_len,
                                          trys,trywait,0,0,
                                          AT_NBP_LOOKUP);
        if (status == -1)
            {
            at_error(2400,fn,"bad return from at_lookup_or_confirm_nve()");
            }
        return(status);
        }
