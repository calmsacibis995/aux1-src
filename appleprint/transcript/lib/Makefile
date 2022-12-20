# transcript/lib/Makefile.sysv
#
# Copyright (c) 1985 Adobe Systems Incorporated. All Rights Reserved. 
#
# RCSID: $Header: Makefile.sysv,v 2.1 85/11/24 12:19:15 shore Rel $
# @(#)Copyright Apple Computer 1987\tVersion 1.1 of Makefile on 87/05/04 18:59:32

SHELL=/bin/sh
TOP=$(ROOT)/
INSLIBDIR=$(TOP)usr/lib/ps
INSBINDIR=$(TOP)usr/bin
FILES = banner.pro enscript.pro pstext.pro \
	ps4014.pro ps630.pro ps4014.pro pscat.pro psplot.pro psdit.pro \
	ehandler.ps uartpatch.ps bogusmsg.ps \
	font.map

programs all:

install: 
	cd $(INSLIBDIR); rm -rf - ${FILES} *.afm
	-cp ${FILES} *.afm $(INSLIBDIR)
	cd $(INSLIBDIR); chown $$OWNER ${FILES} *.afm; \
		chgrp $$GROUP ${FILES} *.afm; chmod 644 ${FILES} *.afm
#	-rm -rf $(INSLIBDIR)/troff.font
	-rm -rf $(INSLIBDIR)/ditroff.font
#	-mkdir $(INSLIBDIR)/troff.font
	-mkdir $(INSLIBDIR)/ditroff.font
#	-cp troff.font/Helvetica.map $(INSLIBDIR)/troff.font/
#	-cp troff.font/Makefile.bsd $(INSLIBDIR)/troff.font/
#	-cp troff.font/doto.awk $(INSLIBDIR)/troff.font/
#	-cp troff.font/Makefile $(INSLIBDIR)/troff.font/
#	-cp troff.font/Times.map $(INSLIBDIR)/troff.font/
#	-cp troff.font/head.awk $(INSLIBDIR)/troff.font/
#	cd $(INSLIBDIR)/troff.font; make ${MFLAGS} ROOT=$(ROOT) install 
#	cd $(INSLIBDIR)/troff.font; chown $$OWNER . * ; chgrp $$GROUP . *
	-cp ditroff.font/B.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/CD.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/HD.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/R.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/SS.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/devspecs $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/BI.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/CO.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/HO.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/README $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/afmdit $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/C.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/H.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/I.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/S.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/afmdit.bsd $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/afmdit.awk $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/CB.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/HB.map $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/Makefile $(INSLIBDIR)/ditroff.font/
	-cp ditroff.font/charset $(INSLIBDIR)/ditroff.font/
	-cd $(INSLIBDIR)/ditroff.font; make ROOT=$(ROOT) ${MFLAGS} install
	-cd $(INSLIBDIR)/ditroff.font; chown $$OWNER . * ; chgrp $$GROUP . *

clean:
	rm -f - *.BAK *.CKP .emacs_[0-9]*
#	cd troff.font; make ${MFLAGS} clean
	cd ditroff.font; make ${MFLAGS} clean
