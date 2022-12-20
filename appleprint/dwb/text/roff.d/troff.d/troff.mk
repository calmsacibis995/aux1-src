#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.3 of troff.mk on 87/11/11 22:08:11
#	makefile for (di) troff.  Also builds subproducts - typesetter
#		drivers, fonts, rasters, etc.
#
# DSL 2.

OL = $(ROOT)/
CFLAGS = -O
INCORE = -DINCORE
USG = -DUSG
LDFLAGS = -s -n
IFLAG = 
CFILES=n1.c n2.c n3.c n4.c n5.c t6.c n7.c n8.c n9.c t10.c ni.c nii.c hytab.c suftab.c
HFILES=../tdef.h ../ext.h dev.h
TFILES=n1.o n2.o n3.o n4.o n5.o t6.o n7.o n8.o n9.o t10.o ni.o nii.o hytab.o suftab.o
INS = :
INSDIR = $(OL)usr/bin
BINDIR = $(OL)bin
SHELL=/bin/sh

all:	troff fonts makedev

troff:	$(TFILES)
	$(CC) $(LDFLAGS) -o troff $(IFLAG) $(TFILES) 

n1.o:	../n1.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -c ../n1.c
n2.o:	../n2.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -c ../n2.c
n3.o:	../n3.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -c ../n3.c
n4.o:	../n4.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -c ../n4.c
n5.o:	../n5.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(USG) $(INCORE) -c ../n5.c
t6.o:	t6.c ../tdef.h dev.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -I../ -c t6.c
n7.o:	../n7.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -c ../n7.c
n8.o:	../n8.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -c ../n8.c
n9.o:	../n9.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -c ../n9.c
t10.o:	t10.c ../tdef.h dev.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -I../ -c t10.c
ni.o:	../ni.c ../tdef.h
	$(CC) $(CFLAGS) $(INCORE) -c ../ni.c
nii.o:	../nii.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(INCORE) -c ../nii.c
hytab.o:	../hytab.c
	$(CC) $(CFLAGS) $(INCORE) -c ../hytab.c
suftab.o:	../suftab.c
	$(CC) $(CFLAGS) $(INCORE) -c ../suftab.c

fonts:	# tc aps i10

tc:	tc.o draw.o
	$(CC) $(LDFLAGS) $(FFLAG) -o tc tc.o draw.o -lm

hc:	hc.o draw.o
	$(CC) $(LDFLAGS) $(FFLAG) -o hc hc.o draw.o -lm

ta:	ta.o draw.o
	$(CC) $(LDFLAGS) $(FFLAG) -o ta ta.o draw.o -lm

tc.o:	dev.h
hc.o:	dev.h
ta.o:	dev.h

aps:	draw.o makedev
	cd devaps;  $(MAKE) -f devaps.mk INS=$(INS) ROOT=$(ROOT) CH=$(CH) \
		CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"

i10:	draw.o makedev
	cd devi10;  $(MAKE) -f devi10.mk INS=$(INS) ROOT=$(ROOT) CH=$(CH) \
		CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS)

makedev:	makedev.c dev.h
	cc $(LDFLAGS) -o makedev makedev.c

Dtroff:
	$(MAKE) -f troff.mk troff CFLAGS="$(CFLAGS) -g -DDEBUG" \
		INCORE=$(INCORE) LDFLAGS=-n INS=: CH=#
	mv troff Dtroff

install:	all tc
	cp tc $(INSDIR)
	$(CH) cd $(INSDIR); chmod 755 tc; chgrp bin tc; chown bin tc
#	cd devaps;  $(MAKE) -f devaps.mk INS=$(INS) ROOT=$(ROOT) CH=$(CH) \
#		CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" install
#	cp hc $(INSDIR)
#	$(CH) cd $(INSDIR); chmod 755 hc; chgrp bin hc; chown bin hc
#	cp ta $(INSDIR)
#	$(CH) cd $(INSDIR); chmod 755 ta; chgrp bin ta; chown bin ta
	cp makedev $(INSDIR)
	$(CH) cd $(INSDIR); chmod 755 makedev; chgrp bin makedev; chown bin makedev
	cp troff $(BINDIR)
	$(CH) cd $(BINDIR); chmod 755 troff; chgrp bin troff; chown bin troff

clean:	hcclean taclean tcclean
	rm -f $(TFILES) draw.o
	cd devaps;  $(MAKE) -f devaps.mk clean
#	cd devi10;  $(MAKE) -f devi10.mk clean

hcclean:  ;  rm -f hc.o
taclean:  ;  rm -f ta.o
tcclean:  ;  rm -f tc.o

clobber:	hcclobber taclobber tcclobber
	rm -f $(TFILES) draw.o
	rm -f troff tc makedev
	cd devaps;  $(MAKE) -f devaps.mk clobber
#	cd devi10;  $(MAKE) -f devi10.mk clobber

hcclobber:	hcclean
	rm -f hc
taclobber:	taclean
	rm -f ta
tcclobber:	tcclean
	rm -f tc
