#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.3 of roff.mk on 87/11/11 22:09:52
#	nroff/troff make file (text subsystem)
#
# DSL 2

CFLAGS = -O
LDFLAGS = -s -n
INS = :
MAKE = make
INCORE = -DINCORE
SHELL=/bin/sh

compile all:  nroff terms troff fonts

nroff:
	cd nroff.d;   $(MAKE) -f nroff.mk nroff INS=$(INS) ROOT=$(ROOT) \
		INCORE=$(INCORE) CH=$(CH) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"

troff:
	cd troff.d;   $(MAKE) -f troff.mk troff INS=$(INS) ROOT=$(ROOT) \
		INCORE=$(INCORE) CH=$(CH) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"

terms:
	cd nroff.d;  $(MAKE) -f nroff.mk terms INS=$(INS) ROOT=$(ROOT) CH=$(CH)
fonts:
	cd troff.d;  $(MAKE) -f troff.mk fonts INS=$(INS) ROOT=$(ROOT) CH=$(CH)

install: 
	cd nroff.d;   $(MAKE) -f nroff.mk INS=$(INS) ROOT=$(ROOT) \
		INCORE=$(INCORE) CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) install
	cd troff.d;   $(MAKE) -f troff.mk INS=$(INS) ROOT=$(ROOT) \
		INCORE=$(INCORE) CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) install

insnroff:
	$(MAKE) -f roff.mk INS=cp ROOT=$(ROOT) CH=$(CH) INCORE=$(INCORE) nroff install
instroff:
	$(MAKE) -f roff.mk INS=cp ROOT=$(ROOT) CH=$(CH) INCORE=$(INCORE) troff install

clean:
	cd nroff.d;  $(MAKE) -f nroff.mk clean
	cd troff.d;  $(MAKE) -f troff.mk clean

clobber:
	cd nroff.d;  $(MAKE) -f nroff.mk clobber
	cd troff.d;  $(MAKE) -f troff.mk clobber
