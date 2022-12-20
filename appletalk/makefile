BOOTD	= /etc/boot.d

unix:	/newunix $(BOOTD)/forwarder $(BOOTD)/fwdloop $(BOOTD)/fwdploop $(BOOTD)/fwdtt \
		$(BOOTD)/fwdddl
	autoconfig -u -L loadfile -o unix -I

# Description file for 68000 UNIX

SHELL	=/bin/sh

#
#	Machine configuration information
#
SYSTEM	=unix
NODE	=psn-pmmu
REL	=5.2
VER	=rl
MACH	=mc68020
MAKEFILE=makefile
S	=psn

#
#	Master root location
#
MASTERROOT	=/nfs

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
FWD	=/kip/fwd
FWDICP	=$(FWD)/fwdicp
LAP	=/kip/at/lap
INCLUDE=		\
	-I$(SINC)	\
	-I$(VINC)	\
	-I$(DINC)	\
	-I.

#
#	Program (Compiler, etc) setup (flags and path names)
#
SROOT020=/
SROOT	=/
AS020	=$(SROOT020)/bin/xas
AS020	=$(SROOT020)/bin/as
AS	=$(SROOT)/bin/xas
CC	=$(SROOT)/bin/xcc
AS	=$(SROOT)/bin/as
CC	=$(SROOT)/bin/cc
CFLAGS	=$(DEFS) $(INCLUDE) $(VERBOSE) $(ZFLAGS) -O -f
CNOFLAGS=$(DEFS) $(INCLUDE) $(VERBOSE) $(ZFLAGS) -f
CPP	=$(SROOT)/lib/xcpp
CPP	=$(SROOT)/lib/cpp
#	Flavors of Memory management
MMU=	-DPMMU -DMMB -DPMMUMMB #	# PMMU dressed like MMB
MMU=	-DMMB #				# MMB
MMU=	-DPMMU -DUMVSEG #		# PMMU 
DEFS=	-DKERNEL -Uvax -Usun -Dm68k -Dmc68020 -Dmc68881 \
	-DINET -DSTREAMS -UQUOTA $(LOCAL) \
	-DSYSCALLS -DPAGING $(MMU) -DNFS
#	Other options:
# NOFPU: is because of the 68881 autoconfig bug in the current hardware
# SCREEN: define to enable screen code
# DVT: define for Rev 5 boards.
LOCAL=	-DAUTOCONFIG -DNFSDEBUG -DDEBUG -DITRACE -DHOWFAR \
	-DSCREEN -DDVT -DFWDPRINTF
LD	=$(SROOT)/bin/ld
LINT	=$(SROOT)/bin/lint
LINT	=/usr/bin/lint
LINTFLAGS=-bn
NM	=$(SROOT)/bin/nm
NMFLAGS	=-xev
VERBOSE	=
ZFLAGS	=-Zn
CONFIG	=$(SROOT)/binv/config		# does not exist on ilsa yet c.lai
FIX	=$(SROOT)/bin/size -x
STRIP	=$(SROOT)/bin/strip
CUT	=rsh thorin /usr/bin/cut

.c.o:
	$(CC) $(CFLAGS) -c $<

tags:		/tmp
		make ctags
		sort +0 tags -o tags

ctags:
		ctags -w $(BNETCONFIGSRC) $(CFILES)

tagstruct:
		./:sed $(TAGSTRUCT)



forwarder.o:	strdebug.o fwd.o $(FWDICP)/fwdicp.o
	$(LD) -o forwarder.o -r strdebug.o fwd.o $(FWDICP)/fwdicp.o


strdebug.o:	$(FWD)/streams/strdebug.c
	$(CC) -I$(FWD) $(CFLAGS) -c $(FWD)/streams/strdebug.c
fwd.o:	$(FWD)/fwd.c $(FWD)/fwd.h
	$(CC) -I$(FWD) $(CFLAGS) -c $(FWD)/fwd.c

$(FWDICP)/fwdicp.o:	$(FWDICP)/fwdicplib.c	\
			$(FWDICP)/fwdicpopen.c	\
			$(FWD)/fwd.h		\
			$(FWDICP)/environ/fwdicp.h 
	cd $(FWDICP); $(MAKE) -f fwdicp.mk CC="$(CC)" FIX="$(FIX)" LD="$(LD)" CFLAGS="$(CFLAGS) -I$(FWD)" fwdicp.o

$(FWDICP)/fwdicploop.o:	$(FWDICP)/fwdicploop.c	\
			$(FWD)/fwd.h		\
			$(FWDICP)/environ/fwdicp.h 
	cd $(FWDICP); $(MAKE) -f fwdicp.mk CC="$(CC)" FIX="$(FIX)" LD="$(LD)" CFLAGS="$(CFLAGS) -I$(FWD)" fwdicploop.o

$(FWDICP)/fwdicptt.o:	$(FWDICP)/fwdicptt.c	\
			$(FWD)/fwd.h		\
			$(FWDICP)/environ/fwdicp.h 
	cd $(FWDICP); $(MAKE) -f fwdicp.mk CC="$(CC)" FIX="$(FIX)" LD="$(LD)" CFLAGS="$(CFLAGS) -I$(FWD)" fwdicptt.o

$(FWDICP)/fwdicpddl.o:	$(FWDICP)/fwdicpddl.c	\
			$(FWD)/fwd.h		\
			$(FWDICP)/environ/fwdicp.h 
	cd $(FWDICP); $(MAKE) -f fwdicp.mk CC="$(CC)" FIX="$(FIX)" LD="$(LD)" CFLAGS="$(CFLAGS) -I$(FWD)" fwdicpddl.o

$(FWDICP)/fwdicpploop.o:$(FWDICP)/fwdicpploop.c	\
			$(FWD)/fwd.h		\
			$(FWDICP)/environ/fwdicp.h 
	cd $(FWDICP); $(MAKE) -f fwdicp.mk CC="$(CC)" FIX="$(FIX)" LD="$(LD)" CFLAGS="$(CFLAGS) -I$(FWD)" fwdicpploop.o

$(BOOTD)/forwarder: forwarder.o
	cp forwarder.o $(BOOTD)/forwarder

$(BOOTD)/fwdloop: $(FWDICP)/fwdicploop.o
	cp $(FWDICP)/fwdicploop.o $(BOOTD)/fwdloop

$(BOOTD)/fwdploop: $(FWDICP)/fwdicpploop.o
	cp $(FWDICP)/fwdicpploop.o $(BOOTD)/fwdploop

$(BOOTD)/fwdtt: $(FWDICP)/fwdicptt.o
	cp $(FWDICP)/fwdicptt.o $(BOOTD)/fwdtt

$(BOOTD)/fwdddl: $(FWDICP)/fwdicpddl.o
	cp $(FWDICP)/fwdicpddl.o $(BOOTD)/fwdddl

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

