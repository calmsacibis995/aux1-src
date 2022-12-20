#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.1 of macros.mk on 87/05/04 13:43:10
#	text Development Support Library (DSL) macros make file
#
# DSL 2

OL = $(ROOT)/
LIST = lp
INS = :
MINSLIB = $(OL)usr/lib/macros
TINSLIB = $(OL)usr/lib/tmac
TMACFILES = tmac.an tmac.m tmac.ptx tmac.v
MAKE = make
SHELL=/bin/sh

compile all:	mmn mmt vmca ptx man

mmn:	mmn.src strings.mm
	sh ./macrunch mmn

strings.mm:	strings.mm.src
	sh ./macrunch strings.mm

mmt:	mmt.src strings.mm
	sh ./macrunch mmt

vmca:	vmca.src
	sh ./macrunch vmca

man:	an.src
	sh ./macrunch an

ptx:	ptx.src
	sh ./macrunch ptx

macdir:
	if [ ! -d $(MINSLIB) ]; then rm -f $(MINSLIB); \
		mkdir $(MINSLIB);  chmod 755 $(MINSLIB);  fi

tmac:	$(TMACFILES)
	if [ ! -d $(TINSLIB) ]; then rm -f $(TINSLIB); \
		mkdir $(TINSLIB);  chmod 755 $(TINSLIB);  fi 
	cp $(TMACFILES) $(TINSLIB)
	cd $(TINSLIB); chmod 644 $(TMACFILES); $(CH) chgrp bin $(TMACFILES); chown bin $(TMACFILES)

listing:    listmmn listmmt listvmca listman listptx

listmmn: ;  nl -ba mmn.src | pr -h "mmn.src" | $(LIST)
	    macref -s -t mmn.src | pr -h "macref of mmn.src" | $(LIST)
listmmt: ;  nl -ba mmt.src | pr -h "mmt.src" | $(LIST)
	    macref -s -t mmt.src | pr -h "macref of mmt.src" | $(LIST)
listvmca: ; nl -ba vmca.src | pr -h "vmca.src" | $(LIST)
	    macref -s -t vmca.src | pr -h "macref of vmca.src" | $(LIST)
listman: ;  nl -ba an.src | pr -h "an.src" | $(LIST)
	    macref -s -t an.src | pr -h "macref of an.src" | $(LIST)
listptx: ;  nl -ba ptx.src | pr -h "ptx.src" | $(LIST)
	    macref -s -t ptx.src | pr -h "macref of ptx.src" | $(LIST)

install:	all macdir tmac
	cp mmn $(MINSLIB)
	cd $(MINSLIB); chmod 644 mmn; $(CH) chgrp bin mmn; chown bin mmn
	cp strings.mm $(MINSLIB)
	cd $(MINSLIB); chmod 644 strings.mm; $(CH) chgrp bin strings.mm; chown bin strings.mm
	cp mmt $(MINSLIB)
	cd $(MINSLIB); chmod 644 mmt; $(CH) chgrp bin mmt; chown bin mmt
	cp vmca $(MINSLIB)
	cd $(MINSLIB); chmod 644 vmca; $(CH) chgrp bin vmca; chown bin vmca
	cp an $(MINSLIB)
	cd $(MINSLIB); chmod 644 an; $(CH) chgrp bin an; chown bin an
	cp ptx $(MINSLIB)
	cd $(MINSLIB); chmod 644 ptx; $(CH) chgrp bin ptx; chown bin ptx


insmmn:  ;  $(MAKE) -f macros.mk INS=cp ROOT=$(ROOT) CH=$(CH) mmn
insmmt:  ;  $(MAKE) -f macros.mk INS=cp ROOT=$(ROOT) CH=$(CH) mmt
insvmca: ;  $(MAKE) -f macros.mk INS=cp ROOT=$(ROOT) CH=$(CH) vmca
insman: ;  $(MAKE) -f macros.mk INS=cp ROOT=$(ROOT) CH=$(CH) man
insptx:  ;  $(MAKE) -f macros.mk INS=cp ROOT=$(ROOT) CH=$(CH) ptx
insstrings.mm:	;  $(MAKE) -f macros.mk INS=cp ROOT=$(ROOT) CH=$(CH) strings.mm
instmac: ;  $(MAKE) -f macros.mk INS=cp ROOT=$(ROOT) CH=$(CH) tmac

clean mmnclean mmtclean vmcaclean ptxclean stringsclean:

clobber:  clean mmnclobber mmtclobber vmcaclobber \
		ptxclobber stringsclobber manclobber

mmnclobber:  ;  rm -f mmn
mmtclobber:  ;  rm -f mmt
vmcaclobber: ;  rm -f vmca
ptxclobber:  ;  rm -f ptx
manclobber:  ;  rm -f an
stringsclobber:	;  rm -f strings.mm
