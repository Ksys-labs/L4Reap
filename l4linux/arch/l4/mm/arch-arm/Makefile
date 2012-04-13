#
# arch/l4/mm/arch-arm/Makefile
#

obj-y		:= init.o flush.o ioremap.o proc-l4.o dma-mapping.o \
                   fault.o fault-armv.o extable.o mmap.o \
		   pgd.o mmu.o iomap.o vmregion.o idmap.o

obj-$(CONFIG_CPU_COPY_V6) += context.o

KBUILD_CPPFLAGS += -I$(srctree)/arch/arm/mm

extable-y	:= ../../../arm/mm/extable.o
fault-armv-y	:= ../../../arm/mm/fault-armv.o
mmap-y		:= ../../../arm/mm/mmap.o
iomap-y		:= ../../../arm/mm/iomap.o
vmregion-y	:= ../../../arm/mm/vmregion.o
idmap-y		:= ../../../arm/mm/idmap.o
