INTERFACE:

#include "types.h"

/**
 * A L4 flex page.
 *
 * A flex page represents a size aligned
 * region of an address space.
 */
class L4_fpage
{
public:
  typedef Mword Raw;

  enum Type
  {
    Special = 0,
    Memory,
    Io,
    Obj
  };

  enum { Addr_shift = 12 };

private:
  /**
   * Create a flexpage with the given parameters.
   */
  L4_fpage(Type type, Mword address, unsigned char order,
           unsigned char rights)
  : _raw(address | Raw(rights) | (Raw(order) << 6) | (Raw(type) << 4))
  {}

public:
  enum { Whole_space = 63 };

  static L4_fpage io(Mword port, unsigned char order)
  { return L4_fpage(Io, port << Addr_shift, order, 0); }

  static L4_fpage obj(Mword idx, unsigned char order, unsigned char rights = 0)
  { return L4_fpage(Obj, idx & (~0UL << Addr_shift), order, rights); }

  static L4_fpage mem(Mword addr, unsigned char order, unsigned char rights = 0)
  { return L4_fpage(Memory, addr & (~0UL << Addr_shift), order, rights); }

  static L4_fpage nil()
  { return L4_fpage(0); }

  static L4_fpage all_spaces(unsigned char rights = 0)
  { return L4_fpage(Special, 0, Whole_space, rights); }

  explicit L4_fpage(Raw raw) : _raw(raw) {}

  Type type() const           { return Type((_raw >> 4) & 3); }
  unsigned char order() const { return (_raw >> 6) & 0x3f; }

  typedef Virt_addr Mem_addr;
  Virt_addr mem_address() const
  { return Virt_addr(_raw & (~0UL << Addr_shift)); }

  Mword obj_address() const   { return _raw & (~0UL << Addr_shift); }
  Mword io_address() const    { return _raw >> Addr_shift; }
  Mword obj_index() const     { return _raw >> Addr_shift; }

  bool is_mempage() const { return type() == Memory; }
  bool is_iopage()  const { return type() == Io; }
  bool is_objpage() const { return type() == Obj; }

  Mword write() const { return _raw & 2; }
  Mword read() const  { return _raw & 4; }
  Mword exec() const  { return _raw & 1; }

  /**
   * Is the flex page the whole address space?
   * @return not zero, if the flex page covers the
   *   whole address space.
   */
  Mword is_all_spaces() const { return (_raw & 0xff8) == (Whole_space << 6); }

  /**
   * Is the flex page valid?
   * @return not zero if the flex page
   *    contains a value other than 0.
   */
  Mword is_valid() const { return _raw; }

  /**
   * Get the binary representation of the flex page.
   * @return this flex page in binary representation.
   */
  Raw raw() const { return _raw; }

private:

  Raw _raw;

  enum {
    /* +- bitsize-12 +- 11-6 -+ 5-3 -+ 2-0 +
     * | page number |  size  | type | rwx |
     * +-------------+--------+------+-----+ */

    // Extension: bits for returning a flexpage's access status.
    // (Make sure these flags do not overlap any significant fpage
    // bits -- but overlapping the Grant and Write bits used in some
    // APIs is OK.)
  };

public:
  enum
  {
    Caching_opt = 1,
    Cached      = 3,
    Buffered    = 5,
    Uncached    = 1,

    Control_addr_shift = 12,
    Control_grant      = 2,
  };

  enum Obj_map_ctl
  {
    C_weak_ref            = 0x10,
    C_ref                 = 0x00,

    C_obj_right_1         = 0x20,
    C_obj_right_2         = 0x40,
    C_obj_right_3         = 0x80,

    C_obj_specific_rights = C_obj_right_1 | C_obj_right_2 | C_obj_right_3,
    C_ctl_rights          = C_obj_specific_rights | C_weak_ref,
  };

  enum Rights
  {
    R   = 4,
    W   = 2,
    X   = 1,
    RX  = R | X,
    RWX = R | W | X,
    RW  = R | W,
    WX  = W | X,

    CD    = 0x8,
    CR    = 0x4,
    CS    = 0x2,
    CW    = 0x1,
    CRW   = CR | CW,
    CRS   = CR | CS,
    CRWS  = CRW | CS,
    CWS   = CW | CS,
    CWSD  = CW | CS | CD,
    CRWSD = CRWS | CD,

    FULL  = 0xf,
  };

  Rights rights() const { return Rights(_raw & FULL); }
  void mask_rights(Rights r) { _raw &= (Mword(r) | ~0x0fUL); }
};

