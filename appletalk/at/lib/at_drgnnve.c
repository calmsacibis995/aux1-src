#ifndef lint	/* .../appletalk/at/lib/at_drgnnve.c */
#define _AC_NAME at_drgnnve_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:58:43}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_drgnnve.c on 87/11/11 20:58:43";
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


int at_deregister_name_nve(object,object_len,type,type_len)
        char    *object;
        int      object_len;
        char    *type;
        int      type_len;
        {
        char  			*fn;
	int			fd;
	char			buff[NBP_NAME_MAX];
	char			*cp;
	at_nbp			*nbp;
	at_nbp_tuple_hdr	*tp;
	struct strioctl		s;
	struct nbp_param	*pp;

        fn = "at_delete_name_nve";
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
            at_error(1950,fn,"object string length range error (%d)",object_len);
            return(-1);
            }
        if (!(1 <= type_len  &&  type_len <= AT_NBP_TUPLE_STRING_MAXLEN))
            {
            at_error(1951,fn,"type string length out of range (%d)",type_len);
            return(-1);
            }

        /*-------------------------------------*/
        /* Make sure that the object and type  */
        /* fields are not wildcard characters. */
        /*-------------------------------------*/
        if (object_len == 1  &&  object[0] == '=')
            {
            at_error(1952,fn,"object field can not be deleted as a wildcard");
            return(-1);
            }
        if (type_len == 1  &&  type[0] == '=')
            {
            at_error(1953,fn,"type field can not be deleted as a wildcard");
            return(-1);
            }


        /*---------------------------------------*/
        /* Setup the NVE with our NVE arguments. */
        /*---------------------------------------*/

	nbp = NBP_NBP(&buff[sizeof(struct nbp_param)]);
        nbp->at_nbp_tuple_count = 1;
	tp = (at_nbp_tuple_hdr *)nbp->at_nbp_tuples;
	tp->at_nbp_tuple_socket = 0;
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
	s.ic_cmd = AT_NBP_DELETE_NAME;

	/*
	 *	set up the socket to make the call on
	 */

	fd = at_open_dynamic_socket(O_RDWR, NULL);
	if (fd < 0) {
                at_error(1954,fn,"socket open failed");
                return(-1);
        }
	if (ioctl(fd, I_PUSH, "at_nbp") < 0) {
                at_error(1955,fn,"socket push failed");
quit:
		at_close_dynamic_socket(fd);
		return(-1);
	}
	if (ioctl(fd, I_STR, &s) < 0) {
                at_error(1956,fn,"name deletion failed");
		goto quit;
	}
	at_close_dynamic_socket(fd);

        /*----------------------------------------*/
        /* Return the NVE ``registration number'' */
        /* for later deregistration use.          */
        /*----------------------------------------*/
        return(0);
        }
