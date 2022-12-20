#ifndef lint	/* .../appletalk/at/lib/at_drgn_en.c */
#define _AC_NAME at_drgn_en_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:40}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_drgn_en.c on 87/11/11 20:58:40";
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


int at_deregister_name_en(en,enlen)
        char   *en;
        int     enlen;
        {
        char   *fn;
        int     status;
        char    object[AT_ENF_SIZE];
        char    type[AT_ENF_SIZE];
        char    zone[AT_ENF_SIZE];
        int     olen;
        int     tlen;
        int     zlen;

        fn = "at_deregister_name_en";
        status = at_decompose_en(en,enlen,object,&olen,type,&tlen,zone,&zlen);
        if (status == -1)
            {
            at_error(1900,fn,"bad return from at_decompose_en()");
            return(-1);
            }
        status = at_delete_name_nve(object,olen,type,tlen,zone,zlen);
        if (status == -1)
            {
            at_error(1901,fn,"bad return from at_delete_name_nve()");
            return(-1);
            }
        return(status);
        }
