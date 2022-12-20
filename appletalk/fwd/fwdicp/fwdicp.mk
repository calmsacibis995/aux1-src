# Makefile 6/12/83
#
#	Master root location
#
MASTERROOT	=/nfs

AT=		/kip/at
TALKDIR=	$(AT)
LAP=		$(TALKDIR)/lap
DDP=		$(TALKDIR)/ddp
CMD=		/kip/cmd
DEST=		/
FWD=		/kip/fwd
FWDICP=		$(FWD)/fwdicp
ENVIRON=	$(FWDICP)/environ
FWDLOAD=	$(CMD)/fwdload
LOOPTEST=	$(CMD)/looptest

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
SROOT	=/nfs
DEFS	=
#DEFS	= $(LOCAL)
LOCAL	=-DSCC_DELAY -DHOWFAR -DDEBUG
LFLAGS	=-N init.ld
#LIB	=-L$(SROOT)/lib -lc
LIB	=
LINT	=$(SROOT)/bin/lint
LINT	=/usr/bin/lint
LINTFLAGS=-bn
NM	=$(SROOT)/bin/nm
NMFLAGS	=-xev
VERBOSE	=
ZFLAGS	=
CONFIG	=$(SROOT)/binv/config		# does not exist on ilsa yet c.lai
FIX	=$(SROOT)/bin/size -x
STRIP	=$(SROOT)/bin/strip

LOCAL_INCLUDES=	-I$(ENVIRON)

.c.o:
		${CC} ${CFLAGS} ${LOCAL_INCLUDES} ${DEFS} -c $*.c

all:            $(DEST)/usr/bin/fwd_lkup $(DEST)/etc/fwdload

fwdicp.o:	fwdicplib.o \
		fwdicpopen.o \
		$(ENVIRON)/fwdicp.h
		${LD} -r -o $@ fwdicplib.o fwdicpopen.o

icplap.o:	$(LAP)/icplap.c			\
		$(FWD)/fwd.h			\
		$(ENVIRON)/fwdicp.h
		${CC} ${CFLAGS} ${LOCAL_INCLUDES} ${DEFS} -DFEP -c $(LAP)/icplap.c

fwdddp.o:	$(DDP)/fwdddp.c			\
		$(FWD)/fwd.h			\
		$(ENVIRON)/fwdicp.h
		${CC} ${CFLAGS} ${LOCAL_INCLUDES} ${DEFS} -DFEP -c $(DDP)/fwdddp.c

fwdicploop.o:	$(FWDICP)/fwdicploop.c		\
		$(FWD)/fwd.h
		${CC} ${CFLAGS} ${LOCAL_INCLUDES} ${DEFS} -DFEP -c $(FWDICP)/fwdicploop.c

fwdicptt.o:	$(FWDICP)/fwdicptt.c		\
		$(FWD)/fwd.h
		${CC} ${CFLAGS} ${LOCAL_INCLUDES} ${DEFS} -DFEP -c $(FWDICP)/fwdicptt.c

fwdicpddl.o:	$(FWDICP)/fwdicpddl.c		\
		$(FWD)/fwd.h
		${CC} ${CFLAGS} ${LOCAL_INCLUDES} ${DEFS} -DFEP -c $(FWDICP)/fwdicpddl.c

fwdicpploop.o:	$(FWDICP)/fwdicpploop.c		\
		$(FWD)/fwd.h
		${CC} ${CFLAGS} ${LOCAL_INCLUDES} ${DEFS} -DFEP -c $(FWDICP)/fwdicpploop.c

fwdicpopen.o:	$(FWDICP)/fwdicpopen.c 	\
		$(FWD)/fwd.h 		\
		$(ENVIRON)/fwdicp.h
		${CC} ${CFLAGS} ${LOCAL_INCLUDES} ${DEFS} -c $(FWDICP)/fwdicpopen.c

fwdicplib.o:	$(FWDICP)/fwdicplib.c $(FWD)/fwd.h $(ENVIRON)/fwdicp.h
		${CC} ${CFLAGS} ${LOCAL_INCLUDES} ${DEFS} -c $(FWDICP)/fwdicplib.c

$(DEST)/etc/fwdload: $(FWD)/fwdload.c	\
		$(FWD)/fwd.h
		$(CC) -I$(FWD) $(CFLAGS) -o $(DEST)/etc/fwdload $(FWD)/fwdload.c -lld
		chown bin $@
		chgrp bin $@
		chmod 755 $@
		
$(DEST)/usr/bin/looptest: $(LOOPTEST)/looptest.c
		cd $(LOOPTEST); $(MAKE) -f *.mk CFLAGS="$(CFLAGS) -I$(FWD)" DEST=$(DEST) LIBROOT="/usr";
		chown bin $@
		chgrp bin $@
		chmod 755 $@
		
$(DEST)/usr/bin/fwd_lkup: $(FWD)/fwd_lkup.c 	\
		$(FWD)/fwd.h
		$(CC) -I$(FWD) $(CFLAGS) -o $(DEST)/usr/bin/fwd_lkup $(FWD)/fwd_lkup.c 
		chown bin $@
		chgrp bin $@
		chmod 755 $@
		
clean:
		rm -f ${ALL} *.o *.s errs core a.out t.?

lint:
		$(LINT) $(LFLAGS) $(FWDICP)/*.c $(FWD)/fwd.c > lint.lst

