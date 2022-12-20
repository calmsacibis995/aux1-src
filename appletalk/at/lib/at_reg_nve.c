#ifndef lint	/* .../appletalk/at/lib/at_reg_nve.c */
#define _AC_NAME at_reg_nve_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:00:14}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_reg_nve.c on 87/11/11 21:00:14";
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
#include <fcntl.h>
#include "sys/types.h"
#include "sys/stropts.h"

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();


int at_register_nve(object,object_len,type,type_len,socket,trys,secs)
        char    *object;
        int      object_len;
        char    *type;
        int      type_len;
        int      socket;
        int      trys;
        int      secs;
        {
        char  			*fn;
	int			fd;
	char			buff[NBP_NAME_MAX];
	char			*cp;
	at_nbp			*nbp;
	at_nbp_tuple_hdr	*tp;
	struct strioctl		s;
	struct nbp_param	*pp;

        fn = "at_register_nve";
        if (object_len == 0)
            {
            object_len = strlen(object);
            }
        if (type_len == 0)
            {
            type_len = strlen(type);
            }
        if (!(1 <= object_len  &&  object_len <= AT_NBP_TUPLE_STRING_MAXLEN))
            {
            at_error(3400,fn,"object string length range error (%d)",object_len);
            return(-1);
            }
        if (!(1 <= type_len  &&  type_len <= AT_NBP_TUPLE_STRING_MAXLEN))
            {
            at_error(3401,fn,"type string length out of range (%d)",type_len);
            return(-1);
            }
        if (!(1 <= socket  &&  socket <= 254))
            {
            at_error(3402,fn,"socket number out of range (%d)",socket);
            return(-1);
            }

        /*-------------------------------------*/
        /* Make sure that the object and type  */
        /* fields are not wildcard characters. */
        /*-------------------------------------*/
        if (object_len == 1  &&  object[0] == '=')
            {
            at_error(3403,fn,"object field can not be registered as a wildcard");
            return(-1);
            }
        if (type_len == 1  &&  type[0] == '=')
            {
            at_error(3404,fn,"type field can not be registered as a wildcard");
            return(-1);
            }


        /*---------------------------------------*/
        /* Setup the NVE with our NVE arguments. */
        /*---------------------------------------*/

	pp = (struct nbp_param *)buff;
	pp->nbp_retries = trys;
	pp->nbp_secs = secs;
	nbp = NBP_NBP(&buff[sizeof(struct nbp_param)]);
        nbp->at_nbp_tuple_count = 1;
	tp = (at_nbp_tuple_hdr *)nbp->at_nbp_tuples;
	tp->at_nbp_tuple_socket = socket;
	cp = (char *)tp->at_nbp_tuple_data;
	*cp++ = object_len;
	strcpy(cp, object);
	cp += object_len;
	*cp++ = type_len;
	strcpy(cp, type);
	cp += type_len;
	*cp++ = 1;
	*cp++ = '*';
	s.ic_len = cp - buff;
	s.ic_dp = buff;
	s.ic_timout = -1;
	s.ic_cmd = AT_NBP_REGISTER;

	/*
	 *	set up the socket to make the call on
	 */

	fd = at_open_dynamic_socket(O_RDWR, NULL);
	if (fd < 0) {
                at_error(3405,fn,"socket open failed");
                return(-1);
        }
	if (ioctl(fd, I_PUSH, "at_nbp") < 0) {
                at_error(3406,fn,"socket push failed");
quit:
		at_close_dynamic_socket(fd);
		return(-1);
	}
	if (ioctl(fd, I_STR, &s) < 0) {
                at_error(3407,fn,"name registration failed");
		goto quit;
	}
	at_close_dynamic_socket(fd);

        /*----------------------------------------*/
        /* Return the NVE ``registration number'' */
        /* for later deregistration use.          */
        /*----------------------------------------*/
        return(*(int *)buff);
        }
