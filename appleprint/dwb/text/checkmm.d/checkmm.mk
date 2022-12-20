#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.2 of checkmm.mk on 87/11/11 21:50:08"
#	checkmm make file
#
# for DSL 2

OL = $(ROOT)/
INS = :
INSDIR = $(OL)usr/bin
SHELL=/bin/sh
B10 =
CFLAGS = -O $(FFLAG) $(B10)
IFLAG = -n
SOURCE = chekl.l  chekmain.c chekrout.c
FILES = chekl.o chekmain.o chekrout.o
SOURCE1 = chekl1.l chekmain1.c chekrout1.c
FILES1 = chekl1.o chekmain1.o chekrout1.o
MAKE = make

compile all: checkmm1 checkmm
	:

checkmm1:	$(FILES1)
	 $(CC) -s $(B10) $(IFLAG) -o checkmm1 $(FILES1) -ll -lPW
checkmm:	$(FILES)
	$(CC) -s  $(B10) $(IFLAG) -o checkmm $(FILES) -ll -lPW

install: all
	cp checkmm $(INSDIR)
	cd $(INSDIR); chmod 755 checkmm; $(CH) chgrp bin checkmm; chown bin checkmm
	cp checkmm1 $(INSDIR)
	cd $(INSDIR); chmod 755 checkmm1; $(CH) chgrp bin checkmm1; chown bin checkmm1

clean:
	  rm -f $(FILES) $(FILES1)

clobber:  clean
	  rm -f checkmm checkmm1
