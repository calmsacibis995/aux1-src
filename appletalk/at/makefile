#  Copyright 1984 UniSoft Systems, Version 1.3
#  @(#)makefile	1.3 4/15/87

#  This makefile makes appletalk user level code and stream/kernel code. User code
#  is made by "make all" and can be run directly on the target machine or by a higher
#  level makefile in a cross environment with the CC, CFLAGS, LIB, ... set correctly.
#  The streams code is made by "make appleload". For downloading onto an intelligent
#  processor, the defines and tools must also be defined.

.SUFFIXES :
.SUFFIXES : .o .c .c~

SHELL	=/bin/sh
STRIP   =strip

HOST=
GET_DIR=	get_dir
Y=		new_release	#		and why
#
#	Master root location
#
MASTERROOT	=/nfs

AT=		/kip/at
LAP=		$(AT)/lap
DDP=		$(AT)/ddp
ATP=		$(AT)/atp
PAP=		$(AT)/pap
NBP=		$(AT)/nbp
ECH=		$(AT)/echo
LIB=		$(DEST)/usr/lib
LIBSRC=		$(AT)/lib
UTIL=		$(AT)/util
DEST=		/

TESTLIB=	$(AT)/testlib
OTHERDIRS=	$(TESTDRIVER) $(TESTLIB) $(LAP) $(DDP)

USER=		$(DEST)/etc/appletalk	\
		$(DEST)/etc/at_laplkup		\
		$(DEST)/etc/at_nbpd		\
		$(DEST)/usr/bin/at_nvelkup	\
		$(DEST)/usr/bin/at_nvedel	\
		$(DEST)/usr/bin/at_nvereg	\
		$(DEST)/etc/at_nveshut		\
		$(DEST)/usr/bin/at_printer	\
		$(DEST)/usr/bin/at_server	\
		$(DEST)/usr/bin/at_status

SOURCES=				\
		$(UTIL)/appletalk.c	\
		$(ATP)/atp.inc.h	\
		$(ATP)/atp_misc.c	\
		$(ATP)/atp_read.c	\
		$(ATP)/atp_alloc.c	\
		$(ATP)/atp_open.c	\
		$(ATP)/atp_write.c	\
		$(DDP)/at_ddp.c		\
		$(DDP)/at_ddpopen.c	\
		$(LAP)/at_lap.c		\
		$(LAP)/at_lapopen.c	\
		$(LAP)/at_laplkup.c	\
		$(UTIL)/at_nbpd.c	\
		$(NBP)/at_nbpd.c	\
		$(UTIL)/at_nvelkup.c	\
		$(UTIL)/at_nvedel.c	\
		$(UTIL)/at_nvereg.c	\
		$(UTIL)/at_nveshut.c	\
		$(PAP)/at_pap.c    	\
		$(PAP)/at_papd.c	\
		$(DDP)/at_pftd.c	\
		$(UTIL)/at_printer.c	\
		$(UTIL)/at_server.c	\
		$(UTIL)/at_status.c	\
		$(DDP)/at_send.c	\
		$(UTIL)/at_server.c	\
		$(UTIL)/at_status.c	\
		$(UTIL)/at_send.c	\
		$(ECH)/echo.c		\
		$(LAP)/laptest.c

SCCS_SOURCES=	s.makefile		\
		s.appletalk.h		\
		$(LAP)/s.at_laplkup.c 	\
		$(LAP)/s.laptest.c 	\
		$(LAP)/s.at_lap.c	\
		$(DDP)/s.at_ddp.c	\
		$(ECH)/s.echo.c		\
		s.appletalk.c		\
		s.at_nbpd.c		\
		s.at_nvelkup.c		\
		s.at_nvereg.c		\
		s.at_nveshow.c		\
		s.at_pftd.c		\
		s.at_rtmpd.c		\
		s.at_scc.c		\
		s.at_send.c		\
		s.at_lib.c		\
		s.atp_pft.c		\
		s.atp_pftd.c

TMP1=		at.mk.junk1

#
# make options or fake targets
#
.c~.c:
	get $<

.c.o:
	$(CC) $(CFLAGS) -c $<

all:		$(DEST)/usr/include/appletalk.h lib_dummy $(USER)

allclean:	clean appletalk.h $(USER)

backup:
		bru -cf - /users/prog/kip $(SCCS_SOURCES) $(OTHERDIRS) renem | remsh charming 'cat >! /tmp/kip/backup'


lint:
		$(LINT) $(LFLAGS) $(SOURCES) > lint.lst

receive:
		unpack tar.z	
		tar xf tar	

recover:
		cp $(SCCS_SOURCES) $(GET_DIR)
		get -r $(RN) $(SCCS_SOURCES)
		cd $(GET_DIR)
		make all_local

release:	dodelta \
		dogete \
		dorenem \
		dodelta1 
		allclean 
		tags

send:
		remsh $(HOST) 'cd $(AT) ; mkdir streams testdriver testlib ; rm -f $(SCCS_SOURCES)'
		bru -cf bru $(SCCS_SOURCES) streams testdriver testlib renem
		rcp bru $(HOST):$(AT) 
		remsh $(HOST) 'cd $(AT) ; bru -xf bru ; make'

sendcu:
		rm -f /tmp/tar.z
		tar -cf /tmp/tar /users/prog/kip $(SCCS_SOURCES) $(OTHERDIRS)
		pack /tmp/tar

tags:
		ctags $(SOURCES) 
		for i in $(OTHERDIRS); do ctags -a $$i/*.c; done
		make htags
		sort +0 tags -o tags

htags:
		make tagstruct TAGSTRUCT="$(DINCSRC)" | \
			egrep -v 'sed|make' >> tags #2>>/dev/null

tagstruct:
		./:sed $(TAGSTRUCT)


depend:
	rm -f makedep eddep
	for i in ${BNETCONFIGSRC} ${CFILES};			do	\
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


#
# fake targets for internal use of this makefile
#
clean:
		rm -f $(USER)

dodelta:	
		@echo ':'		> $(TMP1)
		@echo 'for f in $$*'	>>$(TMP1)
		@echo 'do'		>>$(TMP1)
		@echo 'if test -f p.$$f' >>$(TMP1)
		@echo 'then'		>>$(TMP1)
		@echo 'delta -y$(Y)  s.$$f' >>$(TMP1)
		@echo 'fi'		>>$(TMP1)
		@echo 'done'		>>$(TMP1)
		@echo 'exit 0'		>>$(TMP1)
		@sh $(TMP1) makefile appletalk.h $(SOURCES)
		@rm -f $(TMP1)

#	since make is so smart, it sees dodelta twice and does it once, great!
dodelta1:	
		@echo ':'		> $(TMP1)
		@echo 'for f in $$*'	>>$(TMP1)
		@echo 'do'		>>$(TMP1)
		@echo 'if test -f p.$$f' >>$(TMP1)
		@echo 'then'		>>$(TMP1)
		@echo 'delta -y$(Y)  s.$$f' >>$(TMP1)
		@echo 'fi'		>>$(TMP1)
		@echo 'done'		>>$(TMP1)
		@echo 'exit 0'		>>$(TMP1)
		@sh $(TMP1) makefile appletalk.h $(SOURCES)
		@rm -f $(TMP1)

dogete:	
		@ get -e -r$(RN) $(SCCS_SOURCES)

dorenem:
		echo 1000 > .renem
		sh -c "renem at_lib.c"
		echo 2000 > .renem
		sh -c "renem at_nbpd.c"
		echo 3000 > .renem
		sh -c "renem at_nvereg.c"
		echo 5000 > .renem
		sh -c "renem at_nvelkup.c"
		echo 6000 > .renem
		sh -c "renem appletalk.c"
		echo 7000 > .renem
		sh -c "renem at_send.c"
		echo 8000 > .renem

#
# real targets
#

$(DEST)/usr/include/appletalk.h:	$(AT)/appletalk.h
		cp $(AT)/appletalk.h $(DEST)/usr/include/appletalk.h
		chown bin $@
		chgrp bin $@
		chmod 755 $@

#
#  USER level, utilities
#

$(DEST)/etc/at_laplkup: $(LAP)/at_laplkup.c $(LIB)/libat.a
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/etc/at_laplkup $(LAP)/at_laplkup.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/etc/appletalk:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(UTIL)/appletalk.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/etc/appletalk $(UTIL)/appletalk.c -lat 
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/etc/at_nbpd:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(UTIL)/at_nbpd.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/etc/at_nbpd $(UTIL)/at_nbpd.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/usr/bin/at_nvedel:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(UTIL)/at_nvedel.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/usr/bin/at_nvedel $(UTIL)/at_nvedel.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/etc/at_nveshut:		$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(UTIL)/at_nveshut.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/etc/at_nveshut $(UTIL)/at_nveshut.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/usr/bin/at_nvereg:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(UTIL)/at_nvereg.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/usr/bin/at_nvereg $(UTIL)/at_nvereg.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/usr/bin/at_nvelkup:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(UTIL)/at_nvelkup.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/usr/bin/at_nvelkup $(UTIL)/at_nvelkup.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/usr/bin/at_pftd:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(DDP)/at_pftd.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/usr/bin/at_pftd $(DDP)/at_pftd.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/usr/bin/at_printer:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(UTIL)/at_printer.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/usr/bin/at_printer $(UTIL)/at_printer.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/usr/bin/at_server:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(UTIL)/at_server.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/usr/bin/at_server $(UTIL)/at_server.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/usr/bin/at_status:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(UTIL)/at_status.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/usr/bin/at_status $(UTIL)/at_status.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/usr/bin/at_send:	$(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(DDP)/at_send.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/usr/bin/at_send $(DDP)/at_send.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@

$(DEST)/usr/bin/laptest: $(DEST)/usr/include/appletalk.h $(LIB)/libat.a $(LAP)/laptest.c
		$(CC) $(CFLAGS) -L$(LIB) -o $(DEST)/usr/bin/laptest $(LAP)/laptest.c -lat
		$(STRIP) $@
		chown bin $@
		chgrp bin $@
		chmod 755 $@
		
#
#  User level, libraries
#	this is difficult, should make in a directory that is specific for the box
#	but is not on the box so users will not see all the .o sitting around. At the
#	moment it is made in the sources directory!
#

lib_dummy:
		cd $(LIBSRC); make -f $(LIBSRC)/makefile AT=$(AT) LIB=$(LIB) LIBSRC=$(LIBSRC) all
		chown bin $(DEST)/usr/lib/libat.a
		chgrp bin $(DEST)/usr/lib/libat.a
		chmod 755 $(DEST)/usr/lib/libat.a


#
#  Stream level
#

STR_OBJ=	at_lap.o	\
		at_lapopen.o	\
		at_ddp.o	\
		at_ddpopen.o	\
		at_nbpd.o	\
		atp_misc.o	\
		atp_read.o	\
		atp_alloc.o	\
		atp_open.o	\
		atp_write.o	\
		at_pap.o    	\
		at_papd.o

appleload.o:	$(STR_OBJ)
		$(LD) -r -o $@ $(STR_OBJ)

at_lap.o:	$(DEST)/usr/include/appletalk.h $(LAP)/at_lap.c $(LAP)/at_lap.h scc.h $(DDP)/at_ddp.h
		$(CC) -I$(LAP) -I$(DDP) -I$(AT) $(CFLAGS) -c $(LAP)/at_lap.c

at_lapopen.o:	$(DEST)/usr/include/appletalk.h $(LAP)/at_lapopen.c $(LAP)/at_lap.h
		$(CC) -I$(LAP) -I$(AT) $(CFLAGS) -c $(LAP)/at_lapopen.c

at_ddp.o:	$(DEST)/usr/include/appletalk.h $(DDP)/at_ddp.c $(DDP)/at_ddp.h
		$(CC) -I$(DDP) -I$(AT) $(CFLAGS) -c $(DDP)/at_ddp.c

echo.o:		$(DEST)/usr/include/appletalk.h $(ECH)/echo.c 
		$(CC) -I$(ECH) -I$(AT) $(CFLAGS) -c $(ECH)/echo.c

at_ddpopen.o:	$(DEST)/usr/include/appletalk.h $(DDP)/at_ddpopen.c $(DDP)/at_ddp.h
		$(CC) $(CFLAGS) -I$(DDP) -I$(AT) -c $(DDP)/at_ddpopen.c

at_nbpd.o:	$(DEST)/usr/include/appletalk.h $(NBP)/at_nbpd.c
		$(CC) $(CFLAGS) -I$(DDP) -I$(AT) -c $(NBP)/at_nbpd.c

atp_misc.o:	$(DEST)/usr/include/appletalk.h $(ATP)/atp.inc.h $(ATP)/atp_misc.c
		$(CC) -I$(AT) $(CFLAGS) -c $(ATP)/atp_misc.c

atp_read.o:	$(DEST)/usr/include/appletalk.h $(ATP)/atp.inc.h $(ATP)/atp_read.c
		$(CC) -I$(AT) $(CFLAGS) -c $(ATP)/atp_read.c

atp_alloc.o:	$(DEST)/usr/include/appletalk.h $(ATP)/atp.inc.h $(ATP)/atp_alloc.c
		$(CC) -I$(AT) $(CFLAGS) -c $(ATP)/atp_alloc.c

atp_open.o:	$(DEST)/usr/include/appletalk.h $(ATP)/atp.inc.h $(ATP)/atp_open.c
		$(CC) -I$(AT) $(CFLAGS) -c $(ATP)/atp_open.c

atp_write.o:	$(DEST)/usr/include/appletalk.h $(ATP)/atp.inc.h $(ATP)/atp_write.c
		$(CC) -I$(AT) $(CFLAGS) -c $(ATP)/atp_write.c

at_pap.o:	$(DEST)/usr/include/appletalk.h $(PAP)/at_pap.c
		$(CC) -I$(AT) $(CFLAGS) -c $(PAP)/at_pap.c

at_papd.o:	$(DEST)/usr/include/appletalk.h $(PAP)/at_papd.c
		$(CC) -I$(AT) $(CFLAGS) -c $(PAP)/at_papd.c
