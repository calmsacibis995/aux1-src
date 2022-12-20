#ifndef lint	/* .../appletalk/fwd/fwdicp/environ/scc.h */
#define _AC_NAME scc_h
#define _AC_NO_MAIN "@(#) Copyright (c) 1985-87 UniSoft Corporation (Warning - possible offset problems!!!), All Rights Reserved.  {Apple version 1.2 87/11/11 21:08:09}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	@(#)Copyright Apple Computer 1987	Version 1.2 of scc.h on 87/11/11 21:08:09 */
/*+-------------------------------------------------------------------------+
  |     S C C    I / O    A D D R E S S E S                                 |
  +-------------------------------------------------------------------------+
  |     These are the Z8530 SCC channel A and channel B I/O ports.
  |     control read and control write, data read and data write.
  |
  |     This structure of pointers are used so a hardware independant mechanism
  |	can be used. While most scc interfaces uses R/W and A1 lines
  |	to differentiate read/write and data control ports, such cannot be
  |	counted on. This structure also tries to maintain a seperation between
  |	A and B channels (perhaps incorrectly) though there is some status
  |     and control common to both channels that may only be read from A.
  |
  +*/
typedef struct
        {
        unsigned char  *scc_ctl_read;    /* address to read SCC control port. */
        unsigned char  *scc_ctl_write;   /* address to write SCC control port.*/
        unsigned char  *scc_data_read;   /* address to read SCC data port.    */
        unsigned char  *scc_data_write;  /* address to write SCC data port.   */
        unsigned char  wr9_reset;	 /* what value to write to wr9 to     */
					 /* reset this channel. wr9 is shared */
	char	       pad[15];		 /* pad so power of 2		      */
        }    scc_t;

extern	scc_t	sccs[];

/*+-------------------------------------------------------------------------+
  |     S C C    D E F I N I T I O N S                                      |
  +-------------------------------------------------------------------------+
  |  Description:
  |      These are the definitions for the Zilog/Mostek/AMD Z8530 SCC
  |      (Serial Communications Controller) chip.
  | 
  |  History:
  |      17-May-84: Philip K. Ronzone. Coded for SCC Z8530 for SNA.
  |      04-Jul-85: Philip K. Ronzone. Edited for just SCC defines.
  +*/



/*+-------------------------------------------------------------------------+
  |     T H E    R E G I S T E R S .                                        |
  +-------------------------------------------------------------------------+*/
#define  SCC_RR0    0   /* Transmit/receive buffer & external status. */
#define  SCC_RR1    1   /* Receive condition status/residue codes.    */
#define  SCC_RR2    2   /* Interrupt vector (modified in B channel).  */
#define  SCC_RR3    3   /* Interrupt pending (channel A only).        */
#define  SCC_RR4    4   /* Not used.                                  */
#define  SCC_RR5    5   /* Not used.                                  */
#define  SCC_RR6    6   /* Not used.                                  */
#define  SCC_RR7    7   /* Not used.                                  */
#define  SCC_RR8    8   /* Receive buffer.                            */
#define  SCC_RR9    9   /* Not used.                                  */
#define  SCC_RR10  10   /* Loop/clock status.                         */
#define  SCC_RR11  11   /* Not used.                                  */
#define  SCC_RR12  12   /* Lower byte of time constant.               */
#define  SCC_RR13  13   /* Upper byte of time constant.               */
#define  SCC_RR14  14   /* Not used.                                  */
#define  SCC_RR15  15   /* External status interrupt enable.          */
 
#define  SCC_WR0    0   /* Command register.                               */
#define  SCC_WR1    1   /* Xmit/rcv interrupt & data transfer mode.        */
#define  SCC_WR2    2   /* Interrupt vector.                               */
#define  SCC_WR3    3   /* Receive parameters and control.                 */
#define  SCC_WR4    4   /* Transmitter/receiver misc. parameters & modes.  */
#define  SCC_WR5    5   /* Transmit parameter and controls.                */
#define  SCC_WR6    6   /* Sync character or SDLC address field.           */
#define  SCC_WR7    7   /* Sync character or SDLC flag.                    */
#define  SCC_WR8    8   /* Transmit buffer.                                */
#define  SCC_WR9    9   /* Master interrupt control.                       */
#define  SCC_WR10  10   /* Misc. transmitter/receiver control bits.        */
#define  SCC_WR11  11   /* Clock mode control.                             */
#define  SCC_WR12  12   /* Lower byte baud rate generator time constant.   */
#define  SCC_WR13  13   /* Upper byte baud rate generator time constant.   */
#define  SCC_WR14  14   /* Miscellaneous control bits.                     */
#define  SCC_WR15  15   /* External status/interrupt control.              */

/*+-------------------------------------------------------------------------+
  |     R R 0                                                               |
  +-------------------------------------------------------------------------+*/
                              /* RR0 bits D0-D7.         */
#define  SCC_RR0_RCA  0x01    /* Rx character available. */
#define  SCC_RR0_ZCT  0x02    /* Zero count.             */
#define  SCC_RR0_TBE  0x04    /* Tx buffer empty.        */
#define  SCC_RR0_DCD  0x08    /* DCD state.              */
#define  SCC_RR0_SYH  0x10    /* Sync/hunt.              */
#define  SCC_RR0_CTS  0x20    /* CTS state.              */
#define  SCC_RR0_TXU  0x40    /* Tx underrun.            */
#define  SCC_RR0_BRA  0x80    /* Break/abort.            */



/*+-------------------------------------------------------------------------+
  |     R R 1                                                               |
  +-------------------------------------------------------------------------+*/
                              /* RR1 bits D0-D7.      */
#define  SCC_RR1_ALL  0x01    /* All sent.            */
#define  SCC_RR1_RC2  0x02    /* Residue code 2.      */
#define  SCC_RR1_RC1  0x04    /* Residue code 1.      */
#define  SCC_RR1_RC0  0x08    /* Residue code 0.      */
#define  SCC_RR1_PER  0x10    /* Parity error.        */
#define  SCC_RR1_ROE  0x20    /* Rx overrun error.    */
#define  SCC_RR1_CFE  0x40    /* CRC/framing error.   */
#define  SCC_RR1_EOF  0x80    /* End of frame (SDLC). */



/*+-------------------------------------------------------------------------+
  |     R R 2                                                               |
  +-------------------------------------------------------------------------+*/
                              /* RR2 bits D0-D7.                        */
#define  SCC_RR2_BTE  0x0     /* Channel B transmit buffer empty.       */
#define  SCC_RR2_BEC  0x2     /* Channel B external/status change.      */
#define  SCC_RR2_BRF  0x4     /* Channel B receive character available. */
#define  SCC_RR2_BSC  0x6     /* Channel B special receive condition.   */
#define  SCC_RR2_ATE  0x8     /* Channel A transmit buffer empty.       */
#define  SCC_RR2_AEC  0xa     /* Channel A external/status change.      */
#define  SCC_RR2_ARF  0xc     /* Channel A receive character available. */
#define  SCC_RR2_ASC  0xe     /* Channel A special receive condition.   */


/*+-------------------------------------------------------------------------+
  |     R R 3                                                               |
  +-------------------------------------------------------------------------+*/
                              /* RR3 bits D0-D7.                       */
#define  SCC_RR3_CBE  0x01    /* Channel B EXT/STAT interrupt pending. */
#define  SCC_RR3_CBT  0x02    /* Channel B Tx interrupt pending.       */
#define  SCC_RR3_CBR  0x04    /* Channel B Rx interrupt pending.       */
#define  SCC_RR3_CAE  0x08    /* Channel A EXT/STAT interrupt pending. */
#define  SCC_RR3_CAT  0x10    /* Channel A Tx interrupt pending.       */
#define  SCC_RR3_CAR  0x20    /* Channel A Rx interrupt pending.       */



/*+-------------------------------------------------------------------------+
  |     R R 1 0                                                             |
  +-------------------------------------------------------------------------+*/
                              /* RR10 bits D0-D7.  */
#define  SCC_RR10_OLP  0x02   /* On loop.          */
#define  SCC_RR10_LPP  0x10   /* Loop pending.     */
#define  SCC_RR10_2CM  0x40   /* 2 clocks missing. */
#define  SCC_RR10_1CM  0x80   /* 1 clock missing.  */



/*+-------------------------------------------------------------------------+
  |     R R 1 5                                                             |
  +-------------------------------------------------------------------------+*/
                              /* RR15 bits D0-D7.                  */
#define  SCC_RR15_ZCI  0x02   /* Zero count interrupt enabled.     */
#define  SCC_RR15_DCI  0x08   /* DCD interrupt enabled.            */
#define  SCC_RR15_SHI  0x10   /* Sync/hunt interrupt enabled.      */
#define  SCC_RR15_CSI  0x20   /* CTS interrupt enabled.            */
#define  SCC_RR15_TUI  0x40   /* Tx underrun/eom interrupt enable. */
#define  SCC_RR15_BAI  0x80   /* Break/abort interrupt enable.     */


/*+-------------------------------------------------------------------------+
  |     W R 0                                                               |
  +-------------------------------------------------------------------------+*/
                              /* WR0 bits D5-D3.                         */
#define  SCC_WR0_PTH  0x08    /* Point high.                             */
#define  SCC_WR0_RXI  0x10    /* Reset external/status interrupts.       */
#define  SCC_WR0_SAB  0x18    /* Send abort (SDLC).                      */
#define  SCC_WR0_EIR  0x20    /* Enable interrupts on next Rx character. */
#define  SCC_WR0_RTP  0x28    /* Reset Tx interrupt pending.             */
#define  SCC_WR0_ERE  0x30    /* Error reset.                            */
#define  SCC_WR0_RHI  0x38    /* Reset highest IUS.                      */

                              /* WR0 bits D7-D6.              */
#define  SCC_WR0_RRC  0x40    /* Reset Rx CRC checker.        */
#define  SCC_WR0_RTC  0x80    /* Reset Tx CRC generator.      */
#define  SCC_WR0_RTU  0xc0    /* Reset Tx underrun/eom latch. */


/*+-------------------------------------------------------------------------+
  |     W R 1                                                               |
  +-------------------------------------------------------------------------+*/
                              /* WR1 bits D0, D1, and D2.                 */
#define  SCC_WR1_EIN  0x01    /* External/status master interrupt enable. */
#define  SCC_WR1_TIN  0x02    /* Tx interrupt enable.                     */
#define  SCC_WR1_PSC  0x04    /* Parity is special condition.             */

                              /* WR1 bits D4-D3.                            */
#define  SCC_WR1_RID  0x00    /* Rx interrupt disable.                      */
#define  SCC_WR1_RIF  0x08    /* Rx interrupt on 1st char or spcl. cond.    */
#define  SCC_WR1_RIA  0x10    /* Rx interrupt on all chars or spcl. cond's. */
#define  SCC_WR1_RIS  0x18    /* Rx interrupt on special conditions only.   */

                              /* WR1 bits D5, D6, and D7.                */
#define  SCC_WR1_WDD  0x20    /* Wait/DMA request I/O dir. (0=Tx, 1=Rx). */
#define  SCC_WR1_WDF  0x40    /* Wait/DMA request function on/off.       */
#define  SCC_WR1_WDE  0x80    /* Wait/DMA request enable.                */

/*+-------------------------------------------------------------------------+
  |     W R 3                                                               |
  +-------------------------------------------------------------------------+*/
                              /* WR3 bits D0, D1, D2, D3, D4, and D5. */
#define  SCC_WR3_RXE  0x01    /* Rx enable.                           */
#define  SCC_WR3_SCI  0x02    /* Sync character load inhibit.         */
#define  SCC_WR3_ASM  0x04    /* Address search mode (SDLC).          */
#define  SCC_WR3_RCE  0x08    /* Rx CRC enable.                       */
#define  SCC_WR3_EHM  0x10    /* Enter hunt mode.                     */
#define  SCC_WR3_AUE  0x20    /* Auto enables.                        */

                              /* WR3 bits D6-D7.          */
#define  SCC_WR3_RX5  0x00    /* Rx 5 bits per character. */
#define  SCC_WR3_RX7  0x40    /* Rx 7 bits per character. */
#define  SCC_WR3_RX6  0x80    /* Rx 6 bits per character. */
#define  SCC_WR3_RX8  0xc0    /* Rx 8 bits per character. */


/*+-------------------------------------------------------------------------+
  |     W R 4                                                               |
  +-------------------------------------------------------------------------+*/
                              /* WR4 bits D0-D1.                     */
#define  SCC_WR4_PEN  0x01    /* Parity enable.                      */
#define  SCC_WR4_PEV  0x02    /* Parity even.                        */
#define  SCC_WR4_POD  0x00    /* Parity odd (just for looks I know). */

                              /* WR4 bits D2-D3.              */
#define  SCC_WR4_SME  0x00    /* Sync modes enabled.          */
#define  SCC_WR4_1SB  0x04    /* 1 stop bit per character.    */
#define  SCC_WR4_15S  0x08    /* 1.5 stop bits per character. */
#define  SCC_WR4_2SB  0x0c    /* 2 stop bits per character.   */

                              /* WR4 bits D4-D5.        */
#define  SCC_WR4_8SC   0x00   /* 8 bit sync character.  */
#define  SCC_WR4_16S   0x10   /* 16 bit sync character. */
#define  SCC_WR4_SDLC  0x20   /* SDLC mode.             */
#define  SCC_WR4_ESM   0x30   /* External sync mode.    */

                              /* WR4 bits D6-D7. */
#define  SCC_WR4_X01  0x00    /* 01X clock mode. */
#define  SCC_WR4_X16  0x40    /* 16X clock mode. */
#define  SCC_WR4_X32  0x80    /* 32X clock mode. */
#define  SCC_WR4_X64  0xc0    /* 64X clock mode. */

/*+-------------------------------------------------------------------------+
  |     W R 5                                                               |
  +-------------------------------------------------------------------------+*/
                              /* WR5 bits D0-D4.                       */
#define  SCC_WR5_TCE  0x01    /* Tx CRC enable.                        */
#define  SCC_WR5_RTS  0x02    /* RTS/ control pin.                     */
#define  SCC_WR5_C16  0x04    /* 0=SDLC-CRC polynomial, 1=CRC-16 poly. */
#define  SCC_WR5_TXE  0x08    /* Tx enable.                            */
#define  SCC_WR5_SNB  0x10    /* Send break.                           */

                              /* WR5 bits D5-D6.          */
#define  SCC_WR5_TX5  0x00    /* Tx 5 bits per character. */
#define  SCC_WR5_TX7  0x20    /* Tx 7 bits per character. */
#define  SCC_WR5_TX6  0x40    /* Tx 6 bits per character. */
#define  SCC_WR5_TX8  0x60    /* Tx 8 bits per character. */

                              /* WR5 bit D7.      */
#define  SCC_WR5_DTR  0x80    /* DTR control pin. */


/*+-------------------------------------------------------------------------+
  |     W R 9                                                               |
  +-------------------------------------------------------------------------+*/
                              /* WR9 bits D0-D5.           */
#define  SCC_WR9_VIS  0x01    /* Vector includes status.   */
#define  SCC_WR9_NOV  0x02    /* No vector.                */
#define  SCC_WR9_DLC  0x04    /* Disable lower chain.      */
#define  SCC_WR9_MIE  0x08    /* Master interrupt enable.  */
#define  SCC_WR9_STS  0x10    /* Status high, status low/. */

                              /* WR9 bits D6-D7.       */
#define  SCC_WR9_NOR  0x00    /* No reset.             */
#define  SCC_WR9_CRB  0x40    /* Channel reset B.      */
#define  SCC_WR9_CRA  0x80    /* Channel reset A.      */
#define  SCC_WR9_FHR  0xc0    /* Force hardware reset. */

/*+-------------------------------------------------------------------------+
  |     W R 1 0                                                             |
  +-------------------------------------------------------------------------+*/
                              /* WR10 bits D0-D4.          */
#define  SCC_WR10_6BT  0x01   /* 6 bit, 8 bit/.            */
#define  SCC_WR10_LMD  0x02   /* Loop mode.                */
#define  SCC_WR10_AFU  0x04   /* Abort, flag/ on underrun. */
#define  SCC_WR10_MFI  0x08   /* Mark, flag/ on idle.      */
#define  SCC_WR10_GAP  0x10   /* Go active on poll.        */

                              /* WR10 bits D5-D6.      */
#define  SCC_WR10_NRZ  0x00   /* NRZ encoding.         */
#define  SCC_WR10_NRI  0x20   /* NRZI encoding.        */
#define  SCC_WR10_FM1  0x40   /* FM1 (transition = 1). */
#define  SCC_WR10_FM0  0x60   /* FM0 (transition = 0). */

                              /* WR10 bit D7.    */
#define  SCC_WR10_CPI  0x80   /* CRC preset I/O. */


/*+-------------------------------------------------------------------------+
  |     W R 1 1                                                             |
  +-------------------------------------------------------------------------+*/
                              /* WR11 bits D0-D1.                    */
#define  SCC_WR11_TOX  0x00   /* Xmit ext ctl/ out = xtal output.    */
#define  SCC_WR11_TOT  0x01   /* Xmit ext ctl/ out = transmit clock. */
#define  SCC_WR11_TOB  0x02   /* Xmit ext ctl/ out = baud rate gen.  */
#define  SCC_WR11_TOD  0x03   /* Xmit ext ctl/ out = DPLL output.    */

                              /* WR11 bit D2.                         */
#define  SCC_WR11_TOI  0x04   /* Transmit external control direction. */

                              /* WR11 bits D3-D4.                       */
#define  SCC_WR11_TCR  0x00   /* Transmit clock source = RTxC pin.      */
#define  SCC_WR11_TCT  0x08   /* Transmit clock source = TRxC pin.      */
#define  SCC_WR11_TCB  0x10   /* Transmit clock source = baud rate gen. */
#define  SCC_WR11_TCD  0x18   /* Transmit clock source = DPLL output.   */

                              /* WR11 bits D5-D6.                      */
#define  SCC_WR11_RCR  0x00   /* Receive clock source = RTxC pin.      */
#define  SCC_WR11_RCT  0x20   /* Receive clock source = TRxC pin.      */
#define  SCC_WR11_RCB  0x40   /* Receive clock source = baud rate gen. */
#define  SCC_WR11_RCD  0x60   /* Receive clock source = DPLL output.   */

                              /* WR11 bit D7.            */
#define  SCC_WR11_RXN  0x80   /* RTxC/  XTAL or NO XTAL. */

/*+-------------------------------------------------------------------------+
  |     W R 1 4                                                             |
  +-------------------------------------------------------------------------+*/
                              /* WR14 bits D0-D4.            */
#define  SCC_WR14_BGE  0x01   /* Baud rate generator enable. */
#define  SCC_WR14_BGS  0x02   /* Baud rate generator source. */
#define  SCC_WR14_DRF  0x04   /* DTR/, REQ/ pin function.    */
#define  SCC_WR14_AEC  0x08   /* Auto echo.                  */
#define  SCC_WR14_LLB  0x10   /* Local loopback.             */

                              /* WR14 bits D5-D7.                  */
#define  SCC_WR14_NUL  0x00   /* Null command.                     */
#define  SCC_WR14_ESM  0x20   /* Enter search mode.                */
#define  SCC_WR14_RMC  0x40   /* Reset missing clock.              */
#define  SCC_WR14_DID  0x60   /* Disable DPLL.                     */
#define  SCC_WR14_SBG  0x80   /* Set source = baud rate generator. */
#define  SCC_WR14_SRT  0xa0   /* Set source = RTxC/.               */
#define  SCC_WR14_SFM  0xc0   /* Set FM mode.                      */
#define  SCC_WR14_SNM  0xe0   /* Set NRZI mode.                    */


/*+-------------------------------------------------------------------------+
  |     W R 1 5                                                             |
  +-------------------------------------------------------------------------+*/
                              /* WR15 bits D0-D7.                  */
#define  SCC_WR15_ZCI  0x02   /* Zero count interrupt enable.      */
#define  SCC_WR15_DCI  0x08   /* DCD interrupt enable.             */
#define  SCC_WR15_SHI  0x10   /* Sync/hunt interrupt enable.       */
#define  SCC_WR15_CTI  0x20   /* CTS interrupt enable.             */
#define  SCC_WR15_TUI  0x40   /* Tx underrun/eom interrupt enable. */
#define  SCC_WR15_BAI  0x80   /* Break/abort interrupt enable.     */



/*+-------------------------------------------------------------------------+
  |	S C C   A C C E S S 						    |
  +-------------------------------------------------------------------------+*/
#ifdef SCC_DELAY
#define	SCC_CTL_READ(REG)						\
		( *scc_ctl_write = REG, scc_delay(), *scc_ctl_read)
#else
#ifdef	APPLETALK
#ifdef	SCC_DEFINE
short scc_delay_dummy;	/* just enough to slow down access, but not too much */
#else
extern short scc_delay_dummy;
#endif	SCC_DEFINE
#define	SCC_CTL_READ(REG)						\
		( *scc_ctl_write = REG, scc_delay_dummy = (short) 1234, *scc_ctl_read)
#else	APPLETALK
#define	SCC_CTL_READ(REG)						\
		(*scc_ctl_write = REG, *scc_ctl_read)
#endif	APPLETALK
#endif


#ifdef SCC_DELAY							
#define	SCC_CTL_WRITE(REG,VALUE)					\
		(*scc_ctl_write = REG, scc_delay(), *scc_ctl_write = VALUE
#else
#define	SCC_CTL_WRITE(REG,VALUE)					\
		( *scc_ctl_write = REG, *scc_ctl_write = VALUE)
#endif



#define SCC_EOT		128
/*+-------------------------------------------------------------------------+
  |     R E G I S T E R,    V A L U E,    A N D    D E L A Y    T A B L E   |
  +-------------------------------------------------------------------------+
  |     This table is used to build ``lists'' of SCC register numbers,
  |     values to be put into those registers
  +*/
typedef struct
	{
        unsigned char      scc_register_number;
        unsigned char      scc_register_value;
	} scc_rvd;


#ifdef	SCC_DEFINE
/*+-------------------------------------------------------------------------+
  |     scc_delay the scc access function			    	    |
  |	this routine is used to slow down accesses to scc's that have a	    |
  |	slow p clock or no supporting hardware for this purpose. The reason |
  |	that this is a function call and not inline, is that the compiler   |
  |	objects to the "," with whiles.		    			    |
  +-------------------------------------------------------------------------+*/

scc_delay()

{
	/*
	 * 1.627 us per iteration at 16mhz.
	 * do  { }  while (--delay_value);
	 * Only need 1.627us for 3.687mhz scc
	 */
	return;
}


#ifdef	NOTYET
/*+-------------------------------------------------------------------------+
  |     SCC_CMD the scc access function				    	    |
  |	this routine access the scc in a standard fashion. It take a table  |
  |	as an entry of the form ???
  +-------------------------------------------------------------------------+*/

scc_cmd(tbl, sccp)
register	scc_rvd  *tbl;
scc_t		*sccp;
{
	register  unsigned char	   delay_value;
	register  unsigned char	   *scc_ctl_write;


	scc_ctl_write = sccp->scc_ctl_write;

	do  {
		*scc_ctl_write = tbl->scc_register_number;
#ifdef SCC_DELAY
		scc_delay();
#endif
		*scc_ctl_write = tbl->scc_register_value;
		tbl++;
	} while (tbl->scc_register_number != SCC_EOT);
}
#endif	NOTYET
#endif	SCC_DEFINE



#ifdef	SCC_DEBUG
print_scc(n)

{
	register unsigned char	*scc_ctl_read =  sccs[n].scc_ctl_read;
	register unsigned char	*scc_ctl_write =  sccs[n].scc_ctl_write;
	
	printf("scc_rr0 %x", SCC_CTL_READ(SCC_RR0));
	printf("\tscc_rr1 %x", SCC_CTL_READ(SCC_RR1));
	printf("\tscc_rr2 %x", SCC_CTL_READ(SCC_RR2));
	printf("\tscc_rr3 %x\n", SCC_CTL_READ(SCC_RR3));
	printf("scc_rr10 %x", SCC_CTL_READ(SCC_RR10));
	printf("\tscc_rr12 %x", SCC_CTL_READ(SCC_RR12));
	printf("\tscc_rr13 %x", SCC_CTL_READ(SCC_RR13));
	printf("\tscc_rr15 %x\n", SCC_CTL_READ(SCC_RR15));
}



dump_tbl(tbl, scc_no)
register	scc_rvd  *tbl;
uint		scc_no;
{
	register  unsigned char	   delay_value;


	printf("dump scc tbl = %x to scc %d\n", tbl, scc_no);
	do  {
		printf("SCC_WR%d = ", tbl->scc_register_number);
		printf("%x", tbl->scc_register_value);
		tbl++;
	}       while (tbl->scc_register_number != SCC_EOT) ;
}
#endif	SCC_DEBUG
