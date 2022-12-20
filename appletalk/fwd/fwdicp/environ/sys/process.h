#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/sys/process.h */
#define _AC_NAME process_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:05:59}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.2 of process.h on 87/11/11 21:05:59 */
struct process {
	char		**pr_sp;	/* the saved process's stack pointer */
	char		*pr_addr;	/* the thing the process is sleeping on */
	struct process 	*pr_next;	/* the next process in a waiting list */
	struct process 	*pr_list;	/* the list of known processes */
	char 		*pr_spare[8];	/* space used for per process things ... */
	int		pr_prio;	/* sleep priority */
	short		pr_id;		/* the process's id */
	short		pr_state;	/* current state ... see below */
	short		pr_return;	/* what to return from sleep */
	long		pr_size;	/* The stack size in longwords */
	long		pr_magic;	/* stack guard */
	long		pr_stack[1];	/* The stack */
};

#define	PR_MAGIC	0x5061756c	/* stack guard magic */

/*
 *	process states
 */

#define	PR_FREE		0		/* not in use */
#define	PR_IDLE		1		/* not doing anything */
#define	PR_CURRENT	2		/* the current process */
#define	PR_SLEEPING	3		/* in a sleep queue */
#define	PR_READY	4		/* in the active queue */

extern struct process *pr_current;	/* the current process (NULL if none) */
extern struct process *pr_sleep[];	/* the hash list of sleeping processes */
extern struct process *pr_ready;	/* the ready list of runable processes */
extern struct process *pr_list;		/* the list of all processes */
extern struct process *pr_listend;	/* the end of the list of all processes */
extern struct process *pr_idle;		/* the list of idle processes */

extern char **pr_oldsp;			/* mainline's old sp */

#define	PR_SIZE(x)	((x - sizeof(struct process) + 4) >> 2)
#define	PR_HSIZE	16
#define	PR_HASH(x)	((((int)x)>>2)&0xf)

extern struct process *run_proc();
