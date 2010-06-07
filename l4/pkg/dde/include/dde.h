#ifndef l4_ddekit_h
#define l4_ddekit_h

/* FIXME if this is ddekit.h, it should be moved into dde/ddekit/include/ddekit.h (also
 * all headers under include/ddekit) */

/**
 * Initialize the DDE. Must be called before any other DDE function.
 *
 * FIXME revisit this one
 */
void ddekit_init(void);
void ddekit_do_initcalls(void);

#endif
