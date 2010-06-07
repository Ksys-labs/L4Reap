INTERFACE [arm && realview]:

#include "types.h"

class Board_check
{
public:
  static void check_board();

private:
  static Mword read_board_id();
};

// ------------------------------------------------------------------------
INTERFACE [arm && realview && realview_eb]:

EXTENSION class Board_check
{
  enum {
    id_mask = 0x1ffffe00, id_val = 0x01400400,
  };
};

// ------------------------------------------------------------------------
INTERFACE [arm && realview && realview_pb11mp]:

EXTENSION class Board_check
{
  enum {
    id_mask = 0x0fffff00, id_val = 0x0159f500,
  };
};

// ------------------------------------------------------------------------
INTERFACE [arm && realview && realview_pbx]:

EXTENSION class Board_check
{
  enum {
    id_mask = 0xffffff00, id_val = 0x1182f500,
  };
};

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && realview]:

#include "kmem.h"
#include "io.h"
#include "static_init.h"
#include "processor.h"

#include <cstdio>

enum
{
  SYS_ID = Kmem::System_regs_map_base + 0x0,
};

IMPLEMENT static
Mword
Board_check::read_board_id()
{ return Io::read<Mword>(SYS_ID); }

IMPLEMENT static
void
Board_check::check_board()
{
  Mword id = read_board_id();

  printf("Realview System ID: Rev=%lx HBI=%03lx Build=%lx Arch=%lx FPGA=%02lx\n",
         id >> 28, (id >> 16) & 0xfff, (id >> 12) & 0xf,
	 (id >> 8) & 0xf, id & 0xff);

  if ((id & id_mask) != id_val)
    {
      printf("  Invalid System ID for this kernel config\n"
	     "  Expected (%08lx & %08x) == %08x\n"
	     "  Stopping.\n", id, id_mask, id_val);
      while (1)
	Proc::halt();
    }
}

STATIC_INITIALIZEX_P(Board_check, check_board, GDB_INIT_PRIO);
