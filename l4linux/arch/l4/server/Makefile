
lib-$(CONFIG_L4_SERVER)       += server.o
lib-$(CONFIG_L4_SERVER_INPUT) += input-srv.o

include $(L4OBJ)/l4defs.mk.inc

# see scripts/ for more
define rule_l4re_cxx_o_cc
	$(call echo-cmd,checksrc) $(cmd_checksrc)			  \
	$(call echo-cmd,l4re_cxx_o_cc) $(cmd_l4re_cxx_o_cc);		  \
	$(cmd_modversions)						  \
	$(call echo-cmd,record_mcount)					  \
	$(cmd_record_mcount)						  \
	scripts/basic/fixdep $(depfile) $@ '$(call make-cmd,l4re_cxx_o_cc)' >   \
	                                              $(dot-target).tmp;  \
	rm -f $(depfile);						  \
	mv -f $(dot-target).tmp $(dot-target).cmd
endef

L4RE_CXX_CALL = $(L4_CXX) -Wp,-MD,$(depfile) -c -DL4SYS_USE_UTCB_WRAP=1 \
                -D__KERNEL__ -I$(srctree)/arch/l4/include \
		$(L4_CPPFLAGS) $(L4_CXXFLAGS) -fno-exceptions

quiet_cmd_l4re_cxx_o_cc = C++     $@

ifndef CONFIG_MODVERSIONS
  cmd_l4re_cxx_o_cc = $(L4RE_CXX_CALL) -o $@ $<
else
  cmd_l4re_cxx_o_cc = $(L4RE_CXX_CALL) -o $(@D)/.tmp_$(@F) $<
endif

$(obj)/%.o: $(src)/%.cc FORCE
	$(call cmd,force_checksrc)
	$(call if_changed_rule,l4re_cxx_o_cc)
