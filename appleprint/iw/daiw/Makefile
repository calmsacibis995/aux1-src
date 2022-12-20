# @(#)Copyright Apple Computer 1987 Version 1.5 of Makefile on 87/11/11 21:43:09
CFLAGS	      = -I/usr/include/mac
DEBUG	      = 
DESTDIR	      = $(OL)usr/bin
EXTHDRS	      = /usr/include/assert.h \
		/usr/include/ctype.h \
		/usr/include/errno.h \
		/usr/include/mac/fonts.h \
		/usr/include/mac/quickdraw.h \
		/usr/include/mac/types.h \
		/usr/include/signal.h \
		/usr/include/stdio.h \
		/usr/include/sys/errno.h \
		/usr/include/sys/signal.h \
		/usr/include/sys/types.h
HDRS	      = dev.h \
		local.h
LDFLAGS	      = $(DEBUG) $(MODEL)
LIBS	      = -lmac -lm -lc /usr/lib/low.ld
LINKER	      = ld
MAKE	      = make
MAKEFILE      = Makefile
MODEL	      = -n /usr/lib/maccrt0.o
OBJS	      = badinput.o \
		data.o \
		dbg_cc.o \
		dbg_cc1.o \
		dev_init.o \
		do_Cxy.o \
		do_D.o \
		do_Hn.o \
		do_Vn.o \
		do_cx.o \
		do_fn.o \
		do_hn.o \
		do_nb_a.o \
		do_nnc.o \
		do_pn.o \
		do_sn.o \
		do_vn.o \
		do_x.o \
		eatc.o \
		eatline.o \
		eatnnn.o \
		eatword.o \
		font_init.o \
		harderr.o \
		initme.o \
		inittb.o \
		iw2.o \
		line2av.o \
		load_font.o \
		main.o \
		pageflush.o \
		pf_lp.o \
		process.o \
		softerr.o \
		strsave.o \
		stuffc.o \
		uneatc.o
OL	      = $(ROOT)/
PRINT	      = pr
PROGRAM	      = daiw
SHELL	      = /bin/sh
SRCS	      = badinput.c \
		data.c \
		dbg_cc.c \
		dbg_cc1.c \
		dev_init.c \
		do_Cxy.c \
		do_D.c \
		do_Hn.c \
		do_Vn.c \
		do_cx.c \
		do_fn.c \
		do_hn.c \
		do_nb_a.c \
		do_nnc.c \
		do_pn.c \
		do_sn.c \
		do_vn.c \
		do_x.c \
		eatc.c \
		eatline.c \
		eatnnn.c \
		eatword.c \
		font_init.c \
		harderr.c \
		initme.c \
		inittb.c \
		iw2.c \
		line2av.c \
		load_font.c \
		main.c \
		pageflush.c \
		pf_lp.c \
		process.c \
		softerr.c \
		strsave.c \
		stuffc.c \
		uneatc.c

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS)
		@echo "Loading $(PROGRAM) ... \c"
		@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS)

clobber:;	@rm -f $(OBJS); rm -f $(PROGRAM)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DESTDIR=$(DESTDIR)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DESTDIR)
		@cp $(PROGRAM) $(DESTDIR)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DESTDIR)/$(PROGRAM)

$(DESTDIR)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@$(MAKE) -f $(MAKEFILE) DESTDIR=$(DESTDIR) install
###
badinput.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
data.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
dbg_cc.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
dbg_cc1.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
dev_init.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_Cxy.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_D.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_Hn.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_Vn.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_cx.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_fn.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_hn.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_nb_a.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_nnc.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_pn.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_sn.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_vn.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
do_x.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
eatc.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
eatline.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
eatnnn.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
eatword.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
font_init.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
harderr.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
initme.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
inittb.o: /usr/include/mac/types.h /usr/include/mac/quickdraw.h \
	/usr/include/mac/fonts.h
iw2.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
line2av.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
load_font.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
main.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
pageflush.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
pf_lp.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
process.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
softerr.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
strsave.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
stuffc.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
uneatc.o: local.h /usr/include/stdio.h /usr/include/sys/types.h \
	/usr/include/ctype.h /usr/include/errno.h /usr/include/sys/errno.h \
	/usr/include/signal.h /usr/include/sys/signal.h /usr/include/assert.h \
	/usr/include/mac/types.h /usr/include/mac/quickdraw.h dev.h
