/*
 * arch. independent L4 Types
 */

INTERFACE:

#include "types.h"
#include "l4_fpage.h"
#include "l4_buf_desc.h"
#include "l4_error.h"

typedef Address Local_id;

class Utcb;

class L4_obj_ref
{
public:
  enum Flags
  {
    None = 0,
    Ipc_call = 0,
    Ipc_send = 1,
    Ipc_recv = 2,
    Ipc_open_wait = 4,
    Ipc_reply = 8,

    Ipc_wait           = Ipc_open_wait | Ipc_recv,
    Ipc_send_and_wait  = Ipc_open_wait | Ipc_send | Ipc_recv,
    Ipc_reply_and_wait = Ipc_open_wait | Ipc_send | Ipc_recv | Ipc_reply,
    Ipc_call_ipc       = Ipc_send | Ipc_recv,
  };

  enum Special
  {
    Invalid      = 1UL << 11UL,
    Invalid_bit  = 1UL << 11UL,
    Self         = (~0UL) << 11UL,
    Invalid_mask = (~0UL) << 11UL,
  };

  enum
  {
    Cap_shift = 12UL
  };

  L4_obj_ref(Special s = Invalid) : _raw(s) {}
  static L4_obj_ref from_raw(Mword raw) { return L4_obj_ref(true, raw); }

  bool valid() const { return !(_raw & Invalid_bit); }
  bool invalid() const { return _raw & Invalid_bit; }
  bool self() const { return invalid(); }
  //bool self() const { return (_raw & Invalid_mask) == Self; }
  unsigned have_recv() const { return _raw & Ipc_recv; }

  unsigned long cap() const { return _raw >> 12; }
  unsigned flags() const { return _raw & 0xf; }

  Mword raw() const { return _raw; }

  explicit L4_obj_ref(Mword cap, unsigned flags = 0) : _raw(cap | flags) {}
  L4_obj_ref(Flags flags) : _raw(flags) {}

  bool operator == (L4_obj_ref const &o) const { return _raw == o._raw; }

private:
  L4_obj_ref(bool, Mword raw) : _raw(raw) {}
  Mword _raw;
};


/**
 * L4 fpage unmap map_mask.
 */
class L4_map_mask
{
public:
  explicit L4_map_mask(Mword raw = 0) : _raw(raw) {}

  static L4_map_mask full();

  Mword raw() const { return _raw; }

  /**
   * Unmap from the calling Task too.
   */
  Mword self_unmap() const;
  Mword do_delete() const { return _raw & 0x40000000; }

private:
  Mword _raw;
};


class L4_msg_tag
{
public:
  enum Flags
  {
    Error        = 0x8000, // rcv only flag
    X_cpu        = 0x4000, // rcv only flag
    Rcv_flags    = Error | X_cpu,

    Transfer_fpu = 0x1000, // snd and pass through flag
    Schedule     = 0x2000, // snd and pass through flag
    Propagate    = 0x4000, // snd only flag
  };

  enum Protocol
  {
    Label_none          = 0,
    Label_allow_syscall = 1,

    Label_irq = -1L,
    Label_page_fault = -2L,
    Label_preemption = -3L,
    Label_sys_exception = -4L,
    Label_exception  = -5L,
    Label_sigma0 = -6L,
    Label_io_page_fault = -8L,
    Label_del_observer = -9L,
    Label_kobject = -10L,
    Label_task = -11L,
    Label_thread = -12L,
    Label_log = -13L,
    Label_cpu = -14L,
    Label_factory = -15L,
    Label_vm = -16L,
    Label_semaphore = -20L,
  };
private:
  Mword _tag;
};

/**
 * L4 timeouts data type.
 */
class L4_timeout
{
public:
  /// Typical timout constants.
  enum {
    Never = 0, ///< Never timeout.
    Zero  = 0x400, ///< Zero timeout.
  };

  /**
   * Create the specified timeout.
   * @param man mantissa of the send timeout.
   * @param exp exponent of the send timeout
   *        (exp=0: infinite timeout,
   *        exp>0: t=2^(exp)*man,
   *        man=0 & exp!=0: t=0).
   */
  L4_timeout (Mword man, Mword exp);
  L4_timeout (Mword man, Mword exp, bool clock);

  /**
   * Create a timeout from it's binary representation.
   * @param t the binary timeout value.
   */
  L4_timeout( unsigned short t = 0 );

  /**
   * Get the binary representation of the timeout.
   * @return The timeout as binary representation.
   */
  unsigned short raw() const;

  /**
   * Get the receive exponent.
   * @return The exponent of the receive timeout.
   * @see rcv_man()
   */
  Mword exp() const;

  /**
   * Set the exponent of the receive timeout.
   * @param er the exponent for the receive timeout (see L4_timeout()).
   * @see rcv_man()
   */
  void exp( Mword er );

  /**
   * Get the receive timout's mantissa.
   * @return The mantissa of the receive timeout (see L4_timeout()).
   * @see rcv_exp()
   */
  Mword man() const;

  /**
   * Set the mantissa of the receive timeout.
   * @param mr the mantissa of the recieve timeout (see L4_timeout()).
   * @see rcv_exp()
   */
  void man( Mword mr );

  /**
   * Get the relative receive timeout in microseconds.
   * @param clock Current value of kernel clock
   * @return The receive timeout in micro seconds.
   */
  Unsigned64 microsecs_rel (Unsigned64 clock) const;

  /**
   * Get the absolute receive timeout in microseconds.
   * @param clock Current value of kernel clock
   * @return The receive timeout in micro seconds.
   */
  Unsigned64 microsecs_abs (Utcb *u) const;

private:
  enum
    {
      Clock_mask   = 0x0400,
      Abs_mask     = 0x8000,

      Exp_mask     = 0x7c00,
      Exp_shift    = 10,

      Man_mask     = 0x3ff,
      Man_shift    = 0,
    };

  unsigned short _t;
} __attribute__((packed));

struct L4_timeout_pair
{
  L4_timeout rcv;
  L4_timeout snd;

  L4_timeout_pair(L4_timeout const &rcv, L4_timeout const &snd)
    : rcv(rcv), snd(snd) {}

  L4_timeout_pair(unsigned long v) : rcv(v), snd(v >> 16) {}

  Mword raw() const { return (Mword)rcv.raw() | (Mword)snd.raw() << 16; }
};


class L4_pipc
{
private:
  enum
    {
      Clock_shift        = 0,
      Id_shift           = 56,
      Lost_shift         = 62,
      Type_shift         = 63,
      Clock_mask         = 0x00ffffffffffffffULL,
      Id_mask            = 0x3f00000000000000ULL,
      Lost_mask          = 0x4000000000000000ULL,
      Type_mask          = 0x8000000000000000ULL,
      Low_shift          = 0,
      High_shift         = 32,
      Low_mask           = 0x00000000ffffffffULL,
      High_mask          = 0xffffffff00000000ULL
    };

  /**
   * Raw 64 bit representation
   */
  Unsigned64 _raw;

public:
  /**
   * Extract the low message word.
   */
  Mword low() const;

  /**
   * Extract the high message word.
   */
  Mword high() const;

  /**
   * Create a Preemption-IPC message
   */
  L4_pipc (unsigned type, unsigned lost, unsigned id, Cpu_time clock);
};


class L4_exception_ipc
{
public:
  enum
  {
    //Exception_ipc_cookie_1 = Mword(-0x5),
    //Exception_ipc_cookie_2 = Mword(-0x21504151),
    Protocol = -5
  };
};

class L4_semaphore
{
public:
  Smword counter;
  Mword flags;
};

class L4_err
{
public:
  enum Err
  {
    EPerm         =  1,
    ENoent        =  2,
    ENomem        = 12,
    EAccess       = 13,
    EBusy         = 16,
    EExists       = 17,
    ENodev        = 19,
    EInval        = 22,
    ENosys        = 38,
    EBadproto     = 39,

    EAddrnotavail = 99,
  };
};


//----------------------------------------------------------------------------
INTERFACE [ia32 || ux]:

EXTENSION class L4_exception_ipc
{
public:
  enum { Msg_size = 16 };
};


//----------------------------------------------------------------------------
INTERFACE [arm]:

EXTENSION class L4_exception_ipc
{
public:
  enum { Msg_size = 20 };
};


//----------------------------------------------------------------------------
INTERFACE [amd64]:

EXTENSION class L4_exception_ipc
{
public:
  enum { Msg_size = 23 };
};

INTERFACE [ppc32]:
EXTENSION class L4_exception_ipc
{
public:
  enum { Msg_size = 39 };
};

//----------------------------------------------------------------------------
INTERFACE:

class Utcb
{
  /* must be 2^n bytes */
public:
  enum { Max_words = 63, Max_buffers = 58 };
  Mword           values[Max_words];
  Mword           reserved;

  L4_buf_desc     buf_desc;
  Mword           buffers[Max_buffers];

  L4_error        error;
  L4_timeout_pair xfer;
  Mword           user[3];
};

//----------------------------------------------------------------------------
IMPLEMENTATION:

#include <minmax.h>


PUBLIC inline
bool Utcb::inherit_fpu() const
{ return buf_desc.flags() & L4_buf_desc::Inherit_fpu; }



PUBLIC inline
L4_msg_tag::L4_msg_tag(unsigned words, unsigned items, unsigned long flags,
    unsigned long proto)
  : _tag((words & 0x3f) | ((items << 6) & 0x3f) | flags | (proto << 16))
{}

PUBLIC inline
L4_msg_tag::L4_msg_tag()
{}

PUBLIC inline
L4_msg_tag::L4_msg_tag(L4_msg_tag const &o, Mword flags)
  : _tag((o.raw() & ~Mword(Rcv_flags)) | flags)
{}

PUBLIC explicit inline
L4_msg_tag::L4_msg_tag(Mword raw)
  : _tag(raw)
{}

PUBLIC inline
long
L4_msg_tag::proto() const
{ return long(_tag) >> 16; }

PUBLIC inline
unsigned long
L4_msg_tag::raw() const
{ return _tag; }

PUBLIC inline
unsigned L4_msg_tag::words() const
{ return _tag & 63; }

PUBLIC inline
unsigned L4_msg_tag::items() const
{ return (_tag >> 6) & 0x3f; }

PUBLIC inline
Mword L4_msg_tag::flags() const
{ return _tag; }

PUBLIC inline
bool L4_msg_tag::transfer_fpu() const
{ return _tag & Transfer_fpu; }

PUBLIC inline
bool L4_msg_tag::do_switch() const
{ return !(_tag & Schedule); }

PUBLIC inline
void L4_msg_tag::set_error(bool e = true)
{ if (e) _tag |= Error; else _tag &= ~Mword(Error); }


PUBLIC inline
bool L4_msg_tag::has_error() const
{ return _tag & Error; }
//
// L4_timeout implementation
//

IMPLEMENT inline L4_timeout::L4_timeout (unsigned short t)
  : _t(t)
{}

IMPLEMENT inline unsigned short L4_timeout::raw() const
{ return _t; }

PUBLIC inline
Mword L4_timeout::abs_exp() const
{ return (_t >> 11) & 0xf; }

PUBLIC inline
bool L4_timeout::abs_clock() const
{ return _t & Clock_mask; }

IMPLEMENT inline
Unsigned64
L4_timeout::microsecs_rel (Unsigned64 clock) const
{
  if (man() == 0)
    return 0;
  else
   return clock + ((Unsigned64)man() << exp()); 
}

IMPLEMENT inline NEEDS[<minmax.h>]
Unsigned64
L4_timeout::microsecs_abs (Utcb *u) const
{
  int idx = min<int>(_t & 0x3f, Utcb::Max_buffers);
  return *(Unsigned64*)reinterpret_cast<void const *>(&u->buffers[idx]);
}

PUBLIC inline
bool
L4_timeout::is_absolute() const
{ return _t & Abs_mask; }

PUBLIC inline
Unsigned64
L4_timeout::microsecs (Unsigned64 clock, Utcb *u) const
{ 
  if (is_absolute())
    return microsecs_abs(u);
  else
    return microsecs_rel(clock);
}

PUBLIC inline
bool L4_timeout::is_never() const
{ return !_t; }

PUBLIC inline
bool L4_timeout::is_zero() const
{ return _t == Zero; }

PUBLIC inline
unsigned short L4_timeout::is_finite() const
{ return _t; }


//
// L4_pipc implementation
//

IMPLEMENT inline
L4_pipc::L4_pipc (unsigned type, unsigned lost, unsigned id, Cpu_time clock)
       : _raw ( (((Unsigned64) type  << Type_shift)  & Type_mask) |
                (((Unsigned64) lost  << Lost_shift)  & Lost_mask) |
                (((Unsigned64) id    << Id_shift)    & Id_mask)   |
                (((Unsigned64) clock << Clock_shift) & Clock_mask))
{}

IMPLEMENT inline Mword L4_pipc::low() const
{ return (_raw & Low_mask) >> Low_shift; }

IMPLEMENT inline Mword L4_pipc::high() const
{ return (_raw & High_mask) >> High_shift; }


//
// L4_timeout implementation
//

IMPLEMENT inline
L4_timeout::L4_timeout (Mword man, Mword exp)
          : _t (((man & Man_mask) |
                ((exp << Exp_shift) & Exp_mask)))
{}

IMPLEMENT inline
L4_timeout::L4_timeout (Mword man, Mword exp, bool clock)
          : _t (((man & Man_mask) |
                ((exp << (Exp_shift+1)) & Exp_mask) |
		(clock ? Clock_mask : 0) | Abs_mask))
{}

IMPLEMENT inline Mword L4_timeout::exp() const
{ return (_t & Exp_mask) >> Exp_shift; }

IMPLEMENT inline void L4_timeout::exp (Mword w)
{ _t = (_t & ~Exp_mask) | ((w << Exp_shift) & Exp_mask); }

IMPLEMENT inline Mword L4_timeout::man() const
{ return (_t & Man_mask) >> Man_shift; }

IMPLEMENT inline void L4_timeout::man (Mword w)
{ _t = (_t & ~Man_mask) | ((w << Man_shift) & Man_mask); }


IMPLEMENT inline
Mword L4_map_mask::self_unmap() const
{ return _raw & 0x80000000; }

IMPLEMENT inline
L4_map_mask L4_map_mask::full() { return L4_map_mask(0xc0000002); }


