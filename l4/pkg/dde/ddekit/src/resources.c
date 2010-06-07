#include <l4/dde/ddekit/resources.h>

#include <l4/io/io.h>

#include "config.h"

int ddekit_request_dma(int nr __attribute__((unused))) {
	return -1;
}

int ddekit_release_dma(int nr __attribute__((unused))) {
	return -1;
}

/** Request an IO region
 *
 * \return 0 	success
 * \return -1   error
 */
int ddekit_request_io(ddekit_addr_t start, ddekit_addr_t count) {
	return l4io_request_ioport(start, count);
}

/** Release an IO region.
 *
 * \return 0 	success
 * \return <0   error
 */
int ddekit_release_io(ddekit_addr_t start, ddekit_addr_t count) {
	return l4io_release_ioport(start, count);
}

/** Request a memory region.
 *
 * \return vaddr virtual address of memory region
 * \return 0	 success
 * \return -1	 error
 */
int ddekit_request_mem(ddekit_addr_t start, ddekit_addr_t count, ddekit_addr_t *vaddr) {

	return l4io_request_iomem(start, count, 0, vaddr);
}

/** Release memory region.
 *
 * \return 0 success
 * \return <0 error
 */
int ddekit_release_mem(ddekit_addr_t start, ddekit_addr_t count) {
	return l4io_release_iomem(start, count);
}
