#include "local.h"

#include <linux/kexec.h>

note_buf_t *crash_notes = NULL;

void touch_nmi_watchdog(void)
{
	WARN_UNIMPL;
}

unsigned long pci_mem_start = 0xABCDABCD;
