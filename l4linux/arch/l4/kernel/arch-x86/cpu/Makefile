#
# Makefile for x86-compatible CPU details, features and quirks
#

# Don't trace early stages of a secondary CPU boot
ifdef CONFIG_FUNCTION_TRACER
CFLAGS_REMOVE_common.o = -pg
CFLAGS_REMOVE_perf_event.o = -pg
endif

# Make sure load_percpu_segment has no stackprotector
nostackp := $(call cc-option, -fno-stack-protector)
CFLAGS_common.o		:= $(nostackp)

obj-y			:= intel_cacheinfo.o scattered.o topology.o
obj-y			+= proc.o capflags.o powerflags.o common.o
obj-y			+= vmware.o hypervisor.o sched.o mshyperv.o

obj-$(CONFIG_X86_32)	+= bugs.o
obj-$(CONFIG_X86_64)	+= bugs_64.o

obj-$(CONFIG_CPU_SUP_INTEL)		+= intel.o
obj-$(CONFIG_CPU_SUP_AMD)		+= amd.o
obj-$(CONFIG_CPU_SUP_CYRIX_32)		+= cyrix.o
obj-$(CONFIG_CPU_SUP_CENTAUR)		+= centaur.o
obj-$(CONFIG_CPU_SUP_TRANSMETA_32)	+= transmeta.o
obj-$(CONFIG_CPU_SUP_UMC_32)		+= umc.o

obj-$(CONFIG_PERF_EVENTS)		+= perf_event.o

#obj-$(CONFIG_X86_MCE)			+= mcheck/
#obj-$(CONFIG_MTRR)			+= mtrr/

obj-$(CONFIG_X86_LOCAL_APIC)		+= perfctr-watchdog.o

b                      := ../../../../x86/kernel/cpu

quiet_cmd_mkcapflags = MKCAP   $@
      cmd_mkcapflags = $(PERL) $(srctree)/$(src)/$(b)/mkcapflags.pl $< $@

cpufeature = $(src)/../../../../x86/include/asm/cpufeature.h

targets += capflags.c
$(obj)/capflags.c: $(cpufeature) $(src)/$(b)/mkcapflags.pl FORCE
	$(call if_changed,mkcapflags)

KBUILD_CFLAGS += -I$(srctree)/arch/x86/kernel/cpu

amd-y		       := $(b)/amd.o
cyrix-y		       := $(b)/cyrix.o
centaur-y	       := $(b)/centaur.o
transmeta-y	       := $(b)/transmeta.o
intel_cacheinfo-y      := $(b)/intel_cacheinfo.o
rise-y		       := $(b)/rise.o
nexgen-y	       := $(b)/nexgen.o
umc-y		       := $(b)/umc.o
proc-y		       := $(b)/proc.o
bugs-y		       := $(b)/bugs.o
bugs_64-y	       := $(b)/bugs_64.o
perfctr-watchdog-y     := $(b)/perfctr-watchdog.o
feature_names-y        := $(b)/feature_names.o
powerflags-y           := $(b)/powerflags.o
vmware-y               := $(b)/vmware.o
hypervisor-y           := $(b)/hypervisor.o
sched-y                := $(b)/sched.o
mshyperv-y             := $(b)/mshyperv.o
scattered-y            := $(b)/scattered.o
topology-y             := $(b)/topology.o
