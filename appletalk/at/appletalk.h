#ifndef lint	/* .../appletalk/at/appletalk.h */
#define _AC_NAME appletalk_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:04:44}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.2 of appletalk.h on 87/11/11 21:04:44 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>

/*+
 *
 *  Description:
 *      This is the header file containing all the definitions,
 *      data structures, and magic numbers used in UniSoft's
 *      implementation of Appletalk.
 *
 *  SCCS:
 *      @(#)appletalk.h	1.3 4/9/87
 *
 *  Copyright:
 *      Copyright 1985 by UniSoft Systems Corporation.
 *
 *  History:
 *      24-Jun-85: Created by Philip K. Ronzone.
 *      19-Jul-85: PKR. Start adding #define sizes for the headers.
 *                 C compiler likes to round odd sized structures
 *                 to an even number of bytes. Sigh .....
 *      21-Jul-85: PKR. Finish the adjustments for the fact that the
 *                 C compiler will round up odd-number-of-bytes sized
 *                 data structures to an even number of bytes.
 *	20-August-86: Kip, changes to support the conversion to streams
 *
 *  Warning:
 *      ****************************************************
 *      *                 ** WARNING **                    *
 *      ****************************************************
 *      *  Due to the fact that the C compiler rounds      *
 *      *  up odd-number-of-bytes size data structures     *
 *      *  to an even number of bytes, do NOT use the      *
 *      *  C language sizeof feature to `get' the size     *
 *      *  of the various structures defined in this       *
 *      *  header file. Instead, use the various defines.  *
 *      ****************************************************
 *
-*/

typedef char                  s8;       /* Signed  8 bits.   */
typedef short                s16;       /* Signed 16 bits.   */
typedef int                  s32;       /* Signed 32 bits.   */

typedef unsigned char         u8;       /* Unsigned  8 bits. */
typedef unsigned short       u16;       /* Unsigned 16 bits. */
typedef unsigned int         u32;       /* Unsigned 32 bits. */

typedef unsigned char  at_2bytes[2];    /* AppleTalk 16-bit number.          */
typedef unsigned char  at_4bytes[4];    /* AppleTalk 32-bit number.          */
typedef at_2bytes      at_chksum;       /* AppleTalk DataGram checksum.      */
typedef at_2bytes      at_length;       /* AppleTalk DataGram length.        */
typedef at_2bytes      at_net;          /* AppleTalk network number.         */
typedef unsigned char  at_node;         /* AppleTalk node number.            */
typedef unsigned char  at_socket;       /* AppleTalk socket number.          */
typedef unsigned char  at_type;         /* AppleTalk generic type field.     */

/*+-------------------------------------------------------------------------+
  |     D E F I N E S                                                       |
  +-------------------------------------------------------------------------+*/
#define  AT_APPLETALK_VERSION_GENERAL "@(#)appletalk.h	1.3 4/9/87 (appletalk.h in this module)"
#define  AT_APPLETALK_VERSION_AT_LIB  "@(#)appletalk.h	1.3 4/9/87 (appletalk.h in at_lib.c)"

#define AT_ERROR_ON			1
#define AT_ERROR_OFF			0

#define	AT_ATP_CANCEL_REQUEST		(('|'<<8)|1)
#define	AT_ATP_ISSUE_REQUEST		(('|'<<8)|2)
#define	AT_ATP_ISSUE_REQUEST_DEF	(('|'<<8)|3)
#define	AT_ATP_ISSUE_REQUEST_DEF_NW	(('|'<<8)|4)
#define	AT_ATP_ISSUE_REQUEST_DEF_NOTE	(('|'<<8)|5)
#define	AT_ATP_ISSUE_REQUEST_NOTE	(('|'<<8)|6)
#define	AT_ATP_ISSUE_REQUEST_NW		(('|'<<8)|7)
#define	AT_ATP_GET_REQUEST		(('|'<<8)|8)
#define	AT_ATP_GET_REQUEST_NOTE		(('|'<<8)|9)
#define	AT_ATP_GET_REQUEST_NW		(('|'<<8)|10)
#define	AT_ATP_GET_POLL			(('|'<<8)|11)
#define	AT_ATP_POLL_REQUEST		(('|'<<8)|12)
#define	AT_ATP_RELEASE_RESPONSE		(('|'<<8)|13)
#define	AT_ATP_SEND_RESPONSE		(('|'<<8)|14)
#define	AT_ATP_SEND_RESPONSE_EOF	(('|'<<8)|15)
#define	AT_ATP_SET_DEFAULT		(('|'<<8)|16)
#define	AT_ATP_SET_FINISH		(('|'<<8)|17)


#define  ATP_INFINITE_RETRIES	0xffffffff	/* means retry for ever in the 
						   def_retries field */
#define  AT_ATP_CMD_TREL              0x03  /* TRelease packet.              */
#define  AT_ATP_CMD_TREQ              0x01  /* TRequest packet.              */
#define  AT_ATP_CMD_TRESP             0x02  /* TResponse packet.             */
#define  AT_ATP_DATA_SIZE              578  /* Size of the ATP data area.    */
#define  AT_ATP_HDR_SIZE                 8  /* Size of the ATP header.       */
#define  AT_ATP_TRESP_MAX                8  /* Maximum number of Tresp pkts. */

#define	 ATP_HDR_SIZE			(AT_ATP_HDR_SIZE+AT_DDP_X_HDR_SIZE)
#define  ATP_DATA_SIZE   		(ATP_HDR_SIZE + AT_ATP_DATA_SIZE)

#define  AT_BUF_MAX                     64  /* No. of buffers in the driver. */

#define  AT_CTL_FILENAME              "/dev/appletalk/control"

#define  AT_DDP_CHECKSUM_FUDGE           7  /* DDP X hdr bytes NOT under CRC.*/
#define  AT_DDP_DATA_SIZE              586  /* Maximum DataGram data size.   */
#define  AT_DDP_DATAGRAM_SIZE          600  /* Maximum DataGram size.        */
#define  AT_DDP_HDR_SIZE                 5  /* DDP (short) header size.      */
#define  AT_DDP_X_HDR_SIZE              13  /* DDP extended header size.     */
#define  AT_DDP_SOCKET_MAX             256  /* Maximum number of sockets + 1.*/
#define  AT_DDP_SOCKET_1st_RESERVED      1  /* First in reserved range       */
#define  AT_DDP_SOCKET_RTMP           0x01  /* RTMP socket number.           */
#define  AT_DDP_SOCKET_NIS            0x02  /* NIS socket number.            */
#define  AT_DDP_SOCKET_1st_EXPERIMENTAL 64  /* First in experimental range   */
#define  AT_DDP_SOCKET_1st_DYNAMIC     128  /* First in dynamic range        */
#define  AT_DDP_SOCKET_LAST            254  /* Last socket in any range	     */
#define  AT_DDP_TYPE_ATP              0x03  /* ATP packet.                   */
#define  AT_DDP_TYPE_NBP              0x02  /* NBP packet.                   */
#define  AT_DDP_TYPE_RTMP             0x01  /* RTMP packet.                  */
#define  AT_DDP_TYPE_RTMP_PLUS        0x05  /* RTMP+ packet.                 */

#define  AT_ENF_SIZE                    33  /* Entity name field size.       */

#define  AT_FTOK_ID_NBP_DAEMON         'D'  /* ID argument for ftok(), NBP d.*/
#define  AT_FTOK_ID_USER               'U'  /* ID argument for ftok(), users.*/

#define  AT_LAP_SIZE                   603  /* LAP frame size.               */
#define  AT_LAP_HDR_SIZE                 3  /* LAP header size.              */
#define  AT_LAP_TYPE_ACK              0x82  /* Acknowledge frame.            */
#define  AT_LAP_TYPE_CTS              0x85  /* Clear To Send frame.          */
#define  AT_LAP_TYPE_DDP              0x01  /* DDP short header packet.      */
#define  AT_LAP_TYPE_DDP_X            0x02  /* DDP extended header packet.   */
#define  AT_LAP_TYPE_ENQ              0x81  /* Enquiry frame.                */
#define  AT_LAP_TYPE_RTS              0x84  /* Request To Send frame.        */
#define	 AT_LAP_DEFAULT_NODE	      0x40  /* where to start the ENQ search */
#define	 AT_LAP_DEFAULT_RETRIES	        32  /* default # of time to send RTS */

#define  AT_KILL_NBPD      	      "/usr/lib/appletalk/at_nbpd_kill"

#define  AT_NBP_BRRQ                  0x01  /* Broadcast request.            */
#define  AT_NBP_HDR_SIZE		 2  /* NBP (packet) header size      */
#define	 NBP_HDR_SIZE	(AT_NBP_HDR_SIZE + AT_DDP_X_HDR_SIZE + AT_NBP_TUPLE_HDR_SIZE)
#define  AT_NBP_LKUP                  0x02  /* Lookup.                       */
#define  AT_NBP_LKUP_REPLY            0x03  /* Lookup reply.                 */
#define  AT_NBP_TUPLE_MAX               15  /* Max. no. NBP tuples / packet. */
#define  AT_NBP_TUPLE_HDR_SIZE           5  /* NBP tuple header size.        */
#define  AT_NBP_TUPLE_STRING_MAXLEN     32  /* Max. NBP tuple string size.   */
#define  NBP_NAME_MAX	8192

/*
 *	Returns from AT_NBPD_GET_NEXT 
 */

#define	NBPD_REGISTER		1		/* register a name */
#define	NBPD_DELETE		2		/* delete a name */
#define	NBPD_LOCAL_LOOKUP	3		/* lookup a name */
#define	NBPD_REMOTE_LOOKUP	4		/* lookup a name */
#define	NBPD_SHUTDOWN		5		/* shutdown */
#define	NBPD_CLOSE		6		/* someone closed */
#define	NBPD_DELETE_NAME	7		/* delete a name by name */


#define  AT_NVE_REPEAT_MAXIMUM         128  /* Maximum BrRq repeats.         */
#define  AT_NVE_REPEAT_MINIMUM           3  /* Minimum BrRq repeats.         */
#define  AT_NVE_SECONDS_MAXIMUM         60  /* Maximum seconds between BrRqs.*/
#define  AT_NVE_SECONDS_MINIMUM          2  /* Minimum seconds between BrRqs.*/

#define  AT_PACKET_SIZE                603   /* Maximum packet size.         */


#define	AT_PAP_SETHDR		(('~'<<8)|0)
#define	AT_PAP_READ		(('~'<<8)|1)
#define	AT_PAP_WRITE		(('~'<<8)|2)
#define	AT_PAP_WRITE_EOF	(('~'<<8)|3)
#define	AT_PAP_WRITE_FLUSH	(('~'<<8)|4)
#define	AT_PAP_READ_IGNORE	(('~'<<8)|5)

#define	AT_PAPD_SET_STATUS	(('~'<<8)|40)
#define	AT_PAPD_GET_NEXT_JOB	(('~'<<8)|41)

#define AT_PAP_HDR_SIZE	(AT_DDP_X_HDR_SIZE + AT_ATP_HDR_SIZE)

#ifndef NULL
#define	NULL	0
#endif NULL

#define  AT_PAP_TYPE_OPEN_CONN        0x01   /* Open-Connection packet.      */
#define  AT_PAP_TYPE_OPEN_CONN_REPLY  0x02   /* Open-Connection-Reply packet.*/
#define  AT_PAP_TYPE_SEND_DATA        0x03   /* Send-Data packet.            */
#define  AT_PAP_TYPE_DATA             0x04   /* Data packet.                 */
#define  AT_PAP_TYPE_TICKLE           0x05   /* Tickle packet.               */
#define  AT_PAP_TYPE_CLOSE_CONN       0x06   /* Close-Connection packet.     */
#define  AT_PAP_TYPE_CLOSE_CONN_REPLY 0x07   /* Close-Connection-Reply pkt.  */
#define  AT_PAP_TYPE_SEND_STATUS      0x08   /* Send-Status packet.          */
#define  AT_PAP_TYPE_SEND_STS_REPLY   0x09   /* Send-Status-Reply packet.   */

#define	 FLUSHALL		      1     /* parm for streams flushq */

#define  NBP_DDP(p)	(at_ddp_t *)(p)
#define	 NBP_NBP(c)	((at_nbp *)(&((at_ddp_t *)(c))->data[0]))
#define	 ATP_DDP_HDR(c)	((at_ddp_t *)(c))
#define	 ATP_ATP_HDR(c)	((at_atp *)(&((at_ddp_t *)(c))->data[0]))
#define	 ATP_DATA(c)	(&((char *)(c))[ATP_HDR_SIZE])


#define R16(x)		(((((unsigned char *)(x))[0])<<8)|(((unsigned char *)(x))[1]))
#define W16(x, v)	{unsigned short u = (unsigned short)v;\
			 ((unsigned char *)(x))[0] = u >> 8;\
			 ((unsigned char *)(x))[1] = u;}
#define C16(x, v)	{((unsigned char *)(x))[0] = ((unsigned char *)(v))[0]; \
			 ((unsigned char *)(x))[1] = ((unsigned char *)(v))[1];}


/*+-------------------------------------------------------------------------+
  |  The ioctl definitions.                                                 |
  +-------------------------------------------------------------------------+*/

#define  AT__                     0x41545f30

#define  AT_SYNC                  	(AT__ +1)

#define  AT_LAP_ONLINE			(AT__ +2)
#define  AT_LAP_OFFLINE			(AT__ +3)
#define  AT_LAP_LOOKUP			(AT__ +4)
#define  AT_LAP_REGISTER		(AT__ +5)
#define  AT_LAP_GET_CFG			(AT__ +6)
#define  AT_LAP_SET_CFG			(AT__ +7)
#define  AT_DDP_GET_CFG			(AT__ +8)
#define  AT_DDP_IS_OPEN			(AT__ +9)
/*
 *	ioctls for use by clients
 */
#define	 AT_NBP_LOOKUP		(('$'<<8)|1)	/* do a lookup */
#define	 AT_NBP_CONFIRM		(('$'<<8)|2)	/* do a confirm */
#define	 AT_NBP_REGISTER	(('$'<<8)|3)	/* do a register */
#define	 AT_NBP_REGISTER_DONE	(('$'<<8)|4)	/* do a register  (internal interface) */
#define	 AT_NBP_DELETE		(('$'<<8)|5)	/* do a delete */
#define	 AT_NBP_SHUTDOWN	(('$'<<8)|6)	/* shut down the server */
#define	 AT_NBP_LOOK_LOCAL	(('$'<<8)|7)	/* do a lookup only locally */
#define	 AT_NBP_DELETE_NAME	(('$'<<8)|8)	/* do a delete by name */
/*
 *	ioctls for use by the name server
 */
#define	 AT_NBPD_GET_NEXT	(('$'<<8)|30)	/* get the next function request */
#define	 AT_NBPD_REGISTER_DONE	(('$'<<8)|31)	/* mark a register as done */
#define	 AT_NBPD_DELETE_DONE	(('$'<<8)|32)	/* mark a delete as done */
#define	 AT_NBPD_LOCAL_DONE	(('$'<<8)|33)	/* local lookup done */
#define	 AT_NBPD_LOCAL		(('$'<<8)|34)	/* local reply */
#define	 AT_NBPD_REMOTE		(('$'<<8)|35)	/* remote reply */
#define	 AT_NBPD_SHUTDOWN_DONE	(('$'<<8)|36)	/* shutdown done */


/*+-------------------------------------------------------------------------+
  |     L A P   H E A D E R                                                 |
  +-------------------------------------------------------------------------+*/
typedef struct
        {
        at_node  destination      ;  /* Destination address. */
        at_node  source           ;  /* Source address.      */
        at_type  type             ;  /* Frame type.          */
        }        at_lap_hdr_t     ;  /* LAP frame.           */


/*+-------------------------------------------------------------------------+
  |     L A P   P A C K E T                                                 |
  +-------------------------------------------------------------------------+*/
typedef struct
        {
        at_node  destination      ;  /* Destination address. */
        at_node  source           ;  /* Source address.      */
        at_type  type             ;  /* Frame type.          */
        char     data[600]        ;  /* data area.           */
        }        at_lap_t         ;  /* LAP frame.           */




/*+-------------------------------------------------------------------------+
  |     D A T A G R A M    P A C K E T    (DDP EXTENDED HEADER)             |
  +-------------------------------------------------------------------------+*/
typedef struct
        {
        at_length  length             ;  /* Datagram length & hop count.*/
        at_chksum  checksum           ;  /* Checksum.                   */
        at_net     dst_net            ;  /* Destination network number. */
        at_net     src_net            ;  /* Source network number.      */
        at_node    dst_node           ;  /* Destination node ID.        */
        at_node    src_node           ;  /* Source node ID.             */
        at_socket  dst_socket         ;  /* Destination socket number.  */
        at_socket  src_socket         ;  /* Source socket number.       */
        at_type    type               ;  /* Protocol type.              */
        u8         data[586]          ;  /* The DataGram buffer.        */
        }          at_ddp_t           ;  /* The DataGram (extended hdr).*/

/*+-------------------------------------------------------------------------+
  |     R O U T I N G    T A B L E    M A I N T .    P R O T O C O L        |
  +-------------------------------------------------------------------------+*/
typedef struct
        {
        at_net  at_rtmp_this_net   ;
        u8      at_rtmp_id_length  ;
        u8      at_rtmp_id[1]      ;
        }       at_rtmp            ;

typedef struct
        {
        at_net  at_rtmp_net        ;
        u8      at_rtmp_distance   ;
        }       at_rtmp_tuple      ;



/*+-------------------------------------------------------------------------+
  |     N A M E    B I N D I N G    P R O T O C O L    P A C K E T          |
  +-------------------------------------------------------------------------+*/
struct nbp_param {
	int		nbp_retries;
	int		nbp_secs;
};

struct nbp_addr {
	at_net		 net;
	at_node		 node;
	at_socket	 socket;
};

struct nbp_wild {
	at_net		net;
	at_node		node;
	at_socket	socket;
	unsigned char	enumerator;
	char		*object;
	char		*type;
	char		*zone;
};

typedef struct
        {
        u8      at_nbp_ctl          : 4 ;
        u8      at_nbp_tuple_count  : 4 ;
        u8      at_nbp_id               ;
        u8      at_nbp_tuples[570]      ;
        }       at_nbp                  ;

typedef struct
        {
        at_net     at_nbp_tuple_net        ;
        at_node    at_nbp_tuple_node       ;
        at_socket  at_nbp_tuple_socket     ;
        u8         at_nbp_tuple_enumerator ;
        u8         at_nbp_tuple_data[565]  ;
        }          at_nbp_tuple_hdr        ;


/*+-------------------------------------------------------------------------+
  |     A P P L E T A L K    T R A N S A C T I O N    P R O T O C O L       |
  |                               P A C K E T                               |
  +-------------------------------------------------------------------------+*/
typedef struct
        {
        u8         at_atp_cmd             : 2     ;
        u8         at_atp_xo              : 1     ;
        u8         at_atp_eom             : 1     ;
        u8         at_atp_sts             : 1     ;
        u8         at_atp_unused          : 3     ;
        u8         at_atp_bitmap_seqno            ;
        at_2bytes  at_atp_transaction_id          ;
        at_4bytes  at_atp_user_bytes              ;
        u8         at_atp_data[AT_ATP_DATA_SIZE]  ;
        }          at_atp                         ;

/*
 *	Structure for the ATP_SET_DEFAULT call
 */

struct atp_set_default {
	unsigned int	def_retries;		/* number of retries for a request */
	unsigned int	def_rate;		/* retry rate (in seconds/100) NB: the
						   system may not be able to resolve
						   delays of 100th of a second but will
						   instead make a 'best effort' */
};

/*
 *	Return header from requests
 */

struct atp_result {
	unsigned short	count;		/* the number of packets */
	unsigned short	hdr;		/* offset to header in buffer */
	unsigned short  offset[8];	/* offset to the Nth packet in the buffer */
	unsigned short	len[8];		/* length of the Nth packet */
};


/*+-------------------------------------------------------------------------+
  |     A T P    R E A D / W R I T E    I N T E R F A C E                   |
  +-------------------------------------------------------------------------+*/
typedef struct
        {
        u16        at_atpreq_type                                ;
        at_net     at_atpreq_to_net                              ;
        at_node    at_atpreq_to_node                             ;
        at_socket  at_atpreq_to_socket                           ;
        at_4bytes  at_atpreq_treq_user_bytes                     ;
        u8        *at_atpreq_treq_data                           ;
        u16        at_atpreq_treq_length                         ;
        u8         at_atpreq_treq_bitmap                         ;
        u8         at_atpreq_xo                                  ;
        u16        at_atpreq_retry_timeout                       ;
        u16        at_atpreq_maximum_retries                     ;
        at_4bytes  at_atpreq_tresp_user_bytes[AT_ATP_TRESP_MAX]  ;
        u8        *at_atpreq_tresp_data[AT_ATP_TRESP_MAX]        ;
        u16        at_atpreq_tresp_lengths[AT_ATP_TRESP_MAX]     ;
        u32        at_atpreq_debug[4]                            ;
        u16        at_atpreq_tid                                 ;
        u8         at_atpreq_tresp_bitmap                        ;
        u8         at_atpreq_tresp_eom_seqno                     ;
        u8         at_atpreq_got_trel                            ;
        }          at_atpreq                                     ;




/*+-------------------------------------------------------------------------+
  |     P R I N T E R    A C C E S S    P R O T O C O L                     |
  +-------------------------------------------------------------------------+*/
typedef struct
        {
        u8         at_pap_connection_id        ;
        at_type    at_pap_type                 ;
        at_2bytes  at_pap_sequence_number      ;
        at_socket  at_pap_responding_socket    ;
        u8         at_pap_flow_quantum         ;
        at_2bytes  at_pap_wait_time_or_result  ;
        u8         at_pap_buffer[512]          ;
        }          at_pap                      ;

/*+-------------------------------------------------------------------------+
  |     I O C T L    A N D    I / O    D R I V E R    D E F I N I T I O N S |
  +-------------------------------------------------------------------------+*/

typedef struct
        {
	u32  unknown_irupts;	/* number of unexpected interrupts, recovery */
	u32  unknown_mblks;	/* number of unknown stream messages */
	u32  ioc_unregistered;	/* number of AT_SYNCs on sockets that closed */
				/* before it filtered through q for response */
	u32  timeouts;	        /* number of state timeouts occured */
	u32  rcv_bytes;	  	/* number of productive data bytes received */
	u32  rcv_packets;	/* number of productive packets received */
	u32  type_unregistered; /* number of packets thrown away because */
				/* no one is no one listenning for it */
	u32  overrun_errors;    /* number of overruns received */
	u32  abort_errors;      /* number of aborts received */
	u32  crc_errors;	/* number of crc errors received */
	u32  too_long_errors;   /* number of packets which are too big */
	u32  too_short_errors;  /* number of packets which are too small */
	u32  missing_sync_irupt;/* number of missing sync interrupts while */
				/* while waiting for access to a busy net */
	u32  xmit_bytes;	/* number of productive data bytes xmited */
	u32  xmit_packets;	/* number of productive packets xmited */
	u32  collisions;	/* number of collisions on xmit */
	u32  defers;		/* number of defers on xmit */
	u32  underrun_errors;   /* number of underruns transmitted */
        }       at_lap_stats_t;

typedef struct
        {
        short    network_up        ;  /* 0=network down, nonzero up.  */
        int      node              ;  /* Our node number.             */
        int      initial_node      ;  /* Our initial node address.    */
        int      rts_attempts      ;  /* RTS attempts on write.       */
	at_lap_stats_t stats	   ;  /* general stats		      */
        }        at_lap_cfg_t      ;

typedef struct
	{
	short	type		   ;  /* LAP type */
	short	circuitx	   ;  /* the x in the lap dirrectory file */
	char	name[14]	   ;  /* the name given to lapo by the appl */
	}       at_lap_entry_t	   ;

typedef struct
        {
	/* receive errors */
	int	socket_unregistered;
	int	rcv_socket_outrange;
	int	rcv_length_errors;
	int	rcv_checksum_errors;
	/* transmit errors */
	int	tag_room_errors;
        }       at_ddp_stats_t;

typedef struct
        {
        u16      network_up             ;  /* Nonzero is network number.   */
        u16      this_net               ;  /* Nonzero is network number.   */
        at_node  this_node              ;  /* Our node number.             */
        at_node  a_bridge               ;  /* Node number of Abridge.      */
	at_ddp_stats_t stats	        ;  /* general stats		     */
        }        at_ddp_cfg_t           ;


typedef struct  at_nve_struct
        {
        struct   at_nve_struct  *at_nve_next                 ;
        struct   at_nve_struct  *at_nve_prev                 ;
        int      at_nve_net                                  ;
        int      at_nve_node                                 ;
        int      at_nve_socket                               ;
        char     at_nve_object[AT_NBP_TUPLE_STRING_MAXLEN+1] ;
        char     at_nve_type[AT_NBP_TUPLE_STRING_MAXLEN+1]   ;
        char     at_nve_zone[AT_NBP_TUPLE_STRING_MAXLEN+1]   ;
        int      at_nve_object_length                        ;
        int      at_nve_type_length                          ;
        int      at_nve_zone_length                          ;
        int      at_nve_pid                                  ;
        long     at_nve_timestamp                            ;
        int      at_nve_repeat                               ;
        int      at_nve_seconds                              ;
        }        at_nve                                      ;


/*+-------------------------------------------------------------------------+
  |     L I B A T . A    D E C L A R A T I O N S                            |
  +-------------------------------------------------------------------------+*/
 
#ifndef AT_NO_INCLUDE
extern  int     at_bridge_node                   ;
extern  char   *at_chooser_msg                   ;
extern  int     at_chooser_msg_arg1              ;
extern  int     at_chooser_msg_arg2              ;
extern  int     at_chooser_net                   ;
extern  int     at_chooser_node                  ;
extern  char    at_chooser_object[AT_NBP_TUPLE_STRING_MAXLEN+1];
extern  int     at_chooser_print_net             ;
extern  char   *at_chooser_print_net_fmt         ;
extern  char   *at_chooser_print_net_header      ;
extern  int     at_chooser_print_node            ;
extern  char   *at_chooser_print_node_fmt        ;
extern  char   *at_chooser_print_node_header     ;
extern  int     at_chooser_print_object          ;
extern  char   *at_chooser_print_object_header   ;
extern  int     at_chooser_print_socket          ;
extern  char   *at_chooser_print_socket_fmt      ;
extern  char   *at_chooser_print_socket_header   ;
extern  int     at_chooser_print_type            ;
extern  char   *at_chooser_print_type_header     ;
extern  int     at_chooser_print_zone            ;
extern  char   *at_chooser_print_zone_header     ;
extern  int     at_chooser_socket                ;
#ifdef	FILE
extern  FILE   *at_chooser_stdin                 ;
extern  FILE   *at_chooser_stdout                ;
#endif
extern  char    at_chooser_type[AT_NBP_TUPLE_STRING_MAXLEN+1];
extern  char    at_chooser_zone[AT_NBP_TUPLE_STRING_MAXLEN+1];
extern  int     at_ctl                           ;
extern  char   *at_ctl_filename                  ;
extern  at_ddp_t at_datagram                     ;
extern  int     at_errno                         ;
extern  int     at_got_net_number                ;
extern  int     at_init_ignore                   ;
extern  int     at_net_number                    ;
extern	char    at_network[MAXNAMLEN+1]		 ;
extern  int     at_node_number                   ;
extern  int     at_nve_lkup_reply_count          ;
extern  at_nve *at_nve_lkup_reply_head           ;
extern  at_nve *at_nve_lkup_reply_tail           ;
extern  char   *at_pgm                           ;
extern  int     at_pid                           ;
extern  char    at_pap_status[]			 ;
extern  int     at_error_status			 ;
 
extern  int     at_chooser()                     ;
extern  int     at_close_dynamic_socket()        ;
extern  int     at_close_socket()                ;
extern  int     at_confirm_nve()                 ;
extern  int     at_create_msg_queue()            ;
extern  int     at_decompose_en()                ;
extern  int     at_deregister_all()              ;
extern  int     at_deregister_nve()              ;
extern  int     at_done()                        ;
extern  void    at_error()                       ;
extern  int     at_execute()                     ;
extern  int     at_get_ddp_length()              ;
extern  int     at_get_dynamic_socket_number()   ;
extern  int     at_get_net_number()              ;
extern  int     at_get_node_number()             ;
extern  int     at_init()                        ;
extern  int     at_lookup_nve()                  ;
extern  int     at_lookup_or_confirm_nve()       ;
extern  int     at_open_dynamic_socket()         ;
extern  int     at_open_socket()                 ;
extern  void    at_ptm()                         ;
extern  int     at_read_datagram()               ;
extern  int     at_register_nve()                ;
extern  int     at_remove_msg_queue()            ;
extern  void    at_set_ddp_length()              ;
extern  int     at_write_datagram()              ;
#endif

/*+-------------------------------------------------------------------------+
  |                            E N D                                        |
  +-------------------------------------------------------------------------+*/
