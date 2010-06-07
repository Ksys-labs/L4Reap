INTERFACE:

#include "types.h"

class L4_msg_item
{
public:
  enum Consts
  {
    Grant_bit = 2,
  };

  enum Type
  {
    String = 0,
    Map    = 8,
  };

  explicit L4_msg_item(Mword raw) : _raw(raw) {}
  Mword compund() const { return _raw & 1; }
  Type type() const { return Type(_raw & 8); }
  bool is_void() const { return _raw == 0; }
  bool is_small_obj() const { return _raw & 2; }
  bool is_rcv_id() const { return _raw & 4; }

  Mword raw() const { return _raw; }
  Mword j() const { return _raw >> 4; }

  static L4_msg_item map(Mword base) { return L4_msg_item(base | Map); }

private:
  Mword _raw;
};
