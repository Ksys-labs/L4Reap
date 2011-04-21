/*!
 * \file   support_x86.cc
 * \brief  Support for the x86 platform
 *
 * \date   2008-01-02
 * \author Adam Lackorznynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2008-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "support.h"

namespace L4
{
  class Uart_x86 : public Uart
  {
  private:
    unsigned long _base;

    inline unsigned long rd(unsigned long reg) const;
    inline void wr(unsigned long reg, unsigned long val) const;

  public:
    Uart_x86(int rx_irq, int tx_irq)
      : Uart(rx_irq, tx_irq), _base(~0UL) {}
    bool startup(unsigned long base);
    void shutdown();
    bool enable_rx_irq(bool enable = true);
    bool enable_tx_irq(bool enable = true);
    bool change_mode(Transfer_mode m, Baud_rate r);
    int get_char(bool blocking = true) const;
    int char_avail() const;
    inline void out_char(char c) const;
    int write(char const *s, unsigned long count) const;
  };
};


#include <string.h>
#include "base_critical.h"
#include "ARCH-x86/serial.h"
#include <l4/util/cpu.h>
#include <l4/util/port_io.h>
#include <assert.h>

/** VGA console output */

static void
vga_init()
{
  /* Reset any scrolling */
  l4util_out32(0xc, 0x3d4);
  l4util_out32(0, 0x3d5);
  l4util_out32(0xd, 0x3d4);
  l4util_out32(0, 0x3d5);
}

static void
vga_putchar(unsigned char c)
{
  static int ofs = -1, esc, esc_val, attr = 0x07;
  unsigned char *vidbase = (unsigned char*)0xb8000;

  base_critical_enter();

  if (ofs < 0)
    {
      /* Called for the first time - initialize.  */
      ofs = 80*2*24;
      vga_putchar('\n');
    }

  switch (esc)
    {
    case 1:
      if (c == '[')
	{
	  esc++;
	  goto done;
	}
      esc = 0;
      break;

    case 2:
      if (c >= '0' && c <= '9')
	{
	  esc_val = 10*esc_val + c - '0';
	  goto done;
	}
      if (c == 'm')
	{
	  attr = esc_val ? 0x0f : 0x07;
	  goto done;
	}
      esc = 0;
      break;
    }

  switch (c)
    {
    case '\n':
      memmove(vidbase, vidbase+80*2, 80*2*24);
      memset(vidbase+80*2*24, 0, 80*2);
      /* fall through... */
    case '\r':
      ofs = 0;
      break;

    case '\t':
      ofs = (ofs + 8) & ~7;
      break;

    case '\033':
      esc = 1;
      esc_val = 0;
      break;

    default:
      /* Wrap if we reach the end of a line.  */
      if (ofs >= 80)
	vga_putchar('\n');

      /* Stuff the character into the video buffer. */
	{
	  volatile unsigned char *p = vidbase + 80*2*24 + ofs*2;
	  p[0] = c;
	  p[1] = attr;
	  ofs++;
	}
      break;
    }

done:
  base_critical_leave();
}

/** Poor man's getchar, only returns raw scan code. We don't need to know
 * _which_ key was pressed, we only want to know _if_ a key was pressed. */
static int
raw_keyboard_getscancode(void)
{
  unsigned status, scan_code;

  base_critical_enter();

  l4util_cpu_pause();

  /* Wait until a scan code is ready and read it. */
  status = l4util_in8(0x64);
  if ((status & 0x01) == 0)
    {
      base_critical_leave();
      return -1;
    }
  scan_code = l4util_in8(0x60);

  /* Drop mouse events */
  if ((status & 0x20) != 0)
    {
      base_critical_leave();
      return -1;
    }

  base_critical_leave();
  return scan_code;
}

namespace L4
{
  bool Uart_x86::startup(unsigned long /*base*/)
  // real uart init will be made by startup.cc if told by cmdline
  { vga_init(); return true; }

  void Uart_x86::shutdown() {}
  bool Uart_x86::enable_rx_irq(bool) { return true; }
  bool Uart_x86::enable_tx_irq(bool) { return false; }
  bool Uart_x86::change_mode(Transfer_mode, Baud_rate) { return false; }

  int Uart_x86::get_char(bool blocking) const
  {
    int c;
    do {
      c = com_cons_try_getchar();
      if (c == -1)
        c = raw_keyboard_getscancode();
      l4util_cpu_pause();
    } while (c == -1 && blocking);

    return c;
  }

  int Uart_x86::char_avail() const
  {
    return com_cons_char_avail();
  }

  void Uart_x86::out_char(char c) const
  {
    vga_putchar(c);      // vga out
    com_cons_putchar(c); // serial out
  }

  int Uart_x86::write(char const *s, unsigned long count) const
  {
    unsigned long c = count;
    while (c)
      {
        if (*s == 10)
          out_char(13);
        out_char(*s++);
        --c;
      }
    return count;
  }
};


static inline l4_uint32_t
pci_conf_addr(l4_uint32_t bus, l4_uint32_t dev, l4_uint32_t fn, l4_uint32_t reg)
{ return 0x80000000 | (bus << 16) | (dev << 11) | (fn << 8) | (reg & ~3); }

static l4_uint32_t pci_read(unsigned char bus, l4_uint32_t dev,
                            l4_uint32_t fn, l4_uint32_t reg,
                            unsigned char width)
{
  l4util_out32(pci_conf_addr(bus, dev, fn, reg), 0xcf8);

  switch (width)
    {
    case 8:  return l4util_in8(0xcfc + (reg & 3));
    case 16: return l4util_in16((0xcfc + (reg & 3)) & ~1UL);
    case 32: return l4util_in32(0xcfc);
    }
  return 0;
}

static void pci_write(unsigned char bus, l4_uint32_t dev,
                      l4_uint32_t fn, l4_uint32_t reg,
                      l4_uint32_t val, unsigned char width)
{
  l4util_out32(pci_conf_addr(bus, dev, fn, reg), 0xcf8);

  switch (width)
    {
    case 8:  l4util_out8(val, 0xcfc + (reg & 3)); break;
    case 16: l4util_out16(val, (0xcfc + (reg & 3)) & ~1UL); break;
    case 32: l4util_out32(val, 0xcfc); break;
    }
}

static void pci_enable_io(unsigned char bus, l4_uint32_t dev,
                          l4_uint32_t fn)
{
  unsigned cmd = pci_read(bus, dev, fn, 4, 16);
  pci_write(bus, dev, fn, 4, cmd | 1, 16);
}

#include <stdio.h>

namespace {

struct Resource
{
  enum Type { NO_BAR, IO_BAR, MEM_BAR };
  Type type;
  unsigned long base;
  unsigned long len;
  Resource() : type(NO_BAR) {}
};

enum { NUM_BARS = 6 };

struct Serial_board
{
  int num_ports;
  int first_bar;
  bool port_per_bar;
  Resource bars[NUM_BARS];

  unsigned long get_port(int idx)
  {
    if (idx >= num_ports)
      return 0;

    if (port_per_bar)
      return bars[first_bar + idx].base;

    return bars[first_bar].base + 8 * idx;
  }
};


}

static
int pci_handle_serial_dev(unsigned char bus, l4_uint32_t dev,
                          l4_uint32_t subdev, bool scan_only,
                          Serial_board *board)
{
#if 0
  bool dev_enabled = false;
#endif


  // read bars
  int num_iobars = 0;
  int num_membars = 0;
  int first_port = -1;
  for (int bar = 0; bar < NUM_BARS; ++bar)
    {
      int a = 0x10 + bar * 4;

      unsigned v = pci_read(bus, dev, subdev, a, 32);
      pci_write(bus, dev, subdev, a, ~0U, 32);
      unsigned x = pci_read(bus, dev, subdev, a, 32);
      pci_write(bus, dev, subdev, a, v, 32);

      if (!v)
        continue;

      int s;
      for (s = 2; s < 32; ++s)
        if ((x >> s) & 1)
          break;

      board->bars[bar].base = v & ~3UL;
      board->bars[bar].len = 1 << s;
      board->bars[bar].type = (v & 1) ? Resource::IO_BAR : Resource::MEM_BAR;

      if (scan_only)
	printf("BAR%d: %04x (sz=%d)\n", bar, v & ~3, 1 << s);

      switch (board->bars[bar].type)
	{
	case Resource::IO_BAR:
	  ++num_iobars;
	  if (first_port == -1)
	    first_port = bar;
	  break;
	case Resource::MEM_BAR:
	  ++num_membars;
	  break;
	default:
	  break;
	}
    }

  if (num_membars <= 1 && num_iobars == 1)
    {
      board->first_bar = first_port;
      board->num_ports = board->bars[first_port].len / 8;
      board->port_per_bar = false;
      pci_enable_io(bus, dev, subdev);
      return 1;
    }


  board->num_ports = 0;
  board->first_bar = -1;

  for (int bar = 0; bar < NUM_BARS; ++bar)
    {
      if (board->bars[bar].type == Resource::IO_BAR && board->bars[bar].len == 8
	  && (board->first_bar == -1
	      || (board->first_bar + board->num_ports) == bar))
	{
	  ++board->num_ports;
	  if (board->first_bar == -1)
	    board->first_bar = bar;
	}
    }

  board->port_per_bar = true;
  return board->num_ports;

#if 0

      // for now we only take IO-BARs of size 8
      if (v & 1)
        {

          if (!scan_only && !dev_enabled)
            {
              pci_enable_io(bus, dev, subdev);
              dev_enabled = true;
            }

          if (scan_only)
            printf("BAR%d: %04x (sz=%d)\n", bar, v & ~3, 1 << s);

          if (s == 3)
            {
              if (scan_only)
                printf("   Potential serial port\n");
              else
                return v & ~3;
            }
        }
      else
        if (scan_only)
          printf("BAR%d: %08x (sz=%d)\n", bar, v & ~0xf, 1 << s);
    }
  return 0;
#endif
}

static unsigned long _search_pci_serial_devs(Serial_board *board, bool scan_only)
{
  l4_umword_t bus, buses, dev;

  for (bus=0, buses=20; bus<buses; bus++)
    {
      for (dev = 0; dev < 32; dev++)
        {
          unsigned char hdr_type = pci_read(bus, dev, 0, 0xe, 8);
          l4_umword_t subdevs = (hdr_type & 0x80) ? 8 : 1;

          for (l4_umword_t subdev = 0; subdev < subdevs; subdev++)
            {
              unsigned vendor = pci_read(bus, dev, subdev, 0, 16);
              unsigned device = pci_read(bus, dev, subdev, 2, 16);

              if ((vendor == 0xffff && device == 0xffff) ||
                  (device == 0x0000 && device == 0x0000))
                break;

              unsigned classcode = pci_read(bus, dev, subdev, 0x0b, 8);
              unsigned subclass  = pci_read(bus, dev, subdev, 0x0a, 8);

              if (classcode == 0x06 && subclass == 0x04)
                buses++;

              unsigned prog = pci_read(bus, dev, subdev, 9, 8);

              if (scan_only)
                printf("%02lx:%02lx.%1lx Class %02x.%02x Prog %02x: %04x:%04x\n",
                       bus, dev, subdev, classcode, subclass, prog, vendor, device);

              if (classcode == 7 && subclass == 0)
                if (unsigned long port = pci_handle_serial_dev(bus, dev,
                                                               subdev, scan_only, board))
                  return port;
            }
        }
    }
  return 0;
}

unsigned long search_pci_serial_devs(int port_idx, bool scan_only)
{
  Serial_board board;
  if (!_search_pci_serial_devs(&board, scan_only))
    return 0;

  return board.get_port(port_idx);
}

namespace {

class Platform_x86 : public Platform_base
{
public:
  bool probe() { return true; }
  void init()
  {
    // this is just a wrapper around serial.c
    // if you think this could be done better you're right...
    static L4::Uart_x86 _uart(1,1);
    _uart.startup(0);
    set_stdio_uart(&_uart);
  }

  void setup_memory_map(l4util_mb_info_t *mbi,
                        Region_list *ram, Region_list *regions)
  {
    if (!(mbi->flags & L4UTIL_MB_MEM_MAP))
      {
        assert(mbi->flags & L4UTIL_MB_MEMORY);
        ram->add(Region::n(0, (mbi->mem_upper + 1024) << 10, ".ram",
                  Region::Ram));
      }
    else
      {
        l4util_mb_addr_range_t *mmap;
        l4util_mb_for_each_mmap_entry(mmap, mbi)
          {
            unsigned long long start = (unsigned long long)mmap->addr;
            unsigned long long end = (unsigned long long)mmap->addr + mmap->size;

            switch (mmap->type)
              {
              case 1:
                ram->add(Region::n(start, end, ".ram", Region::Ram));
                break;
              case 2:
              case 3:
              case 4:
                regions->add(Region::n(start, end, ".BIOS", Region::Arch, mmap->type));
                break;
              case 5:
                regions->add(Region::n(start, end, ".BIOS", Region::No_mem));
                break;
              default:
                break;
              }
          }
      }

    regions->add(Region::n(0, 0x1000, ".BIOS", Region::Arch, 0));
  }
};
}

REGISTER_PLATFORM(Platform_x86);

