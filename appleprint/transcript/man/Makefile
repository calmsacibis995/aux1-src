# transcript/man/Makefile
#
# Copyright 1985 Adobe Systems Incorporated
#
# @(#)Copyright Apple Computer 1987\tVersion 1.3 of Makefile on 87/07/28 19:18:33
# RCSID: $Header: Makefile,v 2.1 85/11/24 12:38:23 shore Rel $
# RCSLOG:
# $Log:	Makefile,v $
# Revision 2.1  85/11/24  12:38:23  shore
# Product Release 2.0
# 
# 
#

.SUFFIXES: .1 .1p .5 .5p .5 .5p

SHELL=/bin/sh
TOP=$(ROOT)/
INSMAN1DIR=$(TOP)$$MAN1
INSMAN5DIR=$(TOP)$$MAN5

PAGES1 = enscript.1 ps4014.1 ps630.1 pscat.1 psdit.1 psplot.1 psrev.1 \
	psroff.1 ptroff.1
PAGES5 = afm.5 postscript.5 pscatmap.5 transcript.5

PAGES = ${PAGES1} ${PAGES5}

programs all: ${PAGES}

.1p.1 .5p.5:
	sed	-e s,XPSLIBDIRX,$$PSLIBDIR,g \
		-e s,XTROFFFONTDIRX,$$TROFFFONTDIR,g \
		-e s,XPSTEMPDIR,$$PSTEMPDIR,g \
		-e s,XDITDIRX,$$DITDIR,g \
		$? > $@

install: ${PAGES}
	cd $(INSMAN1DIR); rm -f $(PAGES1)
	cd $(INSMAN5DIR); rm -f $(PAGES5)
	./installman 1 $(INSMAN1DIR)
	./installman 5 $(INSMAN5DIR)

clean:
	rm -f - ${PAGES} *.BAK *.CKP .emacs_[0-9]*
