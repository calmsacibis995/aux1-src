#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/fwdicp.h */
#define _AC_NAME fwdicp_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:08:02}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.2 of fwdicp.h on 87/11/11 21:08:02 */

/*	All Rights Reserved 	  	   			*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF Apple Computer	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*      This is the header file for accesses to the local	*/
/*	environment on a ast serial card			*/

/*
 *	forwarder and UNIX/FEP interface
 */

#ifndef SCC_RR0
#include <scc.h>
#endif

#ifndef VIA_IER_SET
#include <via.h>
#endif

#define NCIRCUITS	100		/* number of virtual circuits allowed*/
#define	NENTRIES	5		/* number of entry_tbl rows */
#define ACCESS_OFFSET	0x600
#define	SIZE_FEPQ	40
#define	SIZE_UNIXQ	40


#define	IRUPT_UNIX	0xc0		/* put in via's pcr (not) to irupt */
#define	IRUPT_UNIX_N	0xe0
#define	IRUPT_FEP	0x0c
#define	IRUPT_FEP_N	0x0e

#define	FEP_UNITIALIZED	0x0		/* access status */
#define	FEP_RESET	0x6780
#define	FEP_READY	0x3450
#define	FEP_NOTREADY	0x0543

#define	IPC_REPLY			0x7000
#define IPC_TAKE_MESSAGE		0x1651		/* send message */
#define IPC_COULD_NOT_TAKE_MESSAGE	0x7652		/* nak from IPC_TAKE_MESSAGE */
#define IPC_GOT_MESSAGE			0x7653		/* ack */
#define IPC_TAKE_MESS_FLOW		0x1654		/* as above but want flow return*/
#define IPC_GOT_RUN			0x1655		/* flow return go */
#define IPC_GOT_MESSAGE_RUN		0x7656		/* ack - flow return go */
#define IPC_COULD_NOT_TAKE_FLOW		0x7657		/* nak - flow return go */
#define IPC_GOT_RUN_ACK			0x7658		/* GOT_RUN ack */
#define IPC_GOT_MESSAGE_STOP		0x7659		/* ack - flow return stop */

/*
 *	IPC command queue defines
 */

#define	IPC_REP		0		/* reply queue */
#define	IPC_CMD_NORM	1		/* normal message queue */
#define	IPC_CMD_EXP	2		/* expedited (faster) message queue */

#ifndef  FEP	/* UNIX side */

/* write to the ddra since who knows what the trojan horse has done! */
/* do not forget to clear interrupts, since rom will loop, and set   */
/* data direction register since rom doesn't or download code is lost*/
#define ICP_RESET	((via_chip_t *)(VIA_ADDRESS+infop->ramp))->ora_ira=0x04;\
			((via_chip_t *)(VIA_ADDRESS+infop->ramp))->ddra = 0xc6; \
			((via_chip_t *)(VIA_ADDRESS+infop->ramp))->ier = 0x7f; \
			((via_chip_t *)(VIA_ADDRESS+infop->ramp))->pcr =      \
						  IRUPT_UNIX_N | IRUPT_FEP_N; \
			DELAY(30);					      \
			((via_chip_t *)(VIA_ADDRESS+infop->ramp))->ora_ira=0x06

#define ICP_SELF_STATUS	((via_chip_t *)(VIA_ADDRESS+infop->ramp))->shift

#define ICP_INTERRUPT	((via_chip_t *)(VIA_ADDRESS+infop->ramp))->pcr =	      \
						   IRUPT_UNIX_N | IRUPT_FEP
#endif

/*
 *	inlines to make us do 8 bit accesses to the board;
 *	NOTE: if we are already on the board, we donot care;
 */

#ifndef FEP	/* UNIX side */
#define write_int_by_short(to, value) {			\
        uint x = (uint) value; 				\
	(((ushort *)(&to))[0]) = (((ushort *)&x)[0]);	\
	(((ushort *)(&to))[1]) = (((ushort *)&x)[1]);	\
}
#else		/* FEP side */
#define write_int_by_short(to, value) 			\
	to = value
#endif


#ifndef FEP	/* UNIX side */
#define read_int_by_short(from)				\
	(((((ushort *)&from)[0]) << 16) | (((ushort *)&from)[1]))
#else   FEP	/* FEP side */
#define read_int_by_short(from)				\
	(uint) from
#endif  FEP


#ifndef FEP	/* UNIX side */
#define splipc()	spl7()
#else   FEP	/* FEP side */
#define splipc()	spl1()
#endif  FEP


typedef struct
{
	ushort	cmd;			/* what the otherside should execute */
	ushort	id;			/* identifier for this transaction   */
	ushort	circuit;		/* which circuit the command is for  */
	uint	arg;
} ipc_t;


typedef struct
{
	ipc_t	*rp;			/* read pointer into fep cmd queue */
	ipc_t	*wp;			/* write pointer into fep cmd queue */
	ipc_t	*start;			/* where the fep cmd queue starts */
	ipc_t	*end;			/* where the fep cmd queue ends */
} ipc_ptrs_t;


typedef struct
{
	ushort	status;			/* boards estimation of its sanity */
	ushort	next_id;		/* for sequencing commands */
	ipc_ptrs_t exp2fep;		/* read pointer into fep expedited queue */
	ipc_ptrs_t cmd2fep;		/* read pointer into fep cmd queue */
	ipc_ptrs_t rep2fep;		/* read pointer into fep reply queue */
	int	cmds_to_fep;		/* # of cmds sent to fep */
	int	bytes_to_fep;		/* # of buffer bytes, db_lim-db_base */
	short	fep_q_size;		/* size of fep command queue */
	ipc_ptrs_t exp2unix;		/* read pointer into unix expedited queue */
	ipc_ptrs_t cmd2unix;		/* read pointer into unix cmd queue */
	ipc_ptrs_t rep2unix;		/* read pointer into unix reply queue */
	int	cmds_to_unix;		/* # of cmds sent to unix */
	int	bytes_to_unix;		/* # of buffer bytes, db_lim-db_base */
	short	unix_q_size;		/* size of unix command queue */
} access_t;


typedef struct 
{
	ushort		status;
	access_t	*accessp;	/* this is it */
	caddr_t		ramp;		/* when unix side: 0xfn00000, n=slot# */
	char		rcvtimer;	/* the receive side is waiting for space */
	int		count;		/* number of slots available */
	int		sleep;		/* someone is sleeping waiting for this */
} icp_info_t;



/*
 *	streams
 */

#ifdef FEP
#define	NSTREAM		0		/* number of stream heads */
#define	NQUEUE		NCIRCUITS*7	/* number of queue heads */
#define	NBLK4096	0		/* number of 4k stream blocks */
#define	NBLK2048	6		/* number of 2k stream blocks */
#define	NBLK1024	100		/* number of 1k stream blocks */
#define	NBLK512		50		/* number of 512 byte stream blocks */
#define	NBLK256		32		/* number of 256 byte stream blocks */
#define	NBLK128		40		/* number of 128 byte stream blocks */
#define	NBLK64		80		/* number of 64 byte stream blocks */
#define	NBLK16		80		/* number of 16 byte stream blocks */
#define	NBLK4		80		/* number of 4 byte stream blocks */


struct var {
	int	v_nstream;	/* NSTREAM */
	int	v_nqueue;	/* NQUEUE */
	int	v_nblk4096;	/* NBLK4096 */
	int	v_nblk2048;	/* NBLK2048 */
	int	v_nblk1024;	/* NBLK1024 */
	int	v_nblk512;	/* NBLK512 */
	int	v_nblk256;	/* NBLK256 */
	int	v_nblk128;	/* NBLK128 */
	int	v_nblk64;	/* NBLK64 */
	int	v_nblk16;	/* NBLK16 */
	int	v_nblk4;	/* NBLK4 */
};

extern struct var v;

#endif FEP



/*
 *	local board environment
 */

#ifdef HZ
#undef HZ
#endif HZ
#define HZ		30
#define	NCALL		NCIRCUITS*10
#define	N_VIA		1
#define	N_TIMERS	2
#define	N_SCC_CHANNELS	4
#define	AT_CHANNEL	1	/* which scc channel is appletalk */
#define	FEP_RAM_START	0x0
#define	FEP_RAM_END	0x7ffff
#define	VIA_ADDRESS	0x0c0000
#define	PAGE_REGISTER	0x0d0000
#define	CLOCK_TIMER	0
#define	AT_TIMER	1


#ifdef  FEP		/* FEP side */

#ifdef FWD_DEFINE		/* in the storage definition sense */
scc_t	sccs[N_SCC_CHANNELS] = {
				  {    (unsigned char *) 0x0b0004,
				       (unsigned char *) 0x0b0004,
				       (unsigned char *) 0x0b0006,
				       (unsigned char *) 0x0b0006, 
				       SCC_WR9_CRA | SCC_WR9_MIE | SCC_WR9_NOV
				  },
				  {    (unsigned char *) 0x0b0000,
				       (unsigned char *) 0x0b0000,
				       (unsigned char *) 0x0b0002,
				       (unsigned char *) 0x0b0002, 
				       SCC_WR9_CRB | SCC_WR9_MIE | SCC_WR9_NOV
				  },
				  {    (unsigned char *) 0x0b4004,
				       (unsigned char *) 0x0b4004,
				       (unsigned char *) 0x0b4006,
				       (unsigned char *) 0x0b4006, 
				       SCC_WR9_CRA | SCC_WR9_MIE | SCC_WR9_NOV
				  },
				  {    (unsigned char *) 0x0b4000,
				       (unsigned char *) 0x0b4000,
				       (unsigned char *) 0x0b4002,
				       (unsigned char *) 0x0b4002, 
				       SCC_WR9_CRB | SCC_WR9_MIE | SCC_WR9_NOV
				  }
			    };

via_t	vias[N_TIMERS] = {
			    (via_chip_t *) VIA_ADDRESS,
			    (via_chip_t *) VIA_ADDRESS
			 };

extern scc_default_irupt();
int (*scc_vecs[N_SCC_CHANNELS]) () =
			{
			    scc_default_irupt,
			    scc_default_irupt,
			    scc_default_irupt,
			    scc_default_irupt
			};



/*
 *	printf
 */

#define	CONSOLE_CHANNEL		3	/* which scc channel is console */

/*+-------------------------------------------------------------------------+
  |     B A U D R A T E   D E F I N I T I O N                               |
  +-------------------------------------------------------------------------+*/
#define  AT_SCC_CONSOLE_RATE	0x0a  /* Baud rate constant.           */


/*
 *     S C C    I N I T I A L I Z A T I O N    R V D    T A B L E          |
 *
 *     This table contains the Z8530 SCC register numbers and the values to
 *     put in those registers to initialize the SCC chip. This table will
 *     leave the chip all ready to go
 *     
 *     Note that the third field, at_scc_extra_delay,, must NEVER be less than
 *     one -- this field is decremented BEFORE being tested, and a zero value
 *     will loop for 0xffffffff iterations!
 *
 *	NOTE: a channel reset must be issued before this point and cannot be
 *	entered in this table. Not only does the channel reset need extra
 *	time to execute, but it is written to a registered shared between the
 *	two channels of the scc! This would make these channel independant
 *	table dependant. The value to load in found in the scc structure which
 *	also contains the chip addressing information.
 */
scc_rvd scc_rvd_serialinit[] =
{
	SCC_WR4, SCC_WR4_PEV | SCC_WR4_1SB | SCC_WR4_X16,
	SCC_WR5, SCC_WR5_TX8 | SCC_WR5_TXE | SCC_WR5_RTS | SCC_WR5_DTR,
	SCC_WR11, SCC_WR11_RXN | SCC_WR11_RCB | SCC_WR11_TCB,
	SCC_WR12, AT_SCC_CONSOLE_RATE % 256,
	SCC_WR13, AT_SCC_CONSOLE_RATE / 256,
	SCC_WR14, 0,
	SCC_WR15, 0,
	SCC_WR14, SCC_WR14_BGS | SCC_WR14_BGE,
	SCC_WR3, SCC_WR3_RX8 | SCC_WR3_RXE,
	SCC_EOT, 0
};

#else  /* FWD_DEFINE */
extern via_t	vias[];
#endif  /* FWD_DEFINE */
#endif  /* FEP side */


#ifdef	FEP		/* FEP side */
/************************************************************************/
/*									*/
/*			forward declarations				*/
/*									*/
/************************************************************************/

extern void s_timeout();
extern void s_untimeout();
#endif  /* FEP */
