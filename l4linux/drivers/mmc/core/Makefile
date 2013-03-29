#
# Makefile for the kernel mmc core.
#

obj-$(CONFIG_MMC)		+= mmc_core.o
mmc_core-y			:= core.o bus.o host.o \
				   mmc.o mmc_ops.o sd.o sd_ops.o \
				   sdio.o sdio_ops.o sdio_bus.o \
				   sdio_cis.o sdio_io.o sdio_irq.o \
				   quirks.o cd-gpio.o

mmc_core-$(CONFIG_DEBUG_FS)	+= debugfs.o

ifeq ($(CONFIG_L4_PLAT_OVERO), y)
KBUILD_CPPFLAGS += -Iarch/l4/kernel/arch-arm/plat-omap/include
KBUILD_CPPFLAGS += -Iarch/l4/kernel/arch-arm/mach-omap2/include
endif

ifeq ($(CONFIG_L4_PLAT_IGEP), y)
KBUILD_CPPFLAGS += -Iarch/l4/kernel/arch-arm/plat-omap/include
KBUILD_CPPFLAGS += -Iarch/l4/kernel/arch-arm/mach-omap2/include
endif
