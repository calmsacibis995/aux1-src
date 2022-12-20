#ifndef lint	/* .../appletalk/at/ddp/at_ddp.h */
#define _AC_NAME at_ddp_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:57:18}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.2 of at_ddp.h on 87/11/11 20:57:18 VV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *      This is the header file for the at_ddp module.
 */


/*+-------------------------------------------------------------------------+
  |     S T R U C T U R E and T Y P E  D E F I N I T I O N S                |
  +-------------------------------------------------------------------------+*/

/*+-------------------------------------------------------------------------+
  |     D A T A G R A M    P A C K E T    (DDP EXTENDED HEADER)             |
  |	used internal to the ddp module, has lap information for passing    |
  |	to the LAP module below.					    |
  +-------------------------------------------------------------------------+*/
typedef struct
        {
        at_node    lap_dst_node       ;  /* LAP, Destination address.  */
        at_node    lap_src_node       ;  /* LAP, Source address.       */
        at_type    lap_type           ;  /* LAP, Frame type.           */
        at_net     length             ;  /* Datagram length & hop count.*/
        at_chksum  checksum           ;  /* Checksum.                   */
        at_net     dst_net            ;  /* Destination network number. */
        at_net     src_net            ;  /* Source network number.      */
        at_node    dst_node           ;  /* Destination node ID.        */
        at_node    src_node           ;  /* Source node ID.             */
        at_socket  dst_socket         ;  /* Destination socket number.  */
        at_socket  src_socket         ;  /* Source socket number.       */
        at_type    type               ;  /* Protocol type.              */
        u8         data[586]          ;  /* The DataGram buffer.        */
        }          lap_ddpx_t         ;

/*+-------------------------------------------------------------------------+
  |     D A T A G R A M    P A C K E T    (DDP SHORT HEADER)                |
  |	used internal to the ddp module, has lap information for passing    |
  |	to the LAP module below.					    |
  +-------------------------------------------------------------------------+*/
typedef struct
        {
        at_node    lap_dst_node       ;  /* LAP, Destination address.  */
        at_node    lap_src_node       ;  /* LAP, Source address.       */
        at_type    lap_type           ;  /* LAP, Frame type.           */
        at_length  length             ;  /* Datagram length (10 bits). */
        at_socket  dst_socket         ;  /* Destination socket number. */
        at_socket  src_socket         ;  /* Source socket number.      */
        at_type    type               ;  /* Protocol type.             */
        u8         data[594]          ;  /* The DataGram buffer.       */
        }          lap_ddp_t          ;



/* forward reference */
typedef struct ddp_specifics ddp_specifics_t;


typedef struct
{
    at_socket	number;		/* the number of this socket, saves dividing */
    u8 		close;		/* notify NIS on close flag */
    queue_t	*upstreamq;	/* pointer to the upstream q, used to demux */
				/* for 0th q, is multiplexer q, points to self*/
    ddp_specifics_t *ddp;	/* pointer to the local data structure which
				 * differentiates this stream from other
				 * network's streams
				 */
}   ddp_socket_t;
#define	SOCKET(Q)		((ddp_socket_t *) Q->q_ptr)




struct ddp_specifics
{
    u16		  nsockets;	/* number of sockets configured */
    ddp_socket_t  *socket0p;	/* so the interrupt routine can find demux qs */
    queue_t	  *downstreamq;	/* pointer to the downstream q, used to mux */
    at_ddp_cfg_t  cfg;		/* the cfg structure */
};



typedef struct
{
    at_socket	socket;
}   ddp_tags_t;

#define	DDP_IOCTAGS(m)	   ((ddp_tags_t *) (m->b_wptr))
