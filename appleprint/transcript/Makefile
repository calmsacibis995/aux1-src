# @(#)Copyright Apple Computer 1987\tVersion 1.1 of Makefile on 87/05/04 18:53:55
# transcript/Makefile.sysv
#
# Copyright 1985 (C) Adobe Systems Incorporated
#
# RCSID: $Header: Makefile.sysv,v 2.1 85/11/24 12:34:25 shore Rel $
#
# to install transcript:
#
#	(Once for your system)
#		sysv
#		edit config & printer
#		make programs
#
#		(become super-user)
#		make install		( install files in system dirs )
#
#	(For each PostScript/TranScript printer)
#		cd etc
#		(become super-user)
#		mkprinter PRINTER TTY
#
SHELL=/bin/sh
TOP=$(ROOT)/
INSLIBDIR=$(TOP)usr/lib/ps
INSBINDIR=$(TOP)usr/bin

explain:
	@cat doc/make.notes

programs:
	. ./config; cd lib; $(MAKE) ${MFLAGS} programs
#	. ./config; cd man; $(MAKE) ${MFLAGS} programs
	. ./config; cd sh;  $(MAKE) ${MFLAGS} programs
	. ./config; cd src; $(MAKE) ${MFLAGS} programs

install:
#	-. ./config; mv $(INSLIBDIR) $(INSLIBDIR).old
	-. ./config; mkdir $(INSLIBDIR); chown $$OWNER $(INSLIBDIR); \
		chgrp $$GROUP $(INSLIBDIR); chmod 755 $(INSLIBDIR)
	. ./config; cd src; $(MAKE) ${MFLAGS} ROOT=$(ROOT) install 
	. ./config; cd sh;  $(MAKE) ${MFLAGS} ROOT=$(ROOT) install
	. ./config; cd lib; $(MAKE) ${MFLAGS} ROOT=$(ROOT) install
#	. ./config; cd man; $(MAKE) ${MFLAGS} ROOT=$(ROOT) install

clean:
	rm -f *BAK *CKP .emacs_[0-9]*

clobber: clean
	cd etc; $(MAKE) ${MFLAGS} clean
	cd lib; $(MAKE) ${MFLAGS} clean
	cd man; $(MAKE) ${MFLAGS} clean
	cd  sh; $(MAKE) ${MFLAGS} clean
	cd src; $(MAKE) ${MFLAGS} clean
