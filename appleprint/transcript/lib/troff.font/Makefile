# transcript/lib/troff.font/Makefile.sysv
#
# Copyright (C) 1985 Adobe Systems Incorporated
#
# @(#)Copyright Apple Computer 1987\tVersion 1.1 of Makefile on 87/05/04 19:02:47
#
# Makefile for troff width tables and pscat correspondence tables
#
# See pscatmap(8) and the .map files for more information.
#
# If you add a font family to your PostScript/troff database,
# add it to the FAMILIES line in this makefile, then do a "make".

.SUFFIXES: .ct .map
SHELL=/bin/sh
TOP=$(ROOT)/
INSLIBDIR=$(TOP)usr/lib/ps
INSBINDIR=$(TOP)usr/bin
INSFONTDIR=$(TOP)usr/lib/font/ps

#######################################################################
# Here are the family names, the face names are gotten from the .map files

FAMILIES = Times.ct Helvetica.ct

#######################################################################

install: ${FAMILIES}
	-mkdir $(INSFONTDIR)
	-cp ft* *.ct *.head $(INSFONTDIR)
	cd $(INSFONTDIR) ; chown $$OWNER . * ; chgrp $$GROUP . * ; \
		chmod 755 . ; chmod 644 * ; 

.map.ct:
	../pscatmap $*.map
	awk -f head.awk $*.map >$*.head

clean:
	rm -f - *.ct *.head *.c *.o ft? ft?? *BAK *CKP .emacs_[0-9]*
