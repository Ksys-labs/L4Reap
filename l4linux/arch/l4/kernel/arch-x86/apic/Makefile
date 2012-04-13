
obj-$(CONFIG_X86_LOCAL_APIC)	+= apic.o apic_noop.o ipi.o


obj-$(CONFIG_X86_IO_APIC)	+= io_apic.o
obj-$(CONFIG_SMP)		+= ipi.o

ifeq ($(CONFIG_X86_64),y)
# APIC probe will depend on the listing order here
obj-$(CONFIG_X86_UV)		+= x2apic_uv_x.o
obj-$(CONFIG_X86_X2APIC)	+= x2apic_phys.o
obj-$(CONFIG_X86_X2APIC)	+= x2apic_cluster.o
obj-y				+= apic_flat_64.o
endif

# APIC probe will depend on the listing order here
obj-$(CONFIG_X86_NUMAQ)		+= numaq_32.o
obj-$(CONFIG_X86_SUMMIT)	+= summit_32.o
obj-$(CONFIG_X86_BIGSMP)	+= bigsmp_32.o
obj-$(CONFIG_X86_ES7000)	+= es7000_32.o

# For 32bit, probe_32 need to be listed last
obj-$(CONFIG_X86_LOCAL_APIC)	+= probe_$(BITS).o

# -------------------------------------------
b		:= ../../../../x86/kernel/apic
apic_noop-y	:= $(b)/apic_noop.o
apic_flat_64-y  := $(b)/apic_flat_64.o
probe_$(BITS)-y := $(b)/probe_$(BITS).o
