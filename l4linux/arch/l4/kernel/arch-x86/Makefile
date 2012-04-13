#
# arch/l4/kernel/arch-x86/Makefile
#

obj-y		:= dispatch.o entry.o ioport.o \
		   ldt.o pci-dma.o pci-nommu.o process.o process_$(BITS).o ptrace.o reboot.o \
		   setup.o signal.o time.o traps.o e820.o \
		   unimpl.o topology.o alternative.o tsc.o \
		   tls.o step.o rtc.o io_delay.o memory.o \
		   kdebugfs.o x86_init.o i8259.o

#extra-$(CONFIG_X86_64)	+= head_$(BITS).o
obj-$(CONFIG_X86_64)	+= head_$(BITS).o

#
# vsyscalls (which work on the user stack) should have
# no stack-protector checks:
#
nostackp := $(call cc-option, -fno-stack-protector)
CFLAGS_vsyscall_64.o	:= $(PROFILING) -g0 $(nostackp)
CFLAGS_hpet.o		:= $(nostackp)
CFLAGS_vread_tsc_64.o	:= $(nostackp)
CFLAGS_paravirt.o	:= $(nostackp)
GCOV_PROFILE_vsyscall_64.o	:= n
GCOV_PROFILE_hpet.o		:= n
GCOV_PROFILE_tsc.o		:= n
GCOV_PROFILE_vread_tsc_64.o	:= n
GCOV_PROFILE_paravirt.o		:= n

# vread_tsc_64 is hot and should be fully optimized:
CFLAGS_REMOVE_vread_tsc_64.o = -pg -fno-optimize-sibling-calls

obj-y				+= i387.o xsave.o
obj-y				+= dumpstack_$(BITS).o
obj-y				+= irq.o irq_$(BITS).o irq_l4.o irqinit.o
obj-y				+= dumpstack.o
obj-y				+= hw_breakpoint.o
obj-y				+= jump_label.o
obj-y				+= pci-iommu_table.o
obj-y				+= resource.o
obj-y				+= head$(BITS).o
obj-y				+= probe_roms.o
obj-$(CONFIG_X86_32)		+= sys_i386_32.o
obj-$(CONFIG_X86_64)            += syscall_64.o vsyscall_64.o vread_tsc_64.o
obj-$(CONFIG_X86_64)            += sys_x86_64.o #x8664_ksyms_64.o
obj-y				+= quirks.o

obj-$(CONFIG_STACKTRACE)	+= stacktrace.o
obj-y				+= cpu/
obj-$(CONFIG_ACPI)		+= acpi.o
obj-$(CONFIG_MODULES)		+= module.o
obj-$(CONFIG_KPROBES)		+= kprobes.o
obj-$(CONFIG_AMD_NB)		+= amd_nb.o
obj-$(CONFIG_PCI)               += early-quirks.o
obj-y				+= apic/
obj-$(CONFIG_X86_MPPARSE)	+= mpparse.o
obj-$(CONFIG_SMP)		+= smp.o smpboot.o setup_percpu.o
obj-y				+= trampoline.o
obj-$(CONFIG_HPET_TIMER)	+= hpet.o

# Make sure this is linked after any other paravirt_ops structs: see head.S
obj-$(CONFIG_PARAVIRT)		+= paravirt.o

obj-$(CONFIG_SWIOTLB)		+= pci-swiotlb.o
obj-$(CONFIG_OF)                += devicetree.o


###
# 64 bit specific files
ifeq ($(CONFIG_X86_64),y)
	obj-y			+= vsmp_64.o
endif

# -------------------------------------------
b		:= ../../../x86/kernel
alternative-y	:= $(b)/alternative.o
ldt-y		:= $(b)/ldt.o
module-y	:= $(b)/module.o
pci-nommu-y	:= $(b)/pci-nommu.o
pci-dma-y	:= $(b)/pci-dma.o
sys_i386_32-y	:= $(b)/sys_i386_32.o
topology-y	:= $(b)/topology.o
xsave-y		:= $(b)/xsave.o
amd_nb-y	+= $(b)/amd_nb.o
stacktrace-y	+= $(b)/stacktrace.o
tls-y		+= $(b)/tls.o
step-y		+= $(b)/step.o
ds-y		+= $(b)/ds.o
io_delay-y	+= $(b)/io_delay.o
trampoline-y	+= $(b)/trampoline.o
setup_percpu-y  += $(b)/setup_percpu.o
probe_roms-y	+= $(b)/probe_roms.o
early-quirks-y	+= $(b)/early-quirks.o
kdebugfs-y	+= $(b)/kdebugfs.o
dumpstack_$(BITS)-y	+= $(b)/dumpstack_$(BITS).o
dumpstack-y	+= $(b)/dumpstack.o
hw_breakpoint-y += $(b)/hw_breakpoint.o
i8259-y         += $(b)/i8259.o
jump_label-y    += $(b)/jump_label.o
resource-y      += $(b)/resource.o
time-y          += $(b)/time.o
pci-iommu_table-y += $(b)/pci-iommu_table.o
syscall_64-y    += $(b)/syscall_64.o
#vsyscall_64-y   += $(b)/vsyscall_64.o
vread_tsc_64-y	+= $(b)/vread_tsc_64.o
hpet-y          += $(b)/hpet.o
quirks-y        += $(b)/quirks.o
sys_x86_64-y	+= $(b)/sys_x86_64.o
pci-swiotlb-y	+= $(b)/pci-swiotlb.o
devicetree-y	+= $(b)/devicetree.o
vsmp_64-y	+= $(b)/vsmp_64.o
x8664_ksyms_64-y += $(b)/x8664_ksyms_64.o

# we have our own versions of destroy_context and init_new_context,
# so rename the original one
CFLAGS_ldt.o    += -Ddestroy_context=destroy_context_origarch
CFLAGS_ldt.o    += -Dinit_new_context=init_new_context_origarch

# syscall_table_32.S
KBUILD_AFLAGS	+= -Iarch/x86/kernel
# headers
KBUILD_CFLAGS	+= -Iarch/x86/kernel
