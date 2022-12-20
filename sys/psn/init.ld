/* 	@(#)Copyright Apple Computer 1987	Version 1.2 of init.ld on 87/11/11 21:41:39 */
/* Due to a BUG in the generic linker
   the size + length must not add up to 
   something with bit 31 set */
MEMORY {
	VALID: org = 0x500, len = 0x7FFFFAFF
}
SECTIONS {
	pstart 0x500: {
			pstart.o (.text, .data)
		}
	.text 0x10000000: {}
	.data 0x11000000: {}
	.bss  0x12000000: {}
	MODULES 0x0(COPY): {}
}
u = 0x1FF00000;
