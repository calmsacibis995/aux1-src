#ifndef lint	/* .../appletalk/at/lib/at_sddplen.c */
#define _AC_NAME at_sddplen_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:00:18}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_sddplen.c on 87/11/11 21:00:18";
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


void at_set_ddp_length(ptr,len)
        u8     *ptr;
        int     len;
        {

        *ptr   &= 0xfc;
        *ptr++ |= (len >> 8) & 3;
        *ptr    = len & 0xff;
        }
