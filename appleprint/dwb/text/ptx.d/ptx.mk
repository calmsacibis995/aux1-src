#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.2 of ptx.mk on 87/11/11 21:58:18
#
#

OL = $(ROOT)/
INS = :
INSLIB = $(ROOT)/usr/lib
INSDIR = $(ROOT)/usr/bin
FILES = ptx.c eign.sh
LDFLAGS = -s -n
MAKE = make
SHELL=/bin/sh

all: ptx eign

ptx:	ptx.c
	$(CC) -O $(LDFLAGS) ptx.c -o ptx

eign:	eign.sh
	cp eign.sh eign

install: all
	cp ptx $(INSDIR)
	cd $(INSDIR); chmod 775 ptx; $(CH) chgrp bin ptx; chown bin ptx
	cp eign $(INSLIB)
	cd $(INSLIB); chmod 644 eign; $(CH) chgrp bin eign; chown bin eign

clean:
	rm -f *.o

clobber: clean
	rm  -f eign ptx
