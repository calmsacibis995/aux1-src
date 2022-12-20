# transcript/sh/Makefile
#
# Copyright 1985 Adobe Systems Incorporated
#
# @(#)Copyright Apple Computer 1987\tVersion 1.1 of Makefile on 87/05/04 19:05:24
# RCSID: $Header: Makefile,v 2.1 85/11/24 12:31:56 shore Rel $
# RCSLOG:
# $Log:	Makefile,v $
# Revision 2.1  85/11/24  12:31:56  shore
# Product Release 2.0
# 
#
SHELL=/bin/sh
TOP=$(ROOT)/
INSLIBDIR=$(TOP)usr/lib/ps
INSBINDIR=$(TOP)usr/bin

programs all: psroff # ptroff 

ptroff: ptroff.sh
	sed	-e s,TROFFFONTDIR,$$TROFFFONTDIR,g \
		ptroff.sh >ptroff

psroff: psroff.sh
	cp psroff.sh psroff

install: psroff # ptroff
	cp psroff $(INSBINDIR)
#	cp ptroff $(INSBINDIR)
	chown $$OWNER $(INSBINDIR)/psroff
#	chown $$OWNER $(INSBINDIR)/ptroff
	chgrp $$GROUP $(INSBINDIR)/psroff
#	chgrp $$GROUP $(INSBINDIR)/ptroff
	chmod 755 $(INSBINDIR)/psroff
#	chmod 755 $(INSBINDIR)/ptroff 

clean:
	-rm -f - ptroff psroff *.BAK *.CKP .emacs_[0-9]*
