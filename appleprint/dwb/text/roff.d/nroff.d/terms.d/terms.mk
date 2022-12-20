#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# @(#)Copyright Apple Computer 1987\tVersion 1.1 of terms.mk on 87/05/04 13:54:03
#	nroff terminal driving tables make file
#
# DSL 2.

OL = $(ROOT)/
INS = :
SHELL=/bin/sh
INSDIR = ${OL}usr/lib/nterm
FILES = tab.8510 tab.2631 tab.2631-c tab.2631-e tab.300 tab.300-12 tab.300s \
	tab.300s-12 tab.37 tab.382 tab.4000a tab.450 \
	tab.450-12 tab.832 tab.lp tab.tn300 tab.X
IFILES = $(FILES) tab.300S tab.300S-12 tab.4000A

all:	$(FILES)

install: all
	if [ ! -d $(INSDIR) ]; then rm -f $(INSDIR);  mkdir $(INSDIR); \
		chmod 755 $(INSDIR);  fi
	cp ${FILES} ${INSDIR}
	cd ${INSDIR}; rm -f tab.300S tab.300S-12 tab.4000A; \
			ln tab.300s tab.300S; ln tab.300s-12 tab.300S-12; \
			ln tab.4000a tab.4000A; \
	cd ${INSDIR}; chmod 644 $(IFILES); \
		$(CH) chgrp bin $(IFILES); chown bin $(IFILES)

tab.2631:	a.2631 b.lp
	cat a.2631 b.lp >tab.2631
tab.2631-c:	a.2631-c b.lp
	cat a.2631-c b.lp >tab.2631-c
tab.2631-e:	a.2631-e b.lp
	cat a.2631-e b.lp >tab.2631-e
tab.300:	a.300 b.300
	cat a.300 b.300 >tab.300
tab.300-12:	a.300-12 b.300
	cat a.300-12 b.300 >tab.300-12
tab.300s:	a.300s b.300
	cat a.300s b.300 >tab.300s
tab.300s-12:	a.300s-12 b.300
	cat a.300s-12 b.300 >tab.300s-12
tab.37:	ab.37
	cat ab.37 >tab.37
tab.382:	a.382 b.300
	cat a.382 b.300 >tab.382
tab.4000a:	a.4000a b.300
	cat a.4000a b.300 >tab.4000a
tab.450:	a.450 b.300
	cat a.450 b.300 >tab.450
tab.450-12:	a.450-12 b.300
	cat a.450-12 b.300 >tab.450-12
tab.832:	a.832 b.300
	cat a.832 b.300 >tab.832
tab.8510:	ab.8510
	cat ab.8510 >tab.8510
tab.X:	ab.X
	cat ab.X >tab.X
tab.lp:	a.lp b.lp
	cat a.lp b.lp >tab.lp
tab.tn300:	ab.tn300
	cat ab.tn300 >tab.tn300

clean:
	rm -f maketerms

clobber:  clean
	rm -f ${FILES}
