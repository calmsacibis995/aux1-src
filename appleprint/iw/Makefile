# @(#)Copyright Apple Computer 1987\tVersion 1.3 of Makefile on 87/06/29 19:47:26
OL	= $(ROOT)/
MAKE	= make
SHELL	= /bin/sh

all:
	cd daiw; $(MAKE) ROOT=$(ROOT)
	cd iwprep; $(MAKE) ROOT=$(ROOT)
	cd fonts; $(MAKE) ROOT=$(ROOT)

install:	installdaiw installiwprep installfonts

clean:
	cd daiw; $(MAKE) clean
	cd iwprep; $(MAKE) clean
	cd fonts; $(MAKE) clean

clobber:
	cd daiw; $(MAKE) clobber
	cd iwprep; $(MAKE) clobber
	cd fonts; $(MAKE) clobber

installdaiw:
	cd daiw; $(MAKE) ROOT=$(ROOT) install

installiwprep:
	cd iwprep; $(MAKE) ROOT=$(ROOT) install

installfonts:
	cd fonts; $(MAKE) ROOT=$(ROOT) install

