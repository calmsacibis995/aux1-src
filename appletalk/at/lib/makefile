#
# 	@(#)Copyright Apple Computer 1987	Version 1.3 of makefile on 87/11/11 21:01:34 
#

CFLAGS=	-I$(AT)

all: $(LIB)/libat.a

OBJ=	at_atp_orp.o	\
	at_atp_orq.o	\
	at_atp_rcv.o	\
	at_atp_rp.o	\
	at_atp_rmb.o	\
	at_atp_rsb.o	\
	at_atp_sn.o	\
	at_atp_smb.o	\
	at_atp_ssb.o	\
	at_chooser.o	\
	at_cl_dskt.o	\
	at_cl_skt.o	\
	at_con_nve.o	\
	at_ddp_rd.o	\
	at_ddp_wr.o	\
	at_dec_en.o	\
	at_declare.o	\
	at_drg_nve.o	\
	at_drgn_en.o	\
	at_drgnnve.o	\
	at_error.o	\
	at_err_set.o	\
	at_execute.o	\
	at_fnd_ntw.o	\
	at_gddplen.o	\
	at_getnet.o	\
	at_lk_cnf.o	\
	at_lk_en.o	\
	at_lk_nve.o	\
	at_nbp_dwn.o	\
	at_op_dskt.o	\
	at_op_skt.o	\
	at_pap_cl.o	\
	at_pap_drg.o	\
	at_pap_get.o	\
	at_pap_hst.o	\
	at_pap_int.o	\
	at_pap_opn.o	\
	at_pap_rig.o	\
	at_pap_rd.o	\
	at_pap_reg.o	\
	at_pap_scl.o	\
	at_pap_sts.o	\
	at_pap_wr.o	\
	at_pap_wre.o	\
	at_pap_wrf.o	\
	at_reg_en.o	\
	at_reg_nve.o	\
	at_sddplen.o	\
	at_snd_dev.o

ATPOBJ= atp_get.o 	\
	atp_gtnote.o 	\
	atp_get_nw.o 	\
	atp_gtpoll.o 	\
	atp_is.o 	\
	atp_is_x.o 	\
	atp_is_nt.o 	\
	atp_is_nw.o 	\
	atp_is_xnt.o 	\
	atp_is_xnw.o 	\
	atp_isd.o 	\
	atp_isd_x.o 	\
	atp_isd_nt.o 	\
	atp_isd_nw.o 	\
	atp_isdxnt.o 	\
	atp_isdxnw.o 	\
	atp_open.o 	\
	atp_releas.o 	\
	atp_rqpoll.o 	\
	atp_send.o 	\
	atp_sndeof.o 	\
	atp_st_def.o	\
	atp_st_hdr.o

$(LIB)/libat.a:	$(OBJ) $(ATPOBJ)
	ar ruv $@ $(OBJ) $(ATPOBJ)

.c.o:
	$(CC) -c $(CFLAGS) $<
	ar rv $(LIB)/libat.a $@
	
#	$(CC) -c $(CFLAGS) $(?:.o=.c)
#	ar rv $(LIB)/libat.a $?
#	rm $?
#
#.c.a:;

at_atp_orp.o:		$(AT)/appletalk.h $(LIBSRC)/at_atp_orp.c
at_atp_orq.o:		$(AT)/appletalk.h $(LIBSRC)/at_atp_orq.c
at_atp_rcv.o:		$(AT)/appletalk.h $(LIBSRC)/at_atp_rcv.c
at_atp_rp.o:		$(AT)/appletalk.h $(LIBSRC)/at_atp_rp.c
at_atp_rmb.o:		$(AT)/appletalk.h $(LIBSRC)/at_atp_rmb.c
at_atp_rsb.o:		$(AT)/appletalk.h $(LIBSRC)/at_atp_rsb.c
at_atp_sn.o:		$(AT)/appletalk.h $(LIBSRC)/at_atp_sn.c
at_atp_smb.o:		$(AT)/appletalk.h $(LIBSRC)/at_atp_smb.c
at_atp_ssb.o:		$(AT)/appletalk.h $(LIBSRC)/at_atp_ssb.c
at_chooser.o:		$(AT)/appletalk.h $(LIBSRC)/at_chooser.c
at_cl_dskt.o:		$(AT)/appletalk.h $(LIBSRC)/at_cl_dskt.c
at_cl_skt.o:		$(AT)/appletalk.h $(LIBSRC)/at_cl_skt.c
at_con_nve.o:		$(AT)/appletalk.h $(LIBSRC)/at_con_nve.c
at_ddp_rd.o:		$(AT)/appletalk.h $(LIBSRC)/at_ddp_rd.c
at_ddp_wr.o:		$(AT)/appletalk.h $(LIBSRC)/at_ddp_wr.c
at_dec_en.o:		$(AT)/appletalk.h $(LIBSRC)/at_dec_en.c
at_declare.o:		$(AT)/appletalk.h $(LIBSRC)/at_declare.c
at_drg_nve.o:		$(AT)/appletalk.h $(LIBSRC)/at_drg_nve.c
at_drgn_en.o:		$(AT)/appletalk.h $(LIBSRC)/at_drgn_en.c
at_drgnnve.o:		$(AT)/appletalk.h $(LIBSRC)/at_drgnnve.c
at_error.o:		$(AT)/appletalk.h $(LIBSRC)/at_error.c
at_err_set.o:		$(AT)/appletalk.h $(LIBSRC)/at_err_set.c
at_execute.o:		$(AT)/appletalk.h $(LIBSRC)/at_execute.c
at_fnd_ntw.o:		$(AT)/appletalk.h $(LIBSRC)/at_fnd_ntw.c
at_gddplen.o:		$(AT)/appletalk.h $(LIBSRC)/at_gddplen.c
at_getnet.o:		$(AT)/appletalk.h $(LIBSRC)/at_getnet.c
at_lk_cnf.o:		$(AT)/appletalk.h $(LIBSRC)/at_lk_cnf.c
at_lk_en.o:		$(AT)/appletalk.h $(LIBSRC)/at_lk_en.c
at_lk_nve.o:		$(AT)/appletalk.h $(LIBSRC)/at_lk_nve.c
at_nbp_dwn.o:		$(AT)/appletalk.h $(LIBSRC)/at_nbp_dwn.c
at_op_dskt.o:		$(AT)/appletalk.h $(LIBSRC)/at_op_dskt.c
at_op_skt.o:		$(AT)/appletalk.h $(LIBSRC)/at_op_skt.c
at_pap_cl.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_cl.c
at_pap_drg.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_drg.c
at_pap_get.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_get.c
at_pap_hst.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_hst.c
at_pap_int.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_int.c
at_pap_opn.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_opn.c
at_pap_rig.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_rig.c
at_pap_rd.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_rd.c
at_pap_reg.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_reg.c
at_pap_scl.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_scl.c
at_pap_sts.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_sts.c
at_pap_wr.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_wr.c
at_pap_wre.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_wre.c
at_pap_wrf.o:		$(AT)/appletalk.h $(LIBSRC)/at_pap_wrf.c
at_reg_en.o:		$(AT)/appletalk.h $(LIBSRC)/at_reg_en.c
at_reg_nve.o:		$(AT)/appletalk.h $(LIBSRC)/at_reg_nve.c
at_sddplen.o:		$(AT)/appletalk.h $(LIBSRC)/at_sddplen.c
at_snd_dev.o:		$(AT)/appletalk.h $(LIBSRC)/at_snd_dev.c
atp_gtnote.o:		$(AT)/appletalk.h $(LIBSRC)/atp_gtnote.o 
atp_get_nw.o:		$(AT)/appletalk.h $(LIBSRC)/atp_get_nw.o 
atp_gtpoll.o:		$(AT)/appletalk.h $(LIBSRC)/atp_gtpoll.o 
atp_is.o:		$(AT)/appletalk.h $(LIBSRC)/atp_is.o 
atp_is_x.o:		$(AT)/appletalk.h $(LIBSRC)/atp_is_x.o 
atp_is_nt.o:		$(AT)/appletalk.h $(LIBSRC)/atp_is_nt.o 
atp_is_nw.o:		$(AT)/appletalk.h $(LIBSRC)/atp_is_nw.o 
atp_is_xnt.o:		$(AT)/appletalk.h $(LIBSRC)/atp_is_xnt.o 
atp_is_xnw.o:		$(AT)/appletalk.h $(LIBSRC)/atp_is_xnw.o 
atp_isd.o:		$(AT)/appletalk.h $(LIBSRC)/atp_isd.o 
atp_isd_x.o:		$(AT)/appletalk.h $(LIBSRC)/atp_isd_x.o 
atp_isd_nt.o:		$(AT)/appletalk.h $(LIBSRC)/atp_isd_nt.o 
atp_isd_nw.o:		$(AT)/appletalk.h $(LIBSRC)/atp_isd_nw.o 
atp_isdxnt.o:		$(AT)/appletalk.h $(LIBSRC)/atp_isdxnt.o 
atp_isdxnw.o:		$(AT)/appletalk.h $(LIBSRC)/atp_isdxnw.o 
atp_open.o:		$(AT)/appletalk.h $(LIBSRC)/atp_open.o 
atp_releas.o:		$(AT)/appletalk.h $(LIBSRC)/atp_releas.o 
atp_rqpoll.o:		$(AT)/appletalk.h $(LIBSRC)/atp_rqpoll.o 
atp_send.o:		$(AT)/appletalk.h $(LIBSRC)/atp_send.o 
atp_sndeof.o:		$(AT)/appletalk.h $(LIBSRC)/atp_sndeof.o 
atp_st_def.o:		$(AT)/appletalk.h $(LIBSRC)/atp_st_def.c
atp_st_hdr.o:		$(AT)/appletalk.h $(LIBSRC)/atp_st_hdr.c
