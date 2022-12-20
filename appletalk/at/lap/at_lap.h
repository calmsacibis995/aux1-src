#ifndef lint	/* .../appletalk/at/lap/at_lap.h */
#define _AC_NAME at_lap_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 20:56:39}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.2 of at_lap.h on 87/11/11 20:56:39 VV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems 			*/
/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *      This is the header file for the at_lap module.
 */


#define	splat()			splhi()

/*----------------------------------------------------------------------*/
/* Define the states of the character at a time state machine.		*/
/*----------------------------------------------------------------------*/
#define	OFFLINE			0

#define	HANGING_UP		1
#define	ONLINE			2

#define	ENQ_WAIT_LINE_FREE	3
#define	ENQ_WAIT_TdTr		4
#define	ENQ_XMIT		5
#define	ENQ_XMIT_OFF		6
#define	ENQ_READY_FOR_ACK	7

#define	RCV_RECEIVING_RTS	8
#define	RCV_XMIT_CTS		9
#define	RCV_XMIT_CTS_OFF	10
#define RCV_IN_FRAME		11

#define XMIT_WAIT_LINE_FREE	12
#define XMIT_WAIT_TdTr		13
#define	XMIT_RTS		14
#define	XMIT_RTS_OFF		15
#define XMIT_READY_FOR_CTS	16
#define XMIT_IN_FRAME		17
#define XMIT_FRAME_OFF		18
#define XMIT_FRAME_BROADCAST	19

#define ACK_XMIT		20
#define ACK_XMIT_OFF		21

#define NEWSTATE(X)		lapp->state = X; TRACE(T_lap_states, ("newstate %d\n", lapp->state))
#define INC_STATE()		lapp->state++; TRACE(T_lap_states, ("newstate %d\n", lapp->state))


/*+-------------------------------------------------------------------------+
  |     D E L A Y    D E F I N I T I O N S                                  |
  +-------------------------------------------------------------------------+*/

/*
 * at_timeout()'s
 *
 * the following delays are suppossed to be passed to at_timeout which will
 * set up the hardware to interrupt later either as a watchdog timer or
 * an event activator.
 */

#define	AT_LAP_TIME_100us	100	/* 100us */
#define	AT_LAP_TIME_XMIT_OFF	 90	/* 140us */
#define	AT_LAP_TIME_INTERFRAME	440 	/* flag+abort+200us+4flags+adr = 445us */
#define	AT_LAP_TIME_Td		400	/* 400us */
#define	AT_LAP_TIME_MAXFRAME  21100	/*  21ms */



/* ??? tune these times for processor speed */
/*
 *  Delay type A's.
 *
 *  These delays are all of the form
 *      do  {
 *          }
 *      while (--i);
 *  where i is a register variable. 2.5 microseconds per iteration.
 */



/*
 *  Delay type B's.
 *
 *  These delays are all of the form
 *      do  {
 *          c = *p;
 *          if (c & 0x01)
 *              break;
 *          }
 *      while (--i);
 *  where i,c,p are register variables, and the if is not true.
 *  6 microseconds per iteration.
 */

#define  AT_DELAY_B_INTERCHAR              8


/*+-------------------------------------------------------------------------+
  |     S T R U C T U R E and T Y P E  D E F I N I T I O N S                |
  +-------------------------------------------------------------------------+*/

/*+-------------------------------------------------------------------------+
  |     D A T A G R A M    P A C K E T    (DDP SHORT HEADER)                |
  |	this is used internal to this module, anything upstream of here	    |
  |	uses only the extended datagram!				    |
  +-------------------------------------------------------------------------+*/

typedef struct
        {
        at_node    at_lap_dst_node    ;  /* Destination address.       */
        at_node    at_lap_src_node    ;  /* Source address.            */
        at_type    at_lap_type        ;  /* Frame type.                */
        at_length  at_ddp_length      ;  /* Datagram length (10 bits). */
        at_socket  at_ddp_dst_socket  ;  /* Destination socket number. */
        at_socket  at_ddp_src_socket  ;  /* Source socket number.      */
        at_type    at_ddp_type        ;  /* Protocol type.             */
        u8         at_ddp_data[586]   ;  /* The DataGram buffer.       */
        u8         at_ddp_fill[8]     ;  /* Allow for extended header. */
        }          at_dgs_t           ;  /* The DataGram.              */



/* forward reference */
typedef struct lap_specifics lap_specifics_t;


typedef struct
{
    u8 		type;		/* the type registered on this circuit */
    u8 		circuit;	/* the circuit number, avoids division */
    char	name[14];	/* the name registered on this circuit */
    queue_t	*upstreamq;	/* pointer to the upstream q, used to demux */
				/* for 0th q, is multiplexer q, points to self*/
    lap_specifics_t *lap;	/* pointer to the local data structure which
				 * differentiates this stream from other
				 * network streams
				 */
}   lap_circuit_t;
#define	CIRCUIT(Q)		((lap_circuit_t *) Q->q_ptr)


				/* this is the structure that the assembly */
				/* code uses to xmit and rcv buffers	   */
typedef struct
{
    caddr_t	ptr;		/* pointer into buffer */
    caddr_t	end;		/* end of buffer */
}   rwp_t;

extern rwp_t	at_rwp[];
extern		scc2i();	/* the standard interrupt handler */
extern		atxmit2();	/* the appletalk routine for xmitting */

struct lap_specifics
{
    u8 		ncircuits;	/* the number of circuits allowed to this net */
    u8 		state;		/* the state of the interrupt state machine */
    u8		defer_history;	/* line busy history */
    u8		defer_count;	/* times line has been busy this frame */
    u8		collision_history;
    u8		collision_count;/* times this frame has been collided with */
    u8		global_backoff;	/* mask to determine range of Tr time */
    u8		local_backoff;
    u8		timer;		/* which timer to use */
    u16 	random;		/* random number to determine Tr time */
    lap_circuit_t *circuit0p;	/* so the interrupt routine can find demux qs */
    queue_t	*downstreamq;	/* pointer to the downstream q, used to mux */
    scc_t	*scc;		/* the pointers to the scc */
    at_lap_hdr_t ctl;		/* used to tranmit control frames: CTS,ENQ,ACK*/
    char	pad[2];		/* for the first byte of the checksum */
    mblk_t	*rcv_msgp;	/* current message we are receiving into */
    mblk_t	*xmit_msgp;	/* current message we are transmitting from */
    rwp_t	*rwp;		/* pointer to the read/write pointers */
    at_lap_cfg_t cfg;		/* the cfg structure */
};


typedef struct
{
    u8		circuit;
}   lap_tags_t;

#define	LAP_IOCTAGS(M)	   ((lap_tags_t *) (M->b_wptr))


/*+-------------------------------------------------------------------------+
  |     B A U D R A T E   D E F I N I T I O N                               |
  +-------------------------------------------------------------------------+*/
#define  AT_SCC_BAUD_RATE             0x06  /* Baud rate constant.           */


#ifdef	DEFINE		/* as in c's definition vs declaration */
/*+-------------------------------------------------------------------------+
  |     S C C    I N I T I A L I Z A T I O N    R V D    T A B L E          |
  +-------------------------------------------------------------------------+
  |     This table contains the Z8530 SCC register numbers and the values to
  |     put in those registers to initialize the SCC chip. This table will
  |     leave the chip all ready to go and setup to receive with interrupts
  |     enabled.
  |     
  |     This table is generated from the MacIntosh Protocol Package (MPP)
  |     OpenTbl and MOpenTbl tables. The DCD interrupt enable has been
  |     omitted as this driver does not support a mouse (which used the channel
  |     A and channel B DCD lines as input lines). MPP/MPP.TEXT of 05-Jun-85.
  |     
  |     Note that the third field, at_scc_extra_delay,, must NEVER be less than
  |     one -- this field is decremented BEFORE being tested, and a zero value
  |     will loop for 0xffffffff iterations!
  |
  |	NOTE: a channel reset must be issued before this point and cannot be
  |	entered in this table. Not only does the channel reset need extra
  |	time to execute, but it is written to a registered shared between the
  |	two channels of the scc! This would make these channel independant
  |	table dependant. The value to load in found in the scc structure which
  |	also contains the chip addressing information.
  +*/

scc_rvd at_scc_rvd_init[] =
                { { SCC_WR4,   SCC_WR4_SDLC                    },
                  { SCC_WR10,  SCC_WR10_CPI | SCC_WR10_FM0     },
                  { SCC_WR7,   0x7e                            },
                  { SCC_WR12,  AT_SCC_BAUD_RATE % 256          },
                  { SCC_WR13,  AT_SCC_BAUD_RATE / 256          },
                  { SCC_WR14,  SCC_WR14_SFM                    },
                  { SCC_WR3,   SCC_WR3_RX8 | SCC_WR3_EHM |
                               SCC_WR3_RCE | SCC_WR3_ASM |
                               SCC_WR3_RXE                     },
                  { SCC_WR2,   0                               },
		  { SCC_WR15,  SCC_WR15_BAI 		       },
                  { SCC_WR11,  SCC_WR11_RCD | SCC_WR11_TCB     },
                  { SCC_WR14,  SCC_WR14_SRT                    },
                  { SCC_WR14,  SCC_WR14_ESM | SCC_WR14_BGE |
			       SCC_WR14_BGS 		       },
                  { SCC_WR5,   SCC_WR5_TX8                     },
                  { SCC_EOT,   0                               }
                };

/*+-------------------------------------------------------------------------+
  |     S C C    T U R N    T R A N S M I T T E R    O N    T A B L E       |
  +-------------------------------------------------------------------------+
  |     This table contains the Z8530 SCC register numbers and the values to
  |     put in those registers to turn the transmitter on (and the receiver
  |     off) in the SCC chip.
  |     
  |     This table is generated from the MacIntosh Protocol Package (MPP)
  |     SHdata table. MPP/MPP.TEXT of 05-Jun-85.
  +*/

scc_rvd at_scc_rvd_xmit_on[] =
                { { SCC_WR5,  SCC_WR5_TX8 | SCC_WR5_TXE |
                              SCC_WR5_DTR | SCC_WR5_TCE     },
                  { SCC_WR3,  SCC_WR3_RX8 | SCC_WR3_EHM     },
#ifdef	XMITIRUPT
		  { SCC_WR1,  SCC_WR1_TIN 		    },
#endif	XMITIRUPT
		  { SCC_WR0,  SCC_WR0_RTC 		    },
                  { SCC_EOT,   0                            }
                };



/*+-------------------------------------------------------------------------+
  |     S C C    T U R N    T R A N S M I T T E R    O F F    T A B L E     |
  +-------------------------------------------------------------------------+
  |     This table contains the Z8530 SCC register numbers and the values to
  |     put in those registers to turn the transmitter off.
  |
  |	One change from the mac is that we use the mark on idle mode.
  |     
  |     This table is generated from the MacIntosh Protocol Package (MPP)
  |     MacInitData table. MPP/MPP.TEXT of 05-Jun-85.
  +*/

scc_rvd at_scc_rvd_xmit_off[] =
                { { SCC_WR5,  SCC_WR5_TX8                   },
                  { SCC_WR14, SCC_WR14_ESM | SCC_WR14_BGE |
			      SCC_WR14_BGS 		    },
                  { SCC_WR3,  SCC_WR3_RX8  | SCC_WR3_EHM |
                              SCC_WR3_RCE  | SCC_WR3_ASM |
                              SCC_WR3_RXE                   },
		  { SCC_WR1,  SCC_WR1_RIA 		    },
		  { SCC_WR10, SCC_WR10_CPI | SCC_WR10_FM0   },
                  { SCC_EOT,   0                            }
                };



/*+-------------------------------------------------------------------------+
  |     S C C    S E N D   S Y C R O N I Z A T I O N   P U L S E	    |
  +-------------------------------------------------------------------------+
  |     This table contains the Z8530 SCC register numbers and the values to
  |     put in those registers to turn on the transmitter and start sending
  |	its idle? flags for a bit time or two.  This results in a faster
  |	carrier detect than the sync/hunt bit.
  +*/

scc_rvd at_scc_rvd_pulse[] =
                {
                  { SCC_WR5,  SCC_WR5_TX8 | SCC_WR5_TXE |
                              SCC_WR5_DTR | SCC_WR5_TCE     },
                  { SCC_WR5,  SCC_WR5_TX8,                  },
                  { SCC_WR3,  SCC_WR3_RX8 | SCC_WR3_EHM     },
                  { SCC_WR5,  SCC_WR5_TX8 | SCC_WR5_TXE |
                              SCC_WR5_DTR | SCC_WR5_TCE     },
#ifdef	XMITIRUPT
		  { SCC_WR1,  SCC_WR1_TIN 		    },
#endif	XMITIRUPT
		  { SCC_WR0,  SCC_WR0_RTC 		    },
                  { SCC_EOT,   0                            }
                };
#endif  /* TABLES */
