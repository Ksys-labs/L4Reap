INTERFACE:

#include "l4_types.h"

class Utcb_support
{
public:
  static Utcb *current();
  static void current(Utcb *utcb);
};
