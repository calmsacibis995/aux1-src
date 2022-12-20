#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/debug.c */
#define _AC_NAME debug_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:06:39}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of debug.c on 87/11/11 21:06:39";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)debug.c	UniPlus V.2.1.16	*/
/*	Copyright 1985 UniSoft */

#ifdef	DEBUG

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/reg.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include <sys/signal.h>
#include <sys/mmu.h>
#ifdef PAGING
#include <sys/page.h>
#endif PAGING
#include <sys/seg.h>
#ifdef PAGING
#include <sys/region.h>
#endif PAGING
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#include "sys/callout.h"
#include "fwd.h"
#include "fwdicp.h"
#include <sys/process.h>

#define TRBUFSIZE	128		/* size of trace buffer 
					   (must be power of 2) */
#define	TRBUFMASK	(TRBUFSIZE-1)
static char trace_quit = 0;		/* quit when the buffer is full */
static char trace_print = 0;		/* printf when tracedebug is called */
static int trace_start = 0;		/* start of buffer pointer */
static int trace_end = 0;		/* end of buffer pointer */
static char *trace_a[TRBUFSIZE];	/* printf args */
static long trace_b[TRBUFSIZE];
static long trace_c[TRBUFSIZE];
static long trace_d[TRBUFSIZE];
static long trace_e[TRBUFSIZE];
static long trace_f[TRBUFSIZE];
static long trace_g[TRBUFSIZE];
static long trace_h[TRBUFSIZE];

static getint();
extern struct var v;

extern int T_streamhead;
int	T_nbp;
int	T_pap;
int	T_papd;
int	T_ddp;
int	T_fwd;
int	T_ipc;
int	T_mainloop;
int	T_scc;
int	T_stream;
int	T_proc;
int	T_lap;
int	T_lap_backoff;
int	T_lap_errors;
int	T_lap_interrupt;
int	T_lap_states;
int	T_atp;
int	T_atp_alloc;
int	T_atp_rep;
int	T_atp_req;

struct Tflags {
	int *flag;
	char *name;
} Tflags[] = {
	&T_lap, "T_lap",
	&T_lap_backoff, "T_lap_backoff",
	&T_lap_errors, "T_lap_errors",
	&T_lap_interrupt, "T_lap_interrupt",
	&T_lap_states, "T_lap_states",
	&T_atp_alloc, "T_atp_alloc",
	&T_atp_rep, "T_atp_rep",
	&T_atp_req, "T_atp_req",
	&T_atp, "T_atp",
	&T_ddp, "T_ddp",
	&T_fwd,	"T_fwd",
	&T_ipc,	"T_ipc",
	&T_mainloop, "T_mainloop",
	&T_nbp, "T_nbp",
	&T_papd, "T_papd",
	&T_pap, "T_pap",
	&T_proc, "T_proc",
	&T_scc, "T_scc",
	&T_stream, "T_stream",
	0, 0
};

char tbuf[80];

/*
 * This module requires that the following routines be added to the
 * standard Unix kernel:
 *
 *	(1) getchar()  -- must be added to the console serial driver.
 *
 *	(2) queprint() -- should be added to whichever disk driver is
 *			  is currently being debugged.  This routine
 *			  can be empty if its functionality is not 
 *			  needed; otherwise it should print the queue
 *			  of requests waiting for the disk.
 *
 * These additions to the kernel (and others like them) should be 
 * contained in "#ifdef	DEBUG" clauses, so the entire debugging module
 * can be easily turned off in the Makefile.
 */

int debugg = 0;				/* global debug flag set by case "T" */


debug()
{
	register struct buf *bp;
	register struct proc *p;
	register int c;
	int spl;
	extern struct buf bfreelist[];
	extern int printfstall;
	int i;
	int ndblock;
	int trace_p;
	extern fep_specifics_t	icp_specifics;
	fep_specifics_t		*fep_specificsp;
 	icp_info_t		*infop;

    spl = spl7();
    for (;;) {
	int SavePrintfstall = printfstall;

	printfstall = 0;
	printf("?");
	printf(" A");
	printf(" D");
	printf(" d");
	printf(" f");
	printf(" p");
	printf(" P");
	printf(" S");
	printf(" t");
	printf(" T");
	printf(" x");
	printf(" z");
	printf(":");
	printfstall = SavePrintfstall;

	c = debuggetchar();
	printf("%c\n", c);
	switch (c) {

	case '?':
		debughelp();
		break;
	case 'A':
		db0stack();
		break;
	case 'd':
		debugdebug();
		break;
	case 'D':
		debugmdump();
		break;
	case 'f':
		fep_specificsp = &icp_specifics;
		infop = (icp_info_t *) fep_specificsp->ginfop;
		c = '?';
		for(;c != 'x';) {
			printf("forwarder - ? c f S u w x: ");
			c = debuggetchar();
			printf("%c\n", c);
			switch (c) {
	
			default:
				printf("\t?\tPrint forwarder help\n");
				printf("\tc\tList busy circuits\n");
				printf("\tf\tList fep queue\n");
				printf("\tS\tPrint statistics\n");
				printf("\tu\tList unix queue\n");
				printf("\tx\tReturn to main debug menu\n");
				break;
			case 'x':
				break;
			case 'S':
				fwdicp_statistics(infop->accessp);
				break;
			case 'c':
				fwd_pr_c(fep_specificsp);
				break;
			case 'f':
				fwdicp_pr_q("Replies  ", &(infop->accessp->rep2fep));
				fwdicp_pr_q("Expedited", &(infop->accessp->exp2fep));
				fwdicp_pr_q("Commands ", &(infop->accessp->cmd2fep));
				break;
			case 'u':
				fwdicp_pr_q("Replies  ", &(infop->accessp->rep2unix));
				fwdicp_pr_q("Expedited", &(infop->accessp->exp2unix));
				fwdicp_pr_q("Commands ", &(infop->accessp->cmd2unix));
				break;
			} /* of switch (c) */
		} /* of for(;c != 'x';) */
		break;
	case 'p':
		debug_proc();
		break;
	case 'P':
		printf("printfstall = 0x%x", printfstall);
		printfstall = debugghex();
		printf("printfstall = 0x%x\n", printfstall);
		break;
	case 'S':
		c = '?';
		for(;c != 'x';) {
			printf("Streams - ? d D f m M q Q r s S x: ");
			c = debuggetchar();
			printf("%c\n", c);
			switch (c) {
	
			default:
				printf("\t?\tPrint streams help\n");
				printf("\td\tList busy data blocks\n");
				printf("\tD\tList a data block\n");
				printf("\tf\tList free message blocks\n");
				printf("\tm\tList busy message blocks\n");
				printf("\tM\tList a message\n");
				printf("\tq\tDisplay queues\n");
				printf("\tQ\tDisplay messages on queue\n");
				printf("\tr\tList the runable stream queues\n");
				printf("\ts\tDump streams\n");
				printf("\tS\tPrint statistics\n");
				printf("\tx\tReturn to main debug menu\n");
				break;
			case 'x':
				break;
			case 's':
	
		/*
		 * The stream table slots indicated in the argument list are
		 * printed.  If there are no arguments, then all allocated
		 * streams are listed.  The function prstream() can be found
		 * in the file stream.c.
		 * WRONG: follows below.
		 */
	
				printf("\nSLOT\tWRQ\tIOCB\tINODE\tPGRP\tIOCID\tIOCWT\tWOFF\tERR\tFLAGS\n");
				for (i=0; i < v.v_nstream; i++) prstream(i,1);
				break;
	
			case 'q':
	
		/*
		 * The stream queue entries corresponding to the slot numbers 
		 * provided are printed.  If no argument is provided, then all 
		 * allocated queues are printed.  The function prqueue() can
		 * be found in the file stream.c.
		*/ 
	
				printf("\nSLOT NEXT LINK\tINFO\tPTR\tCNT HEAD TAIL MINP MAXP HIWT LOWT FLAGS\n");
				for (i=0; i < v.v_nqueue; i++) prqueue(i,1);
				break;


			case 'Q':
		/*
		 * The messages on the queue are displayed
		 */
				printf("q slot = ");
				i = debuggint();
				prqsmsg(i);
				break;

	
			case 'm':
	
		/*
		 * The message headers corresponding to the slot numbers 
		 * provided are printed.  If no argument is given, then all
		 * allocated message headers are printed.  If the argument
		 * 'all' is given, then all message blocks are printed,
		 * whether active or not.  The function prmess() can be
		 * found in the file stream.c.
		 */
	
				printf("\nSLOT\tNEXT\tCONT\tPREV\tRPTR\tWPTR\tDATAB\n");
				for (i=0; i < nmblock; i++) prmess(i,3);
				break;


			case 'M':
		/*
		 * The message is printed, including message block, data block
		 * and the data. The function print_message occurs in strdebug.c
		 */
				printf("msg slot = ");
				i = debuggint();
				print_message(&mblock[i]);
				break;


			case 'd':
		/*
		 * Print the header for the data blocks listed in the argument
		 * list, or all allocated data blocks, or all data blocks of
		 * a given set of classes, or all data blocks from all classes.
		 * The functions prdblk() and prdball() are located in the
		 * file stream.c.
		 */

				printf("\nSLOT CLASS SIZE  REF   TYPE     BASE     LIMIT  FREEP\n");
				ndblock = v.v_nblk4096 + v.v_nblk2048 + v.v_nblk1024 +
					v.v_nblk512 + v.v_nblk256 + v.v_nblk128 +
					v.v_nblk64 + v.v_nblk16 + v.v_nblk4;
				for (i=0; i < ndblock; i++) prdblk(i,1);
				break;


			case 'D':
		/*
		 * The data block is printed, 
		 * and the data. The function print_message occurs in strdebug.c
		 */
				printf("data block slot = ");
				i = debuggint();
				printf("\nSLOT CLASS SIZE  REF   TYPE     BASE     LIMIT  FREEP\n");
				prdblk(i,2);
				break;


			case 'f':

		/* 
		 * List all of the message or data headers on the respective
		 * free lists.  If the argument is 'm', then only the message
		 * headers are listed.  If the argument is 'd', then the following
		 * arguments list the classes of data blocks for which the
		 * free lists are to be printed.  If no classes are explicitly
		 * provided, then the free lists for all data block classes
		 * are printed.  If no argument at all is given, an error 
		 * message is printed.  The functions prmfree() and prdfree()
		 * are located in the file stream.c.
		 */ 


				printf("\nSLOT\tNEXT\tCONT\tPREV\tRPTR\tWPTR\tDATAB\n");
				prmfree();
				printf("\nSLOT CLASS SIZE  REF   TYPE     BASE     LIMIT  FREEP\n");
				for (i=0;i<9;i++) prdfree(i);
				break;


			case 'r':

		/*
		 * Print the slot numbers of each queue that has been enabled
		 * and placed on the qhead list.  This is done using the function
		 * prqrun() which is in the file stream.c.
		 */

				prqrun();
				break;

			case 'S':

		/*
		 * Print various statistics about streams, including current
		 * message block, data block, queue, and stream allocations
		 */

				prstrstat();
				break;
			}
		}
		break;
	case 't':
		{
			extern struct callout calltodo;
			register struct callout *p1;
			register int s;
		
			printf("Time\tFunc\tArg\n\n");
			s = spl7();
			for (p1 = calltodo.c_next; p1 != 0; p1 = p1->c_next) {
				printf("%d\t0x%x\t0x%x\n",
					p1->c_time, p1->c_func, p1->c_arg);
			}
			(void) splx(s);
		}
		break;
	case 'T':
		gettrace();
		break;
	case 'x':
		splx(spl);
		return;
	case 'z':
		c = '?';
		trace_p = (trace_end - trace_start - 44 + TRBUFSIZE)
						&TRBUFMASK;
		for(;c != 'x';) {
			printf("Trace buffer - ? - + . c d f g l n p s x: ");
			c = debuggetchar();
			printf("%c\n", c);
			switch (c) {
			default:
				printf("\t-\tPrevious page\n");
				printf("\t+\tNext page (also CR/LF)\n");
				printf("\t.\tCurrent page\n");
				printf("\tc\tClear trace buffer\n");
				printf("\td\tDump final page\n");
				printf("\tf\tBack to debug when buffer is full\n");
				printf("\tg\tDon't quit when buffer is full\n");
				printf("\tl\tList part of trace buffer\n");
				printf("\tn\tPrinting off\n");
				printf("\tp\tPrinting on\n");
				printf("\ts\tStatus\n");
				printf("\tx\tReturn to main debug menu\n");
				break;
			case 'x':
				break;
			case 's':
				printf("Printing = %d\n", trace_print);
				printf("Stop on full = %d\n", trace_quit);
				printf("Buffer size = %d\n", TRBUFSIZE);
				printf("Amount used = %d\n", (trace_end -
					trace_start + TRBUFSIZE)&TRBUFMASK);
				break;
			case 'c':
				trace_start = 0;
				trace_end = 0;
				break;
			case 'g':
				trace_quit = 0;
				break;
			case 'f':
				trace_quit = 1;
				break;
			case 'n':
				trace_print = 0;
				break;
			case 'p':
				trace_print = 1;
				break;
			case '-':
				trace_p -= 20;
				goto disp;
			case '\r':
			case '\n':
			case '+':
				trace_p += 20;
				goto disp;
			case 'd':
				trace_p = 0;
				goto disp;
			case 'l':
				printf("line: ");
				gets();
				i = getint(tbuf);
				if (i < 0) {
					printf("???\n");
					break;
				}
				trace_p = i;
			case '.':
			disp:
				if (trace_end == trace_start) {
					printf("	No Trace\n");
					break;
				}
				if (trace_p < 0)
					trace_p = 0;
				i = (trace_end - trace_start - 3 + TRBUFSIZE)
						&TRBUFMASK;
				if (trace_p > i)
					trace_p = i;
				i = (trace_start + trace_p)&TRBUFMASK;
				for (c = 0;c < 22;c++) {
					printf("%d:	",trace_p+c);
					printf(trace_a[i],trace_b[i],trace_c[i],
					       trace_d[i],trace_e[i],
					       trace_f[i],trace_g[i],
					       trace_h[i]);
					i++;
					if (i >= TRBUFSIZE)
						i = 0;
					if (i == trace_end)
						break;
				}
				c = 'd';
				break;
			}
		}
		break;
	default:
		printf("%c not implemented - try again\n", c);
		debughelp();
		break;
	}
    }
}

debughelp()
{
	extern int printfstall;
	int SavePrintfstall = printfstall;

	printf("\t?\thelp\n");
	printf("\tA\tstack backtrace\n");
	printf("\tD\tdump memory locations\n");
	printf("\td\tread/write hex locations\n");
	printf("\td\tforwarder\n");
	printf("\tp\tlist processes\n");
	printf("\tP\tset printf stall\n");
	printf("\tS\tstreams\n");
	printf("\tt\tlist timeout table\n");
	printf("\tT\tset/reset tracing flags\n");
	printf("\tt\talter debug trace\n");
	printf("\tx\texit debug\n");
	printf("\tz\ttrace buffer\n");

	printfstall = SavePrintfstall;
}

debugdebug()
{
	int o, l, a, v;

    for (;;) {
	printf("r w x:");
	o = debuggetchar();
	printf("%c\n", o);
	if (o == 'x')
		return;
	if (o != 'r' && o != 'w')
		continue;
	printf("b w l:");
	l = debuggetchar();
	printf("%c\n", l);
	if (l != 'b' && l != 'w' && l != 'l')
		continue;
	printf("addr = 0x");
	a = debugghex();
	switch (l) {
		case 'b':	printf("%x", *(char *)a & 0xFF);	break;
		case 'w':	printf("%x", *(short *)a & 0xFFFF);	break;
		case 'l':	printf("%x", *(long *)a);	break;
	}
	if (o == 'w') {
		printf("?");
		v = debugghex();
		switch (l) {
			case 'b':
				*(char *)a = v;
				printf("%x", *(char *)a & 0xFF);
				break;
			case 'w':
				*(short *)a = v;
				printf("%x", *(short *)a & 0xFFFF);
				break;
			case 'l':
				*(long *)a = v;
				printf("%x", *(long *)a);
				break;
		}
	} else
		printf("\n");
	}
}

debuggint()
{
	int i, c;

	i = 0;
	for (;;) {
		c = debuggetchar();
		printf("%c", c);
		if (c >= '0' && c <= '9')
		{
			i *= 10;
			i += (c - '0');
		}
		else
			break;
	}
	return(i);
}

debugghex()
{
	int i, c;

	i = 0;
	for (;;) {
		c = debuggetchar();
		printf("%c", c);
		if (c >= '0' && c <= '9')
			i = (i << 4) | (c - '0');
		else if (c >= 'a' && c <= 'f')
			i = (i << 4) | (c - 'a' + 10);
		else if (c >= 'A' && c <= 'F')
			i = (i << 4) | (c - 'A' + 10);
		else
			break;
	}
	return(i);
}

static	gethex(), mdump_row(), mdump_byte();
debugmdump()

{
	register char *addr, *newaddr;

	addr = (char *)0;
	newaddr = (char *)0x100;
loop:
	printf("{addr, <CR>, x} ?");
	gets();
	if ((newaddr = (char *) gethex(tbuf)) == (char *) -1) {
		if(tbuf[0] == 'x')
			return;
		printf("huh?\n");
		goto loop;
	}
	else if(tbuf[0] != '\0')
		addr = newaddr;
	newaddr = addr + 0x100;
	while(addr < newaddr) {
		mdump_row(addr);
		addr += 16;
	}
	goto loop;
}

static
mdump_row(addr) 

register char *addr;
{
	register char *end = (char *)addr + 16;
	register char *start;

	start = addr;
	mdump_byte(((unsigned)addr >> 24) & 0xFF);
	mdump_byte(((unsigned)addr >> 16) & 0xFF);
	mdump_byte(((unsigned)addr >> 8) & 0xFF);
	mdump_byte((unsigned)addr & 0xFF);
	outchar(':');
	outchar(' ');
	while(addr < end) {
		mdump_byte(*addr);
		++addr;
		if(((int)(addr-start) & 0x3) == 0)
			outchar(' ');
	}
	outchar(' ');
	for(addr = start; addr < end; ++addr) {
		if(*addr >= ' ' && *addr < 0177)
			outchar(*addr);
		else
			outchar('.');
	}
	printf("\n");
}

static char 	mdump_digits[] = "0123456789ABCDEF";

static
mdump_byte(val)
	
register unsigned val;
{

	outchar(mdump_digits[(val>>4) & 0xF]);
	outchar(mdump_digits[val & 0xF]);
}


static
getint(cp)
register char *cp;
{
	register c, n;

	n = 0;
	while (c = *cp++) {
		if (c >= '0' && c <= '9')
			n = n * 10 + c - '0';
		else return(-1);
	}
	return(n);
}

static
gethex(cp)
register char *cp;
{
	register c, n;

	n = 0;
	while (c = *cp++) {
		if (c >= '0' && c <= '9')
			n = n * 16 + c - '0';
		else if (c >= 'a' && c <= 'f')
			n = n * 16 + c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			n = n * 16 + c - 'A' + 10;
		else return(-1);
	}
	return(n);
}

gettrace()
{
	struct Tflags *tp;
	char *p;
	int i;

	for (;;) {
		printf("T_");
		gets();
		if (tbuf[0] == 0)
			return;
		for (tp = Tflags; tp->name; tp++) {
			i = strlen(tp->name);
			if (strncmp(tbuf, tp->name, i) == 0
			    && (!tbuf[i] || tbuf[i] == ' ')
			   )
				break;
			i -= 2;
			if (strncmp(tbuf, &tp->name[2], i) == 0
			    && (!tbuf[i] || tbuf[i] == ' ')
			   )
				break;
		}
		if (tp->name == 0) {
			for (tp = Tflags; tp->name; tp++)
				printf("%s: %x\n", tp->name, *tp->flag);
			continue;
		}
		if (tbuf[i])
			i++;
		*tp->flag = atoi(&tbuf[i]);
	}
}

atoi(s)
register char *s;
{
	register int i = 0;

	while (s && *s && *s >= '0' && *s <= '9')
		i = i * 10 + (*s++ - '0');
	return(i);
}

gets()
{
	char *p;

	for (p = tbuf; p < &tbuf[80]; p++) {
		*p = debuggetchar();
		if (*p == '\r')
			*p = '\n';
		if (*p == '\b') {
			if (p > tbuf) {
				printf("\b \b");
				p--;
			}
			p--;
		} else
			printf("%c", *p);
		if (*p == '\n')
			break;
	}
	*p = 0;
}

db0stack(dummy)
uint dummy;
{
	uint *p;
	int i, j;

	p = &dummy - 2;
	for (;;) {
		printf("FP:%x", p);
		if (p <= (uint *) 0x7ffff)
			i = (uint *) 0x7fff0 - &p[2];
		else {
			printf("\ndebug: do not know where the stack is\n");
			break;
		}
		if (i < 0)
			break;
		printf(" RA:%x", p[1]);
		if (i > 6)
			i = 6;
		if (i > 0) {
			printf(" PARS:");
			for (j = 0; j < i; j++)
				printf(" %x", p[j+2]);
		}
		printf("\n");
		p = (uint *)p[0];
	}
	printf("\n");
}

/*
 *****	begin of forwarder *********************************************
 */

fwdicp_statistics(accessp)
access_t	*accessp;

{
	printf("status = %x,\tnext_id = %x\n",
				accessp->status, accessp->next_id
	      );
	printf("to fep:  cmds = %d,\tbytes = %d\n", 
				accessp->cmds_to_fep, accessp->bytes_to_fep
	      );
	printf("to unix: cmds = %d,\tbytes = %d\n",
				accessp->cmds_to_unix, accessp->bytes_to_unix
	      );
	printf("to fep cmd ptrs:\trd = %x,\twr = %x,\tstart = %x,\tend = %x\n",
				accessp->cmd2fep.rp, accessp->cmd2fep.wp,
				accessp->cmd2fep.start, accessp->cmd2fep.end
	      );
	printf("to fep exp ptrs:\trd = %x,\twr = %x,\tstart = %x,\tend = %x\n",
				accessp->exp2fep.rp, accessp->exp2fep.wp,
				accessp->exp2fep.start, accessp->exp2fep.end
	      );
	printf("to fep rep ptrs:\trd = %x,\twr = %x,\tstart = %x,\tend = %x\n",
				accessp->rep2fep.rp, accessp->rep2fep.wp,
				accessp->rep2fep.start, accessp->rep2fep.end
	      );
	printf("to unix cmd ptrs:\trd = %x,\twr = %x,\tstart = %x,\tend = %x\n",
				accessp->cmd2unix.rp, accessp->cmd2unix.wp,
				accessp->cmd2unix.start, accessp->cmd2unix.end
	      );
	printf("to unix exp ptrs:\trd = %x,\twr = %x,\tstart = %x,\tend = %x\n",
				accessp->exp2unix.rp, accessp->exp2unix.wp,
				accessp->exp2unix.start, accessp->exp2unix.end
	      );
	printf("to unix rep ptrs:\trd = %x,\twr = %x,\tstart = %x,\tend = %x\n",
				accessp->rep2unix.rp, accessp->rep2unix.wp,
				accessp->rep2unix.start, accessp->rep2unix.end
	      );
}


/*
 * search through the virtual circuits, to find which are allocated, and 
 * then print out those
 */

fwd_pr_c(fep_specificsp)
fep_specifics_t 	*fep_specificsp;

{
	int		circuit;
	int		temp, i;
	fwd_circuit_t	*fwd_circuitp;

	temp = fep_specificsp->ncircuits;
	printf("out of %d circuits, the following are allocated\n", temp);
	printf("circuit\topen\tupstrm\tfepp\twait\thalt\tup\tdown\n");
	for ( fwd_circuitp = &(fep_specificsp->fwd_circuit[0]), circuit = 0;
	  circuit < temp;
	  fwd_circuitp++, circuit++
	)
	if (fwd_circuitp->upstreamq) {
	    printf("%d\t%d\t%x\t%x\t%d\t%d\t%d\t%d",
		    fwd_circuitp->circuit,
		    fwd_circuitp->open,
		    fwd_circuitp->upstreamq,
		    fwd_circuitp->fep,
		    fwd_circuitp->wait,
		    fwd_circuitp->halt,
		    fwd_circuitp->up_wait,
		    fwd_circuitp->down_wait
			
		   );
	    for (i = 0; i < CIRC_MAXID; i++)
	    if (fwd_circuitp->proc[i])
	    	printf(" p(%d)", fwd_circuitp->proc[i]->pr_id);
	    printf("\n");
	}
}



fwdicp_pr_cmd(ipcp)
ipc_t 	*ipcp;

{
	printf("\t\t%x\t%x\t%x\t%x\n",ipcp->cmd,ipcp->id,ipcp->circuit,ipcp->arg);
}


fwdicp_pr_q(type, ipc_qp)
char *type;
ipc_ptrs_t	*ipc_qp;

{
	short		did_it_once = 0;
	ipc_t		*p;

	printf(type);
	printf("\tslot\tcmd\tid\tcircuit\targ\n");
	for (p = ipc_qp->rp; p != ipc_qp->wp; p++)
	{
	    if  (p >= ipc_qp->end)
	    {
		if (did_it_once++)
		{
		    printf("fwdicp_pr_p: pointers must be messed up\n");
		    break;
		}
		p = ipc_qp->start;
	    }
	    printf("\t%d", p - ipc_qp->start);
	    fwdicp_pr_cmd(p);
	}
}


/*
 *****	end of forwarder ***********************************************
 */


extern dblk_t *dbfreelist[];
extern mblk_t *mbfreelist;

/*
 * prstream() formats an stdata entry in the stream table for crash.  It
 * expects to receive 2 arguments - the slot number of the stream entry
 * to be formatted and a flag indicating if all entries are being printed.
 * If the latter is set then an unallocated slot will be ignored; if clear,
 * then an error message will be printed.  
 */

prstream(c, all)
{
	struct stdata* strm;			/* stream entry buffer */
	long str_off;				/* offset of stream in mem */
	int wrq_slot, ioc_slot, ino_slot;	/* computed slot numbers */

	if ((c >= v.v_nstream) || (c < 0)) {
		printf("%d out of range\n",c);	/* requested stream out of range */
		return;
	}
	strm = &streams[c];
	if (strm->sd_wrq == NULL) {
		if (!all) printf("Slot %d not active\n",c);
		return;
	}
	printf("%d\t",c);
	wrq_slot = strm->sd_wrq - queue;
	printf("%d\t",wrq_slot);
	ioc_slot = strm->sd_iocblk - mblock;
	if ( (ioc_slot>=0)&&(ioc_slot<nmblock) )
		printf("%d\t",ioc_slot);
	else printf("-\t");
/*????
	ino_slot = VTOI(strm->sd_vnode) - inode;
*/
	printf("%d\t",ino_slot);
	printf("%d\t%d\t%d\t%d\t%o\t", strm->sd_pgrp, strm->sd_iocid,
		strm->sd_iocwait, strm->sd_wroff, strm->sd_error);
	printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		((strm->sd_flag & IOCWAIT) ? "iocw " : ""),
		((strm->sd_flag & RSLEEP) ? "rslp " : ""),
		((strm->sd_flag & WSLEEP) ? "wslp " : ""),
		((strm->sd_flag & STRHUP) ? "hup " : ""),
		((strm->sd_flag & STWOPEN) ? "stwo " : ""),
		((strm->sd_flag & CTTYFLG) ? "ctty " : ""),
		((strm->sd_flag & RMSGDIS) ? "mdis " : ""),
		((strm->sd_flag & RMSGNODIS) ? "mnds " : ""),
		((strm->sd_flag & STRERR) ? "err " : ""),
		((strm->sd_flag & STRTIME) ? "sttm " : ""),
		((strm->sd_flag & STR2TIME) ? "s2tm " : ""),
		((strm->sd_flag & STR3TIME) ? "s3tm " : ""),
		((strm->sd_flag & ESLEEP) ? "eslp " : ""),
		((strm->sd_flag & RSEL) ? "rsel " : ""),
		((strm->sd_flag & WSEL) ? "wsel " : ""),
		((strm->sd_flag & ESEL) ? "esel " : ""));
	printf("%s%s%s%s%s%s%s%s\n",
		((strm->sd_flag & TIME_OUT) ? "tmot " : ""),
		((strm->sd_flag & TIME_WAIT) ? "tmwt " : ""),
		((strm->sd_flag & STR_RCOLL) ? "rcol " : ""),
		((strm->sd_flag & STR_WCOLL) ? "wcol " : ""),
		((strm->sd_flag & STR_NBIO) ? "nbio " : ""),
		((strm->sd_flag & STR_ASYNC) ? "async " : ""),
		((strm->sd_flag & STSOPEN) ? "open " : ""),
		((strm->sd_flag & STR_CLOSING) ? "clos " : ""));
}


/* 
 * prqueue() - this function prints the queue entry corresponding to the given
 * slot number.  If the queue is not active, then an error message is printed
 * if all is 0, and nothing is printed if all is non-zero.  
 */

prqueue(c, all)
{
	queue_t *que;
	mblk_t *m;
	long que_off, moff;
	int qn, ql;

	if ((c >= v.v_nqueue) || (c<0)) {
		printf("%d out of range\n",c);	/* requested queue out of range */
		return;
	}
	que = &queue[c];
	if (!(que->q_flag & QUSE)) {
		if (!all) printf("Queue slot %d not in use\n",c);
		return;
	}
        printf("%d   ",c);
	qn = que->q_next - queue;
	ql = que->q_link - queue;
	if ((qn >= 0) && (qn < v.v_nqueue))
		printf("%d   ",qn);
	else printf("-   ");
	if ((ql >= 0) && (ql < v.v_nqueue))
		printf("\t%d ",ql);
	else printf("-   ");
	printf("\t%x ",que->q_qinfo);
	printf("\t%x ",que->q_ptr);
	printf("\t%d ",que->q_count);
	m = que->q_first;
	if (m != NULL) {
		moff = m - mblock;
		printf("%d ", moff);
	}
	else printf("   - ");
	m = que->q_last;
	if (m != NULL) {
		moff = m - mblock;
		printf("%d ", moff);
	}
	else printf("   - ");
	printf("%d %d %d %d ",que->q_minpsz,que->q_maxpsz,que->q_hiwat,
		que->q_lowat);
	printf("%s%s%s%s%s%s%s\n",
		((que->q_flag & QENAB) ? "enab " : ""),
		((que->q_flag & QWANTR) ? "wantr " : ""),
		((que->q_flag & QWANTW) ? "wantw " : ""),
		((que->q_flag & QFULL) ? "full " : ""),
		((que->q_flag & QREADR) ? "readr " : ""),
		((que->q_flag & QUSE) ? "use " : ""),
		((que->q_flag & QNOENB) ? "noenb " : ""));
	
}



/* 
 * prqsmsg() - this function prints the messages on the indicated queue.
 */

prqsmsg(c)
{
	queue_t *que;
	mblk_t *m;

	if ((c >= v.v_nqueue) || (c<0)) {
		printf("%d out of range\n",c);	/* queue out of range */
		return;
	}
	que = &queue[c];
	if (!(que->q_flag & QUSE)) {
		printf("Queue slot %d not in use\n",c);
		return;
	}
	printf("\nSLOT\tNEXT\tCONT\tPREV\tRPTR\tWPTR\tDATAB\n");
	m = que->q_first;
	while (m)
	{
		prmess(m - mblock, 0);
		m = m->b_next;
	}
}

/* 
 * prmess() prints out a message block header given a particular slot number.
 * If the slot is not currently active, then the action done depends on the 
 * value of all.  If all is 0, then an error message is printed.  If all is
 * 1, then nothing is printed.  If all is 2 (or greater), then the contents
 * of the slot are printed anyway.  This function returns the slot number
 * of the next message block (obtained from the b_next field), mainly for
 * use by prmfree(), if the slot is valid.
 * If all = 3 then the message will be printed only if b_ref != 0
 */

prmess(c, all)
{
	mblk_t *mblk;
	long  moff;
	int ndblock, mnext, mcont, datab, mprev;


	ndblock = v.v_nblk4096 + v.v_nblk2048 + v.v_nblk1024 + v.v_nblk512 +
	    v.v_nblk256 + v.v_nblk128 + v.v_nblk64 + v.v_nblk16 +
	    v.v_nblk4;
	if ((c >= nmblock) || (c < 0)) {
		printf("%d out of range\n",c);	/* requested message out of range */
		return(-1);
	}
	mblk = &mblock[c];
	if ((mblk->b_datap == NULL) && (all < 2)) {
	/* printf not reached in 2.0p implementation */
		if (!all) printf("Message slot %d not in use\n",c);
		return(-1);
	}
	mnext = mblk->b_next - mblock;
	mcont = mblk->b_cont - mblock;
	mprev = mblk->b_prev - mblock;
	datab = mblk->b_datap - dblock;
	if ((mblk->b_datap->db_ref != 0) || (all < 3))
	{
	    printf("%d ",c);
	    if ((mnext >= 0) && (mnext < nmblock))
		    printf("\t%d",mnext);
	    else printf("\t-");
	    if ((mcont >= 0) && (mcont < nmblock))
		    printf("\t%d",mcont);
	    else printf("\t-");
	    if ((mprev >= 0) && (mprev < nmblock))
		    printf("\t%d",mprev);
	    else printf("\t-");
	    printf("\t%x %x ",mblk->b_rptr, mblk->b_wptr);
	    if ((datab >= 0) && (datab < ndblock))
		    printf("\t%d\n",datab);
	    else printf("\t-\n");
	}
	return(mnext);
	
}

/*
 * prmfree() prints out the contents of the free message block list.  This
 * list is headed in memory by the the variable mbfreelist.  Since the
 * message slot should not be allocated, prmess() is called with all=2.
 */

prmfree()
{
	mblk_t *m;
	long  moff;
	int  mnext;


	m = mbfreelist;
	mnext = m - mblock;
	while ((mnext >=0) && (mnext < nmblock)) mnext = prmess(mnext,2);
}

/*
 * prdblk() prints out the contents of the given data block header slot.  If the
 * slot is not active, then the action taken depends on the value of all.  If all
 * is zero, then an error message is printed.  If all is 1, then nothing is done.
 * If all is 2, then the slot is printed anyway.  If the slot is valid, then 
 * the slot pointed to by the db_freep field is returned by the function (mainly
 * for use by prdfree().  
 */

prdblk(c, all)
{
	dblk_t *dblk;
	long  doff;
	int ndblock, dfree;
	static int lastcls;


	ndblock = v.v_nblk4096 + v.v_nblk2048 + v.v_nblk1024 + v.v_nblk512 +
	    v.v_nblk256 + v.v_nblk128 + v.v_nblk64 + v.v_nblk16 +
	    v.v_nblk4;
	if ((c >= ndblock) || (c < 0)) {
		printf("%d out of range\n",c);	/* requested data block out of range */
		return(-1);
	}
	dblk = &dblock[c];
	if ((dblk->db_ref == 0) && (all < 2)) {
	/* printf not reached in 2.0p implementation */
		if (!all) printf("Data block slot %d not in use\n", c);
		return(-1);
	}
	if (dblk->db_class != lastcls) {
		printf("\n");
		lastcls=dblk->db_class;
	}
	printf("%d %d ",c, dblk->db_class);
	switch (dblk->db_class) {
		case 0: printf("   4 "); break;
		case 1: printf("  16 "); break;
		case 2: printf("  64 "); break;
		case 3: printf(" 128 "); break;
		case 4: printf(" 256 "); break;
		case 5: printf(" 512 "); break;
		case 6: printf("1024 "); break;
		case 7: printf("2048 "); break;
		case 8: printf("4096 "); break;
		default: printf("   - ");
	}
	printf("%d ", dblk->db_ref); 
	switch (dblk->db_type) {
		case M_DATA: printf("data     "); break;
		case M_PROTO: printf("proto    "); break;
		case M_SPROTO: printf("sproto   "); break;
		case M_BREAK: printf("break    "); break;
		case M_SIG: printf("sig      "); break;
		case M_DELAY: printf("delay    "); break;
		case M_CTL: printf("ctl      "); break;
		case M_IOCTL: printf("ioctl    "); break;
		case M_SETOPTS: printf("setopts  "); break;
		case M_ADMIN: printf("admin    "); break;
		case M_EXPROTO: printf("exproto  "); break;
		case M_EXDATA: printf("exdata   "); break;
		case M_EXSPROTO: printf("exspro   "); break;
		case M_EXSIG: printf("exsig    "); break;
		case M_IOCACK: printf("iocack   "); break;
		case M_IOCNAK: printf("iocnak   "); break;
		case M_PCSIG: printf("pcsig    "); break;
		case M_FLUSH: printf("flush    "); break;
		case M_STOP: printf("stop     "); break;
		case M_START: printf("start    "); break;
		case M_HANGUP: printf("hangup   "); break;
		case M_ERROR: printf("error    "); break;
		default: printf("       - ");
	}
	printf("%x %x ", dblk->db_base, dblk->db_lim);
	dfree = dblk->db_freep - dblock;
	if ((dfree >= 0) && (dfree < ndblock))
		printf("%d\n",dfree);
	else printf("   -\n");
	return(dfree);
	
}


/*
 * prdball() prints out all data block header entries for the given class.
 * The hacky looking code is done mainly to get the location of the first
 * block of the class and the number of blocks to print.  Since some of these
 * blocks may not be allocated (or even lost), prdblk() is called with all=2.
 */

prdball(c)
{
	int i,n, *p;
	int clasize[NCLASS];

	if ((c<0)||(c>NCLASS-1)) return;
	p = &(v.v_nblk4096);
	for (i=NCLASS-1; i>=0; i--) clasize[i] = *p++;  /* load size of each block class */
	n=0;
	for (i=NCLASS-1; i>c; i--) n+=clasize[i];	/* compute slot of first block */
	for (i=0; i<clasize[c]; i++) prdblk(n++,2);
}


/*
 * prdfree() prints out the free list for the given data block class.  
 * The list is headed by the c_th entry of the dbfreelist[] array.  Since
 * these blocks should not be allocated, prdblk() is called with all=2.
 */

prdfree(c)
{
	dblk_t *d;
	long  doff;
	int ndblock, dnext;


	ndblock = v.v_nblk4096 + v.v_nblk2048 + v.v_nblk1024 + v.v_nblk512 +
	    v.v_nblk256 + v.v_nblk128 + v.v_nblk64 + v.v_nblk16 +
	    v.v_nblk4;
	if ((c<0) || (c>NCLASS-1)) return;
	d = dbfreelist[c];
	dnext = d - dblock;
	while ((dnext >=0) && (dnext < ndblock)) dnext = prdblk(dnext,2);
}

/* 
 * prqrun() prints out all of the queue slots which have been placed on the
 * queue run list (by a qenable() operation).  Presumably all of these
 * queues are active.  Only the slot numbers are printed; the queue command
 * can then be used to print the individual queue slots.
 */

prqrun()
{
	queue_t *q;
	int  ql;

	q = qhead;
	printf("Queue slots scheduled for service: ");
	while (q != NULL) {
		ql = q - queue;
		printf("%d ",ql);
		q = q->q_link;
	}
	printf("\n");
}

/*
 * prstrstat() - print statistics about streams data structures
 */

prstrstat()
{
	long offset;
	queue_t *q;
	struct stdata *str;
	mblk_t *m;
	dblk_t *d;
	int qusecnt, susecnt, mfreecnt, musecnt,
	    dfreecnt, dusecnt, dfc[NCLASS], duc[NCLASS], dcc[NCLASS], qruncnt;
	int ndblock, i,j, *p;

	ndblock = v.v_nblk4096 + v.v_nblk2048 + v.v_nblk1024 + v.v_nblk512 +
	    v.v_nblk256 + v.v_nblk128 + v.v_nblk64 + v.v_nblk16 +
	    v.v_nblk4;
	qusecnt = susecnt = mfreecnt = 0;
	musecnt = dfreecnt = dusecnt = qruncnt = 0;
	for (i=0; i<NCLASS; i++) dfc[i] = duc[i] =0;
	p = &(v.v_nblk4096);
	for (i=NCLASS-1; i>=0; i--) dcc[i] = *p++;

	printf("queue %x, qhead %x, streams %x\n",
		queue, qhead, streams);
	printf("mblock %x, mbfreelist %x, dblock %x, dbfreelist %x\n",
		mblock, mbfreelist, dblock, dbfreelist);
	printf("ITEM\t\t\tCONFIGURED\tALLOCATED\tFREE\n");

	q = qhead;
	while (q != NULL) {
		qruncnt++;
		q = q->q_link;
	}
	
	q = queue;
	for (i=0; i<v.v_nqueue; i++) {
		if (q->q_flag & QUSE) qusecnt++;
		q++;
	}

	str = streams;
	for (i=0; i<v.v_nstream; i++) {
		if (str->sd_wrq != NULL) susecnt++;
		str++;
	}

	m = mbfreelist;
	while (m != NULL) {
		mfreecnt++;
		m = m->b_next;
	}

	m = mblock;
	for (i=0; i<nmblock; i++){
		if (m->b_datap != NULL) musecnt++;
		m++;
	}

	for (j=0; j<NCLASS; j++){
		d = dbfreelist[j];
		while (d != NULL) {
			dfc[j]++;
			dfreecnt++;
			d = d->db_freep;
		}
	}

	d = dblock;
	for (i=0; i<ndblock; i++) {
		if (d->db_ref) {
			dusecnt++;
			duc[d->db_class]++;
		}
		d++;
	}

	printf("streams\t\t\t%d\t\t%d\t\t%d\n",
		v.v_nstream, susecnt, v.v_nstream - susecnt);
	printf("queues\t\t\t%d\t\t%d\t\t%d\n",
		v.v_nqueue, qusecnt, v.v_nqueue - qusecnt);
	printf("message blocks\t\t%d\t\t%d\t\t%d\n",
		nmblock, musecnt, mfreecnt);
	printf("data block totals\t%d\t\t%d\t\t%d\n",
		ndblock, dusecnt, dfreecnt);
	for (i=0; i<NCLASS; i++) 
		printf("data block class %d\t%d\t\t%d\t\t%d\n",
			i, dcc[i], duc[i], dfc[i]);
	printf("\nCount of scheduled queues: %d\n", qruncnt);


}


/*
 *	Get a character, strip off parity, and ignore ^S/^Q
 */
debuggetchar()
{
	register int c;

	do {
		c = getchar() & 0x7F;
	} while (c == '\021' || c == '\023');
	return (c);
}

/*
 *	Debug trace routine (called like printf)
 */

/* VARARGS */
debugtrace(a, b, c, d, e, f, g, h)
char *a;
long b,c,d,e,f,g,h;
{
	trace_a[trace_end] = a;
	trace_b[trace_end] = b;
	trace_c[trace_end] = c;
	trace_d[trace_end] = d;
	trace_e[trace_end] = e;
	trace_f[trace_end] = f;
	trace_g[trace_end] = g;
	trace_h[trace_end] = h;
	if (trace_print)
		printf(trace_a[trace_end],
		       trace_b[trace_end],
		       trace_c[trace_end],
		       trace_d[trace_end],
		       trace_e[trace_end],
		       trace_f[trace_end],
		       trace_g[trace_end],
		       trace_h[trace_end]);
	trace_end++;
	if (trace_end >= TRBUFSIZE)
		trace_end = 0;
	if (trace_end == trace_start) {
		trace_start++;
		if (trace_start >= TRBUFSIZE)
			trace_start = 0;
	}
	if (trace_quit && ((trace_end+1)&TRBUFMASK) == trace_start)
		debug();
}
#endif	DEBUG
