#
# arch/l4/boot/Makefile
#

quiet_cmd_localinst = LOCINST $@
      cmd_localinst = ([ -x "$(srctree)/arch/l4/install.local" ] && \
                       $(srctree)/arch/l4/install.local $@ $(BASEARCH)) || true

$(obj)/Image: vmlinux FORCE
	$(call if_changed,objcopy)
	@echo '  Kernel: $@ is ready'

lImage: $(obj)/Image vmlinuz$(VMLINUZ_SUFFIX) l4linux$(VMLINUZ_SUFFIX)

vmlinuz$(VMLINUZ_SUFFIX): ldrImage FORCE
	$(call if_changed,gzip)
	$(call cmd,localinst)

l4linux$(VMLINUZ_SUFFIX): ldrImage FORCE
	$(Q)ln -sf $< $@

objects-y := ldr.o res.o image.o

include $(L4OBJ)/l4defs.mk.inc

KBUILD_CFLAGS	:= $(filter-out -mregparm=3,$(KBUILD_CFLAGS)) \
                   $(L4INC) -I$(L4OBJ)/include/uclibc \
		   -Wall -fno-strict-aliasing -O2 -pipe \
		   -DARCH_$(L4_MK_ARCH) -g \
		   -I$(obj) -DL4SYS_USE_UTCB_WRAP=1

KBUILD_AFLAGS   += -DVMLINUX_IMAGE=\"$(obj)/Image\"

$(obj)/res.o: $(obj)/func_list.h

$(obj)/image.o: $(obj)/Image

$(obj)/func_list.h: vmlinux
	$(Q)$(OBJCOPY) -j .data.l4externals.str -O binary $< $@.tmp
	$(Q)perl -p -e 's/(.+?)\0/EF($$1)\n/g' $@.tmp > $@

OBJS := $(addprefix $(obj)/,$(objects-y))

L4_EXT_LIBS := $(call L4_BID_PKG_CONFIG_CALL,$(L4OBJ),--libs --define-variable=libc_variant=libc,$(L4_REQUIRED_MODS))

ifneq ($(call L4_BID_PKG_CONFIG_FAILED,$(L4_EXT_LIBS)),)
$(info  Getting required libraries failed.)
$(info  L4OBJ: $(L4OBJ))
$(info  L4_REQUIRED_MODS: $(L4_REQUIRED_MODS))
$(error Aborting.)
endif

L4LIBS := $(shell $(CC) $(KBUILD_CFLAGS) \
          -print-file-name=libgcc_eh.a) $(L4_EXT_LIBS)

servers-y := $(if $(CONFIG_L4_SERVER),arch/l4/server/lib.a)

LINKADDR-32 = 0xa8000000
LINKADDR-64 = 0x68000000

ldrImage: $(OBJS)
	@echo "  LD      $@"
	$(Q)$(LD) $(LDFLAGS) -Bstatic -o $@ \
	  $(L4_LIBDIRS) \
	  $(L4_CRT0_STATIC) $(OBJS) $(servers-y) \
	  --start-group $(L4LIBS) $(L4_GCCLIB) --end-group \
	  $(L4_CRTN_STATIC) \
	  --defsym __executable_start=$(LINKADDR-$(BITS)) \
	  --defsym __L4_STACK_ADDR__=$(L4_BID_STACK_ADDR) \
	  --defsym __L4_KIP_ADDR__=$(L4_BID_KIP_ADDR) \
          --defsym __l4sys_invoke_direct=$(L4_BID_KIP_ADDR)+$(L4_BID_KIP_OFFS_SYS_INVOKE) \
          --defsym __l4sys_debugger_direct=$(L4_BID_KIP_ADDR)+$(L4_BID_KIP_OFFS_SYS_DEBUGGER) \
	  -T$(L4_LDS_stat_bin)

PHONY += lImage
