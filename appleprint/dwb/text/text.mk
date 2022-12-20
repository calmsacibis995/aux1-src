#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.6 of text.mk on 87/11/11 21:49:25
#	text sub-system make file
#
# DSL 2.


OL = $(ROOT)/
ARGS = all
INSDIR = $(OL)usr/bin
LDFLAGS = -s -n
INS = :
MAKE = make
SHELL=/bin/sh

compile all:	 notices nroff terms troff aps macros shells eqn \
		 neqn tbl checkmm pic macref \
		 ptx grap subndx diffmk hyphen col
	:

notices:	;	@echo ""
	@echo ""
	@echo ""
	@echo "			Copyright (c) 1984 AT&T"
	@echo "			  All Rights Reserved"
	@echo ""
	@echo "   THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T"
	@echo "The copyright notice above does not evidence any actual or"
	@echo "intended publication of such source code."
	@echo ""
	@echo ""
	@echo""


nroff:  ;  cd roff.d/nroff.d; $(MAKE) -f nroff.mk nroff INCORE=-DINCORE \
		ROOT=$(ROOT) CH=$(CH) $(ARGS)

terms:  ;  cd roff.d/nroff.d/terms.d; $(MAKE) -f terms.mk \
		ROOT=$(ROOT) CH=$(CH) $(ARGS)

troff:  ;  cd roff.d/troff.d; $(MAKE) -f troff.mk troff INCORE=-DINCORE \
		ROOT=$(ROOT) CH=$(CH) $(ARGS)

aps:  	;  cd roff.d/troff.d/devaps; $(MAKE) -f devaps.mk \
		ROOT=$(ROOT) CH=$(CH) $(ARGS)

macros:	;  cd macros.d;  $(MAKE) -f macros.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

shells:	;  cd shells.d;  $(MAKE) -f shells.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

eqn:	;  cd eqn.d;  $(MAKE) -f eqn.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

neqn:	;  cd neqn.d; $(MAKE) -f neqn.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

tbl:	;  cd tbl.d;  $(MAKE) -f tbl.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

checkmm: ;  cd checkmm.d; $(MAKE) -f checkmm.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

pic: ;  cd pic.d; $(MAKE) -f pic.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

macref: ;  cd macref.d; $(MAKE) -f macref.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

ptx:	;  cd ptx.d; $(MAKE) -f ptx.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

grap:	;  cd grap.d; $(MAKE) -f grap.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

subndx:	;  cd subndx.d; $(MAKE) -f subndx.mk ROOT=$(ROOT) CH=$(CH) $(ARGS)

diffmk:	diffmk.sh
	cp diffmk.sh diffmk

hyphen:	hyphen.c
	$(CC) -O $(LDFLAGS) hyphen.c -o hyphen

col:	col.c
	$(CC) -O $(LDFLAGS) col.c -o col

install:
	$(MAKE) -f text.mk ARGS=install ROOT=$(ROOT) CH=$(CH) INS=cp $(ARGS)
	cp diffmk $(INSDIR)
	cd $(INSDIR); chmod 775 diffmk; $(CH) chgrp bin diffmk; chown bin diffmk
	cp hyphen $(INSDIR)
	cd $(INSDIR); chmod 775 hyphen; $(CH) chgrp bin hyphen; chown bin hyphen
	cp col $(INSDIR)
	cd $(INSDIR); chmod 775 col; $(CH) chgrp bin col; chown bin col

clean:
	cd roff.d;  $(MAKE) -f roff.mk clean
	cd eqn.d;   $(MAKE) -f eqn.mk clean
	cd neqn.d;  $(MAKE) -f neqn.mk clean
	cd tbl.d;   $(MAKE) -f tbl.mk clean
	cd macros.d; $(MAKE) -f macros.mk clean
	cd shells.d; $(MAKE) -f shells.mk clean
	cd checkmm.d; $(MAKE) -f checkmm.mk clean
	cd pic.d; $(MAKE) -f pic.mk clean
	cd macref.d; $(MAKE) -f macref.mk clean
	cd ptx.d; $(MAKE) -f ptx.mk clean
	cd grap.d; $(MAKE) -f grap.mk clean
	cd subndx.d; $(MAKE) -f subndx.mk clean
	rm -f hyphen.o
	
clobber:
	cd roff.d;  $(MAKE) -f roff.mk clobber
	cd eqn.d;   $(MAKE) -f eqn.mk clobber
	cd neqn.d;  $(MAKE) -f neqn.mk clobber
	cd tbl.d;   $(MAKE) -f tbl.mk clobber
	cd macros.d;  $(MAKE) -f macros.mk clobber
	cd shells.d;  $(MAKE) -f shells.mk clobber
	cd checkmm.d; $(MAKE) -f checkmm.mk clobber
	cd pic.d; $(MAKE) -f pic.mk clobber
	cd macref.d; $(MAKE) -f macref.mk clobber
	cd ptx.d; $(MAKE) -f ptx.mk clobber
	cd grap.d; $(MAKE) -f grap.mk clobber
	cd subndx.d; $(MAKE) -f subndx.mk clobber
	rm -f hyphen diffmk col

