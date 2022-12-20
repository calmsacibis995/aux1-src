#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.1 of shells.mk on 87/05/04 14:26:41
#	text subsystem shells make file
#
# DSL 2.

OL = $(ROOT)/
INS = :
INSDIR = $(OL)usr/bin
INSLIB = $(OL)usr/lib
HINSDIR = $(OL)usr/pub
FILES =  mm.sh mmt.sh 
SFILES = nroff.letter mm.report mm.sales mm.letter tbl.language tbl.bridges \
	tbl.pres eqn.stats troff.fonts troff.sizes troff.ad \
	troff.aeneid pic.forms
LDFLAGS = -s
MAKE = make
SHELL=/bin/sh

compile all:  mm mmt mvt samples
	:

mm:	mm.sh
	cp mm.sh mm

mmt:	mmt.sh
	cp mmt.sh mmt

mvt:	mmt

helpdir:
	if [ ! -d $(HINSDIR) ] ; then rm -f $(HINSDIR);  \
		mkdir $(HINSDIR);  chmod 755 $(HINSDIR);  fi

termh:	

samples: $(SFILES)

install: all helpdir
	cp mm $(INSDIR)
	cd $(INSDIR); chmod 755 mm; $(CH) chgrp bin mm; chown bin mm
	cp mmt $(INSDIR)
	cd $(INSDIR); chmod 755 mmt; $(CH) chgrp bin mmt; chown bin mmt
	rm -f $(INSDIR)/mvt
	ln $(INSDIR)/mmt $(INSDIR)/mvt
	cd $(INSDIR); chmod 755 mvt; $(CH) chgrp bin mvt; chown bin mvt
	cp terminals $(HINSDIR)
	cd ${HINSDIR}; chmod 664 terminals; $(CH) chgrp bin terminals; chown bin terminals
	if [ ! -d $(INSLIB)/dwb ] ; then rm -f $(INSLIB)/dwb;  \
		mkdir $(INSLIB)/dwb;  chmod 755 $(INSLIB)/dwb;  fi
	if [ ! -d $(INSLIB)/dwb/samples ] ;  \
		then rm -f $(INSLIB)/dwb/samples; \
		mkdir $(INSLIB)/dwb/samples;  chmod 755 $(INSLIB)/dwb/samples;  fi
	cp $(SFILES) $(INSLIB)/dwb/samples 
	cd $(INSLIB)/dwb/samples; chmod 644 $(SFILES); \
		$(CH) chgrp bin $(SFILES); chown bin $(SFILES)

clean mmclean mmtclean mvtclean:
clobber:  clean mmclobber mmtclobber
	:
mmclobber:   ;  rm -f mm
mmtclobber mvtclobber:  ;  rm -f mmt
