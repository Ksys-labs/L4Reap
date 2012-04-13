#
# l4/Makefile
#
# Makefile for the L4 architecture.
#

# BITS is used as extension for files which are available in a 32 bit
# and a 64 bit version to simplify shared Makefiles.
# e.g.: obj-y += foo_$(BITS).o
BITS := 32
export BITS

# Set this to really switch to ARM when coming from another arch
ifeq ($(L4ARCH),arm)
CONFIG_L4_ARCH_ARM := y
endif

ifneq ($(CONFIG_L4_ARCH_ARM),)
# arm selection
L4API		:= f
BASEARCH	:= arm
LINSRCARCH      := arm

CHECKFLAGS	:= -D__arm__

L4_MK_ARCH	:= arm
OBJCOPYARCH	:= elf32-littlearm

else

# x86 selection
L4API		:= f
BASEARCH	:= x86
LINSRCARCH      := x86

ifneq ($(CONFIG_X86_32),)
L4_MK_ARCH	:= x86
OBJCOPYARCH	:= elf32-i386
else
L4_MK_ARCH	:= amd64
OBJCOPYARCH	:= elf64-x86-64
endif

endif

ifeq ($(ARCH),x86)
	KBUILD_DEFCONFIG := x86-native_defconfig
else
	KBUILD_DEFCONFIG := $(ARCH)_defconfig
endif

VMLINUZ_SUFFIX = $(patsubst "%",%,$(CONFIG_L4_VMLINUZ_SUFFIX))

export BASEARCH L4_MK_ARCH L4_MK_API OBJCOPYARCH VMLINUZ_SUFFIX

# crude hack
TRASH := $(shell cd $(srctree)/arch/l4 \
                 && ln -fs Kconfig.$(LINSRCARCH) Kconfig.arch)

#HAS_BIARCH	:= $(call cc-option-yn, -m32)
#ifeq ($(HAS_BIARCH),y)
#AS		:= $(AS) --32
#LD		:= $(LD) -m elf_i386
#CC		:= $(CC) -m32
#endif

BASEENV		:= l4env

OBJCOPYFLAGS    := -O $(OBJCOPYARCH) -R .comment -S

L4_MK_API	:= l4$(L4API)

# overwrite UTS_MACHINE to get proper architecture in user land
UTS_MACHINE	:= $(BASEARCH)

KBUILD_CFLAGS   += -pipe -msoft-float -DARCH_$(L4_MK_ARCH)

ifeq ($(BASEARCH),x86)
ifeq ($(CONFIG_X86_32),y)
        BITS := 32
        UTS_MACHINE := i386
        CHECKFLAGS += -D__i386__

        biarch := $(call cc-option,-m32)
        KBUILD_AFLAGS += $(biarch)
        KBUILD_CFLAGS += $(biarch)

        ifdef CONFIG_RELOCATABLE
                LDFLAGS_vmlinux := --emit-relocs
        endif

        #KBUILD_CFLAGS += -msoft-float -mregparm=3 -freg-struct-return
	KBUILD_CFLAGS += -mregparm=3

        # prevent gcc from keeping the stack 16 byte aligned
        KBUILD_CFLAGS += $(call cc-option,-mpreferred-stack-boundary=2)

        # Disable unit-at-a-time mode on pre-gcc-4.0 compilers, it makes gcc use
        # a lot more stack due to the lack of sharing of stacklots:
        KBUILD_CFLAGS += $(call cc-ifversion, -lt, 0400, \
				$(call cc-option,-fno-unit-at-a-time))

        # CPU-specific tuning. Anything which can be shared with UML should go here.
        include $(srctree)/arch/x86/Makefile_32.cpu
        KBUILD_CFLAGS += $(cflags-y)

        # temporary until string.h is fixed
        KBUILD_CFLAGS += -ffreestanding
else
        BITS := 64
        UTS_MACHINE := x86_64
        CHECKFLAGS += -D__x86_64__ -m64

        KBUILD_AFLAGS += -m64
        KBUILD_CFLAGS += -m64

        # FIXME - should be integrated in Makefile.cpu (Makefile_32.cpu)
        cflags-$(CONFIG_MK8) += $(call cc-option,-march=k8)
        cflags-$(CONFIG_MPSC) += $(call cc-option,-march=nocona)

        cflags-$(CONFIG_MCORE2) += \
                $(call cc-option,-march=core2,$(call cc-option,-mtune=generic))
	cflags-$(CONFIG_MATOM) += $(call cc-option,-march=atom) \
		$(call cc-option,-mtune=atom,$(call cc-option,-mtune=generic))
        cflags-$(CONFIG_GENERIC_CPU) += $(call cc-option,-mtune=generic)
        KBUILD_CFLAGS += $(cflags-y)

        KBUILD_CFLAGS += -mno-red-zone
        KBUILD_CFLAGS += -mcmodel=kernel

        # -funit-at-a-time shrinks the kernel .text considerably
        # unfortunately it makes reading oopses harder.
        KBUILD_CFLAGS += $(call cc-option,-funit-at-a-time)

        # this works around some issues with generating unwind tables in older gccs
        # newer gccs do it by default
        KBUILD_CFLAGS += -maccumulate-outgoing-args
endif


# Don't unroll struct assignments with kmemcheck enabled
ifeq ($(CONFIG_KMEMCHECK),y)
	KBUILD_CFLAGS += $(call cc-option,-fno-builtin-memcpy)
endif

# Stackpointer is addressed different for 32 bit and 64 bit x86
sp-$(CONFIG_X86_32) := esp
sp-$(CONFIG_X86_64) := rsp

# do binutils support CFI?
cfi := $(call as-instr,.cfi_startproc\n.cfi_rel_offset $(sp-y)$(comma)0\n.cfi_endproc,-DCONFIG_AS_CFI=1)
# is .cfi_signal_frame supported too?
cfi-sigframe := $(call as-instr,.cfi_startproc\n.cfi_signal_frame\n.cfi_endproc,-DCONFIG_AS_CFI_SIGNAL_FRAME=1)
cfi-sections := $(call as-instr,.cfi_sections .debug_frame,-DCONFIG_AS_CFI_SECTIONS=1)

# does binutils support specific instructions?
asinstr := $(call as-instr,fxsaveq (%rax),-DCONFIG_AS_FXSAVEQ=1)

KBUILD_AFLAGS += $(cfi) $(cfi-sigframe) $(cfi-sections) $(asinstr)
KBUILD_CFLAGS += $(cfi) $(cfi-sigframe) $(cfi-sections) $(asinstr)

LDFLAGS := -m elf_$(UTS_MACHINE)

# Speed up the build
KBUILD_CFLAGS += -pipe
# Workaround for a gcc prelease that unfortunately was shipped in a suse release
KBUILD_CFLAGS += -Wno-sign-compare
#
KBUILD_CFLAGS += -fno-asynchronous-unwind-tables
# prevent gcc from generating any FP code by mistake
KBUILD_CFLAGS += $(call cc-option,-mno-sse -mno-mmx -mno-sse2 -mno-3dnow,)

endif

ifeq ($(BASEARCH),arm)
ifeq ($(CONFIG_CPU_ENDIAN_BE8),y)
LDFLAGS_vmlinux += --be8
endif
# This selects which instruction set is used.
# Note that GCC does not numerically define an architecture version
# macro, but instead defines a whole series of macros which makes
# testing for a specific architecture or later rather impossible.
arch-$(CONFIG_CPU_32v7)		:=-D__LINUX_ARM_ARCH__=7 $(call cc-option,-march=armv7-a,-march=armv5t -Wa$(comma)-march=armv7-a)
arch-$(CONFIG_CPU_32v6)		:=-D__LINUX_ARM_ARCH__=6 $(call cc-option,-march=armv6,-march=armv5t -Wa$(comma)-march=armv6)
# Only override the compiler option if ARMv6. The ARMv6K extensions are
# always available in ARMv7
ifeq ($(CONFIG_CPU_32v6),y)
arch-$(CONFIG_CPU_32v6K)	:=-D__LINUX_ARM_ARCH__=6 $(call cc-option,-march=armv6k,-march=armv5t -Wa$(comma)-march=armv6k)
endif
arch-$(CONFIG_CPU_32v5)		:=-D__LINUX_ARM_ARCH__=5 $(call cc-option,-march=armv5te,-march=armv4t)
arch-$(CONFIG_CPU_32v4T)	:=-D__LINUX_ARM_ARCH__=4 -march=armv4t
arch-$(CONFIG_CPU_32v4)		:=-D__LINUX_ARM_ARCH__=4 -march=armv4
arch-$(CONFIG_CPU_32v3)		:=-D__LINUX_ARM_ARCH__=3 -march=armv3

# This selects how we optimise for the processor.
tune-$(CONFIG_CPU_ARM610)	:=-mtune=arm610
tune-$(CONFIG_CPU_ARM710)	:=-mtune=arm710
tune-$(CONFIG_CPU_ARM7TDMI)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM720T)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM740T)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM9TDMI)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM940T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM946E)	:=$(call cc-option,-mtune=arm9e,-mtune=arm9tdmi)
tune-$(CONFIG_CPU_ARM920T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM922T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM925T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM926T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_FA526)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_SA110)	:=-mtune=strongarm110
tune-$(CONFIG_CPU_SA1100)	:=-mtune=strongarm1100
tune-$(CONFIG_CPU_XSCALE)	:=$(call cc-option,-mtune=xscale,-mtune=strongarm110) -Wa,-mcpu=xscale
tune-$(CONFIG_CPU_XSC3)		:=$(call cc-option,-mtune=xscale,-mtune=strongarm110) -Wa,-mcpu=xscale
tune-$(CONFIG_CPU_FEROCEON)	:=$(call cc-option,-mtune=marvell-f,-mtune=xscale)
tune-$(CONFIG_CPU_V6)		:=$(call cc-option,-mtune=arm1136j-s,-mtune=strongarm)
tune-$(CONFIG_CPU_V6K)		:=$(call cc-option,-mtune=arm1136j-s,-mtune=strongarm)

ifeq ($(CONFIG_AEABI),y)
CFLAGS_ABI	:=-mabi=aapcs-linux -mno-thumb-interwork
else
CFLAGS_ABI	:=$(call cc-option,-mapcs-32,-mabi=apcs-gnu) $(call cc-option,-mno-thumb-interwork,)
endif

ifeq ($(CONFIG_ARM_UNWIND),y)
CFLAGS_ABI	+=-funwind-tables
endif

ifeq ($(CONFIG_THUMB2_KERNEL),y)
AFLAGS_AUTOIT	:=$(call as-option,-Wa$(comma)-mimplicit-it=always,-Wa$(comma)-mauto-it)
AFLAGS_NOWARN	:=$(call as-option,-Wa$(comma)-mno-warn-deprecated,-Wa$(comma)-W)
CFLAGS_THUMB2	:=-mthumb $(AFLAGS_AUTOIT) $(AFLAGS_NOWARN)
AFLAGS_THUMB2	:=$(CFLAGS_THUMB2) -Wa$(comma)-mthumb
# Work around buggy relocation from gas if requested:
ifeq ($(CONFIG_THUMB2_AVOID_R_ARM_THM_JUMP11),y)
CFLAGS_MODULE	+=-fno-optimize-sibling-calls
endif
endif

# Need -Uarm for gcc < 3.x
KBUILD_CFLAGS	+=$(CFLAGS_ABI) $(CFLAGS_THUMB2) $(arch-y) $(tune-y) $(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,)) -msoft-float -Uarm
KBUILD_AFLAGS	+=$(CFLAGS_ABI) $(AFLAGS_THUMB2) $(arch-y) $(tune-y) -include asm/unified.h -msoft-float

# Never generate .eh_frame
KBUILD_CFLAGS   += $(call cc-option,-fno-dwarf2-cfi-asm)

ifeq ($(CONFIG_FRAME_POINTER),y)
KBUILD_CFLAGS	+=-fno-omit-frame-pointer -mapcs -mno-sched-prolog
endif

ifeq ($(CONFIG_CC_STACKPROTECTOR),y)
KBUILD_CFLAGS  +=-fstack-protector
endif

endif


# -----------------------------------------------

-include $(objtree)/Makeconf.l4conf

L4OBJ := $(patsubst "%",%,$(CONFIG_L4_OBJ_TREE))
L4DIR := $(shell readlink $(L4OBJ)/source)
export L4OBJ
export L4DIR


# Extra Libs
L4LX_E_L-$(CONFIG_L4_VCPU)         += libvcpu
L4LX_E_L-$(CONFIG_L4_USE_L4SHMC)   += shmc
L4LX_E_L-$(CONFIG_L4_BLK_DRV)      += generic_blk
L4LX_E_L-$(CONFIG_L4_EXTERNAL_RTC) += rtc
L4LX_E_L-$(CONFIG_L4_NET_ANKH_DRV) += ankh
L4LX_E_L-$(CONFIG_L4_IRQ_OMEGA0)   += omega0
L4LX_E_L-$(CONFIG_L4_PDSPBLK_DRV)  += pers_dsp
L4LX_E_L-$(CONFIG_L4_TCG_STPM)     += stpm-client
L4LX_E_L-$(CONFIG_L4_FERRET)       += ferret

L4INC		= -I$(L4OBJ)/include/$(L4_MK_ARCH)/$(L4_MK_API) \
		  -I$(L4OBJ)/include/$(L4_MK_ARCH) \
		  -I$(L4OBJ)/include/$(L4_MK_API) \
		  -I$(L4OBJ)/include

head-y := arch/l4/kernel/head.o arch/l4/kernel/init_task.o

libs-y				+= arch/l4/lib/arch-$(BASEARCH)/	\
				   arch/l4/l4lxlib/generic/		\
				   arch/l4/l4lxlib/$(BASEENV)/

drivers-$(CONFIG_PCI)		+= arch/l4/pci/

ifeq ($(BASEARCH),x86)
drivers-$(CONFIG_PM)		+= arch/l4/power/arch-x86/
drivers-$(CONFIG_FB)		+= arch/x86/video/
core-$(CONFIG_KVM)		+= arch/l4/kvm/arch-x86/
endif

ifeq ($(BASEARCH),arm)
core-y				+= arch/l4/common/arch-arm/
endif

core-y += arch/l4/

-include $(L4OBJ)/l4defs.mk.inc

L4_REQUIRED_MODS		:= libc_be_minimal_log_io \
                                   libc_minimal libc_minimal_l4re \
				   libsupc++_minimal log l4re_c-util \
                                   $(if $(CONFIG_L4_LINK_LIBIO_DIRECT),libio-direct,libio) \
				   $(L4LX_E_L-y)

L4_EXT_CFLAGS			:= $(call L4_BID_PKG_CONFIG_CALL,$(L4OBJ),--cflags --define-variable=libc_variant=libc,$(L4_REQUIRED_MODS))
ifneq ($(call L4_BID_PKG_CONFIG_FAILED,$(L4_EXT_CFLAGS)),)
$(info  Getting required compilation flags failed.)
$(info  L4OBJ: $(L4OBJ))
$(info  L4_REQUIRED_MODS: $(L4_REQUIRED_MODS))
$(error Aborting.)
endif

libs-y				+= -Bstatic $(L4_LIBDIRS) -ll4sys -ll4sys-direct
export L4INC L4_REQUIRED_MODS

LDFLAGS_vmlinux  =  \
  --defsym __L4_KIP_ADDR__=$(L4_BID_KIP_ADDR) \
  --defsym __l4sys_invoke_direct=$(L4_BID_KIP_ADDR)+$(L4_BID_KIP_OFFS_SYS_INVOKE) \
  --defsym __l4sys_debugger_direct=$(L4_BID_KIP_ADDR)+$(L4_BID_KIP_OFFS_SYS_DEBUGGER)

KBUILD_CPPFLAGS	+= -Iarch/l4/include/asm/l4-arch \
		   -I$(src)/arch/$(LINSRCARCH)/include \
		   -Iarch/l4/include/asm/$(LINSRCARCH)-arch \
		   -Iarch/l4/include/asm/orig-arch \
                   -Iarch/l4/include \
                   $(L4INC) $(L4_EXT_CFLAGS) \
		   $(CPPFLAGS_SYSENTER-y) \
		   $(L4BID_CPPFLAGS_SYSCALLS-y)


# for DICE
KBUILD_CPPFLAGS	+= -DL4API_$(L4_MK_API)

KBUILD_CPPFLAGS += -DL4SYS_USE_UTCB_WRAP=1

ifneq ($(CONFIG_L4_ARCH_ARM),)
KBUILD_CPPFLAGS += -DTEXT_OFFSET=0x0100000
endif

all: lImage

boot := arch/l4/boot

lImage: check_for_l4defs $(if $(CONFIG_L4_SERVER),server) vmlinux
	$(Q)$(MAKE) $(build)=$(boot) $@

check_for_l4defs:
	$(Q)if [ ! -e $(L4OBJ)/l4defs.mk.inc ]; then \
	  echo "No $(L4OBJ)/l4defs.mk.inc available."; \
	  echo "Did you run 'make' in $(L4OBJ) to a successful end?"; \
	  exit 1; \
	fi

server:
	$(Q)$(MAKE) $(build)=arch/l4/server

PHONY += server check_for_l4defs

mtags:
	(cd $(srctree) && $(MAKE) tags > /dev/null 2>&1; \
	sort tags > .tags.sorting && mv .tags.sorting tags )

arch/l4/include/asm/$(LINSRCARCH)-arch/mach:
	@echo '  Creating asm-l4/$(LINSRCARCH)-arch/mach symlink'
	$(Q)mkdir -p $(@D)
	$(Q)ln -s $(src)/arch/l4/include/asm/arch-$(LINSRCARCH)/mach $@

arch/l4/include/asm/l4-arch/asm:
	@echo '  Creating asm-l4/l4-arch/asm symlink'
	$(Q)mkdir -p $(@D)
	$(Q)ln -s $(src)/arch/l4/include/asm/arch-$(BASEARCH) $@

arch/l4/include/asm/arch:
	@echo '  Creating asm-l4/arch-$(BASEARCH)/arch -> asm/arch symlink'
	$(Q)mkdir -p $(@D)
	$(Q)ln -s $(src)/arch/l4/include/asm/arch-$(BASEARCH)/arch $@

arch/l4/include/asm/api:
	@echo '  Creating asm-l4/api-$(BASEENV) -> asm/api symlink'
	$(Q)mkdir -p $(@D)
	$(Q)ln -s $(src)/arch/l4/include/asm/api-$(BASEENV) $@

arch/l4/include/asm/l4x:
	@echo '  Creating asm-l4/l4x-$(BASEARCH) -> asm/l4x symlink'
	$(Q)mkdir -p $(@D)
	$(Q)ln -s $(src)/arch/l4/include/asm/l4x-$(BASEARCH) $@

arch/l4/include/asm/l4lxapi/impl:
	@echo '  Creating asm-l4/l4lxapi/impl symlink'
	$(Q)mkdir -p $(@D)
	$(Q)ln -s $(src)/arch/l4/l4lxlib/l4env $@

arch/l4/include/asm/orig-arch:
	@echo '  Creating asm-l4/orig-arch symlink'
	$(Q)mkdir -p $@
	$(Q)ln -s $(src)/arch/$(LINSRCARCH)/include/asm $@/asm-$(LINSRCARCH) 

include/generated/mach-types.h: $(srctree)/arch/arm/tools/gen-mach-types $(srctree)/arch/arm/tools/mach-types
	$(Q)cp $(srctree)/arch/arm/tools/mach-types arch/l4/include/asm/mach-types.list
	$(Q)echo "l4	MACH_L4	L4	9999" >> arch/l4/include/asm/mach-types.list
	$(Q)$(AWK) -f $< arch/l4/include/asm/mach-types.list > $@ || { rm -f $@; /bin/false; }
	$(Q)rm -f arch/l4/include/asm/mach-types.list

symlink_files_arm  = include/generated/mach-types.h \
		     arch/l4/include/asm/$(LINSRCARCH)-arch/mach

symlink_files      = arch/l4/include/asm/api \
                     arch/l4/include/asm/l4lxapi/impl \
		     arch/l4/include/asm/l4x \
		     arch/l4/include/asm/l4-arch/asm \
                     $(symlink_files_$(BASEARCH))

symlink_dirs       = arch/l4/include/asm/orig-arch

MRPROPER_FILES	  += $(symlink_files)
MRPROPER_DIRS     += $(symlink_dirs)
CLEAN_FILES	  += Makeconf.l4conf

ifeq ($(config-targets),0)
ifneq ($(wildcard .config),)
$(objtree)/Makeconf.l4conf: $(srctree)/arch/l4/Makefile.l4conf $(srctree)/arch/l4/Makefile
	$(Q)$(if $(L4DIR),PWD=$(L4DIR),) $(MAKE) $(if $(L4DIR),-C $(L4DIR),) -f $< O=$(L4OBJ) OUTPUT=$@

archprepare: print_instructions check_linux_config $(symlink_files) $(symlink_dirs) $(objtree)/Makeconf.l4conf
	$(Q)mkdir -p arch/$(BASEARCH)/lib

endif # .config available
else # no archprepare when configuring the kernel
archprepare:
endif # config-targets == 1

# ----

archclean:

chkl4conf:
	@if [ -d $(L4OBJ)/pkg/l4sys ]; then                                  \
		echo "Configuration looks Ok.";                              \
	else                                                                 \
		echo \"$(L4OBJ)\" does not look like an L4 build directory!; \
	fi

## Check a few options that should not be enabled
# regparm: gcc may call memset from our libs, so obey calling convention
#          this should probably be fixed

DIS_OPTS-x86-32 := HPET_TIMER
DIS_OPTS := HIGHMEM4G HIGHMEM64G PCI_GOANY PCI_GOBIOS \
            X86_MCE X86_MCE MTRR PARAVIRT KEXEC MCA SOFTWARE_SUSPEND APM \
	    CPU_FREQ VM86 X86_DS X86_PTRACE_BTS HT_IRQ DMI \
	    TRANSPARENT_HUGEPAGE COMPACTION X86_PAE \
	    $(DIS_OPTS-$(BASEARCH)-$(BITS))
ENA_OPTS := $(ENA_OPTS-$(BASEARCH))

print_instructions:
	@if [ ! -e vmlinuz$(VMLINUZ_SUFFIX) ]; then \
	  echo ===============================================; \
	  echo "  For quick build instructions, please visit:"; \
	  echo "    http://wiki.tudos.org/Quickstart"; \
	  echo "    http://l4linux.org/build.shtml"; \
	  echo ===============================================; \
	fi

check_linux_config:
	@$(foreach i,$(addprefix CONFIG_,$(DIS_OPTS)),     \
	  if [ "$($(i))" = "y" ]; then                     \
	    echo; echo "ERROR: $(i) must not be enabled."; \
	    failed=1;                                      \
	  fi;)                                             \
	  if [ -n "$$failed" ]; then                       \
	    echo; exit 1;                                  \
	  fi
	@$(foreach i,$(addprefix CONFIG_,$(ENA_OPTS)),     \
	  if [ "$($(i))" != "y" ]; then                    \
	    echo; echo "ERROR: $(i) must be enabled.";     \
	    failed=1;                                      \
	  fi;)                                             \
	  if [ -n "$$failed" ]; then                       \
	    echo; exit 1;                                  \
	  fi
	@if [ "$(CONFIG_X86_32)" = "y" -a "$(CONFIG_PAGE_OFFSET)" != "0" ]; then \
	  echo "CONFIG_PAGE_OFFSET must be 0."; \
	  exit 1; \
	fi

PHONY += chkl4conf check_linux_config

define archhelp
  echo '* vmlinuz	- Compressed kernel image'
  echo '  chkl4conf     - Check L4 specific configuration'
endef

