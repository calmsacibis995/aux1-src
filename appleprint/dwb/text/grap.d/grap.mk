#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.2 of grap.mk on 87/11/11 21:53:08
#	makefile for grap.
#


OL = $(ROOT)/
# ALLOC = malloc.o
YFLAGS = -d -D
OFILES = main.o input.o print.o frame.o for.o coord.o ticks.o plot.o label.o misc.o $(ALLOC)
CFILES = main.c input.c print.c frame.c for.c coord.c ticks.c plot.c label.c misc.c
SRCFILES = grap.y grapl.l grap.h $(CFILES)
INS = :
SHELL=/bin/sh
INSDIR = $(OL)usr/bin
LIBDIR = $(OL)usr/lib/dwb
IFLAG = 
LDFLAGS = -s -n

all:	grap

grap:	grap.o grapl.o $(OFILES) grap.h 
	$(CC) -o grap $(IFLAG) $(FFLAG) $(LDFLAGS) grap.o grapl.o $(OFILES) -lm

$(LIBDIR)/grap.defines:	grap.defines
	if [ ! -d $(LIBDIR) ]; then rm -f $(LIBDIR); mkdir $(LIBDIR); \
		chmod 755 $(LIBDIR);  fi
	cp grap.defines $(LIBDIR)
	cd $(LIBDIR); chmod 644 grap.defines;
	$(CH) chgrp bin grap.defines; chown bin grap.defines

$(OFILES) grapl.o:	grap.h prevy.tab.h

grap.o:	grap.h

y.tab.h:	grap.o

prevy.tab.h:	y.tab.h
	-cmp -s y.tab.h prevy.tab.h || cp y.tab.h prevy.tab.h

install: $(LIBDIR)/grap.defines grap
	cp grap $(INSDIR)
	cd $(INSDIR); chmod 755 grap; $(CH) chgrp bin grap; chown bin grap

clean:
	rm -f grap.o grapl.o $(OFILES) y.tab.h prevy.tab.h

clobber:	clean
	rm -f grap
