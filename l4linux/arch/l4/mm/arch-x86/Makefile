#
# arch/l4/mm/Makefile
#


obj-y	:= fault.o extable.o init.o init_$(BITS).o ioremap.o pageattr.o \
           pgtable.o mmap.o pat.o setup_nx.o physaddr.o

# Make sure __phys_addr has no stackprotector
nostackp := $(call cc-option, -fno-stack-protector)
CFLAGS_physaddr.o               := $(nostackp)
CFLAGS_setup_nx.o               := $(nostackp)

obj-$(CONFIG_X86_32)            += pgtable_32.o

obj-$(CONFIG_X86_PAT)		+= pat_rbtree.o
obj-$(CONFIG_SMP)		+= tlb.o
obj-$(CONFIG_HUGETLB_PAGE)	+= hugetlbpage.o
obj-$(CONFIG_HAVE_MEMBLOCK)	+= memblock.o

extable-y	+= ../../../x86/mm/extable.o
mmap-y		+= ../../../x86/mm/mmap.o
pat-y		+= ../../../x86/mm/pat.o
pageattr-y	+= ../../../x86/mm/pageattr.o
pgtable_32-y	+= ../../../x86/mm/pgtable_32.o
setup_nx-y	+= ../../../x86/mm/setup_nx.o
hugetlbpage-y	+= ../../../x86/mm/hugetlbpage.o
memblock-y	+= ../../../x86/mm/memblock.o

EXTRA_CFLAGS    += -Iarch/x86/mm
