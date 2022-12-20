# @(#)Copyright Apple Computer 1987\tVersion 1.4 of Makefile on 87/12/02 12:00:57
OL	= $(ROOT)/
INSDIR	= $(OL)usr/lib/font/deviw
FILES	= Adobe.map Apple.map B B.out BI BI.out C C.out CB CB.out DESC \
	  DESC.out H H.out HB HB.out I I.out IWFONTS  R R.out \
	  S S.out S1 S1.out Symbol.map iw.desc special.map special
GFILES  = B B.out BI BI.out C C.out CB CB.out DESC DESC.out \
	  H H.out HB HB.out I I.out R R.out S S.out S1 S1.out
SHELL	= /bin/sh

all:
	$(OL)usr/bin/iwprep iw.desc
	$(OL)usr/bin/makedev DESC

install:	all
	-mkdir $(OL)usr/lib/font
	-mkdir $(INSDIR)
	-cp $(FILES) $(INSDIR)/
	-cd $(INSDIR); chown bin $(FILES); chgrp bin $(FILES)

clean: 
	rm -f $(GFILES)

clobber: clean
