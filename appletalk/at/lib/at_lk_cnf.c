#ifndef lint	/* .../appletalk/at/lib/at_lk_cnf.c */
#define _AC_NAME at_lk_cnf_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:59:06}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of at_lk_cnf.c on 87/11/11 20:59:06";
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
#include <sys/stropts.h>

#define _AC_MODS
static char *Version3 = AT_APPLETALK_VERSION_AT_LIB;

extern  void    exit();


int at_lookup_or_confirm_nve(object,object_len,type,type_len,zone,zone_len,
                             trys,trywait,net,node,mtype)
        char    *object;
        int      object_len;
        char    *type;
        int      type_len;
        char    *zone;
        int      zone_len;
        int      trys;
        int      trywait;
	int	 net;
	int	 node;
        int      mtype;
        {
        char   *fn;
        at_nve *nveptr;
        at_nve *nveptr_next;
	int fd;
	static char buff[NBP_NAME_MAX+100];
	int cnt;
	char *cp, *xcp;
	at_nbp *nbp;
	at_nbp_tuple_hdr *tp;
	struct strioctl s;
	int i;
	struct nbp_param *pp;

        fn = "at_lookup_or_confirm_nve";

        /*------------------------------------------------*/
        /* This function handles both confirm and lookup. */
        /*------------------------------------------------*/
        if (mtype != AT_NBP_LOOKUP  && 
            mtype != AT_NBP_CONFIRM &&
            mtype != AT_NBP_LOOK_LOCAL)
            {
            at_error(2300,fn,"bad mtype value (0x%x)",mtype);
            return(-1);
            }

        /*----------------------------------*/
        /* Check the NVE name for validity. */
        /*----------------------------------*/
        if (object_len == 0)
            {
            object_len = strlen(object);
            }
        if (type_len == 0)
            {
            type_len = strlen(type);
            }
        if (zone_len == 0)
            {
            zone_len = strlen(zone);
            }
        if (!(1 <= object_len  &&  object_len <= AT_NBP_TUPLE_STRING_MAXLEN))
            {
            at_error(2301,fn,"object str length range error (%d)",object_len);
            return(-1);
            }
        if (!(1 <= type_len  &&  type_len <= AT_NBP_TUPLE_STRING_MAXLEN))
            {
            at_error(2302,fn,"type string length out of range (%d)",type_len);
            return(-1);
            }
        if (!(1 <= zone_len  &&  zone_len <= AT_NBP_TUPLE_STRING_MAXLEN))
            {
            at_error(2303,fn,"zone string length out of range (%d)",zone_len);
            return(-1);
            }

        /*--------------------------------------------*/
        /* Clean up the NVE lookup reply linked list. */
        /*--------------------------------------------*/
        nveptr = at_nve_lkup_reply_head;
        while (nveptr != (at_nve *)NULL)
            {
            nveptr_next = nveptr->at_nve_next;
            free((char *)nveptr);
            nveptr = nveptr_next;
            }
        at_nve_lkup_reply_head  = (at_nve *)NULL;
        at_nve_lkup_reply_tail  = (at_nve *)NULL;
        at_nve_lkup_reply_count = 0;

        /*--------------------------------------------------*/
        /* Fill in the fields of the NVE structure we will  */
        /* be sending to the NBP daemon to do a lookup for. */
        /*--------------------------------------------------*/

	pp = (struct nbp_param *)buff;
	pp->nbp_retries = trys;
	pp->nbp_secs = trywait;
	nbp = NBP_NBP(&buff[sizeof(struct nbp_param)]);
        nbp->at_nbp_tuple_count = 1;
	tp = (at_nbp_tuple_hdr *)nbp->at_nbp_tuples;
	tp->at_nbp_tuple_socket = 0;
	W16(tp->at_nbp_tuple_net, net);		/* Only meaningful when confirming. */
	tp->at_nbp_tuple_node = node;		/* Only meaningful when confirming. */
	cp = (char *)tp->at_nbp_tuple_data;
	*cp++ = object_len;
	strcpy(cp, object);
	cp += object_len;
	*cp++ = type_len;
	strcpy(cp, type);
	cp += type_len;
	*cp++ = zone_len;
	strcpy(cp, zone);
	cp += zone_len;
	s.ic_len = cp - buff;
	s.ic_dp = buff;
	s.ic_timout = -1;
	s.ic_cmd = mtype;

	/*
	 *	set up the socket to make the call on
	 */

	fd = at_open_dynamic_socket(O_RDWR, NULL);
	if (fd < 0) {
                at_error(2304,fn,"socket open failed");
                return(-1);
        }
	if (ioctl(fd, I_PUSH, "at_nbp") < 0) {
                at_error(2305,fn,"socket push failed");
		at_close_dynamic_socket(fd);
		return(-1);
	}
	if (ioctl(fd, I_STR, &s) < 0) {
		at_close_dynamic_socket(fd);
		return(0);
	}
	at_close_dynamic_socket(fd);

	/*
	 *	Unpack the name
	 */

	nbp = NBP_NBP(buff);
	tp = (at_nbp_tuple_hdr *)nbp->at_nbp_tuples;
	cnt = (((unsigned char *)nbp)[0]<<8) | ((unsigned char *)nbp)[1];
	cp = (char *)tp;
	cnt -= (AT_NBP_HDR_SIZE + AT_DDP_X_HDR_SIZE);
	while (cnt > 0) {
                nveptr = (at_nve *)malloc((unsigned)sizeof(at_nve));
                if (nveptr == (at_nve *)NULL)
                    {
                    at_error(2306,fn,"bad return from malloc()");
                    return(-1);
                    }
                nveptr->at_nve_next = (at_nve *)NULL;
                nveptr->at_nve_prev = (at_nve *)NULL;
                if (at_nve_lkup_reply_head == (at_nve *)NULL)
                    {
                    at_nve_lkup_reply_head = nveptr;
                    at_nve_lkup_reply_tail = nveptr;
                    }
                else
                    {
                    at_nve_lkup_reply_tail->at_nve_next = nveptr;
                    nveptr->at_nve_prev = at_nve_lkup_reply_tail;
                    at_nve_lkup_reply_tail = nveptr;
                    }
                at_nve_lkup_reply_count++;
        	nveptr->at_nve_net = R16(tp->at_nbp_tuple_net);
        	nveptr->at_nve_node = tp->at_nbp_tuple_node;
        	nveptr->at_nve_socket = tp->at_nbp_tuple_socket;
        	cp = (char *)tp->at_nbp_tuple_data;
        	nveptr->at_nve_object_length = *cp++;
		xcp = cp;
		cp += nveptr->at_nve_object_length;
        	nveptr->at_nve_type_length = *cp;
		*cp++ = '\0';
		strcpy(nveptr->at_nve_object, xcp);
		xcp = cp;
		cp += nveptr->at_nve_type_length;
        	nveptr->at_nve_zone_length = *cp;
		*cp++ = '\0';
		strcpy(nveptr->at_nve_type, xcp);
		xcp = cp;
		cp += nveptr->at_nve_zone_length;
		i = *cp;
		*cp = '\0';
		strcpy(nveptr->at_nve_zone, xcp);
		*cp = i;
		if ((cp - (char *)tp) > cnt)
			break;
		cnt -= (cp - (char *)tp);
		tp = (at_nbp_tuple_hdr *)(cp);
	}
        return(at_nve_lkup_reply_count);
        }
