#ifndef lint	/* .../sys/COMMON/os/sysent.c */
#define _AC_NAME sysent_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1987 Apple Computer, Inc., All Rights Reserved.  {Apple version 1.5 87/12/01 18:02:47}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.5 of sysent.c on 87/12/01 18:02:47";
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/* @(#)sysent.c	1.3 */

#include "sys/types.h"
#include "sys/sysent.h"

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 */

int	errsys();
int	netdown();
int	nosys();
int	nullsys();

int	access();
int	adjtime();
int	alarm();
#ifdef	AUTOCONFIG
#define	async_daemon	nosys
#else
int	async_daemon();
#endif	/* AUTOCONFIG */
int	chdir();
int	chmod();
int	chown();
int	chroot();
int	close();
int	creat();
int	dup();
int	exec();
int	exece();
#ifdef	AUTOCONFIG
#define	exportfs	nosys
#else
int	exportfs();
#endif	/* AUTOCONFIG */
int	fchmod();
int	fchown();
int	fcntl();
int	flock();
int	fork();
int	fstat();
int	fstatfs();
int	fsync();
int	ftruncate();
int	getcompat();
int	getdirentries();
int	getdomainname();
int	getdtablesize();
int	getgid();
int	getgroups();
int	getitimer();
int	getpid();
int	gettimeofday();
int	getuid();
int	gtime();
int	ioctl();
int	kill();
int	link();
int	lock();
int	lstat();
int	mkdir();
int	mknod();
int	mount();
int	msgsys();
#ifdef	AUTOCONFIG
#define	nfs_getfh	nosys
#define	nfs_svc		nosys
#else
int	nfs_getfh();
int	nfs_svc();
#endif	/* AUTOCONFIG */
int	nice();
int	ofstat();
int	open();
int	ostat();
int	pause();
int	pipe();
int	profil();
int	ptrace();
#ifdef	QUOTA
int	quotactl();
#else
#define	quotactl	errsys
#endif
int	read();
int	readv();
int	readlink();
int	rename();
int	rexit();
int	rmdir();
int	sbreak();
int	seek();
int	semsys();
int	setcompat();
int	setdomainname();
int	setgid();
int	setgroups();
int	setitimer();
int	setpgrp();
int	setregid();
int	setreuid();
int	settimeofday();
int	setuid();
int	shmsys();
int	sigblock();
int	sigpause();
int	sigsetmask();
int	sigstack();
int	sigvec();
int	ssig();
int	stat();
int	statfs();
int	stime();
int	symlink();
int	sync();
int	sysacct();
#if	defined(m68k)
int	sysm68k();
#else
#define	sysm68k	nosys
#endif
int	times();
int	truncate();
int	ulimit();
int	umask();
int	umount();
int	unlink();
int	unmount();
int	utimes();
int	utssys();
#ifdef	TRACE
int	vtrace();
#else
#define	vtrace	nosys
#endif	TRACE
int	wait();
int	write();
int	writev();

/* net stuff */
int	gethostid();
int	gethostname();
int	sethostid();
int	sethostname();
int 	select();

#ifdef	AUTOCONFIG
#define	accept		netdown
#define	bind		netdown
#define	connect		netdown
#define	getpeername	netdown
#define	getsockname	netdown
#define	getsockopt	netdown
#define	listen		netdown
#define	recv		netdown
#define	recvfrom	netdown
#define	recvmsg		netdown
#define	send		netdown
#define	sendmsg		netdown
#define	sendto		netdown
#define	setsockopt	netdown
#define	shutdown	netdown
#define	socket		netdown
#define	socketpair	netdown
#else
int	accept();
int	bind();
int	connect();
int	getpeername();
int	getsockname();
int	getsockopt();
int	listen();
int	recv();
int	recvfrom();
int	recvmsg();
int	send();
int	sendmsg();
int	sendto();
int	setsockopt();
int	shutdown();
int	socket();
int	socketpair();
#endif	/* AUTOCONFIG */

/*
 * Local system calls
 */
int	locking();
int	phys();
int	reboot();
int	powerdown();

struct sysent sysent[] =
{
	0, 0, nosys,			/*  0 = indir */
	1, 0, rexit,			/*  1 = exit */
	0, 0, fork,			/*  2 = fork */
	3, 1, read,			/*  3 = read */
	3, 1, write,			/*  4 = write */
	3, 0, open,			/*  5 = open */
	1, 1, close,			/*  6 = close */
	1, 1, wait,			/*  7 = wait */
	2, 0, creat,			/*  8 = creat */
	2, 0, link,			/*  9 = link */
	1, 0, unlink,			/* 10 = unlink */
	2, 0, exec,			/* 11 = exec */
	1, 0, chdir,			/* 12 = chdir */
	1, 0, gtime,			/* 13 = time */
	3, 0, mknod,			/* 14 = mknod */
	2, 0, chmod,			/* 15 = chmod */
	3, 0, chown,			/* 16 = chown; now 3 args */
	1, 0, sbreak,			/* 17 = break */
	2, 0, ostat,			/* 18 = ostat */
	3, 0, seek,			/* 19 = seek */
	0, 0, getpid,			/* 20 = getpid */
	0, 0, nosys,			/* 21 = old svfs_mount */
	1, 0, unmount,			/* 22 = unmount */
	1, 0, setuid,			/* 23 = setuid */
	0, 0, getuid,			/* 24 = getuid */
	1, 0, stime,			/* 25 = stime */
	4, 0, ptrace,			/* 26 = ptrace */
	1, 0, alarm,			/* 27 = alarm */
	2, 0, ofstat,			/* 28 = ofstat */
	0, 1, pause,			/* 29 = pause */
	2, 0, utimes,			/* 30 = utimes */
	2, 0, nosys,			/* 31 = nosys */
	2, 0, nosys,			/* 32 = nosys */
	2, 0, access,			/* 33 = access */
	1, 0, nice,			/* 34 = nice */
	0, 0, nosys,			/* 35 = sleep; inoperative */
	0, 0, sync,			/* 36 = sync */
	2, 0, kill,			/* 37 = kill */
	5, 0, sysm68k,			/* 38 = m68k/3B specific */
	3, 0, setpgrp,			/* 39 = setpgrp */
	0, 0, nosys,			/* 40 = tell - obsolete */
	1, 0, dup,			/* 41 = dup */
	0, 0, pipe,			/* 42 = pipe */
	1, 0, times,			/* 43 = times */
	4, 0, profil,			/* 44 = prof */
	1, 0, lock,			/* 45 = proc lock */
	1, 0, setgid,			/* 46 = setgid */
	0, 0, getgid,			/* 47 = getgid */
	2, 0, ssig,			/* 48 = sig */
	6, 1, msgsys,			/* 49 = msg queue entry point */
	5, 1, sysm68k,			/* 50 = m68k/3B specific */
	1, 0, sysacct,			/* 51 = turn acct off/on */
	4, 1, shmsys,			/* 52 = shared memory */
	5, 1, semsys,			/* 53 = semaphore entry point */
	3, 1, ioctl,			/* 54 = ioctl */
	4, 0, phys,			/* 55 = phys */
	3, 0, locking,			/* 56 = file locking */
	3, 0, utssys,			/* 57 = utssys */
	0, 0, nosys,			/* 58 = reserved for USG */
	3, 0, exece,			/* 59 = exece */
	1, 0, umask,			/* 60 = umask */
	1, 0, chroot,			/* 61 = chroot */
	3, 0, fcntl,			/* 62 = fcntl */
	2, 0, ulimit,			/* 63 = ulimit */
	1, 0, reboot,			/* 64 = reboot */
	1, 0, powerdown,		/* 65 = x */
	0, 0, nosys,			/* 66 = x */
	0, 0, nosys,			/* 67 = x */
	0, 0, nosys,			/* 68 = x */
	0, 0, nosys,			/* 69 = x */
	3, 0, accept,			/* 70 = accept */
	3, 0, bind,			/* 71 = bind */
	3, 0, connect,			/* 72 = connect */
	0, 0, gethostid,		/* 73 = gethostid */
	2, 0, gethostname,		/* 74 = gethostname */
	3, 0, getpeername,		/* 75 = getpeername */
	3, 0, getsockname,		/* 76 = getsockname */
	5, 0, getsockopt,		/* 77 = getsockopt */
	2, 0, listen,			/* 78 = listen */
	4, 0, recv,			/* 79 = recv */
	6, 0, recvfrom,			/* 80 = recvfrom */
	3, 0, recvmsg,			/* 81 = recvmsg */
	5, 0, select,			/* 82 = select */
	4, 0, send,			/* 83 = send */
	3, 0, sendmsg,			/* 84 = sendmsg */
	6, 0, sendto,			/* 85 = sendto */
	1, 0, sethostid,		/* 86 = sethostid */
	2, 0, sethostname,		/* 87 = sethostname */
	2, 0, setregid,			/* 88 = setregid */
	2, 0, setreuid,			/* 89 = setreuid */
	5, 0, setsockopt,		/* 90 = setsockopt */
	2, 0, shutdown,			/* 91 = shutdown */
	3, 0, socket,			/* 92 = socket */
	4, 0, socketpair,		/* 93 = socketpair */
	0, 0, nosys,			/* 94 = nosys */
	0, 0, nosys,			/* 95 = x */
	0, 0, nosys,			/* 96 = x */
	0, 0, nosys,			/* 97 = x */
	0, 0, nosys,			/* 98 = x */
	0, 0, nosys,			/* 99 = x */
	2, 0, getdomainname,		/* 100 = getdomainname */
	2, 0, setdomainname,		/* 101 = setdomainname */
	2, 0, getgroups,		/* 102 = getgroups */
	2, 0, setgroups,		/* 103 = setgroups */
	0, 0, getdtablesize,		/* 104 = getdtablesize */
	2, 0, flock,			/* 105 = flock */
	3, 0, readv,			/* 106 = readv */
	3, 0, writev,			/* 107 = writev */
	2, 0, mkdir,			/* 108 = mkdir */
	1, 0, rmdir,			/* 109 = rmdir */
	4, 0, getdirentries,		/* 110 = getdirentries */
	2, 0, lstat,			/* 111 = lstat */
	2, 0, symlink,			/* 112 = symlink */
	3, 0, readlink,			/* 113 = readlink */
	2, 0, truncate,			/* 114 = truncate */
	2, 0, ftruncate,		/* 115 = ftruncate */
	1, 0, fsync,			/* 116 = fsync */
	2, 0, statfs,			/* 117 = statfs */
	2, 0, fstatfs,			/* 118 = fstatfs */
	0, 0, async_daemon,		/* 119 = async_daemon */
	0, 0, nosys,			/* 120 = old nfs_mount */
	1, 0, nfs_svc,			/* 121 = nfs_svc */
	2, 0, nfs_getfh,		/* 122 = nfs_getfh */
	2, 0, rename,			/* 123 = rename */
	2, 0, fstat,			/* 124 = fstat */
	2, 0, stat,			/* 125 = stat */
	2, 0, vtrace,			/* 126 = vtrace */
	0, 0, getcompat,		/* 127 = getcompat */
	1, 0, setcompat,		/* 128 = setcompat */
	3, 0, sigvec,			/* 129 = sigvec */
	1, 0, sigblock,			/* 130 = sigblock */
	1, 0, sigsetmask,		/* 131 = sigsetmask */
	1, 0, sigpause,			/* 132 = sigpause */
	2, 0, sigstack,			/* 133 = sigstack */
	2, 0, getitimer,		/* 134 = getitimer */
	3, 0, setitimer,		/* 135 = setitimer */
	1, 0, gettimeofday,		/* 136 = gettimeofday */
	1, 0, settimeofday,		/* 137 = settimeofday */
	2, 0, adjtime,			/* 138 = adjtime */
	4, 0, quotactl,			/* 139 = quotactl */
	3, 0, exportfs,			/* 140 = exportfs */
	4, 0, mount,			/* 141 = mount */
	1, 0, umount,			/* 142 = umount */
	3, 0, fchmod,			/* 143 = fchmod */
	3, 0, fchown,			/* 144 = fchown */
	0, 0, nosys,			/* 145 = x */
	0, 0, nosys,			/* 146 = x */
	0, 0, nosys,			/* 147 = x */
	0, 0, nosys,			/* 148 = x */
	0, 0, nosys,			/* 149 = x */
	0, 0, nosys,			/* 150 = x */
	0, 0, nosys,			/* 151 = x */
	0, 0, nosys,			/* 152 = x */
	0, 0, nosys,			/* 153 = x */
	0, 0, nosys,			/* 154 = x */
	0, 0, nosys,			/* 155 = x */
	0, 0, nosys,			/* 156 = x */
	0, 0, nosys,			/* 157 = x */
	0, 0, nosys,			/* 158 = x */
	0, 0, nosys,			/* 159 = x */
	0, 0, nosys,			/* 160 = x */
	0, 0, nosys,			/* 161 = x */
	0, 0, nosys,			/* 162 = x */
	0, 0, nosys,			/* 163 = x */
	0, 0, nosys,			/* 164 = x */
	0, 0, nosys,			/* 165 = x */
	0, 0, nosys,			/* 166 = x */
	0, 0, nosys,			/* 167 = x */
	0, 0, nosys,			/* 168 = x */
	0, 0, nosys,			/* 169 = x */
	0, 0, nosys,			/* 170 = x */
	0, 0, nosys,			/* 171 = x */
	0, 0, nosys,			/* 172 = x */
	0, 0, nosys,			/* 173 = x */
	0, 0, nosys,			/* 174 = x */
	0, 0, nosys,			/* 175 = x */
	0, 0, nosys,			/* 176 = x */
	0, 0, nosys,			/* 177 = x */
	0, 0, nosys,			/* 178 = x */
	0, 0, nosys,			/* 179 = x */
	0, 0, nosys,			/* 180 = x */
	0, 0, nosys,			/* 181 = x */
	0, 0, nosys,			/* 182 = x */
	0, 0, nosys,			/* 183 = x */
	0, 0, nosys,			/* 184 = x */
	0, 0, nosys,			/* 185 = x */
	0, 0, nosys,			/* 186 = x */
	0, 0, nosys,			/* 187 = x */
	0, 0, nosys,			/* 188 = x */
	0, 0, nosys,			/* 189 = x */
	0, 0, nosys,			/* 190 = x */
	0, 0, nosys,			/* 191 = x */
	0, 0, nosys,			/* 192 = x */
	0, 0, nosys,			/* 193 = x */
	0, 0, nosys,			/* 194 = x */
	0, 0, nosys,			/* 195 = x */
	0, 0, nosys,			/* 196 = x */
	0, 0, nosys,			/* 197 = x */
	0, 0, nosys,			/* 198 = x */
	0, 0, nosys,			/* 199 = x */
	0, 0, nosys,			/* 200 = x */
	0, 0, nosys,			/* 201 = x */
	0, 0, nosys,			/* 202 = x */
	0, 0, nosys,			/* 203 = x */
	0, 0, nosys,			/* 204 = x */
	0, 0, nosys,			/* 205 = x */
	0, 0, nosys,			/* 206 = x */
	0, 0, nosys,			/* 207 = x */
	0, 0, nosys,			/* 208 = x */
	0, 0, nosys,			/* 209 = x */
	0, 0, nosys,			/* 210 = x */
	0, 0, nosys,			/* 211 = x */
	0, 0, nosys,			/* 212 = x */
	0, 0, nosys,			/* 213 = x */
	0, 0, nosys,			/* 214 = x */
	0, 0, nosys,			/* 215 = x */
	0, 0, nosys,			/* 216 = x */
	0, 0, nosys,			/* 217 = x */
	0, 0, nosys,			/* 218 = x */
	0, 0, nosys,			/* 219 = x */
	0, 0, nosys,			/* 220 = x */
	0, 0, nosys,			/* 221 = x */
	0, 0, nosys,			/* 222 = x */
	0, 0, nosys,			/* 223 = x */
	0, 0, nosys,			/* 224 = x */
	0, 0, nosys,			/* 225 = x */
	0, 0, nosys,			/* 226 = x */
	0, 0, nosys,			/* 227 = x */
	0, 0, nosys,			/* 228 = x */
	0, 0, nosys,			/* 229 = x */
	0, 0, nosys,			/* 230 = x */
	0, 0, nosys,			/* 231 = x */
	0, 0, nosys,			/* 232 = x */
	0, 0, nosys,			/* 233 = x */
	0, 0, nosys,			/* 234 = x */
	0, 0, nosys,			/* 235 = x */
	0, 0, nosys,			/* 236 = x */
	0, 0, nosys,			/* 237 = x */
	0, 0, nosys,			/* 238 = x */
	0, 0, nosys,			/* 239 = x */
	0, 0, nosys,			/* 240 = reserved for OEMs */
	0, 0, nosys,			/* 241 = reserved for OEMs */
	0, 0, nosys,			/* 242 = reserved for OEMs */
	0, 0, nosys,			/* 243 = reserved for OEMs */
	0, 0, nosys,			/* 244 = reserved for OEMs */
	0, 0, nosys,			/* 245 = reserved for OEMs */
	0, 0, nosys,			/* 246 = reserved for OEMs */
	0, 0, nosys,			/* 247 = reserved for OEMs */
	0, 0, nosys,			/* 248 = reserved for OEMs */
	0, 0, nosys,			/* 249 = reserved for OEMs */
	0, 0, nosys,			/* 250 = reserved for OEMs */
	0, 0, nosys,			/* 251 = reserved for OEMs */
	0, 0, nosys,			/* 252 = reserved for OEMs */
	0, 0, nosys,			/* 253 = reserved for OEMs */
	0, 0, nosys,			/* 254 = reserved for OEMs */
	0, 0, nosys,			/* 255 = reserved for OEMs */
};
int	nsysent = sizeof(sysent) / sizeof(struct sysent);

/* <@(#)sysent.c	6.2> */
