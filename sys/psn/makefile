#
# @(#)Copyright Apple Computer 1987	Version 1.27 of makefile on 87/11/18 16:38:59 
#
#
# Description file for Milwaukee virtual UNIX

# -------------------------------------------------------------------
# This is the beginning of the make macros you can change for local
# source reconfiguration. DO NOT CHANGE ANYTHING ELSE.

#
#	Master root location
#	This is the head of the tree in which we can find the source.
#	For example if you have 
#		/usr/unisoft/src/sys/SVFS
#		/usr/unisoft/src/sys/PAGING
#		/usr/unisoft/src/sys/<etc.>
#	you should set
#		MASSTERROOT=/usr/unisoft/src
#
# If you are always running out of sys/psn, you can change it to
#		MASTERROOT=../..
# This is the recomended way.
#

MASTERROOT	=../..

# This is the END of the local configuration macros.
# --------------------------------------------------------------------
#


SHELL	=/bin/sh

#
#	Machine configuration information
#
#	***** DO NOT CHANGE REL or VER!!!! They MUST be this to match manuals. <----
#	***** PKR.  Wed Nov 18 16:37:47 PST 1987 <----------------------------------
#
SYSTEM	=A/UX
NODE	=notnamed
REL	=SVR2
#
# changed this VER to be the latest version number
#
VER	=1.0
MACH	=mc68020
MAKEFILE=makefile
S	=psn
TTY	=1

#
#	Memory layout information
#
#		KMEMORG is the address of the start of the pstart section.
#			It should be the address of physical memory, usually
#			the beginning, just after the vectors. It must be
#			followed by enough contiguous memory to load the 
#			kernel.
#
#		KMEMLEN is used to tell the linker the max size of memory to
#			be loaded into. It must be bigger than BSSSTART plus
#			the size of the bss section.
#
#		TEXTSTART is the start address of the virtual text section
#			it normally should be at a 0x1000000 boundary and
#			at a virtual address that corresponds to no 'usefull'
#			physical address (ie doesn't hide memory or IO devices)
#
#		DATASTART is the start of the virtual data region (see
#			TEXTSTART above for restrictions) it is normally 
#			at TEXTSTART+0x1000000.
#
#		BSSSTART is the start of the virtual bss region (see
#			TEXTSTART above for restrictions) it is normally 
#			at DATASTART+0x1000000.
#
#
#
KMEMORG		=0x500
KMEMLEN		=0x7ffffb00
TEXTSTART	=0x10000000
DATASTART	=0x11000000
BSSSTART	=0x12000000
MEMDEFS		= -DAUTOCONFIG \
		-DKMEMORG=$(KMEMORG) \
		-DKMEMLEN=$(KMEMLEN) \
		-DTEXTSTART=$(TEXTSTART) \
		-DDATASTART=$(DATASTART) \
		-DBSSSTART=$(BSSSTART)
#
#		MODULES is the list of modules to be put into the kernel
#			by default (ie those that are 'born' there rather
#			than added by autoconfig at a later date)
#
MODULES =	-n scc -m 0 -t ct \
		-n scsi -m 24 -t b \
		-n tty \
		-n streams 
#
#	Include location information
#
SINC	=/usr/include
INCLUDE=		\
	-I.		\
	-I$(SINC)
#
#	Source location information
#
SYSTREE	=$(MASTERROOT)
COMMON	=$(SYSTREE)/sys/COMMON
PAGING	=$(SYSTREE)/sys/PAGING
SVFS	=$(SYSTREE)/sys/SVFS/sys
NET	=$(SYSTREE)/sys/NET/sys
NETinet	=$(SYSTREE)/sys/NETinet/sys
NFS	=$(SYSTREE)/sys/NFS/sys
RPC	=$(SYSTREE)/sys/RPC/sys
SPEC 	=$(SYSTREE)/sys/$S
CF	=$(SPEC)/cf
IO	=$(SPEC)/io
ML	=$(SPEC)/ml
NODEML	=$(SYSTREE)/sys/$(NODE)/ml
OS	=$(SPEC)/os
CCF	=$(COMMON)/cf
CIO	=$(COMMON)/io
CML	=$(COMMON)/ml
COS	=$(COMMON)/os
LIBC	=$(COMMON)/libc
PCF	=$(PAGING)/cf
PIO	=$(PAGING)/io
PML	=$(PAGING)/ml
POS	=$(PAGING)/os

#
#	Program (Compiler, etc) setup (flags and path names)
#
AS	=/bin/as
CC	=/bin/cc
CFLAGS	=$(DEFS) $(INCLUDE) $(VERBOSE) $(ZFLAGS) -O
CNOFLAGS=$(DEFS) $(INCLUDE) $(VERBOSE) $(ZFLAGS) 
CPP	=/lib/cpp
#	Flavors of Memory management
MMU=	-DPMMU -DMMB -DPMMUMMB #	# PMMU dressed like MMB
MMU=	-DMMB #				# MMB
MMU=	-DPMMU -DNEW_PMMU #		# PMMU 
DEFS=	-DKERNEL -Uvax -Usun -Dm68k -Dmc68020 -Dmc68881 \
	-DINET -DETHERLINK -DSTREAMS -UQUOTA $(LOCAL) \
	-DPAGING $(MMU) -DNFS
#	Other options:
# SCREEN: define to enable screen code
# DVT: define for Rev 5 boards.
LOCAL=	-DAUTOCONFIG -DNFSDEBUG -DDEBUG -DITRACE -DHOWFAR \
	-DSCREEN -DDVT -DSYSCALLS -DSLOTS -DPASS_MAJOR
LOCAL=	-DAUTOCONFIG -DSCREEN -DDVT -DSLOTS -DPASS_MAJOR
AUTOCONFIG = /etc/autoconfig
LD	=/bin/ld
LFLAGS	=-N init.ld
LINT	=/usr/bin/lint
LINTFLAGS=-bn
NM	=/bin/nm
NMFLAGS	=-xev
VERBOSE	=
ZFLAGS	=-Zn -F
FIX	=/bin/size -x
STRIP	=/bin/strip

.c.o:
	$(CC) $(CFLAGS) -c $<

CONFIGSRC=			\
	$(CF)/nfsconf.c	\
	$(CF)/bnetuconf.c     \
	$(IO)/if_ae6.c		\
	$(COS)/vfs_conf.c

DEVSRC=	$(NET)/tty_pty.c	\
	$(IO)/hd.c		\
	$(IO)/gdisk.c		\
	$(IO)/gdisksubr.c	\
	$(IO)/sdisk.c		\
	$(IO)/sccio.c		\
	$(IO)/via.c		\
	$(IO)/scsi.c		\
	$(IO)/sony.c		\
	$(IO)/fmt.s		\
	$(IO)/mst.s		\
	$(IO)/rdhead.s		\
	$(IO)/snrd.s		\
	$(IO)/fdb.c		\
	$(IO)/fdbs.s		\
	$(IO)/font.c		\
	$(IO)/fontdef.c		\
	$(IO)/font1.s		\
	$(IO)/font2.s		\
	$(IO)/key.c		\
	$(IO)/keyboard.c	\
	$(IO)/mouse.c		\
	$(IO)/video.c		\
	$(IO)/vt100.c		\
	$(IO)/sound.s		\
	$(IO)/debug_data.c	\
	$(IO)/debug.c

MACHDEPSRC=			\
	$(COS)/genassym.c	\
	$(COS)/geninit.c	\
	$(COS)/cmachdep.c 	\
	$(POS)/machdep.c 	\
	$(NET)/nmachdep.c	\
	$(POS)/umachdep.c 	\
	$(ML)/bnetivec.s	\
	$(OS)/startup.c

PCFSRC=	$(PCF)/name.c

PIOSRC=	$(PIO)/clist.c \
	$(CIO)/clone.c \
	$(CIO)/err.c \
	$(PIO)/fp.c \
	$(CIO)/line.c \
	$(PIO)/mem.c \
	$(PIO)/osm.c \
	$(CIO)/partab.c \
	$(PIO)/prof.c \
	$(CIO)/sxt.c \
	$(CIO)/sys.c \
	$(CIO)/ttx.c \
	$(CIO)/tt$(TTY).c \
	$(CIO)/tty.c

PMLSRC=	$(PML)/local.s

POSSRC=	$(COS)/acct.c \
	$(POS)/bio.c \
	$(POS)/bitmasks.c \
	$(POS)/blklist.c \
	$(POS)/clock.c  \
	$(COS)/ctype.c  \
	$(POS)/exec.c \
	$(POS)/exit.c \
	$(POS)/fault.c \
	$(COS)/flock.c \
	$(POS)/fork.c \
	$(POS)/getpages.c \
	$(POS)/grow.c \
	$(COS)/heap_kmem.c \
	$(POS)/init.c \
	$(POS)/intrmsg.c \
	$(POS)/ipc.c \
	$(COS)/kern_softint.c \
	$(COS)/kern_time.c \
	$(POS)/lock.c \
	$(OS)/main.c \
	$(POS)/malloc.c \
	$(POS)/meminit.c \
	$(POS)/msg.c \
	$(LIBC)/nfsstrncpy.c \
	$(POS)/page.c \
	$(POS)/physio.c \
	$(POS)/prf.c \
	$(POS)/probe.c \
	$(LIBC)/qsort.c \
	$(POS)/region.c \
	$(POS)/sched.c \
	$(POS)/sem.c \
	$(POS)/shm.c \
	$(POS)/sig.c \
	$(POS)/slp.c \
	$(COS)/stream.c \
	$(COS)/streamio.c \
	$(LIBC)/strncpy.c \
	$(COS)/subr.c \
	$(POS)/swapalloc.c \
	$(POS)/swtch.c \
	$(POS)/sys1.c \
	$(COS)/sys2.c \
	$(COS)/sys3.c \
	$(POS)/sys4.c \
	$(COS)/sysent.c \
	$(POS)/syslocal.c \
	$(POS)/sysm68k.c \
	$(POS)/trap.c \
	$(POS)/text.c \
	$(POS)/uerrlog.c \
	$(POS)/ulocking.c \
	$(POS)/umove.c \
	$(POS)/usys4.c \
	$(POS)/usyslocal.c \
	$(COS)/vfs.c \
	$(COS)/vfs_bio.c \
	$(COS)/vfs_dev.c \
	$(COS)/vfs_dnlc.c \
	$(COS)/vfs_io.c \
	$(COS)/vfs_lookup.c \
	$(COS)/vfs_pathname.c \
	$(COS)/vfs_syscalls.c \
	$(COS)/vfs_vnode.c \
	$(COS)/vfs_xxx.c

NETSRC=	$(NET)/af.c		\
	$(NET)/bnetinit.c	\
	$(NET)/domain.c		\
	$(NET)/if.c		\
	$(NET)/in_cksum.c	\
	$(NET)/netipc.c		\
	$(NET)/raw_cb.c		\
	$(NET)/raw_usrreq.c	\
	$(NET)/route.c		\
	$(NET)/socket.c		\
	$(NET)/socket2.c	\
	$(NET)/sys_socket.c	\
	$(NET)/uipc_mbuf.c	\
	$(NET)/uipc_proto.c	\
	$(NET)/uipc_usrreq.c	\
	$(NETinet)/if_ether.c	\
	$(NETinet)/if_loop.c	\
	$(NETinet)/in.c		\
	$(NETinet)/in_pcb.c	\
	$(NETinet)/in_proto.c	\
	$(NETinet)/ip_icmp.c	\
	$(NETinet)/ip_input.c	\
	$(NETinet)/ip_output.c	\
	$(NETinet)/raw_ip.c	\
	$(NETinet)/tcp_debug.c	\
	$(NETinet)/tcp_input.c	\
	$(NETinet)/tcp_output.c	\
	$(NETinet)/tcp_subr.c	\
	$(NETinet)/tcp_timer.c	\
	$(NETinet)/tcp_usrreq.c	\
	$(NETinet)/udp_usrreq.c

NFSSRC= $(NFS)/nfs_server.c	\
	$(NFS)/nfs_subr.c	\
	$(NFS)/nfs_vfsops.c	\
	$(NFS)/nfs_vnodeops.c	\
	$(NFS)/nfs_xdr.c

RPCSRC=	$(RPC)/auth_kern.c	\
	$(RPC)/auth_none.c	\
	$(RPC)/auth_uprot.c	\
	$(RPC)/clnt_kudp.c	\
	$(RPC)/kudp_fsend.c	\
	$(RPC)/rpc_prot.c	\
	$(RPC)/subr_kudp.c	\
	$(RPC)/svc.c		\
	$(RPC)/svc_auth.c	\
	$(RPC)/svc_auth_u.c	\
	$(RPC)/svc_kudp.c	\
	$(RPC)/xdr.c		\
	$(RPC)/xdr_array.c	\
	$(RPC)/xdr_mbuf.c	\
	$(RPC)/xdr_mem.c	\
	$(RPC)/xdr_ref.c

SVFSSRC=$(SVFS)/5_alloc.c	\
	$(SVFS)/5_bmap.c	\
	$(SVFS)/5_dir.c	\
	$(SVFS)/5_dsort.c	\
	$(SVFS)/5_inode.c	\
	$(SVFS)/5_pipe.c	\
	$(SVFS)/5_subr.c	\
	$(SVFS)/5_utssys.c	\
	$(OS)/5_vfsops.c	\
	$(SVFS)/5_vnodeops.c

TOOLBOXSRC=uinter.c		\
	uinter.s

CFILES=	$(DEVSRC)		\
	$(MACHDEPSRC)		\
	$(PCFSRC)		\
	$(PIOSRC)		\
	$(PMLSRC)		\
	$(POSSRC)		\
	$(NETSRC)		\
	$(SVFSSRC)		\
	$(NFSSRC)		\
	$(RPCSRC)

SINCSRC=				\
	$(SINC)/errno.h			\
	$(SINC)/fcntl.h			\
	$(SINC)/setjmp.h

SINCNET=				\
	$(SINC)/net/af.h		\
	$(SINC)/net/if.h		\
	$(SINC)/net/netisr.h		\
	$(SINC)/net/raw_cb.h		\
	$(SINC)/net/route.h		\
	$(SINC)/netinet/icmp_var.h	\
	$(SINC)/netinet/if_ether.h	\
	$(SINC)/netinet/in.h		\
	$(SINC)/netinet/in_pcb.h	\
	$(SINC)/netinet/in_systm.h	\
	$(SINC)/netinet/in_systm.h	\
	$(SINC)/netinet/ip.h		\
	$(SINC)/netinet/ip_icmp.h	\
	$(SINC)/netinet/ip_var.h	\
	$(SINC)/netinet/tcp.h		\
	$(SINC)/netinet/tcp_debug.h	\
	$(SINC)/netinet/tcp_fsm.h	\
	$(SINC)/netinet/tcp_seq.h	\
	$(SINC)/netinet/tcp_timer.h	\
	$(SINC)/netinet/tcp_var.h	\
	$(SINC)/netinet/tcpip.h		\
	$(SINC)/netinet/udp.h		\
	$(SINC)/netinet/udp_var.h	\
	$(SINC)/netpup/pup.h

SINCNFS=				\
	$(SINC)/nfs/nfs.h		\
	$(SINC)/nfs/nfs_clnt.h		\
	$(SINC)/nfs/rnode.h

SINCRPC=				\
	$(SINC)/rpc/auth.h		\
	$(SINC)/rpc/auth_unix.h		\
	$(SINC)/rpc/clnt.h		\
	$(SINC)/rpc/pmap_clnt.h		\
	$(SINC)/rpc/pmap_prot.h		\
	$(SINC)/rpc/rpc.h		\
	$(SINC)/rpc/rpc_msg.h		\
	$(SINC)/rpc/svc.h		\
	$(SINC)/rpc/svc_auth.h		\
	$(SINC)/rpc/types.h		\
	$(SINC)/rpc/xdr.h

SINCSVFS=				\
	$(SINC)/svfs/fsdir.h		\
	$(SINC)/svfs/fblk.h		\
	$(SINC)/svfs/filsys.h		\
	$(SINC)/svfs/inode.h		\
	$(SINC)/svfs/mount.h

SINCSYS=				\
	$(SINC)/sys/bitmasks.h		\
	$(SINC)/sys/buserr.h		\
	$(SINC)/sys/conf.h		\
	$(SINC)/sys/debug.h		\
	$(SINC)/sys/fpioctl.h		\
	$(SINC)/sys/fptrap.h		\
	$(SINC)/sys/getpages.h		\
	$(SINC)/sys/ivec.h		\
	$(SINC)/sys/map.h		\
	$(SINC)/sys/msg.h		\
	$(SINC)/sys/page.h		\
	$(SINC)/sys/pfdat.h		\
	$(SINC)/sys/pmmu.h		\
	$(SINC)/sys/proc.h		\
	$(SINC)/sys/region.h		\
	$(SINC)/sys/seg.h		\
	$(SINC)/sys/sem.h		\
	$(SINC)/sys/shm.h		\
	$(SINC)/sys/swap.h		\
	$(SINC)/sys/sysent.h		\
	$(SINC)/sys/sysinfo.h		\
	$(SINC)/sys/sysmacros.h		\
	$(SINC)/sys/systm.h		\
	$(SINC)/sys/tty.h		\
	$(SINC)/sys/tuneable.h		\
	$(SINC)/sys/types.h		\
	$(SINC)/sys/user.h		\
	$(SINC)/sys/var.h		\
	$(SINC)/sys/vnode.h
SINCSYSa=				\
	$(SINC)/sys/acct.h		\
	$(SINC)/sys/altblk.h		\
	$(SINC)/sys/buf.h		\
	$(SINC)/sys/callout.h		\
	$(SINC)/sys/cpuid.h		\
	$(SINC)/sys/dir.h		\
	$(SINC)/sys/diskformat.h	\
	$(SINC)/sys/disktune.h		\
	$(SINC)/sys/dnlc.h		\
	$(SINC)/sys/domain.h		\
	$(SINC)/sys/elog.h		\
	$(SINC)/sys/erec.h		\
	$(SINC)/sys/err.h		\
	$(SINC)/sys/errno.h		\
	$(SINC)/sys/file.h		\
	$(SINC)/sys/flock.h		\
	$(SINC)/sys/iobuf.h		\
	$(SINC)/sys/ioctl.h		\
	$(SINC)/sys/ipc.h		\
	$(SINC)/sys/lock.h		\
	$(SINC)/sys/locking.h		\
	$(SINC)/sys/mbuf.h		\
	$(SINC)/sys/pathname.h		\
	$(SINC)/sys/protosw.h

SINCSYSb=				\
	$(SINC)/sys/psl.h		\
	$(SINC)/sys/reg.h		\
	$(SINC)/sys/seg.h		\
	$(SINC)/sys/signal.h		\
	$(SINC)/sys/socket.h		\
	$(SINC)/sys/socketvar.h		\
	$(SINC)/sys/stat.h		\
	$(SINC)/sys/sxt.h		\
	$(SINC)/sys/termio.h		\
	$(SINC)/sys/text.h		\
	$(SINC)/sys/time.h		\
	$(SINC)/sys/trace.h		\
	$(SINC)/sys/trap.h		\
	$(SINC)/sys/ttold.h		\
	$(SINC)/sys/uio.h		\
	$(SINC)/sys/uioctl.h		\
	$(SINC)/sys/utsname.h		\
	$(SINC)/sys/vfs.h		\
	$(SINC)/sys/wait.h

SINCVAX=				\
	$(SINC)/vax/vaxque.h		\
	$(SINC)/vaxuba/ubavar.h

SINCSRC=			\
	$(CF)/bnetconfig.h	\
	$(CF)/config.h		\
	$(SINC)/sys/clock.h	\
	$(SINC)/sys/uconfig.h	\
	$(SINC)/sys/useg.h	\
	$(SINC)/sys/param.h	\
	$(SINC)/sys/space.h


HFILES=	$(SINCSRC)		\
	$(SINCNET)		\
	$(SINCNFS)		\
	$(SINCRPC)		\
	$(SINCSVFS)		\
	$(SINCSYS)		\
	$(SINCSYSa)		\
	$(SINCSYSb)		\
	$(SINCVAX)		\
	$(SINCSRC)

CONFIGOBJ=			\
	nfsconf.o		\
	bnetivec.o		\
	vfs_conf.o

DEVOBJ=	tty_pty.o		\
	hd.o			\
	gdisk.o			\
	gdisksubr.o		\
	sdisk.o			\
	nvram.o			\
	sccio.o			\
	via.o			\
	scsi.o			\
	sony.o			\
	fmt.o			\
	mst.o			\
	rdhead.o		\
	snrd.o			\
	screen.o		\
	linea.o			\
	debug_data.o		\
	debug.o		

MACHDEPOBJ= machdep.o 	\
	    cmachdep.o	\
	    nmachdep.o	

#
#	NOTE: the order within  the following file list is important
#		*mch.o MUST be first

PSTARTOBJ = \
		nfsmch.o	\
		clear.o		\
		local.o		\
		startup.o	\
		bnetuconf.o	\
		umachdep.o

PCFOBJ=	name.o

PIOOBJ=	clist.o \
	clone.o \
	err.o \
	fp.o \
	line.o \
	mem.o \
	osm.o \
	partab.o \
	prof.o \
	shl.o \
	shlr.o \
	sxt.o \
	sys.o \
	ttx.o \
	tt$(TTY).o \
	tty.o

PMLOBJ=	

POSOBJ=	acct.o \
	bio.o \
	bitmasks.o \
	blklist.o \
	clock.o  \
	ctype.o  \
	exec.o \
	exit.o \
	fault.o \
	flock.o \
	fork.o \
	getpages.o \
	grow.o \
	heap_kmem.o \
	init.o \
	intrmsg.o \
	ipc.o \
	kern_softint.o \
	kern_time.o \
	lock.o \
	main.o \
	malloc.o \
	meminit.o \
	msg.o \
	nfsstrncpy.o \
	page.o \
	physio.o \
	prf.o \
	probe.o \
	qsort.o \
	region.o \
	sched.o \
	sem.o \
	shm.o \
	sig.o \
	slp.o \
	stream.o \
	streamio.o \
	strncpy.o \
	subr.o \
	swapalloc.o \
	swtch.o \
	sys1.o \
	sys2.o \
	sys3.o \
	sys4.o \
	sysent.o \
	syslocal.o \
	sysm68k.o \
	trap.o \
	text.o \
	uerrlog.o \
	ulocking.o \
	umove.o \
	usys4.o \
	usyslocal.o \
	vfs.o \
	vfs_bio.o \
	vfs_dev.o \
	vfs_dnlc.o \
	vfs_io.o \
	vfs_lookup.o \
	vfs_pathname.o \
	vfs_syscalls.o \
	vfs_vnode.o \
	vfs_xxx.o

NETOBJ=	af.o			\
	bnetinit.o		\
	domain.o		\
	if.o			\
	in_cksum.o		\
	netipc.o		\
	raw_cb.o		\
	raw_usrreq.o		\
	route.o			\
	socket.o		\
	socket2.o		\
	sys_socket.o		\
	uipc_mbuf.o		\
	uipc_proto.o		\
	uipc_usrreq.o		\
	if_ether.o		\
	if_loop.o		\
	in.o			\
	in_pcb.o		\
	in_proto.o		\
	ip_icmp.o		\
	ip_input.o		\
	ip_output.o		\
	raw_ip.o		\
	tcp_debug.o		\
	tcp_input.o		\
	tcp_output.o		\
	tcp_subr.o		\
	tcp_timer.o		\
	tcp_usrreq.o		\
	udp_usrreq.o

NFSOBJ= nfs_server.o		\
	nfs_subr.o		\
	nfs_vfsops.o		\
	nfs_vnodeops.o		\
	nfs_xdr.o

RPCOBJ=	auth_kern.o		\
	auth_none.o		\
	auth_uprot.o		\
	clnt_kudp.o		\
	kudp_fsend.o		\
	rpc_prot.o		\
	subr_kudp.o		\
	svc.o			\
	svc_auth.o		\
	svc_auth_u.o		\
	svc_kudp.o		\
	xdr.o			\
	xdr_array.o		\
	xdr_mbuf.o		\
	xdr_mem.o		\
	xdr_ref.o

SVFSOBJ=5_alloc.o		\
	5_bmap.o		\
	5_dir.o			\
	5_dsort.o		\
	5_inode.o		\
	5_pipe.o		\
	5_subr.o		\
	5_utssys.o		\
	5_vfsops.o		\
	5_vnodeops.o

LIBCOBJ=blt.o			\
	blt512.o		\
	ldiv.o			\
	lmul.o			\
	qsort.o			\
	setjmp.o		\
	strcmp.o		\
	strlen.o		\
	strncmp.o		\
	nfsstrncpy.o

TOOLBOXOBJ=uinter.o		\
	uinters.o

AUTOCONFIGOBJ=		
BNETAUTOCONFIGOBJ=	boot.d/ae6 boot.d/bnet
NFSAUTOCONFIGOBJ=	boot.d/ae6 boot.d/bnet boot.d/nfs
TBAUTOCONFIGOBJ=	boot.d/toolbox

KERNEL=	$(MACHDEPOBJ) $(PIOOBJ) $(PMLOBJ) $(POSOBJ) $(LIBCOBJ)

all:		nfsunix bnetunix nonetunix

#tags:		$(CFILES)	\
#		$(SINCSRC)	\
#		$(SINCNET)	\
#		$(SINCSVFS)	\
#		$(SINCSYS)	\
#		$(SINCSYSa)	\
#		$(SINCSYSb)	\
#		$(SINCVAX)	\
#		$(SINCSRC)
tags:		/tmp
		make ctags
		make htags
		sort +0 tags -o tags

ctags:
		ctags -w $(CONFIGSRC) $(CFILES)
		ctags -a $(SINCSRC)
		ctags -a $(SINCNET)
		ctags -a $(SINCNFS)
		ctags -a $(SINCRPC)
		ctags -a $(SINCSVFS)
		ctags -a $(SINCSYS)
		ctags -a $(SINCSYSa)
		ctags -a $(SINCSYSb)
		ctags -a $(SINCVAX)
		ctags -a $(SINCSRC)

htags:
		make tagstruct TAGSTRUCT="$(SINCSRC)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null
		make tagstruct TAGSTRUCT="$(SINCNET)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null
		make tagstruct TAGSTRUCT="$(SINCNFS)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null
		make tagstruct TAGSTRUCT="$(SINCRPC)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null
		make tagstruct TAGSTRUCT="$(SINCSVFS)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null
		make tagstruct TAGSTRUCT="$(SINCSYS)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null
		make tagstruct TAGSTRUCT="$(SINCSYSa)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null
		make tagstruct TAGSTRUCT="$(SINCSYSb)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null
		make tagstruct TAGSTRUCT="$(SINCVAX)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null
		make tagstruct TAGSTRUCT="$(SINCSRC)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null

tagstruct:
		./:sed $(TAGSTRUCT)

newunix:	init.ld			\
		unix.o			\
		svfs.o			\
		pstart.o		\
		$(CONFIGOBJ)		\
		$(DEVOBJ)		\
		$(PCFOBJ)		\
		module.o
#		cmc
		@echo "	loading $@"
		@$(LD) -o $@ $(LFLAGS)	\
			pstart.o 		\
			$(CONFIGOBJ)		\
			$(PCFOBJ)		\
			$(DEVOBJ)		\
			unix.o			\
			svfs.o			\
			module.o		\
			-lslots
		$(SROOT)/usr/bin/patch_kernel $@ 
		$(FIX) $@

nonetunix:	newunix			\
		$(AUTOCONFIGOBJ)	\
		$(TBAUTOCONFIGOBJ)
		$(AUTOCONFIG) -u		\
			-i newunix		\
			-o nonetunix		\
			-m master.d		\
			-b boot.d		\
			-s startup.d		\
			-d devinit.d		\
			-l $(LD)
		$(FIX) $@

bnetunix:	newunix			\
		$(BNETAUTOCONFIGOBJ)	\
		$(TBAUTOCONFIGOBJ)
		$(AUTOCONFIG) -u		\
			-i newunix		\
			-o bnetunix		\
			-m bnetmaster.d		\
			-b boot.d		\
			-s startup.d		\
			-d devinit.d		\
			-L loadfile		\
			-l $(LD)
		$(FIX) $@

nfsunix:	newunix			\
		$(NFSAUTOCONFIGOBJ)	\
		$(TBAUTOCONFIGOBJ)
		$(AUTOCONFIG) -u		\
			-i newunix		\
			-o nfsunix		\
			-m nfsmaster.d		\
			-b boot.d		\
			-s startup.d		\
			-d devinit.d		\
			-L loadfile		\
			-l $(LD)
		$(FIX) $@


unix.o:		$(KERNEL)
		@echo "	loading $@"
		@$(LD) -o unix.o -r $(KERNEL)
 
svfs.o:		$(SVFSOBJ)
		@echo "	loading $@"
		@$(LD) -o svfs.o -r $(SVFSOBJ)
 
bnet.o:		$(NETOBJ)
		@echo "	loading $@"
		@$(LD) -o bnet.o -r $(NETOBJ)
 
nfs.o:		$(NFSOBJ) $(RPCOBJ)
		@echo "	loading $@"
		@$(LD) -o nfs.o -r $(NFSOBJ) $(RPCOBJ)

toolbox.o:	$(TOOLBOXOBJ)
		@echo " loading $@"
		@$(LD) -o toolbox.o -r $(TOOLBOXOBJ)

pstart.o:	$(PSTARTOBJ)
		$(LD) -r -o pstart.o $(PSTARTOBJ)
 
module.o:	
		/usr/bin/module_data -f module.o $(MODULES)
 
init.ld:	$(COS)/geninit.c
		cc -DPFILE=\"pstart.o\" $(MMU) $(MEMDEFS) $(INCLUDE) $(COS)/geninit.c; ./a.out > init.ld; rm -f a.out

$(REFRESH).o:	$(REFRESH).s
		$(AS) -o $(REFRESH).o $(REFRESH).s

$(PML)/nfsmch.s:	mch.h
nfsmch.o:	$(PML)/nfsmch.s $(SINC)/sys/param.h $(SINC)/sys/page.h \
	$(SINC)/sys/uconfig.h $(SINC)/sys/sysmacros.h \
	$(SINC)/sys/mmu.h $(SINC)/sys/seg.h $(SINC)/sys/useg.h
	-$(CPP) -P $(DEFS) $(INCLUDE) $(PML)/nfsmch.s > nfsmch.i
		$(AS) -o nfsmch.o nfsmch.i
		rm -f nfsmch.i

bnetivec.o:	$(ML)/bnetivec.s mch.h \
			$(SINC)/sys/param.h $(SINC)/sys/page.h \
			$(SINC)/sys/uconfig.h $(SINC)/sys/sysmacros.h \
			$(SINC)/sys/mmu.h $(SINC)/sys/seg.h $(SINC)/sys/useg.h
		$(CPP) -P $(DEFS) $(INCLUDE) $(ML)/bnetivec.s > bnetivec.i
		$(AS) -o bnetivec.o bnetivec.i
		rm -f bnetivec.i

mch.h:		$(COS)/genassym.c $(SINC)/sys/param.h $(SINC)/sys/types.h \
 		$(SINC)/sys/sysinfo.h $(SINC)/sys/mmu.h $(SINC)/sys/time.h \
 		$(SINC)/sys/page.h $(SINC)/sys/seg.h $(SINC)/sys/user.h \
 		$(SINC)/sys/reg.h
		cc $(DEFS) $(INCLUDE) $(COS)/genassym.c; ./a.out > mch.h; rm -f a.out


boot.d/ae6:	if_ae6.o
		cp if_ae6.o boot.d/ae6

boot.d/bnet:	bnet.o
		cp bnet.o boot.d/BNET

boot.d/nfs:	nfs.o
		cp nfs.o boot.d/nfs

boot.d/toolbox:	toolbox.o
		cp toolbox.o boot.d/toolbox

prf.o:		$(POS)/prf.c
		$(CC) $(CFLAGS) -c $(POS)/prf.c

lint:
		@date
		@echo Linting $(S) ...
		@$(LINT) $(LINTFLAGS) $(DEFS) $(INCLUDE) -Ilinclude \
			$(PCF)/Locore.c $(CF)/Locoreu.c $(CFILES)
		@echo End linting
lintnonet:
		@date
		@echo Linting Nonet $(S) ...
		@$(LINT) $(LINTFLAGS) $(DEFS) $(INCLUDE) -Ilinclude \
			$(PCF)/Locore.c $(CF)/Locoreu.c $(CNONETFILES)
		@echo End linting
		
#CF
nfsconf.o:	$(CF)/nfsconf.c $(SINC)/sys/space.h $(CF)/bnetconfig.h
	$(CC) $(CFLAGS) -c $(CF)/nfsconf.c
bnetuconf.o:	$(CF)/bnetuconf.c
	$(CC) $(CNOFLAGS) -c $(CF)/bnetuconf.c

#IO
ram.o:	$(IO)/ram.c
	$(CC) $(CFLAGS) -c $(IO)/ram.c
fp.o:		$(PIO)/fp.c
	$(CC) $(CNOFLAGS) -c $(PIO)/fp.c
debug_data.o:	$(IO)/debug_data.c
	$(CC) $(CNOFLAGS) -c $(IO)/debug_data.c
debug.o:	$(IO)/debug.c
	$(CC) $(CNOFLAGS) -c $(IO)/debug.c
if_ae6.o:	$(IO)/if_ae6.c
	$(CC) $(CNOFLAGS) -c $(IO)/if_ae6.c
nvram.o:	$(IO)/nvram.c
	$(CC) $(CNOFLAGS) -c $(IO)/nvram.c
hd.o:	io/hd.c
	$(CC) $(CFLAGS) -c io/hd.c
gdisk.o: io/gdisk.c
	$(CC) $(CFLAGS) -c io/gdisk.c
gdisksubr.o:	io/gdisksubr.c
	$(CC) $(CFLAGS) -c io/gdisksubr.c
sdisk.o:	io/sdisk.c
	$(CC) $(CFLAGS) -c io/sdisk.c
sccio.o:	$(IO)/sccio.c
	$(CC) $(CNOFLAGS) -c $(IO)/sccio.c
via.o:	$(IO)/via.c
	$(CC) $(CNOFLAGS) -c $(IO)/via.c
scsi.o:	$(IO)/scsi.c
	$(CC) $(CNOFLAGS) -c $(IO)/scsi.c
sony.o:	$(IO)/sony.c
	$(CC) $(CNOFLAGS) -c $(IO)/sony.c
fmt.o:	$(IO)/fmt.s $(SINC)/sys/uconfig.h 
		$(CPP) -P $(DEFS) $(INCLUDE) $(IO)/fmt.s > fmt.i
		$(AS) -o fmt.o fmt.i
		rm -f fmt.i
mst.o:	$(IO)/mst.s $(SINC)/sys/uconfig.h 
		$(CPP) -P $(DEFS) $(INCLUDE) $(IO)/mst.s > mst.i
		$(AS) -o mst.o mst.i
		rm -f mst.i
rdhead.o: $(IO)/rdhead.s $(SINC)/sys/uconfig.h 
		$(CPP) -P $(DEFS) $(INCLUDE) $(IO)/rdhead.s > rdhead.i
		$(AS) -o rdhead.o rdhead.i
		rm -f rdhead.i
snrd.o:	$(IO)/snrd.s $(SINC)/sys/uconfig.h 
		$(CPP) -P $(DEFS) $(INCLUDE) $(IO)/snrd.s > snrd.i
		$(AS) -o snrd.o snrd.i
		rm -f snrd.i
linea.o:$(IO)/linea.c
	$(CC) $(CNOFLAGS) -c $(IO)/linea.c

#IO - screen stuff

SCREEN	=  \
	fdb.o \
	fdbs.o \
	font.o \
	features.o \
	fontdef.o \
	font1.o \
	font2.o \
	key.o \
	keyboard.o \
	mouse.o \
	video.o \
	vt100.o \
	sound.o

screen.o:	$(SCREEN)
	$(LD) -r -o screen.o $(SCREEN)

fdb.o:	$(IO)/fdb.c
	$(CC) $(CFLAGS) -c $(IO)/fdb.c

mouse.o:$(IO)/mouse.c
	$(CC) $(CFLAGS) -c $(IO)/mouse.c

fontdef.o:$(IO)/fontdef.c
	$(CC) $(CFLAGS) -c $(IO)/fontdef.c

features.o: features.c
	$(CC) $(CFLAGS) -c features.c

features.c:	mk_feature
	./mk_feature	>features.c

mk_feature:$(IO)/mk_feature.c $(IO)/screen_data.c
	cc -o mk_feature $(IO)/mk_feature.c $(IO)/screen_data.c

font.o:$(IO)/font.c
	$(CC) $(CFLAGS) -c $(IO)/font.c

vt100.o:$(IO)/vt100.c
	$(CC) $(CFLAGS) -c $(IO)/vt100.c

sound.o:$(IO)/sound.s
	$(AS) -o sound.o $(IO)/sound.s

video.o:$(IO)/video.c
	$(CC) $(CFLAGS) -c $(IO)/video.c

key.o:	$(IO)/key.c
	$(CC) $(CFLAGS) -c $(IO)/key.c

keyboard.o:	$(IO)/keyboard.c
	$(CC) $(CFLAGS) -c $(IO)/keyboard.c

font1.o:	$(IO)/font1.s
	-$(CPP) -P $(DEFS) $(INCLUDE) $(IO)/font1.s > $(IO)/font1.i
	$(AS) -o font1.o $(IO)/font1.i
	rm -f $(IO)/font1.i

font2.o:	$(IO)/font2.s
	-$(CPP) -P $(DEFS) $(INCLUDE) $(IO)/font2.s > $(IO)/font2.i
	$(AS) -o font2.o $(IO)/font2.i
	rm -f $(IO)/font2.i

fdbs.o:	$(IO)/fdbs.s
	-$(CPP) -P $(DEFS) $(INCLUDE) $(IO)/fdbs.s > $(IO)/fdbs.i
	$(AS) -o fdbs.o $(IO)/fdbs.i
	rm -f $(IO)/fdbs.i

#OS
startup.o:	$(OS)/startup.c
	$(CC) $(CFLAGS) -c $(OS)/startup.c

#PCF
name.o:	$(CFILES) $(MAKEFILE)
	$(CC) \
	$(INCLUDE) -c \
		-DSYS=\"`expr $(SYSTEM) : '\(.\{1,8\}\)'`\" \
		-DNODE=\"`expr $(NODE) : '\(.\{1,8\}\)'`\" \
		-DREL=\"`expr $(REL) : '\(.\{1,8\}\)'`\" \
		-DVER=\"`expr $(VER) : '\(.\{1,8\}\)'`\" \
		-DMACH=\"`expr $(MACH) : '\(.\{1,8\}\)'`\" \
		-DTIMESTAMP="\"`date`\"" \
		$(PCF)/name.c

#PIO
clist.o:	$(PIO)/clist.c
	$(CC) $(CFLAGS) -c $(PIO)/clist.c
clone.o:	$(CIO)/clone.c
	$(CC) $(CFLAGS) -c $(CIO)/clone.c
err.o:	$(CIO)/err.c
	$(CC) $(CFLAGS) -c $(CIO)/err.c
line.o:	$(CIO)/line.c
	$(CC) $(CFLAGS) -c $(CIO)/line.c
mem.o:	$(PIO)/mem.c
	$(CC) $(CFLAGS) -c $(PIO)/mem.c
osm.o:	$(PIO)/osm.c
	$(CC) $(CFLAGS) -c $(PIO)/osm.c
partab.o:	$(CIO)/partab.c
	$(CC) $(CFLAGS) -c $(CIO)/partab.c
prof.o:	$(PIO)/prof.c
	$(CC) $(CFLAGS) -c $(PIO)/prof.c
shl.o:	$(CIO)/shl.c
	$(CC) $(CFLAGS) -c $(CIO)/shl.c
shlr.o:	$(CIO)/shlr.c
	$(CC) $(CFLAGS) -c $(CIO)/shlr.c
sxt.o:	$(CIO)/sxt.c
	$(CC) $(CFLAGS) -c $(CIO)/sxt.c
sys.o:	$(CIO)/sys.c
	$(CC) $(CFLAGS) -c $(CIO)/sys.c
ttx.o:	$(CIO)/ttx.c
	$(CC) $(CFLAGS) -c $(CIO)/ttx.c
tt$(TTY).o:	$(CIO)/tt$(TTY).c
	$(CC) $(CFLAGS) -c $(CIO)/tt$(TTY).c
tty.o:	$(CIO)/tty.c 
	$(CC) $(CFLAGS) -c $(CIO)/tty.c

#PML
local.o:$(PML)/local.s $(SINC)/sys/param.h
	-$(CPP) -P $(DEFS) $(INCLUDE) $(PML)/local.s > local.i
	$(AS) -o local.o local.i
	rm -f local.i

#NET
af.o:		$(NET)/af.c
		$(CC) $(CFLAGS) -c $(NET)/af.c
bnetinit.o:	$(NET)/bnetinit.c
		$(CC) $(CFLAGS) -c $(NET)/bnetinit.c
if_loop.o:	$(NETinet)/if_loop.c
		$(CC) $(CFLAGS) -c $(NETinet)/if_loop.c
if_ether.o:	$(NETinet)/if_ether.c
		$(CC) $(CFLAGS) -c $(NETinet)/if_ether.c
if.o:		$(NET)/if.c
		$(CC) $(CFLAGS) -c $(NET)/if.c
in.o:		$(NETinet)/in.c
		$(CC) $(CFLAGS) -c $(NETinet)/in.c
in_cksum.o:	$(NET)/in_cksum.c
		$(CC) $(CFLAGS) -c $(NET)/in_cksum.c
in_pcb.o:	$(NETinet)/in_pcb.c
		$(CC) $(CFLAGS) -c $(NETinet)/in_pcb.c
ip_icmp.o:	$(NETinet)/ip_icmp.c
		$(CC) $(CFLAGS) -c $(NETinet)/ip_icmp.c
ip_input.o:	$(NETinet)/ip_input.c
		$(CC) $(CFLAGS) -c $(NETinet)/ip_input.c
ip_output.o:	$(NETinet)/ip_output.c
		$(CC) $(CFLAGS) -c $(NETinet)/ip_output.c
raw_cb.o:	$(NET)/raw_cb.c
		$(CC) $(CFLAGS) -c $(NET)/raw_cb.c
raw_ip.o:	$(NETinet)/raw_ip.c
		$(CC) $(CFLAGS) -c $(NETinet)/raw_ip.c
raw_usrreq.o:	$(NET)/raw_usrreq.c
		$(CC) $(CFLAGS) -c $(NET)/raw_usrreq.c
route.o:	$(NET)/route.c
		$(CC) $(CFLAGS) -c $(NET)/route.c
tcp_debug.o:	$(NETinet)/tcp_debug.c
		$(CC) $(CFLAGS) -c $(NETinet)/tcp_debug.c
tcp_input.o:	$(NETinet)/tcp_input.c
		$(CC) $(CFLAGS) -c $(NETinet)/tcp_input.c
tcp_output.o:	$(NETinet)/tcp_output.c
		$(CC) $(CFLAGS) -c $(NETinet)/tcp_output.c
tcp_subr.o:	$(NETinet)/tcp_subr.c
		$(CC) $(CFLAGS) -c $(NETinet)/tcp_subr.c
tcp_timer.o:	$(NETinet)/tcp_timer.c
		$(CC) $(CFLAGS) -c $(NETinet)/tcp_timer.c
tcp_usrreq.o:	$(NETinet)/tcp_usrreq.c
		$(CC) $(CFLAGS) -c $(NETinet)/tcp_usrreq.c
udp_usrreq.o:	$(NETinet)/udp_usrreq.c
		$(CC) $(CFLAGS) -c $(NETinet)/udp_usrreq.c
uipc_mbuf.o:	$(NET)/uipc_mbuf.c
		$(CC) $(CFLAGS) -c $(NET)/uipc_mbuf.c
uipc_proto.o:	$(NET)/uipc_proto.c
		$(CC) $(CFLAGS) -c $(NET)/uipc_proto.c
uipc_usrreq.o:	$(NET)/uipc_usrreq.c
		$(CC) $(CFLAGS) -c $(NET)/uipc_usrreq.c
tty_pty.o:	$(NET)/tty_pty.c
		$(CC) $(CFLAGS) -c $(NET)/tty_pty.c
netipc.o:	$(NET)/netipc.c
		$(CC) $(CFLAGS) -c $(NET)/netipc.c
nmachdep.o:	$(NET)/nmachdep.c
		$(CC) $(CFLAGS) -c $(NET)/nmachdep.c
socket.o:	$(NET)/socket.c
		$(CC) $(CFLAGS) -c $(NET)/socket.c
socket2.o:	$(NET)/socket2.c
		$(CC) $(CFLAGS) -c $(NET)/socket2.c
in_proto.o:	$(NETinet)/in_proto.c
		$(CC) $(CFLAGS) -c $(NETinet)/in_proto.c
sys_socket.o:	$(NET)/sys_socket.c
		$(CC) $(CFLAGS) -c $(NET)/sys_socket.c
domain.o:	$(NET)/domain.c
		$(CC) $(CFLAGS) -c $(NET)/domain.c

#NFS
nfs_server.o:	$(NFS)/nfs_server.c
		$(CC) $(CFLAGS) -c $(NFS)/nfs_server.c
nfs_subr.o:	$(NFS)/nfs_subr.c
		$(CC) $(CFLAGS) -c $(NFS)/nfs_subr.c
nfs_vfsops.o:	$(NFS)/nfs_vfsops.c
		$(CC) $(CFLAGS) -c $(NFS)/nfs_vfsops.c
nfs_vnodeops.o:	$(NFS)/nfs_vnodeops.c
		$(CC) $(CFLAGS) -c $(NFS)/nfs_vnodeops.c
nfs_xdr.o:	$(NFS)/nfs_xdr.c
		$(CC) $(CFLAGS) -c $(NFS)/nfs_xdr.c

#RPC
auth_kern.o:	$(RPC)/auth_kern.c
		$(CC) $(CFLAGS) -c $(RPC)/auth_kern.c
auth_none.o:	$(RPC)/auth_none.c
		$(CC) $(CFLAGS) -c $(RPC)/auth_none.c
auth_uprot.o:	$(RPC)/auth_uprot.c
		$(CC) $(CFLAGS) -c $(RPC)/auth_uprot.c
clnt_kudp.o:	$(RPC)/clnt_kudp.c
		$(CC) $(CFLAGS) -c $(RPC)/clnt_kudp.c
kudp_fsend.o:	$(RPC)/kudp_fsend.c
		$(CC) $(CFLAGS) -c $(RPC)/kudp_fsend.c
rpc_prot.o:	$(RPC)/rpc_prot.c
		$(CC) $(CFLAGS) -c $(RPC)/rpc_prot.c
subr_kudp.o:	$(RPC)/subr_kudp.c
		$(CC) $(CFLAGS) -c $(RPC)/subr_kudp.c
svc.o:		$(RPC)/svc.c
		$(CC) $(CFLAGS) -c $(RPC)/svc.c
svc_auth.o:	$(RPC)/svc_auth.c
		$(CC) $(CFLAGS) -c $(RPC)/svc_auth.c
svc_auth_u.o:	$(RPC)/svc_auth_u.c
		$(CC) $(CFLAGS) -c $(RPC)/svc_auth_u.c
svc_kudp.o:	$(RPC)/svc_kudp.c
		$(CC) $(CFLAGS) -c $(RPC)/svc_kudp.c
xdr.o:		$(RPC)/xdr.c
		$(CC) $(CFLAGS) -c $(RPC)/xdr.c
xdr_array.o:	$(RPC)/xdr_array.c
		$(CC) $(CFLAGS) -c $(RPC)/xdr_array.c
xdr_mbuf.o:	$(RPC)/xdr_mbuf.c
		$(CC) $(CFLAGS) -c $(RPC)/xdr_mbuf.c
xdr_mem.o:	$(RPC)/xdr_mem.c
		$(CC) $(CFLAGS) -c $(RPC)/xdr_mem.c
xdr_ref.o:	$(RPC)/xdr_ref.c
		$(CC) $(CFLAGS) -c $(RPC)/xdr_ref.c

#SVFS
5_alloc.o:	$(SVFS)/5_alloc.c
		$(CC) $(CFLAGS) -c $(SVFS)/5_alloc.c
5_bmap.o:	$(SVFS)/5_bmap.c
		$(CC) $(CFLAGS) -c $(SVFS)/5_bmap.c
5_dir.o:	$(SVFS)/5_dir.c
		$(CC) $(CFLAGS) -c $(SVFS)/5_dir.c
5_dsort.o:	$(SVFS)/5_dsort.c
		$(CC) $(CFLAGS) -c $(SVFS)/5_dsort.c
5_inode.o:	$(SVFS)/5_inode.c
		$(CC) $(CFLAGS) -c $(SVFS)/5_inode.c
5_pipe.o:	$(SVFS)/5_pipe.c
		$(CC) $(CFLAGS) -c $(SVFS)/5_pipe.c
5_subr.o:	$(SVFS)/5_subr.c
		$(CC) $(CFLAGS) -c $(SVFS)/5_subr.c
5_utssys.o:	$(SVFS)/5_utssys.c 
		$(CC) $(CFLAGS) -c $(SVFS)/5_utssys.c
5_vfsops.o:	$(OS)/5_vfsops.c
		$(CC) $(CFLAGS) -c $(OS)/5_vfsops.c
5_vnodeops.o:	$(SVFS)/5_vnodeops.c
		$(CC) $(CFLAGS) -c $(SVFS)/5_vnodeops.c

#TOOLBOX
uinter.o:	$(IO)/uinter.c
		$(CC) $(CNOFLAGS) -c $(IO)/uinter.c
uinters.o:	$(IO)/uinters.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(IO)/uinters.s > uinters.i
		$(AS) -o uinters.o uinters.i
		rm -f uinters.i

nfsclean:
		rm -f			\
		nfs_server.o		\
		nfs_subr.o		\
		nfs_vfsops.o		\
		nfs_vnodeops.o		\
		nfs_xdr.o		\
		auth_kern.o		\
		auth_none.o		\
		auth_uprot.o		\
		clnt_kudp.o		\
		kudp_fsend.o		\
		rpc_prot.o		\
		subr_kudp.o		\
		svc.o			\
		svc_auth.o		\
		svc_auth_u.o		\
		svc_kudp.o		\
		xdr.o			\
		xdr_array.o		\
		xdr_mbuf.o		\
		xdr_mem.o		\
		xdr_ref.o

depend:
	rm -f makedep eddep
	for i in ${NFSCONFIGSRC} ${CFILES};			do	\
		${CC} -E ${CFLAGS} $$i				|	\
		grep '^#'					|	\
		awk '{ print $$3 }'				|	\
		sed -e 's|/[^/]*/\.\.||g' -e 's|"||g'		|	\
		sort -u						|	\
		grep -v "$$i"					|	\
		awk "BEGIN { of = \"`basename $$i .c`\" \".o: \"; rec = of }\
		     { if (length(rec \$$0) > 78) { print rec; rec = of \$$0 }\
		      else rec = rec \" \" \$$0 }			\
		     END { print rec }" >> makedep		;	\
	done
	echo '$$r makedep' >>eddep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >>eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp makefile makefile.bak
	ex - makefile < eddep
	rm eddep makedep

#POS and COS
acct.o:	$(COS)/acct.c
	$(CC) $(CFLAGS) -c $(COS)/acct.c
alloc.o:	$(POS)/alloc.c
	$(CC) $(CFLAGS) -c $(POS)/alloc.c
bio.o:	$(POS)/bio.c
	$(CC) $(CFLAGS) -c $(POS)/bio.c
bitmasks.o:	$(POS)/bitmasks.c
	$(CC) $(CFLAGS) -c $(POS)/bitmasks.c
blklist.o:	$(POS)/blklist.c
	$(CC) $(CFLAGS) -c $(POS)/blklist.c
ccintr.o:	$(POS)/ccintr.c
	$(CC) $(CFLAGS) -c $(POS)/ccintr.c
clock.o:	$(POS)/clock.c
	$(CC) $(CFLAGS) -c $(POS)/clock.c
cmachdep.o:	$(COS)/cmachdep.c
	$(CC) $(CFLAGS) -c $(COS)/cmachdep.c
exec.o:	$(POS)/exec.c
	$(CC) $(CFLAGS) -c $(POS)/exec.c
exit.o:	$(POS)/exit.c
	$(CC) $(CNOFLAGS) -c $(POS)/exit.c
fault.o:	$(POS)/fault.c
	$(CC) $(CFLAGS) -c $(POS)/fault.c
fio.o:	$(POS)/fio.c
	$(CC) $(CFLAGS) -c $(POS)/fio.c
flock.o:	$(COS)/flock.c
	$(CC) $(CFLAGS) -c $(COS)/flock.c
fork.o:	$(POS)/fork.c
	$(CC) $(CFLAGS) -c $(POS)/fork.c
getpages.o:	$(POS)/getpages.c
	$(CC) $(CFLAGS) -c $(POS)/getpages.c
grow.o:	$(POS)/grow.c
	$(CC) $(CFLAGS) -c $(POS)/grow.c
heap_kmem.o:	$(COS)/heap_kmem.c
	$(CC) $(CFLAGS) -c $(COS)/heap_kmem.c
iget.o:	$(POS)/iget.c
	$(CC) $(CFLAGS) -c $(POS)/iget.c
init.o:	$(POS)/init.c
	$(CC) $(CFLAGS) -c $(POS)/init.c
intrmsg.o:	$(POS)/intrmsg.c
	$(CC) $(CFLAGS) -c $(POS)/intrmsg.c
ipc.o:	$(POS)/ipc.c
	$(CC) $(CFLAGS) -c $(POS)/ipc.c
kern_softint.o:	$(COS)/kern_softint.c
	$(CC) $(CFLAGS) -c $(COS)/kern_softint.c
kern_time.o:	$(COS)/kern_time.c
	$(CC) $(CFLAGS) -c $(COS)/kern_time.c
lock.o:	$(POS)/lock.c
	$(CC) $(CFLAGS) -c $(POS)/lock.c
machdep.o:	$(POS)/machdep.c
	$(CC) $(CFLAGS) -c $(POS)/machdep.c
main.o:	$(OS)/main.c
	$(CC) $(CFLAGS) -c $(OS)/main.c
malloc.o:	$(POS)/malloc.c
	$(CC) $(CFLAGS) -c $(POS)/malloc.c
meminit.o:	$(POS)/meminit.c
	$(CC) $(CFLAGS) -c $(POS)/meminit.c
move.o:	$(POS)/move.c
	$(CC) $(CFLAGS) -c $(POS)/move.c
msg.o:	$(POS)/msg.c
	$(CC) $(CFLAGS) -c $(POS)/msg.c
nami.o:	$(POS)/nami.c
	$(CC) $(CFLAGS) -c $(POS)/nami.c
page.o:	$(POS)/page.c
	$(CC) $(CFLAGS) -c $(POS)/page.c
physio.o:	$(POS)/physio.c
	$(CC) $(CFLAGS) -c $(POS)/physio.c
pipe.o:	$(POS)/pipe.c
	$(CC) $(CFLAGS) -c $(POS)/pipe.c
probe.o:	$(POS)/probe.c
	$(CC) $(CFLAGS) -c $(POS)/probe.c
qsort.o:	$(LIBC)/qsort.c
	$(CC) $(CFLAGS) -c $(LIBC)/qsort.c
rdwri.o:	$(POS)/rdwri.c
	$(CC) $(CFLAGS) -c $(POS)/rdwri.c
region.o:	$(POS)/region.c
	$(CC) $(CFLAGS) -c $(POS)/region.c
sched.o:	$(POS)/sched.c
	$(CC) $(CFLAGS) -c $(POS)/sched.c
sem.o:	$(POS)/sem.c
	$(CC) $(CFLAGS) -c $(POS)/sem.c
shm.o:	$(POS)/shm.c
	$(CC) $(CFLAGS) -c $(POS)/shm.c
sig.o:	$(POS)/sig.c
	$(CC) $(CFLAGS) -c $(POS)/sig.c
slp.o:	$(POS)/slp.c
	$(CC) $(CFLAGS) -c $(POS)/slp.c
stream.o:	$(COS)/stream.c
	$(CC) $(CFLAGS) -c $(COS)/stream.c
streamio.o:	$(COS)/streamio.c
	$(CC) $(CFLAGS) -c $(COS)/streamio.c
strncpy.o:	$(LIBC)/strncpy.c
	$(CC) $(CFLAGS) -c $(LIBC)/strncpy.c
subr.o:	$(COS)/subr.c
	$(CC) $(CFLAGS) -c $(COS)/subr.c
swapalloc.o:	$(POS)/swapalloc.c
	$(CC) $(CFLAGS) -c $(POS)/swapalloc.c
swtch.o:	$(POS)/swtch.c
	$(CC) $(CNOFLAGS) -c $(POS)/swtch.c
sys1.o:	$(POS)/sys1.c
	$(CC) $(CFLAGS) -c $(POS)/sys1.c
sys2.o:	$(COS)/sys2.c
	$(CC) $(CFLAGS) -c $(COS)/sys2.c
sys3.o:	$(COS)/sys3.c
	$(CC) $(CFLAGS) -c $(COS)/sys3.c
sys4.o:	$(POS)/sys4.c
	$(CC) $(CFLAGS) -c $(POS)/sys4.c
sysent.o:	$(COS)/sysent.c
	$(CC) $(CFLAGS) -c $(COS)/sysent.c
syslocal.o:	$(POS)/syslocal.c
	$(CC) $(CFLAGS) -c $(POS)/syslocal.c
sysm68k.o:	$(POS)/sysm68k.c
	$(CC) $(CFLAGS) -c $(POS)/sysm68k.c
text.o:	$(POS)/text.c
	$(CC) $(CFLAGS) -c $(POS)/text.c
trap.o:	$(POS)/trap.c 
	$(CC) $(CFLAGS) -c $(POS)/trap.c
udsort.o:	$(POS)/udsort.c
	$(CC) $(CFLAGS) -c $(POS)/udsort.c
uerrlog.o:	$(POS)/uerrlog.c
	$(CC) $(CFLAGS) -c $(POS)/uerrlog.c
ulocking.o:	$(POS)/ulocking.c
	$(CC) $(CFLAGS) -c $(POS)/ulocking.c
umachdep.o:	$(POS)/umachdep.c
	$(CC) $(CNOFLAGS) -c $(POS)/umachdep.c
umove.o:	$(POS)/umove.c
	$(CC) $(CFLAGS) -c $(POS)/umove.c
usys4.o:	$(POS)/usys4.c
	$(CC) $(CFLAGS) -c $(POS)/usys4.c
usyslocal.o:	$(POS)/usyslocal.c
	$(CC) $(CFLAGS) -c $(POS)/usyslocal.c
utssys.o:	$(POS)/utssys.c
	$(CC) $(CFLAGS) -c $(POS)/utssys.c
vfs.o:		$(COS)/vfs.c
		$(CC) $(CFLAGS) -c $(COS)/vfs.c
vfs_bio.o:	$(COS)/vfs_bio.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_bio.c
vfs_conf.o:	$(COS)/vfs_conf.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_conf.c
vfs_dev.o:	$(COS)/vfs_dev.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_dev.c
vfs_dnlc.o:	$(COS)/vfs_dnlc.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_dnlc.c
vfs_io.o:	$(COS)/vfs_io.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_io.c
vfs_lookup.o:	$(COS)/vfs_lookup.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_lookup.c
vfs_pathname.o:	$(COS)/vfs_pathname.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_pathname.c
vfs_syscalls.o:	$(COS)/vfs_syscalls.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_syscalls.c
vfs_vnode.o:	$(COS)/vfs_vnode.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_vnode.c
vfs_xxx.o:	$(COS)/vfs_xxx.c
		$(CC) $(CFLAGS) -c $(COS)/vfs_xxx.c

#PML
blt.o:		$(LIBC)/blt.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/blt.s > blt.i
		$(AS) -o blt.o blt.i
		rm -f blt.i
blt512.o:	$(LIBC)/blt512.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/blt512.s > blt512.i
		$(AS) -o blt512.o blt512.i
		rm -f blt512.i
clear.o:	$(LIBC)/clear.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/clear.s > clear.i
		$(AS) -o clear.o clear.i
		rm -f clear.i
ctype.o:	$(COS)/ctype.c
		$(CC) $(CFLAGS) -c $(COS)/ctype.c
dbcmp.o:	$(LIBC)/dbcmp.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/dbcmp.s > dbcmp.i
		$(AS) -o dbcmp.o dbcmp.i
		rm -f dbcmp.i
ldiv.o:		$(LIBC)/ldiv.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/ldiv.s > ldiv.i
		$(AS) -o ldiv.o ldiv.i
		rm -f ldiv.i
lmul.o:		$(LIBC)/lmul.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/lmul.s > lmul.i
		$(AS) -o lmul.o lmul.i
		rm -f lmul.i
setjmp.o:	$(LIBC)/setjmp.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/setjmp.s > setjmp.i
		$(AS) -o setjmp.o setjmp.i
		rm -f setjmp.i
strcmp.o:	$(LIBC)/strcmp.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/strcmp.s > strcmp.i
		$(AS) -o strcmp.o strcmp.i
		rm -f strcmp.i
strlen.o:	$(LIBC)/strlen.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/strlen.s > strlen.i
		$(AS) -o strlen.o strlen.i
		rm -f strlen.i
strncmp.o:	$(LIBC)/strncmp.s
		$(CPP) -P $(DEFS) $(INCLUDE) $(LIBC)/strncmp.s > strncmp.i
		$(AS) -o strncmp.o strncmp.i
		rm -f strncmp.i
nfsstrncpy.o:	$(LIBC)/nfsstrncpy.c
		$(CC) $(CFLAGS) -c $(LIBC)/nfsstrncpy.c


# DO NOT DELETE THIS LINE
