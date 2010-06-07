PKGDIR ?= ../..
L4DIR ?= $(PKGDIR)/../..

SYSTEMS = x86-l4f arm-l4f

# the name of your library
TARGET	= liblwip.a

# list your .c or .cc files here
SRC_C	= arch/sys_arch.c \
		  arch/perf.c \
		  arch/ankh_if.c \
		  contrib/src/core/def.c \
		  contrib/src/core/dhcp.c \
		  contrib/src/core/init.c \
		  contrib/src/core/dns.c \
		  contrib/src/core/mem.c \
		  contrib/src/core/memp.c \
		  contrib/src/core/netif.c \
		  contrib/src/core/pbuf.c \
		  contrib/src/core/raw.c \
		  contrib/src/core/stats.c \
		  contrib/src/core/sys.c \
		  contrib/src/core/tcp.c \
		  contrib/src/core/tcp_in.c \
		  contrib/src/core/tcp_out.c \
		  contrib/src/core/timers.c \
		  contrib/src/core/udp.c \
		  contrib/src/api/api_lib.c \
		  contrib/src/api/api_msg.c \
		  contrib/src/api/err.c \
		  contrib/src/api/netbuf.c \
		  contrib/src/api/netdb.c \
		  contrib/src/api/netifapi.c \
		  contrib/src/api/sockets.c \
		  contrib/src/api/tcpip.c \
		  contrib/src/netif/etharp.c \
		  contrib/src/core/ipv4/autoip.c \
		  contrib/src/core/ipv4/icmp.c \
		  contrib/src/core/ipv4/igmp.c \
		  contrib/src/core/ipv4/inet.c \
		  contrib/src/core/ipv4/inet_chksum.c \
		  contrib/src/core/ipv4/ip_addr.c \
		  contrib/src/core/ipv4/ip.c \
		  contrib/src/core/ipv4/ip_frag.c \
		  contrib/src/core/snmp/asn1_dec.c \
		  contrib/src/core/snmp/asn1_enc.c \
		  contrib/src/core/snmp/mib2.c \
		  contrib/src/core/snmp/mib_structs.c \
		  contrib/src/core/snmp/msg_in.c \
		  contrib/src/core/snmp/msg_out.c 

PRIVATE_INCDIR += $(PKGDIR)/lib/lwip/contrib/src/include \
				  $(PKGDIR)/lib/lwip/contrib/src/include/ipv4 \
				  $(PKGDIR)/lib/lwip

PC_FILENAME = ankh_lwip

L4_MULTITHREADED = y

include $(L4DIR)/mk/lib.mk

CFLAGS += -Wno-unused-function
