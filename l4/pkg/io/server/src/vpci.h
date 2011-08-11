/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "vdevice.h"
#include "pci.h"

namespace Vi {

/**
 * \brief Generic virtual PCI device.
 * This class provides the basic functionality for a device on a 
 * virtual PCI bus.  Implementations may provide proxy access to a real PCI
 * device or a completely virtualized PCI device.
 */
class Pci_dev
{
private:
  void operator = (Pci_dev const &);
  Pci_dev(Pci_dev const &);

public:
  typedef Hw::Pci::Cfg_width Cfg_width;

  struct Irq_info
  {
    int irq;
    unsigned char trigger;
    unsigned char polarity;
  };

  Pci_dev() {}
  virtual int cfg_read(int reg, l4_uint32_t *v, Cfg_width) = 0;
  virtual int cfg_write(int reg, l4_uint32_t v, Cfg_width) = 0;
  virtual int irq_enable(Irq_info *irq) = 0;
  virtual bool is_same_device(Pci_dev const *o) const = 0;
  virtual ~Pci_dev() = 0;
};

inline
Pci_dev::~Pci_dev()
{}

/**
 * \brief A basic really virtualized PCI device.
 */
class Pci_virtual_dev : public Pci_dev, public Dev_feature
{
public:
  struct Pci_cfg_header
  {
    l4_uint32_t vendor_device;
    l4_uint16_t cmd;
    l4_uint16_t status;
    l4_uint32_t class_rev;
    l4_uint8_t  cls;
    l4_uint8_t  lat;
    l4_uint8_t  hdr_type;
    l4_uint8_t  bist;
  } __attribute__((packed));

  Pci_cfg_header *cfg_hdr() { return (Pci_cfg_header*)_h; }
  Pci_cfg_header const *cfg_hdr() const { return (Pci_cfg_header const *)_h; }

  int cfg_read(int reg, l4_uint32_t *v, Cfg_width);
  int cfg_write(int reg, l4_uint32_t v, Cfg_width);
  bool is_same_device(Pci_dev const *o) const { return o == this; }

  ~Pci_virtual_dev() = 0;

  Pci_virtual_dev();

  void set_host(Device *d) { _host = d; }
  Device *host() const { return _host; }

protected:
  Device *_host;
  unsigned char *_h;
  unsigned _h_len;

};

inline
Pci_virtual_dev::~Pci_virtual_dev()
{}



/**
 * \brief A virtual PCI proxy for a real PCI device.
 */
class Pci_proxy_dev : public Pci_dev, public virtual Dev_feature
{
public:

  Pci_proxy_dev(Hw::Pci::If *hwf);

  int cfg_read(int reg, l4_uint32_t *v, Cfg_width);
  int cfg_write(int reg, l4_uint32_t v, Cfg_width);
  int irq_enable(Irq_info *irq);
  int bridge_cfg_read(unsigned, l4_uint32_t, int, l4_uint32_t *, Cfg_width)
  { return -L4_ENODEV; }

  int bridge_cfg_write(unsigned, l4_uint32_t, int, l4_uint32_t, Cfg_width)
  { return -L4_ENODEV; }

  l4_uint32_t read_bar(int bar);
  void write_bar(int bar, l4_uint32_t v);

  l4_uint32_t read_rom() const { return _rom; }
  void write_rom(l4_uint32_t v);

  int vbus_dispatch(l4_umword_t, l4_uint32_t, L4::Ipc::Iostream &)
  { return -L4_ENOSYS; }

  Hw::Pci::If *hwf() const { return _hwf; }

  void dump() const;
  bool is_same_device(Pci_dev const *o) const
  {
    if (Pci_proxy_dev const *op = dynamic_cast<Pci_proxy_dev const *>(o))
      return (hwf()->bus_nr() == op->hwf()->bus_nr())
             && ((hwf()->host()->adr() >> 16) == (op->hwf()->host()->adr() >> 16));
    return false;
  }

  bool match_hw_feature(const Hw::Dev_feature *f) const
  { return f == _hwf; }

  int dispatch(l4_umword_t, l4_uint32_t, L4::Ipc::Iostream&)
  { return -L4_ENOSYS; }

  void set_host(Device *d) { _host = d; }
  Device *host() const { return _host; }

private:
  Device *_host;
  Hw::Pci::If *_hwf;

  l4_uint32_t _vbars[6];
  l4_uint32_t _rom;
};



/**
 * \brief a basic virtual PCI bridge.
 * This class is the base for virtual Host-to-PCI bridges,
 * for virtual PCI-to-PCI bridges, and also for this such as
 * virtual PCI-to-Cardbus brdiges.
 */
class Pci_bridge : public Device
{
public:
  class Dev
  {
  public:
    enum { Fns = 8 };

  private:
    Pci_dev *_fns[Fns];

  public:
    Dev();

    bool empty() const { return !_fns[0]; }
    void add_fn(Pci_dev *f);
    void sort_fns();

    Pci_dev *fn(unsigned f) const { return _fns[f]; }
    void fn(unsigned f, Pci_dev *fn) { _fns[f] = fn; }
    bool cmp(Pci_dev const *od) const
    {
      if (empty())
	return false;

      return _fns[0]->is_same_device(od);
    }
  };

  class Bus
  {
  public:
    enum { Devs = 32 };

  private:
    Dev _devs[Devs];

  public:
    Dev const *dev(unsigned slot) const { return &_devs[slot]; }
    Dev *dev(unsigned slot) { return &_devs[slot]; }

    void add_fn(Pci_dev *d, int slot = -1);
  };

  Pci_bridge() : _free_dev(0), _primary(0), _secondary(0), _subordinate(0) {}

protected:
  Bus _bus;
  unsigned _free_dev;
  union
  {
    struct
    {
      l4_uint32_t _primary:8;
      l4_uint32_t _secondary:8;
      l4_uint32_t _subordinate:8;
      l4_uint32_t _lat:8;
    };
    l4_uint32_t _bus_config;
  };

public:

  void primary(unsigned char v) { _primary = v; }
  void secondary(unsigned char v) { _secondary = v; }
  void subordinate(unsigned char v) { _subordinate = v; }
  bool child_dev(unsigned bus, unsigned char dev, unsigned char fn, Pci_dev **rd);
  void add_child(Device *d);
  void add_child_fixed(Device *d, Pci_dev *vp, unsigned dn, unsigned fn);

  Pci_bridge *find_bridge(unsigned bus);
  void setup_bus();
  void finalize_setup();

};

}

