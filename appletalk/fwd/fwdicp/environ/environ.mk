#
# 	@(#)Copyright Apple Computer 1987	Version 1.3 of environ.mk on 87/11/11 21:08:27 
#
# Makefile 1/12/87
#
#	Master root location
#
MASTERROOT	=../../../..

AT		=/kip/at
DDP=		$(AT)/ddp
NBP=		$(AT)/nbp
ATP=		$(AT)/atp
PAP=		$(AT)/pap
CMD=		/kip/cmd
FWD=		/kip/fwd
FWDICP=		$(FWD)/fwdicp
ENVIRON=	$(FWDICP)/environ
FWDLOAD=	$(CMD)/fwdload
LOOPTEST=	$(CMD)/looptest
STREAMS=	$(FWD)/streams

#
#	Include location information
#
IROOT	=$(MASTERROOT)/test
SIROOT	=$(IROOT)/distv/$S
SINC	=$(SIROOT)/usr/include
VIROOT	=$(IROOT)/distv/dist
VINC	=$(VIROOT)/usr/include
DIROOT	=$(IROOT)/dist/dist
DINC	=$(DIROOT)/usr/include
INCLUDE = -I$(ENVIRON)		\
	  -I$(FWD)
#
#	Source location information
#
SYSTREE	=$(MASTERROOT)
GEN	=$(SYSTREE)/sys/GENV
SVFS	=$(SYSTREE)/sys/SVFS/sys
NET	=$(SYSTREE)/sys/NET/sys
NETinet	=$(SYSTREE)/sys/NETinet/sys
NFS	=$(SYSTREE)/sys/NFS/sys
RPC	=$(SYSTREE)/sys/RPC/sys
SPEC 	=$(SYSTREE)/sys/$S
CF	=$(SPEC)/cf
IO	=$(SPEC)/io
ML	=$(SPEC)/ml
OS	=$(SPEC)/os
GCF	=$(GEN)/cf
GIO	=$(GEN)/io
GML	=$(GEN)/ml
GOS	=$(GEN)/os

#
#	Program (Compiler, etc) setup (flags and path names)
#
SROOT	=/
AS	=oas
CC	=occ
M4	=m4
MV	=mv
RM	=rm
UCC	=cc
CFLAGS	=$(DEFS) $(INCLUDE) $(VERBOSE) $(ZFLAGS) -O
CNOFLAGS=$(DEFS) $(INCLUDE) $(VERBOSE) $(ZFLAGS)
DEFS	= $(LOCAL) $(GLOBAL)
GLOBAL  = -DPMMU -Uvax -Usun -Dm68k -DPAGING 
#
#	Configuration constants
#

PROCESS_COUNT=10		# the number of processes at once
NATP_TRANS=30			# the number of ATP requests at once
NATP_RCB=60			# the number of ATP RCBs at once
NATP_STATE=30			# the number of ATP sockets open at once
NPAPSERVERS=10			# the number of active PAP servers/node
NPAPSESSIONS=40			# the number of active PAP sockets/node

LOCAL	=					\
		-DFEP				\
		-DGET_PUT_HI			\
		-DAPPLETALK			\
		-DPROCESS_COUNT=$(PROCESS_COUNT)\
		-DNATP_TRANS=$(NATP_TRANS)	\
		-DNATP_RCB=$(NATP_RCB)		\
		-DNATP_STATE=$(NATP_STATE)	\
		-DNPAPSERVERS=$(NPAPSERVERS)	\
		-DNPAPSESSIONS=$(NPAPSESSIONS)

#LOCAL	=					\
#		-DFEP				\
#		-DHOWFAR			\
#		-DDEBUG				\
#		-DSTRDEBUG			\
#		-DGET_PUT_HI			\
#		-DAPPLETALK			\
#		-DFWDPRINTF			\
#		-DSCC_DEBUG2			\
#		-DPROCESS_COUNT=$(PROCESS_COUNT)\
#		-DNATP_TRANS=$(NATP_TRANS)	\
#		-DNATP_RCB=$(NATP_RCB)		\
#		-DNATP_STATE=$(NATP_STATE)	\
#		-DNPAPSERVERS=$(NPAPSERVERS)	\
#		-DNPAPSESSIONS=$(NPAPSESSIONS)

LD	=old
LFLAGS	=-N init.ld
LINT	=/usr/bin/lint
LINTFLAGS=-bn
NM	=$(SROOT)/bin/nm
NMFLAGS	=-xev
VERBOSE	=
ZFLAGS	=
CONFIG	=$(SROOT)/binv/config		# does not exist on ilsa yet c.lai
FIX	=$(SROOT)/bin/size -x
STRIP	=$(SROOT)/bin/strip
CUT	=rsh thorin /usr/bin/cut

GEN_OBJ=	icpenviron.o	\
		icpaccess.o	\
		clock.o		\
		debug.o		\
		fwd.o		\
		fwdicplib.o	\
		fwdicpopen.o	\
		lmul.o		\
		ldiv.o		\
		loop.o	 	\
		ploop.o		\
		process.o	\
		proc_switch.o	\
		proc_work.o	\
		stream.o	\
		streamHI.o	\
		streamio.o	\
		strdebug.o	\
		strncmp.o	\
		strncpy.o	\
		subr.o		\
		fep_var.o

AT_OBJ=		at_locore.o	\
		at_conf.o	\
		at_mch.o	\
		appleload.o	\
		$(AT_DEV)	\
		$(GEN_OBJ)

TT_OBJ=		tt_locore.o	\
		tt_conf.o	\
		$(TT_DEV)	\
		$(GEN_OBJ)

TST_OBJ=	at_locore.o	\
		tst_conf.o	\
		appleload.o	\
		$(TST_DEV)	\
		$(GEN_OBJ)

AT_DEV=		ddloop.o

TT_DEV=				\
		line.o partab.o	\
		ast.o ttx.o	\
		ddloop.o	\
		loop.o	 	\
		ploop.o

TST_DEV=			\
		xloop.o	 	\
		atp.o		\
		ddloop.o	\
		ploop.o


.c.o:
		${CC} ${CFLAGS} -c $*.c

all:            at_load

at_load:	$(AT_OBJ)		\
		$(ENVIRON)/via.h	\
		$(ENVIRON)/scc.h	\
		$(ENVIRON)/fwdicp.h	\
		$(ENVIRON)/at_ifile	\
		$(FWD)/fwd.h
		$(LD) -o $@ at_ifile $(AT_OBJ)
		chown bin $@
		chgrp bin $@
		chmod 755 $@

tt_load:	$(TT_OBJ)		\
		$(ENVIRON)/via.h	\
		$(ENVIRON)/scc.h	\
		$(ENVIRON)/fwdicp.h	\
		$(ENVIRON)/tt_ifile	\
		$(FWD)/fwd.h
		$(LD) -o $@ tt_ifile $(TT_OBJ)
		chown bin $@
		chgrp bin $@
		chmod 755 $@

tst_load:	$(TST_OBJ)			\
		$(ENVIRON)/via.h		\
		$(ENVIRON)/scc.h		\
		$(ENVIRON)/fwdicp.h		\
		$(ENVIRON)/at_ifile		\
		$(FWD)/fwd.h
		$(LD) -o $@ at_ifile $(TST_OBJ)
		chown bin $@
		chgrp bin $@
		chmod 755 $@

icpenviron.o:	$(ENVIRON)/icpenviron.c	\
		$(ENVIRON)/via.h	\
		$(ENVIRON)/scc.h	\
		$(ENVIRON)/fwdicp.h 
		${CC} ${CFLAGS} -DSCC_DEFINE -DFWD_DEFINE -c $*.c

appleload.o:	$(ENVIRON)/via.h		\
		$(ENVIRON)/scc.h		\
		$(ENVIRON)/fwdicp.h		\
		$(AT)/lap/at_lap.c		\
		$(AT)/lap/at_lap.h		\
		$(AT)/lap/at_lapopen.c		\
		$(DDP)/at_ddp.h			\
		$(DDP)/at_ddp.c			\
		$(DDP)/at_ddpopen.c		\
		$(NBP)/at_nbpd.c		\
		$(ATP)/atp_alloc.c		\
		$(ATP)/atp_misc.c		\
		$(ATP)/atp_open.c 		\
		$(ATP)/atp_read.c		\
		$(ATP)/atp_write.c		\
		$(ATP)/atp.inc.h		\
		$(PAP)/at_pap.c			\
		$(PAP)/at_papd.c
		$(MAKE) -f $(AT)/makefile CFLAGS="$(CFLAGS)" CC=$(CC) LD=$(LD) AT=$(AT) FWD=$(FWD) DEST=$(DEST) appleload.o;
		
clock.o:	$(ENVIRON)/clock.c		\
		$(ENVIRON)/fwdicp.h		\
		$(ENVIRON)/scc.h
		${CC} ${CFLAGS} -DKERNEL -c $(ENVIRON)/clock.c

at_conf.o:	$(ENVIRON)/at_conf.c
		${CC} ${CFLAGS} -c $(ENVIRON)/at_conf.c

tt_conf.o:	$(ENVIRON)/tt_conf.c
		${CC} ${CFLAGS} -c $(ENVIRON)/tt_conf.c

tst_conf.o:	$(ENVIRON)/tst_conf.c
		${CC} ${CFLAGS} -c $(ENVIRON)/tst_conf.c

debug.o:	$(ENVIRON)/debug.c
		${CC} ${CFLAGS} -c $(ENVIRON)/debug.c

fep_var.o:	$(ENVIRON)/fep_var.c		\
		$(ENVIRON)/fwdicp.h		\
		$(FWD)/fwd.h
		${CC} ${CFLAGS} -c $(ENVIRON)/fep_var.c
		
fwd.o:		$(FWD)/fwd.c			\
		$(FWD)/fwd.h			\
		$(ENVIRON)/via.h		\
		$(ENVIRON)/scc.h		\
		$(ENVIRON)/fwdicp.h
		${CC} ${CFLAGS} -c $(FWD)/fwd.c

fwdicplib.o:	$(FWDICP)/fwdicplib.c		\
		$(ENVIRON)/via.h		\
		$(ENVIRON)/scc.h		\
		$(ENVIRON)/fwdicp.h		\
		$(FWD)/fwd.h
		${CC} ${CFLAGS} -c $(FWDICP)/fwdicplib.c

fwdicpopen.o:	$(FWDICP)/fwdicpopen.c		\
		$(ENVIRON)/via.h		\
		$(ENVIRON)/scc.h		\
		$(ENVIRON)/fwdicp.h		\
		$(FWD)/fwd.h
		${CC} ${CFLAGS} -c $(FWDICP)/fwdicpopen.c

at_locore.o:	$(ENVIRON)/at_locore.s
		${AS} -o at_locore.o $(ENVIRON)/at_locore.s
		${FIX} $@

at_mch.o:	$(ENVIRON)/at_mch.s
		${AS} -o at_mch.o $(ENVIRON)/at_mch.s
		${FIX} $@

tt_locore.o:	$(ENVIRON)/tt_locore.s
		${AS} -o tt_locore.o $(ENVIRON)/tt_locore.s
		${FIX} $@

line.o:		../../../../sys/COMMON/io/line.c
		${CC} ${CFLAGS} -DNTTQ=4 -c ../../../../sys/COMMON/io/line.c

ttx.o:		../../../../sys/COMMON/io/ttx.c
		${CC} ${CFLAGS} -c ../../../../sys/COMMON/io/ttx.c

partab.o:	../../../../sys/COMMON/io/partab.c
		${CC} ${CFLAGS} -c ../../../../sys/COMMON/io/partab.c

trans.o:	$(ENVIRON)/trans.c
		${CC} ${CFLAGS} -c $(ENVIRON)/trans.c

ast.o:		$(ENVIRON)/ast.c 		\
		$(ENVIRON)/scc.h
		${CC} ${CFLAGS} -c $(ENVIRON)/ast.c

loop.o:		$(ENVIRON)/loop.c
		${CC} ${CFLAGS} -c $(ENVIRON)/loop.c

ddloop.o:	$(ENVIRON)/ddloop.c
		${CC} ${CFLAGS} -c $(ENVIRON)/ddloop.c

xloop.o:	$(ENVIRON)/xloop.c
		${CC} ${CFLAGS} -c $(ENVIRON)/xloop.c

ploop.o:	$(ENVIRON)/ploop.c
		${CC} ${CFLAGS} -c $(ENVIRON)/ploop.c

stream.o:	$(STREAMS)/stream.c
		${CC} ${CFLAGS} -c $(STREAMS)/stream.c
		
streamHI.o:	$(STREAMS)/streamHI.c
		${CC} ${CFLAGS} -c $(STREAMS)/streamHI.c
		
streamio.o:	$(STREAMS)/streamio.c		\
		$(FWD)/fwd.h
		${CC} ${CFLAGS} -c $(STREAMS)/streamio.c

strdebug.o:	$(STREAMS)/strdebug.c
		${CC} ${CFLAGS} -c $(STREAMS)/strdebug.c
		
subr.o:		$(ENVIRON)/subr.c
		${CC} ${CFLAGS} -c $(ENVIRON)/subr.c

proc_work.o:	$(ENVIRON)/proc_work.c $(ENVIRON)/sys/process.h
		${CC} ${CFLAGS} -c $(ENVIRON)/proc_work.c

process.o:	$(ENVIRON)/process.c $(ENVIRON)/sys/process.h
		${CC} ${CFLAGS} -c $(ENVIRON)/process.c

proc_switch.o:	$(ENVIRON)/proc_switch.s
		${CC} ${CFLAGS} -c $(ENVIRON)/proc_switch.s

strncmp.o:	$(ENVIRON)/strncmp.s
		$(M4) $(ENVIRON)/m4.def $(ENVIRON)/strncmp.s > tmp.s && \
		$(CC) $(CFLAGS) -c tmp.s && \
		$(MV) tmp.o $@ && \
		$(RM) tmp.s

strncpy.o:	$(ENVIRON)/strncpy.s
		$(M4) $(ENVIRON)/m4.def $(ENVIRON)/strncpy.s > tmp.s && \
		$(CC) $(CFLAGS) -c tmp.s && \
		$(MV) tmp.o $@ && \
		$(RM) tmp.s

ldiv.o:		$(ENVIRON)/ldiv.s
		$(M4) $(ENVIRON)/m4.def $(ENVIRON)/ldiv.s > tmp.s && \
		$(CC) $(CFLAGS) -c tmp.s && \
		$(MV) tmp.o $@ && \
		$(RM) tmp.s

lmul.o:		$(ENVIRON)/lmul.s
		$(M4) $(ENVIRON)/m4.def $(ENVIRON)/lmul.s > tmp.s && \
		$(CC) $(CFLAGS) -c tmp.s && \
		$(MV) tmp.o $@ && \
		$(RM) tmp.s

clean:
		rm -f ${ALL} *.o errs core a.out t.?
