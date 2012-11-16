PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= libacpica.a libacpica.so
CONTRIB_INCDIR  = acpica

IASL ?= iasl

ACPICA_SRC_C		= \
	disassembler/dmbuffer.c \
	disassembler/dmnames.c \
	disassembler/dmobject.c \
	disassembler/dmopcode.c \
	disassembler/dmresrc.c \
	disassembler/dmresrcl.c \
	disassembler/dmresrcl2.c \
	disassembler/dmresrcs.c \
	disassembler/dmutils.c \
	disassembler/dmwalk.c \
	dispatcher/dsargs.c \
	dispatcher/dscontrol.c \
	dispatcher/dsfield.c \
	dispatcher/dsinit.c \
	dispatcher/dsmethod.c \
	dispatcher/dsmthdat.c \
	dispatcher/dsobject.c \
	dispatcher/dsopcode.c \
	dispatcher/dsutils.c \
	dispatcher/dswexec.c \
	dispatcher/dswload.c \
	dispatcher/dswload2.c \
	dispatcher/dswscope.c \
	dispatcher/dswstate.c \
	events/evevent.c \
	events/evglock.c \
	events/evgpe.c \
	events/evgpeblk.c \
	events/evgpeinit.c \
	events/evgpeutil.c \
	events/evmisc.c \
	events/evregion.c \
	events/evrgnini.c \
	events/evsci.c \
	events/evxface.c \
	events/evxfevnt.c \
	events/evxfregn.c \
	executer/exconfig.c \
	executer/exconvrt.c \
	executer/excreate.c \
	executer/exdebug.c \
	executer/exdump.c \
	executer/exfield.c \
	executer/exfldio.c \
	executer/exmisc.c \
	executer/exmutex.c \
	executer/exnames.c \
	executer/exoparg1.c \
	executer/exoparg2.c \
	executer/exoparg3.c \
	executer/exoparg6.c \
	executer/exprep.c \
	executer/exregion.c \
	executer/exresnte.c \
	executer/exresolv.c \
	executer/exresop.c \
	executer/exstore.c \
	executer/exstoren.c \
	executer/exstorob.c \
	executer/exsystem.c \
	executer/exutils.c \
	hardware/hwacpi.c \
	hardware/hwgpe.c \
	hardware/hwpci.c \
	hardware/hwregs.c \
	hardware/hwsleep.c \
	hardware/hwesleep.c \
	hardware/hwvalid.c \
	hardware/hwxface.c \
	namespace/nsaccess.c \
	namespace/nsalloc.c \
	namespace/nsdump.c \
	namespace/nseval.c \
	namespace/nsinit.c \
	namespace/nsload.c \
	namespace/nsnames.c \
	namespace/nsobject.c \
	namespace/nsparse.c \
	namespace/nspredef.c \
	namespace/nsrepair.c \
	namespace/nsrepair2.c \
	namespace/nssearch.c \
	namespace/nsutils.c \
	namespace/nswalk.c \
	namespace/nsxfeval.c \
	namespace/nsxfname.c \
	namespace/nsxfobj.c \
	parser/psargs.c \
	parser/psloop.c \
	parser/psopcode.c \
	parser/psparse.c \
	parser/psscope.c \
	parser/pstree.c \
	parser/psutils.c \
	parser/pswalk.c \
	parser/psxface.c \
	resources/rsaddr.c \
	resources/rscalc.c \
	resources/rscreate.c \
	resources/rsdump.c \
	resources/rsinfo.c \
	resources/rsio.c \
	resources/rsirq.c \
	resources/rslist.c \
	resources/rsmemory.c \
	resources/rsmisc.c \
	resources/rsserial.c \
	resources/rsutils.c \
	resources/rsxface.c \
	tables/tbfadt.c \
	tables/tbfind.c \
	tables/tbinstal.c \
	tables/tbutils.c \
	tables/tbxface.c \
	tables/tbxfload.c \
	tables/tbxfroot.c \
	utilities/utaddress.c \
	utilities/utalloc.c \
	utilities/utcache.c \
	utilities/utclib.c \
	utilities/utcopy.c \
	utilities/utdebug.c \
	utilities/utdecode.c \
	utilities/utdelete.c \
	utilities/uteval.c \
        utilities/utexcep.c \
	utilities/utglobal.c \
	utilities/utids.c \
	utilities/utinit.c \
	utilities/utlock.c \
	utilities/utmath.c \
	utilities/utmisc.c \
	utilities/utmutex.c \
	utilities/utobject.c \
	utilities/utosi.c \
	utilities/utresrc.c \
	utilities/utstate.c \
	utilities/utxface.c \
	utilities/utxferror.c \
	utilities/utxfinit.c \

SRC_C := $(addprefix acpica/components/,$(ACPICA_SRC_C))
SRC_C += osl-stdio.c osl-thread.c debug.c
SRC_CC+= osl-basic.cc

DEFINES += -DACPI_LIBRARY -DL4_ACPICA

include $(L4DIR)/mk/lib.mk

main.o: dsdt-static.hex
vpath dsdt-static.dsl $(PKGDIR)/server/lib-acpi/src/
dsdt-static.hex: dsdt-static.dsl
	$(VERBOSE)echo -e "  ... Generating dsdt-static.hex"
	$(VERBOSE)$(IASL) -tc $<

extract_infos.o: isaids.h
vpath isaids.txt $(PKGDIR)/server/lib-acpi/src/
vpath generate_isaids.sh $(PKGDIR)/server/lib-acpi/src/
isaids.h: isaids.txt generate_isaids.sh
	$(VERBOSE)echo -e "  ... Generating isaids.h"
	$(VERBOSE)$(PKGDIR)/server/lib-acpi/src/generate_isaids.sh $< $@

clean::
	$(VERBOSE)$(RM) dsdt-static.hex
	$(VERBOSE)$(RM) acpi-dsdt.aml
	$(VERBOSE)$(RM) isaids.h

CFLAGS := $(filter-out -Wextra,$(CFLAGS))
