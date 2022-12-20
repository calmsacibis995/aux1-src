# Makefile 6/12/83
LFLAGS=		$(SHARED) $(LDCMDFILE)
OBJS=		looptest.o
DESTDIR=	${DEST}
FIX=		size

.c.o:
		${CC} ${CFLAGS} ${DEFS} ${INCLUDES} -c $*.c

all:            $(DESTDIR)/looptest

${DESTDIR}/looptest:		${OBJS}
#		${CC} ${OBJS} ${LIBS} -o $@ $(LFLAGS)
		${CC} ${OBJS} ${LIBS} -o ../usr/bin/looptest $(LFLAGS)
		${FIX} $@

clean:
		rm -f ${ALL} *.o *.s errs core a.out t.?
