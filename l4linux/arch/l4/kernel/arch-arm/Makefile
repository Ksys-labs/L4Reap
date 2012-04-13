#
# arch/l4/kernel/arch-arm/Makefile
#

obj-y		:= process.o sys_arm.o time.o traps.o stacktrace.o \
		   switch_to.o setup.o l4.o signal.o irq.o \
		   ptrace.o entry.o dispatch.o mach_setup.o io.o elf.o \
		   return_address.o head.o

obj-$(CONFIG_DEPRECATED_PARAM_STRUCT) += compat.o

obj-$(CONFIG_SMP)				+= smp.o smp_tlb.o platsmp.o
obj-$(CONFIG_HAVE_SCHED_CLOCK)			+= sched_clock.o
obj-$(CONFIG_MODULES)				+= armksyms.o module.o
obj-$(CONFIG_ISA_DMA_API)			+= dma.o
obj-$(CONFIG_HOTPLUG_CPU)			+= hotplug.o
obj-$(CONFIG_ARM_UNWIND)			+= unwind.o
obj-$(CONFIG_OABI_COMPAT)                       += sys_oabi-compat.o
obj-$(CONFIG_CRASH_DUMP)			+= crash_dump.o
obj-$(CONFIG_OF)				+= devtree.o
obj-$(CONFIG_SWP_EMULATE)			+= swp_emulate.o
CFLAGS_swp_emulate.o				:= -Wa,-march=armv7-a

dma-y		:= ../../../arm/kernel/dma.o
irq-y		:= ../../../arm/kernel/irq.o
module-y	:= ../../../arm/kernel/module.o
time-y		:= ../../../arm/kernel/time.o
io-y		:= ../../../arm/kernel/io.o
elf-y		:= ../../../arm/kernel/elf.o
unwind-y	:= ../../../arm/kernel/unwind.o
stacktrace-y	:= ../../../arm/kernel/stacktrace.o
return_address-y := ../../../arm/kernel/return_address.o
sys_oabi-compat-y := ../../../arm/kernel/sys_oabi-compat.o
compat-y        := ../../../arm/kernel/compat.o
crash_dump-y    := ../../../arm/kernel/crash_dump.o
smp_tlb-y       := ../../../arm/kernel/smp_tlb.o
sched_clock-y   := ../../../arm/kernel/sched_clock.o
devtree-y       := ../../../arm/kernel/devtree.o

KBUILD_CPPFLAGS	+= -I$(srctree)/arch/arm/kernel

CPPFLAGS_vmlinux.lds += -DTEXT_OFFSET=0x0100000

CFLAGS_REMOVE_return_address.o = -pg


# ----------------------------------------------------------------------
# _upage_start must(!) be last so that ld gets its calculation right!
define get_upage_defsyms
	upage_base=0xbf008000;						\
	for i in $$($(NM) $< | cut -d\  -f3 | grep -vw l4x_upage_start | grep -ie '^[a-z]');	\
	do								\
		echo -defsym $$i=$$i-l4x_upage_start+$$upage_base;	\
	done;								\
	echo -defsym l4x_upage_start=$$upage_base;
endef

obj-y		+= upage.reloced.o
targets		+= upage.o

quiet_cmd_upage = UPAGE   $@
      cmd_upage = $(LD) $(EXTRA_LDFLAGS) -N \
                        $(shell $(get_upage_defsyms)) -T /dev/null -o $@ $<

$(obj)/upage.reloced.o: $(obj)/upage.o FORCE
	$(call if_changed,upage)
