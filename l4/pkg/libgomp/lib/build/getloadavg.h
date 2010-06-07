#pragma once

/* Adam: this is an commented snippet from stdlib.h and it is indeed
         also commented in the upstream SVN. */

#ifndef __THROW
#define __THROW
#endif
#ifndef __nonnull
#define __nonnull(x) __attribute__ ((__nonnull__ x))
#endif

/* Put the 1 minute, 5 minute and 15 minute load averages into the first
   NELEM elements of LOADAVG.  Return the number written (never more than
   three, but may be less than NELEM), or -1 if an error occurred.  */
extern int getloadavg (double __loadavg[], int __nelem)
     __THROW __nonnull ((1));

#undef __THROW
#undef __nonnull
