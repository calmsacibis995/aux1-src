#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.3 of devaps.mk on 87/11/11 22:01:41
#	makefile for aps-5 driver, fonts, etc.
#
# DSL 2

OL = $(ROOT)/
CFLAGS = -O
LDFLAGS = -s -n
INS = :
INSDIR = $(OL)usr/bin
FONTHOME = $(OL)usr/lib/font
FONTDIR = $(OL)usr/lib/font/devaps
MAKEDEV = ./makedev
SHELL=/bin/sh
FFILES = [A-Z] [A-Z][0-9A-Z] DESC
OFILES = [A-Z].[oa][ud][td] [A-Z][0-9A-Z].[oa][ud][td] DESC.out

all:	daps

daps:	daps.o ../draw.o build.o
	$(CC) $(LDFLAGS) $(FFLAG) -o daps daps.o ../draw.o build.o -lm

daps.o:	aps.h ../dev.h daps.h daps.g
	$(CC) $(CFLAGS) -I../ -c daps.c

../draw.o:	../draw.c
	cd ..;  $(MAKE) draw.o

aps_fonts:	makedir $(MAKEDEV)
	$(MAKEDEV) DESC
	for i in $(FFILES); \
	do	if [ ! -r $$i.out ] || [ -n "`find $$i -newer $$i.out -print`" ]; \
		   then	$(MAKEDEV) $$i; \
		fi; \
	done
	-if [ -r LINKFILE ]; then \
	    sh ./LINKFILE; \
	fi
	cp $(OFILES) version $(FONTDIR);
	cd $(FONTDIR); chmod 644 $(OFILES) version;  \
		$(CH) chgrp bin $(OFILES) version; chown bin $(OFILES) version

$(MAKEDEV):	$(MAKEDEV).c ../dev.h
	cc $(CFLAGS) -I../ $(LDFLAGS) -o $(MAKEDEV) $(MAKEDEV).c
makedir:
	if [ ! -d $(FONTHOME) ] ; then rm -f $(FONTHOME);  mkdir $(FONTHOME); \
		chmod 755 $(FONTHOME);  fi
	if [ ! -d $(FONTDIR) ] ; then rm -f $(FONTDIR);  mkdir $(FONTDIR); \
		chmod 755 $(FONTDIR);  fi

install: makedir daps aps_fonts
	cp  daps $(INSDIR)
	cd $(INSDIR); chmod 755 daps; $(CH) chgrp bin daps; chown bin daps

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILES) daps makedev
