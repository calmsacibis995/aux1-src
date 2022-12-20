#ifndef lint	/* .../appletalk/fwd/fwd.h */
#define _AC_NAME fwd_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.2 87/11/11 21:10:33}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.2 of fwd.h on 87/11/11 21:10:33 VV.2.1.1	*/

/*	Copyright (c) 1986 UniSoft Systems */
/*	  All Rights Reserved   	   */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UniSoft	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* define the I_STRs */
#define		I_FWD_RESET	_IO(f,0)
#define		I_FWD_DOWNLD	_IO(f,1)
#define		I_FWD_UPLD	_IO(f,2)
#define		I_FWD_START	_IO(f,3)
#define		I_FWD_LOOKUP	_IO(f,4)

/* define the message types used to pass to the forwarder peer */
#define		M_FWD		65   /* make sure less than QEXP */
#define		M_FWD_1stOPEN	M_FWD
#define		M_FWD_OPEN	M_FWD + 1
#define		M_FWD_ACK	M_FWD + 2
#define		M_FWD_CLOSE	M_FWD + 3
#define		M_FWD_PUSH	M_FWD + 4
#define		M_FWD_POP	M_FWD + 5
#define		M_FWD_PRINTF	M_FWD + 6
#define		M_FWD_CANCEL	M_FWD + 7
#define		M_FWD_FIND	M_FWD + 8
#define		M_FWD_LOOK	M_FWD + 9
#define		M_FWD_MNAME	M_FWD + 10

/* define the queue flush flags from the old stream documentation */
#define		FLUSHALL	1

#define	FEP_BOOT_TIMEOUT	1666666  /* about 1 sec at 16mhz */
#define DELAY(X)	{						\
			    register int zzz;				\
			    zzz = X;					\
			    do	{}  while (--zzz);			\
			}

/* if no one defines the number of fwd table entries */
#ifndef FWD_NENTRIES
#define		FWD_NENTRIES	8
#endif



#define	FWD_NAME_LENGTH		14
typedef struct
{
    caddr_t begin;	/* the start of the section durring reads,	*/
			/* the entry point to the application	  	*/
    caddr_t start;	/* the entry point for the section 		*/
    char name[FWD_NAME_LENGTH];	/* name given to section by the application */
}   fwd_entry_t;




typedef struct fwd_record
{
    caddr_t begin;	/* the address of where to start loading 	*/
    union
    {
	int	ld_length;	/* used in up loading */
	char data[MAXIOCBSZ-4];	/* the data to be down loaded start at begin */
    }   opt;
}   fwd_record_t;



			/* where we tag the packet with ack struct for	*/
			/* open and closing				*/
typedef struct
{
    dev_t		dev;
    int			flag;
    int			errno;
    ushort		circuit;
    char		name[8+1];	/* FMNAMESZ from conf.h			*/
    int			setpgrp;	/* set processgrp on successfull open 	*/
    struct ucred 	cred;		/* uid/gid etc 				*/
    int			sflag;		/* open flag (for cloned opens)		*/
					/* on return this contains the sleep    */
					/* state (see below)			*/
    int			id;		/* transaction id 			*/
    short		minpsz;		/* min packetsize from first open 	*/
    short		maxpsz;		/* max packetsize from first open 	*/
}   fwd_acktags_t;
#define	FWD_ACKTAGS(M)	   ((fwd_acktags_t *) (M->b_rptr))

/*
 *	sleep states (these are really qualifiers for open/close etc acks)
 *		a table of sleep states is kept in the circuit descriptor,
 *		indexed by the transaction id
 */

#define	TRANS_FREE	0	/* this transaction id is free                */
#define	TRANS_DEAD	1	/* this transaction died                      */
#define	TRANS_DONE	2	/* this transaction completed		      */
#define	TRANS_WAITING	3	/* this transaction is in progress	      */
#define	TRANS_SLEEP	4	/* this transaction is waiting for a wake     */
#define	TRANS_ALLOC	5	/* this transaction is in use		      */

#define	CIRC_MAXID	16

typedef int (*procedure_t)();

typedef struct
{
    ushort	circuit;	/* virtual circuit #, set by open, for mux    */
    ushort	open;		/* open count 				      */
    queue_t	*upstreamq;	/* pointer to the upstream q, used to demux   */
    struct fep_specifics *fep;	/* pointer to the rountines and data specific */
				/* for the particular fep downstream 	      */
    int		state;		/* current state - see below 		      */
#ifndef FEP 	/* Unix side */
    char	*rname[CIRC_MAXID];/* resulting name from look etc 	      */
    int	 	errno[CIRC_MAXID];/* resulting error from open etc 	      */
    int	 	setpgrp[CIRC_MAXID];/* request to set pgrp	 	      */
    int	 	dev[CIRC_MAXID];/* request to set clone device	 	      */
    char	idwanted;	/* set if someone wants a transaction id      */
    short	minpsz;		/* min packet size from upstream	      */
    short	maxpsz;		/* max packet size from upstream	      */
#else
    long	sd_flag;	/* stream head flags */
    struct process *proc[CIRC_MAXID];/* process to wake on cancel 	      */
#endif FEP 	/* Unix side */
    char	idstate[CIRC_MAXID];/* sleep states - see above 	      */
    int		wait;		/* true if the other end is waiting for flow control */
    int		halt;		/* > 0 if we are waiting for flow control */
    char	up_wait;	/* upstream waiting for ipc space		*/
    char	down_wait;	/* downstream waiting for ipc space		*/
}   fwd_circuit_t;


/*
 *	These defines are states a circuit can be in
 */

#define	CIRC_CLOSED	0	/* link closed - circuit free */
#define	CIRC_OPENING	1	/* link opening - waiting for open ack */
#define	CIRC_OPEN	2	/* link open - running */
#define	CIRC_PUSHING	3	/* link pushing - waiting for open ack */
#define	CIRC_POPPING	4	/* link popping - waiting for close ack */
#define	CIRC_CLOSING	5	/* link closing - waiting for close ack */
#define	CIRC_DEAD	6	/* link dead - board failure */


struct fep_specifics
{
    ushort	ncircuits;
    caddr_t	begin_ram;		/* where the onboard ram starts */
    caddr_t	end_ram;		/* where the onboard ram ends   */
    int		(*reset)();
    int		(*downld)();
    int		(*upld)();
    int		(*start)();
    int		(*putq)();
    int		(*flow)();
    fwd_circuit_t *fwd_circuit;		/* used to go from irupt to upstream */
    caddr_t	ginfop;			/* cannot say that this is a pointer to a infop
			 		 * structure since that is not GENERIC to all
					 * feps
			 		 */
    short	entry_index;		/* which is the current active row of */
    fwd_entry_t entry_tbl[FWD_NENTRIES];/* for a record of things loaded */
    int		open;			/* open circuit count */
    int		state;			/* current state - see below */
#ifndef FEP	/* Unix side */
    mblk_t	*reset_msg;			/* message waiting for reset to happen */
    int		reset_count;		/* timer count for reseting */
#endif 	FEP
};
typedef struct fep_specifics	fep_specifics_t;
#define	CIRCUIT(Q)	((fwd_circuit_t *) Q->q_ptr)
#define	GINFOP(Q)	((fwd_circuit_t *) Q->q_ptr)->fep->ginfop
#define	EX(Q,FUNC)	(*(((fwd_circuit_t *) Q->q_ptr)->fep->FUNC))

/*
 *	FEP states
 */

#define FEP_CLOSED	0		/* the device has no code loaded on it */
#define FEP_RUNNING	1		/* the device is running */
#define FEP_DEAD	2		/* the device is dead */
#define FEP_RESET1	3		/* in the 1st stage of restting */
#define FEP_RESET2	4		/* in the 2nd stage of restting */
#define FEP_START	5 		/* in the process of starting */
