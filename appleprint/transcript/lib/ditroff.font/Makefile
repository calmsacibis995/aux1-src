# lib/ditroff.font/Makefile
#
# Copyright (c) 1984 Adobe Systems Incorporated. All Rights Reserved.
# PostScript is a trademark of Adobe Systems Incorporated.
# RCSID: $Header: Makefile,v 2.1 85/11/24 12:24:55 shore Rel $
# @(#)Copyright Apple Computer 1987\tVersion 1.1 of Makefile on 87/05/04 19:04:23

# see README for more information
# makedev is the ditroff program that builds device descriptions

SHELL=/bin/sh
TOP=$(ROOT)/
INSLIBDIR=$(TOP)usr/lib/ps
INSBINDIR=$(TOP)usr/bin
INSDITDIR=$(TOP)usr/lib/font
LOADFONTS = R I B BI H HB C CB S SS
MOREFONTS = HO HD CO CD

FONTNAMES = ${LOADFONTS} ${MOREFONTS}
TEMPFILES = temp.header temp.spaces temp.trailer temp.aux

programs:

install: makedev all
	rm -rf $(INSDITDIR)/devpsc
	mkdir $(INSDITDIR)/devpsc
	cp *.aux *.map *.out DESC $(INSDITDIR)/devpsc
	cd $(INSDITDIR)/devpsc ; chown $$OWNER . * ; chgrp $$GROUP . * ; \
		chmod 755 . ; chmod 644 *.aux *.map *.out DESC

makedev:
	test -r "$$MAKEDEV"

all DESC.out: ${LOADFONTS} moreout
	$$MAKEDEV DESC

allfonts: ${FONTNAMES}

moreout: ${MOREFONTS}
	$$MAKEDEV $?

${LOADFONTS}:
	make ${MFLAGS} $@.font

${MOREFONTS}:
	-rm -f $@.font
	make ${MFLAGS} $@.font

${FONTNAMES}: DESC afmdit.awk Makefile

DESC: afmdit.awk Makefile charset devspecs
	rm -f DESC
	echo "# ditroff device description for PostScript" >> DESC
	echo "# PostScript is a trademark of Adobe Systems Incorporated">>DESC
	echo ${LOADFONTS} | awk '{print "fonts", NF, $$0}' >> DESC
	cat devspecs >>DESC
	echo "charset" >> DESC
	cat charset >> DESC

clean:
	rm -f ${FONTNAMES} ${TEMPFILES} core DESC *.out *.font *.aux *.CKP *.BAK .emacs_[0-9]* temp*

.SUFFIXES: .out .font .map

# how to make a .font from a .map
# note that .font is a placeholder,  the file of interest has
# the same name with no extension (this is input to makedev)

.map.font:
	/bin/rm -f - ${TEMPFILES}
	chmod +x ./afmdit
	./afmdit $* ..
	touch $*.font
	/bin/rm -f -  ${TEMPFILES}

# how to make a .out from a .font (again, the .font is dropped)

.font.out: DESC
	$$MAKEDEV $*

# how to make a .out from a .map
# just do the above two steps

.map.out:
	make ${MFLAGS} $@.font
	make ${MFLAGS} $@.out
