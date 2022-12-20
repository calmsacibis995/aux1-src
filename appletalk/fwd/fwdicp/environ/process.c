#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/process.c */
#define _AC_NAME process_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:07:25}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of process.c on 87/11/11 21:07:25";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
#include <sys/debug.h>
#include <sys/process.h>

extern int T_proc;

#ifndef NULL
#define	NULL	0
#endif NULL

struct process *pr_current = NULL;	/* the current process (NULL if none) */
struct process *pr_sleep[PR_HSIZE];	/* the hash list of sleeping processes */
struct process *pr_ready = NULL;	/* the ready list of runable processes */
struct process *pr_readyend = NULL;	/* the ready list tail */
struct process *pr_list = NULL;		/* the list of all processes */
struct process *pr_listend = NULL;	/* the end of the list of all processes */
struct process *pr_idle = NULL;		/* list of processes waiting for work */

char **pr_oldsp;			/* main line's sp */
static int proc_id = 1;

int
sleep(addr, prio)
char *addr;
int prio;
{
	register struct process *p;
	register struct process **pp;
	register int s;

	if ((p = pr_current) == NULL)
		panic("Sleep of non-process");
	TRACE(T_proc, ("sleep(0x%x, %d) proc = %d\n",addr,prio,p->pr_id));
	p->pr_return = 0;		/* set to 1 if woken */
	p->pr_prio = prio;
	p->pr_addr = addr;
	p->pr_state = PR_SLEEPING;
	pr_current = NULL;
	pp = &pr_sleep[PR_HASH(addr)];
	s = splhi();
	p->pr_next = *pp;
	*pp = p;
	splx(s);
	proc_switch(pr_oldsp, &p->pr_sp);
	return(p->pr_return);
}

wakeup(addr)
register char *addr;
{
	register struct process *p;
	register struct process *oldp;
	register struct process *xp;
	register int s;

	TRACE(T_proc, ("wakeup(0x%x)\n",addr));
	oldp = NULL;
	s = splhi();
	p = pr_sleep[PR_HASH(addr)];
	for (;p;) {
		if (p->pr_addr == addr) {
			xp = p;
			if (oldp) {
				oldp->pr_next = p = p->pr_next;
			} else {
				pr_sleep[PR_HASH(addr)] = p = p->pr_next;
			}
			TRACE(T_proc, ("waking %d\n",xp->pr_id));
			xp->pr_state = PR_READY;
			if (pr_ready) {
				pr_readyend->pr_next = xp;
			} else {
				pr_ready = xp;
			}
			xp->pr_next = NULL;
			pr_readyend = xp;
			continue;
		}
		oldp = p;
		p = p->pr_next;
	} 
	splx(s);
}

make_proc(p, size)
struct process	*p;			/* the process being allocated */
int		size;			/* it's total size in bytes */
{
	register int s;

	if (p->pr_state == PR_FREE) {
		p->pr_state = PR_IDLE;
		p->pr_id = proc_id++;
		p->pr_list = NULL;
		if (pr_list) {
			pr_listend->pr_list = p;
		} else {
			pr_list = p;
		}
		pr_listend = p;
	} else
	if (p->pr_state != PR_FREE)
		panic("create existing process");
	TRACE(T_proc, ("make_proc(0x%x,0x%x) proc = %d\n",p,size,p->pr_id));
	p->pr_size = PR_SIZE(size);
	s = splhi();
	p->pr_next = pr_idle;
	pr_idle = p;
	splx(s);
}

kill_proc(p)
register struct process *p;
{
	char **sp;
	register int s;

	TRACE(T_proc, ("kill_proc(%d)\n",p->pr_id));
	if (p->pr_state != PR_CURRENT) 
		panic("kill_proc: proc not running");
	s = splhi();
	p->pr_next = pr_idle;
	pr_idle = p;
	splx(s);
	p->pr_state = PR_IDLE;
	pr_current = NULL;
	proc_switch(pr_oldsp, &sp);
}
	

struct process *
run_proc(start, param)
int	(*start)();		/* the start address of the process */
int	param;			/* the parameter being passed */
{
	register struct process	*p;	/* the process being run */
	register char		**sp;
	register int		s;

	s = splhi();
	if ((p = pr_idle) == NULL) {
		splx(s);
		return(NULL);
	}
	pr_idle = p->pr_next;
	splx(s);
	TRACE(T_proc, ("run_proc(0x%x, 0x%x) proc = %d\n",start, param, p->pr_id));
	p->pr_state = PR_READY;
	p->pr_magic = PR_MAGIC;
	sp = (char **)&p->pr_stack[p->pr_size-1];
	*--sp = (char *)p;		/* parameter to kill_proc */
	*--sp = (char *)param;		/* parameter to process */
	*--sp = (char *)kill_proc;	/* proc exit routine */
	*--sp = (char *)start;		/* running process routine */
	sp -= 6+5;			/* %d2-%d7, %a2-%a6 */
	*--((short *)sp) = 0x2000;	/* %sr */
	p->pr_sp = sp;
	s = splhi();
	if (pr_ready) {
		pr_readyend->pr_next = p;
	} else {
		pr_ready = p;
	}
	p->pr_next = NULL;
	pr_readyend = p;
	splx(s);
	return(p);
}

schedule_proc()
{
	register int s;
	register struct process *p;

	s = spl7();
	if ((p = pr_ready) == NULL) {
		splx(s);
		return(0);
	}
	pr_ready = p->pr_next;
	splx(s);
	TRACE(T_proc, ("schedule_proc(%d)\n",p->pr_id));
	p->pr_state = PR_CURRENT;
	pr_current = p;
	proc_switch(p->pr_sp, &pr_oldsp);
}

#ifdef DEBUG
debug_proc()
{
	register struct process *p;

	if (pr_current) {
		printf("\nCurrent process = %d\n", pr_current->pr_id);
	} else {
		printf("\nNo current process\n");
	}
	printf("\nId\tAddr\tState\tSleep\tPrio\tSP\n");
	for (p = pr_list;p;p = p->pr_list) {
		printf("%d\t0x%x\t",p->pr_id,p);
		switch(p->pr_state) {
		case PR_FREE:
			printf("free\t-\t-\t-\n");
			break;
		case PR_IDLE:
			printf("idle\t-\t-\t-\n");
			break;
		case PR_CURRENT:
			printf("current\t-\t-\t-\n");
			break;
		case PR_SLEEPING:
			printf("sleep\t0x%x\t%d\t0x%x\n",p->pr_addr,p->pr_prio,p->pr_sp);
			break;
		case PR_READY:
			printf("ready\t0x%x\t%d\t0x%x\n",p->pr_addr,p->pr_prio,p->pr_sp);
			break;
		default:
			printf("INSANE\t0x%x\t%d\t0x%x\n",p->pr_addr,p->pr_prio,p->pr_sp);
			break;
		}
	}
}
#endif DEBUG

resched()
{
	register struct process *p;
	register int s;

	if ((p = pr_current) == NULL)
		panic("Sleep of non-process");
	TRACE(T_proc, ("resched(%d)\n",p->pr_id));
	p->pr_state = PR_READY;
	p->pr_next = NULL;
	s = splhi();
	if (pr_ready) {
		pr_readyend->pr_next = p;
	} else {
		pr_ready = p;
	}
	pr_readyend = p;
	splx(s);
	proc_switch(pr_oldsp, &p->pr_sp);
}

unsleep_proc(p)
register struct process *p;
{
	register int s;
	register struct process **pp;

	pp = &pr_sleep[PR_HASH(p->pr_addr)];
	s = splhi();
	if (p->pr_state == PR_SLEEPING) {
		for (;*pp;pp = &((*pp)->pr_next))
		if (*pp == p) {
			*pp = p->pr_next;
			splx(s);
			p->pr_state = PR_READY;
			p->pr_next = NULL;
			s = splhi();
			if (pr_ready) {
				pr_readyend->pr_next = p;
			} else {
				pr_ready = p;
			}
			pr_readyend = p;
			splx(s);
			p->pr_return = 1;
			return;
		}
	}
	splx(s);
}
