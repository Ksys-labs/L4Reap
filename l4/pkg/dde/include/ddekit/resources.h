#pragma once

#include <l4/sys/compiler.h>
#include <l4/dde/ddekit/types.h>

EXTERN_C_BEGIN

int ddekit_request_dma(int nr);
int ddekit_release_dma(int nr);
int ddekit_request_io (ddekit_addr_t start, ddekit_addr_t count);
int ddekit_release_io (ddekit_addr_t start, ddekit_addr_t count);
int ddekit_request_mem(ddekit_addr_t start, ddekit_addr_t count, ddekit_addr_t *vaddr);
int ddekit_release_mem(ddekit_addr_t start, ddekit_addr_t count);

EXTERN_C_END
