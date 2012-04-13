#
# arch/l4/kernel/Makefile
#

extra-y		:= head.o init_task.o vmlinux.lds

AFLAGS_head.o   := -DARCH_$(L4_MK_ARCH)

obj-y		:= main.o process_gen.o hybrid.o syscall_guard.o irq.o \
		   signal.o clocksource.o

obj-$(CONFIG_EARLY_PRINTK)	+= early_printk.o
obj-$(CONFIG_L4_TAMED)		+= tamed.o
obj-$(CONFIG_L4_FERRET)		+= ferret.o

obj-$(CONFIG_L4_SAMPLE_MODULE)	+= l4x_module.o

CFLAGS_l4lib.o-$(CONFIG_X86_32) := -Werror -mregparm=0
CFLAGS_l4lib.o                  := $(CFLAGS_l4lib.o-y)

export CPPFLAGS_vmlinux.lds += -U$(UTS_MACHINE) -DARCH_$(L4_MK_ARCH)
