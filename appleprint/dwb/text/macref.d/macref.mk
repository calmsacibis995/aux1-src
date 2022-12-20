#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.2 of macref.mk on 87/11/11 21:54:07
#	text subsystem macref make file
#
# DSL 2

OL = $(ROOT)/
INS = :
INSDIR = $(OL)usr/bin
IFLAG = 
LDFLAGS = -s -n
SFILES = 
FFILES = macref.c macrform.c macrstat.c macrtoc.c main.c match.c
FILES = macref.o macrform.o macrstat.o macrtoc.o main.o match.o
MAKE = make
SHELL=/bin/sh

compile all:  macref
	:

macref:	$(FILES)
	$(CC) $(LDFLAGS) $(IFLAG) -o macref $(FILES) 

install: all
	cp macref $(INSDIR)
	cd $(INSDIR); chmod 755 macref; $(CH) chgrp bin macref; chown bin macref

clean:
	rm -f $(FILES)
clobber:  clean 
		rm -f macref
