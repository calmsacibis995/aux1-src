# transcript/src/Makefile.sysv
#
# Copyright (C) 1985 Adobe Systems Incorporated
#
# RCSID: $Header: Makefile.sysv,v 2.1 85/11/24 11:48:47 shore Rel $
# @(#)Copyright Apple Computer 1987\tVersion 1.1 of Makefile on 87/05/04 19:05:54

LPROGS = map psbanner pscomm psrv pstext
BPROGS = enscript psdit # ps4014 ps630 pscat pscatmap psplot psrev

OBJECTS = map.o psbanner.o pscomm.o psrv.o pstext.o \
	enscript.o ps4014.o ps630.o pscat.o pscatmap.o \
	psdit.o psplot.o psrev.o \
	mapname.o psutil.o

CFLAGS = -O -DSYSV -n
SHELL=/bin/sh
TOP=$(ROOT)/
INSLIBDIR=$(TOP)usr/lib/ps
INSBINDIR=$(TOP)usr/bin


programs all:	${LPROGS} ${BPROGS}

enscript: enscript.o psutil.o mapname.o config.o
	cc ${CFLAGS} -o enscript enscript.o psutil.o mapname.o config.o

pscat: pscat.o psutil.o config.o 
	cc ${CFLAGS} -o pscat pscat.o psutil.o config.o 

psbanner: psbanner.o psutil.o config.o
	cc ${CFLAGS} -o psbanner psbanner.o psutil.o config.o

pscomm: pscomm.o psutil.o config.o
	cc ${CFLAGS} -o pscomm pscomm.o psutil.o config.o

pstext: pstext.o psutil.o config.o
	cc ${CFLAGS} -o pstext pstext.o psutil.o config.o

psplot: psplot.o psutil.o config.o
	cc ${CFLAGS} -o psplot psplot.o psutil.o config.o

psrv: psrv.o psutil.o config.o
	cc ${CFLAGS} -o psrv psrv.o psutil.o config.o

psrev: psrev.o psutil.o config.o
	cc ${CFLAGS} -o psrev psrev.o psutil.o config.o

ps630: ps630.o psutil.o config.o
	cc ${CFLAGS} -o ps630 ps630.o psutil.o config.o

ps4014: ps4014.o psutil.o config.o
	cc ${CFLAGS} -o ps4014 ps4014.o psutil.o config.o

pscatmap: pscatmap.o psutil.o config.o mapname.o
	cc ${CFLAGS} -o pscatmap pscatmap.o psutil.o config.o mapname.o

psdit: psdit.o psutil.o config.o
	cc ${CFLAGS} -o psdit psdit.o psutil.o config.o 

map: map.o mapname.o psutil.o config.o
	cc ${CFLAGS} -o map map.o mapname.o psutil.o config.o

${OBJECTS}: transcript.h
pscomm.o pscat.o psrv.o: psspool.h
pscat.o pscatmap.o: action.h 
psdit.o: dev.h

config.c: config.proto ../config
	-rm -f - config.c
	sed	-e s,PSLIBDIR,$$PSLIBDIR,g \
		-e s,TROFFFONTDIR,$$TROFFFONTDIR,g \
		-e s,DITDIR,$$DITDIR,g \
		-e s,PSTEMPDIR,$$PSTEMPDIR,g \
		config.proto >config.c

install: ${LPROGS} ${BPROGS}
	-strip ${LPROGS} ${BPROGS}
	-cd $(INSLIBDIR); rm -f ${LPROGS} ${BPROGS}
	-cd $(INSBINDIR); rm -f ${BPROGS}
	cp ${BPROGS} $(INSBINDIR)
	cd $(INSBINDIR); chown $$OWNER ${BPROGS}; chgrp $$GROUP ${BPROGS}; \
		chmod 755 ${BPROGS}
	cp ${LPROGS} $(INSLIBDIR)
	cd $(INSLIBDIR); chown $$OWNER ${LPROGS}; chgrp $$GROUP ${LPROGS}; \
		chmod 755 ${LPROGS}
	cd $(INSBINDIR) ; $$LINK ${BPROGS} $(INSLIBDIR)

clean:
	rm -f - *.o config.c ${BPROGS} ${LPROGS} *BAK *CKP .emacs_[0-9]*
