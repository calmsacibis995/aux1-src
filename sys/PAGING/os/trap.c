#ifndef lint	/* .../sys/PAGING/os/trap.c */
#define _AC_NAME trap_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation, All Rights Reserved.  {Apple version 1.6 87/12/01 18:04:13}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.6 of trap.c on 87/12/01 18:04:13";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	@(#)trap.c	UniPlus VVV.2.1.6	*/

#ifdef HOWFAR
extern int	T_utrap;
#endif HOWFAR
#include "sys/debug.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/time.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/trap.h"
#ifdef mc68881
#include "sys/fptrap.h"
#endif mc68881
#include "sys/sysinfo.h"
#include "sys/sysmacros.h"
#include "sys/buserr.h"
#include "sys/sysent.h"
#include "compat.h"

#define	EBIT	1		/* user error bit in PS: C-bit */
#define	USER	0x1000		/* user-mode flag added to number */

#ifdef SYSCALLS
typedef enum {false, true} bool;
static char reserved[] = "reserved";

bool	SyscallTraceTable[] = {
/* 0	indir		exit		fork		read		*/
	false,		false,		false,		false,

/* 4	write		open		close		wait		*/
	false,		false,		false,		false,

/* 8	creat		link		unlink		exec		*/
	false,		false,		false,		false,

/* 12	chdir		time		mknod		chmod		*/
	false,		false,		false,		false,

/* 16	chown		break		ostat		seek		*/
	false,		false,		false,		false,

/* 20	getpid		old svfs_mount	umount		setuid		*/
	false,		false,		false,		false,

/* 24	getuid		stime		ptrace		alarm		*/
	false,		false,		false,		false,

/* 28	ofstat		pause		utime		nosys		*/
	false,		false,		false,		false,

/* 32	nosys		access		nice		sleep		*/
	false,		false,		false,		false,

/* 36	sync		kill		sysm68k(0)	setpgrp		*/
	false,		false,		false,		false,

/* 40	tell		dup		pipe		times		*/
	false,		false,		false,		false,

/* 44	prof		lock		setgid		getgid		*/
	false,		false,		false,		false,

/* 48	sig		msgsys		sysm68k(1)	acct		*/
	false,		false,		false,		false,

/* 52	shmsys		semsys		ioctl		phys		*/
	false,		false,		false,		false,

/* 56	locking		utssys		nosys		exece		*/
	false,		false,		false,		false,

/* 60	umask		chroot		fcntl		ulimit		*/
	false,		false,		false,		false,

/* 64	reboot		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 68	nosys		nosys		accept		bind		*/
	false,		false,		false,		false,

/* 72	connect		gethostid	gethostname	getpeername	*/
	false,		false,		false,		false,

/* 76	getsockname	getsockopt	listen		recv		*/
	false,		false,		false,		false,

/* 80	recvfrom	recvmsg		select		send		*/
	false,		false,		false,		false,

/* 84	sendmsg		sendto		sethostid	sethostname	*/
	false,		false,		false,		false,

/* 88	setregid	setreuid	setsockopt	shutdown	*/
	false,		false,		false,		false,

/* 92	socket		socketpair	nosys		nosys		*/
	false,		false,		false,		false,

/* 96	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 100	getdomainname	setdomainname	getgroups	setgroups	*/
	false,		false,		false,		false,

/* 104	getdtablesize	flock		readv		writev		*/
	false,		false,		false,		false,

/* 108	mkdir		rmdir		getdirentries	lstat		*/
	false,		false,		false,		false,

/* 112	symlink		readlink	truncate	ftruncate	*/
	false,		false,		false,		false,

/* 116	fsync		statfs		fstatfs		async_daemon	*/
	false,		false,		false,		false,

/* 120	old nfs_mount	nfs_svc		nfs_getfh	rename		*/
	false,		false,		false,		false,

/* 124	fstat		stat		vtrace		getcompat	*/
	false,		false,		false,		false,

/* 128	setcompat	sigvec		sigblock	sigsetmask	*/
	false,		false,		false,		false,

/* 132	sigpause	sigstack	gettimeofday	settimeofday	*/
	false,		false,		false,		false,

/* 136	adjtime		quotactl	exportfs	mount		*/
	false,		false,		false,		false,

/* 140	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 144	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 148	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 152	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 156	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 160	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 164	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 168	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 172	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 176	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 180	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 184	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 188	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 192	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 196	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 200	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 204	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 208	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 212	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 216	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 220	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 224	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 228	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 232	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 236	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 240	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 244	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 248	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false,

/* 252	nosys		nosys		nosys		nosys		*/
	false,		false,		false,		false
};

char *callnames[] = {
	/*  0 */	"indir", "exit", "fork", "read",
	/*  4 */	"write", "open", "close", "wait",
	/*  8 */	"creat", "link", "unlink", "exec",
	/* 12 */	"chdir", "time", "mknod", "chmod",
	/* 16 */	"chown", "break", "ostat", "seek",
	/* 20 */	"getpid", "old mount (obsolete)", "unmount", "setuid",
	/* 24 */	"getuid", "stime", "ptrace", "alarm",
	/* 28 */	"ofstat", "pause", "utime", "stty",
	/* 32 */	"gtty", "access", "nice", "sleep",
	/* 36 */	"sync", "kill", "sysm68k(0)", "setpgrp",
	/* 40 */	"tell", "dup", "pipe", "times",
	/* 44 */	"prof", "lock", "setgid", "getgid",
	/* 48 */	"sig", "msgsys", "sysm68k(1)", "acct",
	/* 52 */	"shmsys", "semsys", "ioctl", "phys",
	/* 56 */	"locking", "utssys", reserved, "exece",
	/* 60 */	"umask", "chroot", "fcntl", "ulimit",

	/* 64 */	"reboot", reserved, reserved, reserved,
	/* 68 */	reserved, reserved, "accept", "bind",
	/* 72 */	"connect", "gethostid", "gethostname", "getpeername",
	/* 76 */	"getsockname", "getsockopt", "listen", "recv",
	/* 80 */	"recvfrom", "recvmsg", "select", "send",
	/* 84 */	"sendmsg", "sendto", "sethostid", "sethostname",
	/* 88 */	"setregid", "setreuid", "setsockopt", "shutdown",
	/* 92 */	"socket", "socketpair", reserved, reserved,
	/* 96 */	reserved, reserved, reserved, reserved,
	/* 100 */	"getdomainname", "setdomainname", "getgroups", "setgroups",
	/* 104 */	"getdtablesize", "flock", "readv", "writev",
	/* 108 */	"mkdir", "rmdir", "getdirentries", "lstat",
	/* 112 */	"symlink", "readlink", "truncate", "ftruncate",
	/* 116 */	"fsync", "statfs", "fstatfs", "async_daemon",
	/* 120 */	"old NFS mount (obsolete)", "nfs_svc", "nfs_getfh", "rename",
	/* 124 */	"fstat", "stat", "vtrace", "getcompat",
	/* 128 */	"setcompat", "sigvec", "sigblock", "sigsetmask",
	/* 132 */	"sigpause", "sigstack", "getitimer", "setitimer",
	/* 136 */	"gettimeofday", "settimeofday", "adjtime", "quotactl",
	/* 140 */	"exportfs", "mount", reserved, reserved,
	/* 144 */	reserved, reserved, reserved, reserved,
	/* 148 */	reserved, reserved, "sigcleanup", reserved,
	/* 152 */	reserved, reserved, reserved, reserved,
	/* 156 */	reserved, reserved, reserved, reserved,
	/* 160 */	reserved, reserved, reserved, reserved,
	/* 164 */	reserved, reserved, reserved, reserved,
	/* 168 */	reserved, reserved, reserved, reserved,
	/* 172 */	reserved, reserved, reserved, reserved,
	/* 176 */	reserved, reserved, reserved, reserved,
	/* 180 */	reserved, reserved, reserved, reserved,
	/* 184 */	reserved, reserved, reserved, reserved,
	/* 188 */	reserved, reserved, reserved, reserved,
	/* 192 */	reserved, reserved, reserved, reserved,
	/* 196 */	reserved, reserved, reserved, reserved,
	/* 200 */	reserved, reserved, reserved, reserved,
	/* 204 */	reserved, reserved, reserved, reserved,
	/* 208 */	reserved, reserved, reserved, reserved,
	/* 212 */	reserved, reserved, reserved, reserved,
	/* 216 */	reserved, reserved, reserved, reserved,
	/* 220 */	reserved, reserved, reserved, reserved,
	/* 224 */	reserved, reserved, reserved, reserved,
	/* 228 */	reserved, reserved, reserved, reserved,
	/* 232 */	reserved, reserved, reserved, reserved,
	/* 236 */	reserved, reserved, reserved, reserved,
	/* 240 */	reserved, reserved, reserved, reserved,
	/* 244 */	reserved, reserved, reserved, reserved,
	/* 248 */	reserved, reserved, reserved, reserved,
	/* 252 */	reserved, reserved, reserved, reserved
};

char *errornames[] = {
	0, "EPERM", "ENOENT", "ESRCH", "EINTR",
	"EIO", "ENXIO", "E2BIG", "ENOEXEC", "EBADF",
	"ECHILD", "EAGAIN", "ENOMEM", "EACCES", "EFAULT",
	"ENOTBLK", "EBUSY", "EEXIST", "EXDEV", "ENODEV",
	"ENOTDIR", "EISDIR", "EINVAL", "ENFILE", "EMFILE",
	"ENOTTY", "ETXTBSY", "EFBIG", "ENOSPC", "ESPIPE",
	"EROFS", "EMLINK", "EPIPE", "EDOM", "ERANGE",
	"ENOMSG", "EIDRM", "ECHRNG", "EL2NSYNC", "EL3HLT",
	"EL3RST", "ELNRNG", "EUNATCH", "ENOCSI", "EL2HLT",
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	"EWOULDBLOCK", "EINPROGRESS", "EALREADY", "ENOTSOCK", "EDESTADDRREQ",
	"EMSGSIZE", "EPROTOTYPE", "ENOPROTOOPT", "EPROTONOSUPPORT", "ESOCKTNOSUPPORT",
	"EOPNOTSUPP", "EPFNOSUPPORT", "EAFNOSUPPORT", "EADDRINUSE", "EADDRNOTAVAIL",
	"ENETDOWN", "ENETUNREACH", "ENETRESET", "ECONNABORTED", "ECONNRESET",
	"ENOBUFS", "EISCONN", "ENOTCONN", "ESHUTDOWN", "ETOOMANYREFS",
	"ETIMEDOUT", "ECONNREFUSED", "ELOOP", "ENAMETOOLONG", "EHOSTDOWN",
	"EHOSTUNREACH", "ENOTEMPTY", 0, 0, 0,
	0, 0, 0, 0, 0,
	"ESTALE", "EREMOTE", "EDQUOT", 0, 0,
	"EDEADLOCK",
};

static int ernamcnt = sizeof(errornames)/sizeof(errornames[0]);

#endif SYSCALLS

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
char	regloc[8+8+1+1] = {
	R0, R1, R2, R3, R4, R5, R6, R7,
	AR0, AR1, AR2, AR3, AR4, AR5, AR6, SP, PC,
	RPS
};

/*
 * Called from the trap handler when a processor trap occurs.
 */
trap(number, regs)
short number;
{
	register struct user *up;
	register struct proc *p;
	extern int parityno;
	register i;			/* must be first data register - d2 */
	time_t syst;
	int retval, lasttrap;
	register code;			/* pc is the most common use */
	int *oldar0;
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	extern short fp881;		/* is there an MC68881? */
#endif mc68881

TRACE(T_utrap,("trap(%d, 0x%x)\n", number, regs));
	up = &u;
	p = up->u_procp;
	retval = 0;
	syst = up->u_stime;
#if defined(FLOAT) || defined(mc68881)	/* sky floating point or MC68881 */
	up->u_fpsaved = 0;
#endif
	oldar0 = up->u_ar0;
	up->u_ar0 = &regs;
	lasttrap = up->u_traptype;
	code = up->u_ar0[PC];
	switch(((struct buserr *)(&regs))->ber_format) {
	case FMT_LONG:
		up->u_traptype = TRAPLONG;
		break;
	case FMT_SHORT:
		up->u_traptype = TRAPSHORT;
		break;
	case FMT_LONG10:
		up->u_traptype = TRAPLONG10;
		break;
	case FMT_MIDDLE:	/* ill inst, trap, etc */
		code = *((long *)((long)(&up->u_ar0[PC])+6));
		up->u_traptype = TRAPNORM;
		break;
	default:
		up->u_traptype = TRAPNORM;
		break;
	}
	if (usermode(up->u_ar0[RPS]))
		number |= USER;
#ifdef HOWFAR
#ifdef KDB
	if (number != RESCHED && number != RESCHED+USER &&
	    number != TRAP2 && number != TRCTRAP &&
	    !(number == TRCTRAP+USER && (u.u_flags&UF_COPR_TRACE)))) {
#else KDB
	if (number != RESCHED && number != RESCHED+USER
		&& !(number == TRCTRAP+USER && (u.u_flags&UF_COPR_TRACE))) {
#endif KDB
		printf("trap number=0x%x ps=0x%x\n", number,
			up->u_ar0[RPS]&0xFFFF);
		showregs(1);
	}
#endif HOWFAR
	/*
	 * Handle parity specially to make it processor independent
	 */
	if (number==parityno || number==(parityno|USER)) {
		if ((i = parityerror()) == 0) {
			logparity((paddr_t)&up->u_ar0[PC]);
			goto out;
		}
		if (i > 0) {
			number = i | (number & USER);
			goto sw;
		}
		if (number & USER) {
			logparity((paddr_t)&up->u_ar0[PC]);
			i = SIGBUS;
		} else {
			if (nofault) {
				up->u_ar0 = oldar0;
 				up->u_traptype = lasttrap;
				longjmp(nofault, -1);
			}
			showbus();
			panic("kernel parity error");
		}
	} else {
sw:
	switch(number) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 */
	default:
		if ((number & USER) == 0) {
			panicstr = "trap";	/* fake it for printfs */
			printf("\ntrap type %d\n", number);
			showregs(1);
			panic("unexpected kernel trap");
		}

	case CHK + USER:	/* CHK instruction */
	case TRAPV + USER:	/* TRAPV instruction */
	case PRIVVIO + USER:	/* Priviledge violation */
	case L1010 + USER:	/* Line 1010 emulator */
	case L1111 + USER:	/* Line 1111 emulator */
	case TRAP4 + USER:
	case TRAP5 + USER:
	case TRAP6 + USER:
	case TRAP7 + USER:
	case TRAP8 + USER:
	case TRAP9 + USER:
	case TRAP10 + USER:
	case TRAP11 + USER:
	case TRAP12 + USER:
	case TRAP13 + USER:
	case TRAP14 + USER:
	case ILLINST + USER:	/* illegal instruction */
		i = SIGILL;
		break;

	case DIVZERO + USER:	/* zero divide */
		i = SIGFPE;
		code = KINTDIV;
		break;

#ifdef mc68881		/* MC68881 floating-point coprocessor */
	case FPBSUN + USER:	/* Branch or Set on Unordered Condition */
	case FPINEX + USER:	/* Inexact Result */
	case FPDZ + USER:	/* Floating Point Divide by Zero */
	case FPUNFL + USER:	/* Underflow */
	case FPOPERR + USER:	/* Operand Error */
	case FPOVFL + USER:	/* Overflow */
	case FPSNAN + USER:	/* Signalling NAN (Not-A-Number) */
		if (fp881) {
			/* save EXC byte from STATUS register */
			up->u_fpexc = (fp_status() >> 8) & 0xff;
		}
		i = SIGFPE;
		break;
#endif mc68881

#ifdef KDB
	case TRAP2:	/* bpt - trap #2 */
		{
			extern int *kdb_registers;

			SPL7();
			kdb_registers = up->u_ar0;
			kdb_break();
			up->u_ar0 = oldar0;
			up->u_traptype = lasttrap;
			return(retval);	
		}
#endif KDB
	case TRCTRAP:		/* trace out of kernel mode - */
#ifdef KDB
		{
			extern int *kdb_registers;

			i = spl7();
			kdb_registers = up->u_ar0;
			if (kdb_trace()== 0) {
				splx(i);
			}
		}
#endif KDB
		up->u_ar0 = oldar0;
		up->u_traptype = lasttrap;
		return(retval);	/* this is happens when a trap instruction */
		  	 	/* is executed with the trace bit set */

	case TRAP1 + USER:	/* bpt - trap #1 */
		up->u_ar0[PC] -= 2;
	case TRCTRAP + USER:	/* trace */
		if (u.u_flags&UF_COPR_TRACE && number == (TRCTRAP+USER)) {
			u.u_ar0[RPS] &= ~PS_T;
			u.u_flags &= ~UF_COPR_TRACE;
			goto out;
		}
		i = SIGTRAP;
		up->u_ar0[RPS] &= ~PS_T;
		break;

	case TRAP2 + USER:	/* iot - trap #2 */
		i = SIGIOT;
		code = up->u_ar0[R0];
		break;

	case TRAP3 + USER:	/* emt - trap #3 */
		i = SIGEMT;
		code = up->u_ar0[R0];
		break;

	case SYSCLL0 + USER:	/* sys call 0 - trap #0 - M68k generic*/
		panic("syscall0");
		/*NOTREACHED*/

	case SYSCLL1 + USER:	/* sys call 1 - trap #15 - UniSoft */
		panic("syscall1");
		/*NOTREACHED*/

	/*
	 * Allow process switch
	 */
	case RESCHED + USER:
	case RESCHED:
		qswtch();
		goto out;

	/*
	 * If the user SP is below the stack segment,
	 * tough (see ml/mch.s && hardflt)
	 * This relies on the ability of the hardware
	 * to restart a half executed instruction.
	 * On the 68000 this is not the case and
	 * the routine machdep/backup() will fail.
	 */

	case ADDRERR + USER:	 /* bus error - address error */
		i = SIGBUS;
		retval = 1;
		trapaddr((struct buserr *)&regs);
		up->u_traptype |= TRAPADDR;
		code = (((struct buserr *)&regs)->ber_faddr);
		mmuerror(1);
		break;

	case BUSERR + USER:	/* memory management error - bus error */
		i = SIGSEGV;
		retval = 1;
		trapaddr((struct buserr *)&regs);
		up->u_traptype |= TRAPBUS;
		code = (((struct buserr *)&regs)->ber_faddr);
		mmuerror(1);
		break;

	case ADDRERR:	/* kernel address error */
		if (nofault) {
			up->u_ar0 = oldar0;
			up->u_traptype = lasttrap;
			longjmp(nofault, -1);
		}
		trapaddr((struct buserr *)&regs);
		printf("Kernel address error\n");
		showbus();
		mmuerror(1);
		panic("kernel memory management error\n");
	case BUSERR:	/* kernel bus error */
		if (nofault) {
			up->u_ar0 = oldar0;
			up->u_traptype = lasttrap;
			longjmp(nofault, -1);
		}
		trapaddr((struct buserr *)&regs);
		printf("Kernel bus error\n");
		showbus();
		mmuerror(1);
		panic("kernel memory management error\n");

	/*
	 * On a format trap, we check to see if we have just restored
	 * hardware state information that the user could have modified,
	 * if so, we blow away the user process.
	 */

	case FMTTRAP:
		if(lasttrap & TRAPREST) {
			register signo;

#ifdef	mc68881
			fpnull();
#endif	mc68881
			up->u_signal[SIGILL-1] = 0;
			signo = sigmask(SIGILL);
			p->p_sigignore &= ~signo;
			p->p_sigcatch &= ~signo;
			p->p_sigmask &= ~signo;
			i = SIGILL;
			break;
		}
		panic("CPU stack frame format error");

	/*
	 * Unused trap vectors generate this trap type.
	 * Reciept of this trap is a
	 * symptom of hardware problems and may
	 * represent a real interrupt that got
	 * sent to the wrong place.  Watch out
	 * for hangs on disk completion if this message appears.
	 */
	case SPURINT:
	case SPURINT + USER:
		printf("\nRandom interrupt ignored\n");
		up->u_ar0 = oldar0;
		up->u_traptype = lasttrap;
		return(retval);
	}
	}	/* end else ...			*/
	psignal(p, i);

out:
	if (p->p_pri >= (PUSER-NZERO))
		curpri = p->p_pri = calcppri(p);

	/* The following code implements 
	 *	if (p->p_sig && issig()) psig();
	 */
	if (p->p_cursig || p->p_sig) {
		SPLHI();
		if (issig()) {
			SPL0();
			psig(code);
		} else
			SPL0();
	}

	if (up->u_prof.pr_scale)
		addupc((unsigned)up->u_ar0[PC], &up->u_prof,
						(int)(up->u_stime-syst));
#ifdef FLOAT
	if (up->u_fpinuse && up->u_fpsaved)
		restfp();
#endif
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	if (fp881 && up->u_fpsaved)
		fprest();
#endif mc68881
	up->u_ar0 = oldar0;
	up->u_traptype = lasttrap;
	return(retval);
}

extern int	nsysent;
/*
 * process a system call
 */
/* ARGSUSED */
/* VARARGS */
syscall0(r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,sp,bsr,ps,pc)
{
	register struct user *up;
	register *regp, *argp;
	int sys;
	int opc;
	register time_t syst;
	register struct sysent *callp;
	register struct proc *p;
	int i;	
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	extern short fp881;		/* is there an MC68881? */
#endif mc68881

	up = &u;
#if defined(FLOAT) || defined(mc68881)	/* sky floating point or MC68881 */
	up->u_fpsaved = 0;
#endif
	syst = up->u_stime;
	sysinfo.syscall++;
	up->u_error = 0;
	up->u_ar0 = regp = &r0;
	up->u_traptype = 0;
	opc = up->u_ar0[PC] - 2;
	p = up->u_procp;

	if (p->p_flag & SCOFF) {
		ps &= ~PS_C;
		argp = (int *)sp;		/* point to first arg */
		argp++;		/* skip word with param count */
		sys = r0&0377;
		if (sys >= nsysent)
			sys = 0;
		else if (sys==0) {	/* indirect */
			sys = fuword((caddr_t)argp++)&0377;
			if (sys >= nsysent)
				sys = 0;
		}
		if (sys == 150) {
			sigcleanup();
			goto out;
		}
		callp = &sysent[sys];
#ifdef UMVSEG
		if (callp->sy_narg)
		if (callp->sy_narg == 1) {
			up->u_arg[0] = fuword((caddr_t)argp);
		} else {
			copyin(argp, up->u_arg, sizeof(int)*callp->sy_narg);
		}
#else
		for(i=0; i<callp->sy_narg; i++) {
			up->u_arg[i] = fuword((caddr_t)argp++);
		}
#endif UMVSEG
#ifdef SYSCALLS
		if (SyscallTraceTable[sys] == true) { 
			argp = up->u_arg;
			printf("***** %s (COFF, pid %d) : %x %x %x %x\n",
				callnames[sys], p->p_pid, argp[0], argp[1],
					argp[2], argp[3], argp[4], argp[5]);
		}
#endif
		up->u_dirp = (caddr_t)up->u_arg[0];
		up->u_rval1 = 0;
		up->u_rval2 = r1;
		up->u_ap = up->u_arg;
		if (save(up->u_qsav)) {
			up = &u;
			if (up->u_error == 0)
				if ((p->p_compatflags & COMPAT_SYSCALLS) == 0 || up->u_eosys == JUSTRETURN)
				up->u_error = EINTR;
		} else {
			up->u_eosys = JUSTRETURN;
			(*(callp->sy_call))(up->u_ap);
		}
		if (up->u_eosys == RESTARTSYS)
			regp[PC] = opc;
		else if (up->u_error) {
			r0 = up->u_error;
			ps |= PS_C;	/* carry bit */
			if (++up->u_errcnt > 16) {
				up->u_errcnt = 0;
				runrun++;
			}
#ifdef SYSCALLS
			if (SyscallTraceTable[sys] == true)
				if (up->u_error > ernamcnt ||
				    errornames[up->u_error] == 0)
					printf("      %s:  syscall error = %d, pc = 0x%x\n",
						callnames[sys], up->u_error, pc);
				else
					printf("      %s:  syscall error = %s, pc = 0x%x\n",
						callnames[sys], errornames[up->u_error], pc);
#endif
		} else {
			r0 = up->u_rval1;
			r1 = up->u_rval2;
		}
	       /*
		********************************************************
		If the system call trap was executed with the T bit set,
		the trace trap will be ignored.  So, the bit is reset
		and the SIGTRAP signal is sent to the process.
		********************************************************
		*/
		if (ps & PS_T) {	/* was trace bit set ? */
			i = SIGTRAP;
			ps &= ~PS_T;	/* reset trace bit */
					/* break; send signal to process */
		} else {
			p->p_pri = (p->p_cpu>>1) + PUSER + p->p_nice - NZERO;
			curpri = p->p_pri;
			if (runrun == 0)
				goto out;
			qswtch();
			goto out;
		}
		psignal(p, i);
	} else {
		up->u_ap = argp = up->u_arg;
		sys = regp[R0] & 0377;
		if (sys >= nsysent)
			sys = 0;
		if (sys == 150) {
			sigcleanup();
			goto out;
		}
		callp = &sysent[sys];
		argp[0] = regp[AR0];
		argp[1] = regp[R1];
		argp[2] = regp[AR1];
		argp[3] = regp[R2];
		argp[4] = regp[AR2];
		argp[5] = regp[R3];
	
#ifdef SYSCALLS
		if (SyscallTraceTable[sys] == true)
			printf("***** %s (pid %d): %x %x %x %x\n",
				callnames[sys], p->p_pid,
				argp[0], argp[1], argp[2], 
				argp[3], argp[4], argp[5]);
#endif
		up->u_dirp = (caddr_t)argp[0];
		up->u_rval1 = 0;
		up->u_rval2 = regp[R1];
		if (save(up->u_qsav)) {
			if (up->u_error==0)
				if ((p->p_compatflags & COMPAT_SYSCALLS) == 0 || up->u_eosys == JUSTRETURN)
					up->u_error = EINTR;
		} else {
			up->u_eosys = JUSTRETURN;
			(*(callp->sy_call))(up->u_ap);
		}
		if (up->u_eosys == RESTARTSYS)
			regp[PC] = opc;
		else if (up->u_error) {
			regp[R0] = up->u_error;
			regp[RPS] |= PS_C;		/* carry bit */
			if (++up->u_errcnt > 16) {
				up->u_errcnt = 0;
				runrun++;
			}
#ifdef SYSCALLS
			if (SyscallTraceTable[sys] == true)
				if (up->u_error > ernamcnt ||
				    errornames[up->u_error] == 0)
					printf("      %s:  syscall error = %d, pc = 0x%x\n",
						callnames[sys], up->u_error, regp[PC]);
				else
					printf("      %s:  syscall error = %s, pc = 0x%x\n",
						callnames[sys], errornames[up->u_error], regp[PC]);
#endif
		} else {
			regp[RPS] &= ~PS_C;		/* carry bit */
			regp[R0] = up->u_rval1;
			regp[R1] = up->u_rval2;
		}
		curpri = p->p_pri = calcppri(p);
		if (runrun != 0)
			qswtch();
	
		}
out:
	/* The following code implements 
	 *	if (p->p_sig && issig()) psig();
	 */
	if (p->p_cursig || p->p_sig) {
		SPLHI();
		if (issig()) {
			SPL0();
			psig(pc);
		}
		else
			SPL0();
	}

	if(up->u_prof.pr_scale)
		addupc((unsigned)up->u_ar0[PC], &up->u_prof, (int)(up->u_stime-syst));
#ifdef FLOAT
	if (up->u_fpinuse && up->u_fpsaved)
		restfp();
#endif
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	if (fp881 && up->u_fpsaved)
		fprest();
#endif mc68881
}

syscall1(regs)
{
	register struct user *up;
	register *regp, *argp;
	int sys;
	int opc;
	register time_t syst;
	register struct sysent *callp;
	register struct proc *p;
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	extern short fp881;		/* is there an MC68881? */
#endif mc68881

	up = &u;
#if defined(FLOAT) || defined(mc68881)	/* sky floating point or MC68881 */
	up->u_fpsaved = 0;
#endif
	p = up->u_procp;
	syst = up->u_stime;
	sysinfo.syscall++;
	up->u_error = 0;
	up->u_traptype = 0;
	up->u_ar0 = regp = &regs;
	opc = regp[PC] - 2;
	up->u_ap = argp = up->u_arg;
	sys = regp[R0] & 0377;
	if (sys >= nsysent)
		sys = 0;

	if (sys == 150) {
		sigcleanup();
		goto out;
	}

	callp = &sysent[sys];
	argp[0] = regp[AR0];
	argp[1] = regp[R1];
	argp[2] = regp[AR1];
	argp[3] = regp[R2];
	argp[4] = regp[AR2];
	argp[5] = regp[R3];

#ifdef SYSCALLS
	if (SyscallTraceTable[sys] == true)
		printf("***** %s (UniPlus+, pid %d): %x %x %x %x\n",
			callnames[sys], p->p_pid,
			argp[0], argp[1], argp[2], argp[3], argp[4], argp[5]);
#endif
	up->u_dirp = (caddr_t)argp[0];
	up->u_rval1 = 0;
	up->u_rval2 = regp[R1];
	if (save(up->u_qsav)) {
		up = &u;
		if (up->u_error==0)
			if ((p->p_compatflags & COMPAT_SYSCALLS) == 0 || up->u_eosys == JUSTRETURN)
				up->u_error = EINTR;
	} else {
		up->u_eosys = JUSTRETURN;
		(*(callp->sy_call))(up->u_ap);
	}
	if (up->u_eosys == RESTARTSYS)
		regp[PC] = opc;
	else if (up->u_error) {
		regp[R0] = up->u_error;
		regp[RPS] |= PS_C;		/* carry bit */
		if (++up->u_errcnt > 16) {
			up->u_errcnt = 0;
			runrun++;
		}
#ifdef SYSCALLS
		if (SyscallTraceTable[sys] == true)
			if (up->u_error > ernamcnt || errornames[up->u_error] == 0)
				printf("      %s:  syscall error = %d, pc = 0x%x\n",
					callnames[sys], up->u_error, regp[PC]);
			else
				printf("      %s:  syscall error = %s, pc = 0x%x\n",
					callnames[sys], errornames[up->u_error], regp[PC]);
#endif
	} else {
		regp[RPS] &= ~PS_C;		/* carry bit */
		regp[R0] = up->u_rval1;
		regp[R1] = up->u_rval2;
	}
	curpri = p->p_pri = calcppri(p);
	if (runrun != 0)
		qswtch();
out:
	/* The following code implements 
	 *	if (p->p_sig && issig()) psig();
	 */
	if (p->p_cursig || p->p_sig) {
		SPLHI();
		if (issig()) {
			SPL0();
			psig(regp[PC]);
		}
		else
			SPL0();
	}

	if(u.u_prof.pr_scale)
		addupc((unsigned)regp[PC], &u.u_prof, (int)(u.u_stime-syst));
#ifdef FLOAT
	if (up->u_fpinuse && up->u_fpsaved)
		restfp();
#endif
#ifdef mc68881		/* MC68881 floating-point coprocessor */
	if (fp881 && up->u_fpsaved)
		fprest();
#endif mc68881
}


/*
 * nonexistent system call-- signal bad system call.
 */
nosys()
{
	psignal(u.u_procp, SIGSYS);
}

/*
 * Ignored system call
 */
nullsys()
{
}

errsys()
{
	u.u_error = EINVAL;
}

stray(addr)
physadr addr;
{
	logstray(addr);
	printf("stray interrupt at %x\n", addr);
}

/*
 * trapaddr - Save the info from a 68010 bus or address error.
 */
trapaddr(ap)
register struct buserr *ap;
{
	if (cputype == 68000)
		return;
	u.u_fcode = ap->ber_sstat;
	u.u_aaddr = ap->ber_faddr;
#ifdef mc68020
	u.u_ireg = ap->ber_inspb;
#else mc68020
	u.u_ireg = ap->ber_iib;
#endif mc68020
}

/*
 * showbus - print out status on mmgt error
 */
showbus()
{
	register struct user *up;
	register physaddr;

	up = &u;
#ifndef NEW_PMMU
	if ((unsigned)svtopte(up->u_aaddr) > (unsigned)ptob(physmem))
		physaddr = -1;
	else
#endif
		physaddr = (int)svirtophys((caddr_t)up->u_aaddr);

	printf("vaddr = 0x%x physaddr = 0x%x ireg = 0x%x fcode = 0x%x\n",
		up->u_aaddr, physaddr, up->u_ireg&0xFFFF,
		up->u_fcode&0xF);
	showregs(1);
}

/*
 * Show a processes registers
 */
showregs(mmuflg)
int mmuflg;
{
	register struct user *up;
	register int i, j;
	char command[COMMSIZ+1];

	up = &u;
#ifdef HOWFAR
	if (mmuflg)
		dumpmm(-1);
#endif HOWFAR
#ifdef lint
	dumpmm(mmuflg);
#endif lint
	for (i=0; i<COMMSIZ; i++) {
		j = up->u_comm[i];
		if (j<=' ' || j>=0x7F)
			break;
		command[i] = j;
	}
	command[i] = 0;
	/*
	 * separate prints in case up or u_procp trashed
	 */
	printf("pc = 0x%x sr = 0x%x up->u_procp = 0x%x",
		up->u_ar0[PC], up->u_ar0[RPS]&0xFFFF, up->u_procp);
	printf(" pid = %d exec = '%s'\n", up->u_procp->p_pid, command);
	for (i = 0; i < 16; i++) {
		printf("0x%x ", up->u_ar0[i]);
		if (i == 7 || i == 15) printf("\n");
	}
}
