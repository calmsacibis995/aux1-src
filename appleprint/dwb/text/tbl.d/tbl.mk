#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.2 of tbl.mk on 87/11/11 22:14:36
#	text Development Support Library (DSL) tbl make file
#
# DSL 2

OL = $(ROOT)/
LDFLAGS = -s
INS = :
INSDIR = $(OL)usr/bin
CFLAGS = -O
SFILES = t..c t[0-9].c t[bcefgimrstuv].c
OFILES = t0.o t1.o t2.o t3.o t4.o t5.o t6.o t7.o t8.o t9.o tb.o tc.o\
	te.o tf.o tg.o ti.o tm.o tr.o ts.o tt.o tu.o tv.o
IFLAG = -n
MAKE = make 
SHELL=/bin/sh

compile all: tbl
	:

tbl:	$(OFILES) 
	$(CC) $(IFLAG) $(CFLAGS) $(LDFLAGS) -o tbl $(OFILES)
	
$(OFILES):: t..c
	:

install: tbl
	cp tbl $(INSDIR)
	cd $(INSDIR); chmod 755 tbl; $(CH) chgrp bin tbl; chown bin tbl

clean:
	rm -f *.o

clobber: clean
	rm -f tbl
