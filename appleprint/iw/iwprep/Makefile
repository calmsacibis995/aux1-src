# @(#)Copyright Apple Computer 1987 Version 1.4 of Makefile on 87/11/11 21:41:53
CFLAGS        = -O -I/usr/include/mac
DEBUG         = 
DESTDIR       = $(OL)usr/bin
EXTHDRS       = /usr/include/mac/fonts.h \
		/usr/include/mac/types.h \
		/usr/include/stdio.h
HDRS          = troffprep.h \
		util.h
LDFLAGS       = $(DEBUG) $(MODEL)
LIBS          = -lmac -lc /usr/lib/low.ld
LINKER        = ld
MAKE          = make
MAKEFILE      = Makefile
MODEL         = -n /usr/lib/maccrt0.o
OBJS          = bld_chset.o \
		bld_size.o \
		cmd_font.o \
		cmd_hor.o \
		cmd_name.o \
		cmd_paplen.o \
		cmd_papwid.o \
		cmd_res.o \
		cmd_uwidth.o \
		cmd_vert.o \
		command.o \
		error.o \
		prep.o \
		util.o \
		wr_desc.o \
		wr_fonts.o
OL	      = $(ROOT)/
PRINT         = pr
PROGRAM       = iwprep
SHELL         = /bin/sh
SRCS          = bld_chset.c \
		bld_size.c \
		cmd_font.c \
		cmd_hor.c \
		cmd_name.c \
		cmd_paplen.c \
		cmd_papwid.c \
		cmd_res.c \
		cmd_uwidth.c \
		cmd_vert.c \
		command.c \
		error.c \
		prep.c \
		util.c \
		wr_desc.c \
		wr_fonts.c

all:        $(PROGRAM)

$(PROGRAM):     $(OBJS) 
	@echo "Loading $(PROGRAM) ... \c"
	@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
	@echo "done"

clean:;     @rm -f $(OBJS)

clobber:;     @rm -f $(OBJS); rm -f $(PROGRAM)

depend:;    @mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DESTDIR=$(DESTDIR)

index:;     @ctags -wx $(HDRS) $(SRCS)

install:    $(PROGRAM)
	@echo Installing $(PROGRAM) in $(DESTDIR)
	@cp $(PROGRAM) $(DESTDIR)

print:;     @$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:     $(DESTDIR)/$(PROGRAM)

$(DESTDIR)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
	@$(MAKE) -f $(MAKEFILE) DESTDIR=$(DESTDIR) install
###
bld_chset.o: troffprep.h util.h /usr/include/stdio.h
bld_size.o: /usr/include/stdio.h troffprep.h /usr/include/mac/types.h \
	/usr/include/mac/fonts.h
cmd_font.o: troffprep.h util.h /usr/include/stdio.h \
	/usr/include/mac/types.h /usr/include/mac/fonts.h
cmd_hor.o: troffprep.h util.h /usr/include/stdio.h
cmd_name.o: troffprep.h util.h /usr/include/stdio.h
cmd_paplen.o: troffprep.h util.h /usr/include/stdio.h
cmd_papwid.o: troffprep.h util.h /usr/include/stdio.h
cmd_res.o: troffprep.h util.h /usr/include/stdio.h
cmd_uwidth.o: troffprep.h util.h /usr/include/stdio.h
cmd_vert.o: troffprep.h util.h /usr/include/stdio.h
command.o: troffprep.h
error.o: /usr/include/stdio.h
prep.o: /usr/include/stdio.h troffprep.h
util.o: /usr/include/stdio.h util.h
wr_desc.o: troffprep.h /usr/include/stdio.h
wr_fonts.o: troffprep.h /usr/include/stdio.h /usr/include/mac/types.h \
	/usr/include/mac/fonts.h
