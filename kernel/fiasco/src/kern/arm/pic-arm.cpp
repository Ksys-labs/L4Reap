//-------------------------------------------------------------------
IMPLEMENTATION [arm]:

#include "config.h"

PUBLIC static inline NEEDS["config.h"]
unsigned
Pic::nr_irqs()
{ return Config::Max_num_dirqs; }


