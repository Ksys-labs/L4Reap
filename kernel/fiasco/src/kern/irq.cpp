INTERFACE:

#include "ipc_sender.h"
#include "irq_pin.h"
#include "kobject_helper.h"
#include "member_offs.h"
#include "sender.h"
#include "context.h"

class Ram_quota;
class Receiver;



/** Hardware interrupts.  This class encapsulates handware IRQs.  Also,
    it provides a registry that ensures that only one receiver can sign up
    to receive interrupt IPC messages.
 */
class Irq : public Irq_base, public Kobject
{
  MEMBER_OFFSET();
  FIASCO_DECLARE_KOBJ();

private:
  typedef slab_cache_anon Allocator;

public:
  enum Mode
  {
    Set_irq_mode  = 1,
    Trigger_edge  = 0,
    Trigger_level = 2,
    Polarity_high = 0,
    Polarity_low  = 4,
  };

  enum Op
  {
    Op_eoi_1      = 0,
    Op_attach     = 1,
    Op_trigger    = 2,
    Op_chain      = 3,
    Op_eoi_2      = 4,
  };

private:
  Irq(Irq&);

protected:
  Ram_quota *_q;
  Context::Drq _drq;

public:
  virtual ~Irq() {}

};

class Irq_sender
: public Kobject_h<Irq_sender, Irq>,
  public Ipc_sender<Irq_sender>
{
public:
  Mword kobject_size() const { return sizeof(*this); }

private:
  Irq_sender(Irq_sender &);

protected:
  Smword _queued;
  Receiver *_irq_thread;

private:
  Mword _irq_id;
};

class Irq_muxer : public Kobject_h<Irq_muxer, Irq>
{
  friend class Chain_irq_pin;

private:
  Smword _mask_cnt;
};

class Irq_debugger : public Irq_base
{
};


class Chain_irq_pin : public Sw_irq_pin
{};

//-----------------------------------------------------------------------------
INTERFACE [debug]:

EXTENSION class Irq
{
public:
  struct Irq_log
  {
    Mword   irq_obj;
    Address user_ip;
    int     irq_number;
  };

  static unsigned irq_log_fmt(Tb_entry *, int, char *)
  asm ("__irq_log_fmt");

};


//-----------------------------------------------------------------------------
IMPLEMENTATION:

#include "atomic.h"
#include "config.h"
#include "cpu_lock.h"
#include "entry_frame.h"
#include "globals.h"
#include "ipc_sender.h"
#include "kdb_ke.h"
#include "kmem_slab.h"
#include "lock_guard.h"
#include "minmax.h"
#include "receiver.h"
#include "std_macros.h"
#include "thread_object.h"
#include "thread_state.h"
#include "l4_buf_iter.h"

FIASCO_DEFINE_KOBJ(Irq);

namespace {
static Irq_base *irq_base_dcast(Kobject_iface *o)
{ return Kobject::dcast<Irq*>(o); }

struct Irq_base_cast
{
  Irq_base_cast()
  { Irq_base::dcast = &irq_base_dcast; }
};

static Irq_base_cast register_irq_base_cast;
}

PUBLIC static inline
Irq *
Irq::self(Irq_pin const *pin)
{
#define MYoffsetof(TYPE, MEMBER) (((size_t) &((TYPE *)10)->MEMBER) - 10)
  return reinterpret_cast<Irq*>(reinterpret_cast<Mword>(pin)
      - MYoffsetof(Irq, _pin));
#undef MYoffsetof
}



PUBLIC inline explicit
Chain_irq_pin::Chain_irq_pin(Irq_muxer *i)
{ payload()[0] = Mword(i); }

PUBLIC inline
Irq_muxer *
Chain_irq_pin::irq() const
{ return (Irq_muxer*)payload()[0]; }

PUBLIC
void
Chain_irq_pin::do_unmask()
{
  Smword old;
  do
    old = irq()->_mask_cnt;
  while (!mp_cas(&irq()->_mask_cnt, old, old - 1));

  if (old == 1)
    irq()->pin()->unmask();
}


PUBLIC
void
Chain_irq_pin::do_mask()
{
  Smword old;
  do
    old = irq()->_mask_cnt;
  while (!mp_cas(&irq()->_mask_cnt, old, old + 1));

  if (old == 0)
    irq()->pin()->mask();
}


PUBLIC
void
Chain_irq_pin::unbind_irq()
{
  Irq_base *self = Irq::self(this);
  Irq_base *n;
  for (n = irq(); n->_next && n->_next != self; n = n->_next)
    ;

  assert (n->_next == self);
  n->_next = n->_next->_next;
  if (masked())
    do_unmask();

  replace<Sw_irq_pin>();
}


PUBLIC
void
Chain_irq_pin::do_mask_and_ack()
{
}


PUBLIC inline
void *
Irq::operator new (size_t, void *p)
{ return p; }

PUBLIC
void
Irq::operator delete (void *_l)
{
  Irq *l = reinterpret_cast<Irq*>(_l);
  if (l->_q)
    l->_q->free(sizeof(Irq));

  allocator()->free(l);
}

PUBLIC template<typename T> inline NEEDS[Irq::allocator, Irq::operator new]
static
T*
Irq::allocate(Ram_quota *q)
{
  void *nq =allocator()->q_alloc(q);
  if (nq)
    return new (nq) T(q);

  return 0;
}

static Kmem_slab _irq_allocator(max(sizeof (Irq_sender), sizeof(Irq_muxer)),
                                __alignof__ (Irq), "Irq");

PRIVATE static
Irq::Allocator *
Irq::allocator()
{ return &_irq_allocator; }


/** Bind a receiver to this device interrupt.
    @param t the receiver that wants to receive IPC messages for this IRQ
    @return true if the binding could be established
 */
PUBLIC inline NEEDS ["atomic.h", "cpu_lock.h", "lock_guard.h"]
bool
Irq_sender::alloc(Receiver *t)
{
  bool ret = mp_cas(&_irq_thread, reinterpret_cast<Receiver*>(0), t);

  if (ret)
    {
      if (EXPECT_TRUE(t != 0))
	{
          t->inc_ref();
	  pin()->set_cpu(t->cpu());
	}

      _queued = 0;
    }

  return ret;
}

PUBLIC
Receiver *
Irq_sender::owner() const { return _irq_thread; }

/** Release an device interrupt.
    @param t the receiver that ownes the IRQ
    @return true if t really was the owner of the IRQ and operation was 
            successful
 */
PUBLIC
bool
Irq_sender::free(Receiver *t)
{
  bool ret = mp_cas(&_irq_thread, t, reinterpret_cast<Receiver*>(0));

  if (ret)
    {
      Lock_guard<Cpu_lock> guard(&cpu_lock);
      pin()->mask();

      if (EXPECT_TRUE(t != 0))
	{
	  t->abort_send(this);

	  // release cpu-lock early, actually before delete
	  guard.release();

	  if (t->dec_ref() == 0)
	    delete t;
	}
    }

  return ret;
}


PUBLIC explicit inline
Irq::Irq(Ram_quota *q = 0)
: _q(q)
{
  new (pin()) Sw_irq_pin();
  pin()->mask();
}

PUBLIC
void
Irq::destroy(Kobject ***rl)
{
  Irq_base::destroy();
  Kobject::destroy(rl);
}

PUBLIC explicit
Irq_sender::Irq_sender(Ram_quota *q = 0)
: Kobject_h<Irq_sender, Irq>(q), _queued(0), _irq_thread(0), _irq_id(~0UL)
{}

PUBLIC
void
Irq_sender::destroy(Kobject ***rl)
{
  Lock_guard<Cpu_lock> g(&cpu_lock);
  if (_irq_thread)
    free(_irq_thread);

  Irq::destroy(rl);
}

PUBLIC inline
unsigned long
Irq::irq() const
{ return pin()->payload()[0]; }

/** Consume one interrupt.
    @return number of IRQs that are still pending.
 */
PRIVATE inline NEEDS ["atomic.h"]
Smword
Irq_sender::consume()
{
  Smword old;

  do
    {
      old = _queued;
    }
  while (!mp_cas (&_queued, old, old - 1));

  return old - 1;
}

PUBLIC inline
int
Irq_sender::queued()
{
  return _queued;
}


/**
 * Predicate used to figure out if the sender shall be enqueued
 * for sending a second message after sending the first.
 */
PUBLIC inline NEEDS[Irq_sender::consume]
bool
Irq_sender::requeue_sender()
{ return consume() > 0; }

/**
 * Predicate used to figure out if the sender shall be deqeued after
 * sending the request.
 */
PUBLIC inline NEEDS[Irq_sender::consume]
bool
Irq_sender::dequeue_sender()
{ return consume() < 1; }

PUBLIC inline
Syscall_frame *
Irq_sender::transfer_msg(Receiver *recv)
{
  Syscall_frame* dst_regs = recv->rcv_regs();

  // set ipc return value: OK
  dst_regs->tag(L4_msg_tag(0));

  // set ipc source thread id
  dst_regs->from(_irq_id);

  return dst_regs;
}

PUBLIC void
Irq_sender::modify_label(Mword const *todo, int cnt)
{
  for (int i = 0; i < cnt*4; i += 4)
    {
      Mword const test_mask = todo[i];
      Mword const test      = todo[i+1];
      if ((_irq_id & test_mask) == test)
	{
	  Mword const set_mask = todo[i+2];
	  Mword const set      = todo[i+3];

	  _irq_id = (_irq_id & ~set_mask) | set;
	  return;
	}
    }
}


PRIVATE static
unsigned
Irq_sender::handle_remote_hit(Context::Drq *, Context *, void *arg)
{
  Irq_sender *irq = (Irq_sender*)arg;
  irq->pin()->set_cpu(current_cpu());
  irq->send_msg(irq->_irq_thread);
  return Context::Drq::No_answer;
}

PRIVATE
void
Irq_muxer::hit()
{
  assert (cpu_lock.test());
  pin()->mask_and_ack();

  if (EXPECT_FALSE (!Irq_base::_next))
    return;

  int irqs = 0;
  for (Irq_base *n = Irq_base::_next; n;)
    {
      Irq *i = nonull_static_cast<Irq*>(n);
      ++irqs;
      i->pin()->__mask();
      n = i->Irq_base::_next;
    }

    {
      Smword old;
      do
	old = _mask_cnt;
      while (!mp_cas(&_mask_cnt, old, old + irqs));
    }

  for (Irq_base *n = Irq_base::_next; n;)
    {
      Irq *i = nonull_static_cast<Irq*>(n);
      i->hit();
      n = i->Irq_base::_next;
    }
}


PUBLIC inline
void
Irq_debugger::hit()
{
  assert (cpu_lock.test());
  pin()->mask_and_ack();

#if defined(CONFIG_KDB) || defined(CONFIG_JDB)
  if (pin()->check_debug_irq())
    kdb_ke("IRQ ENTRY");
#endif
  pin()->unmask();
}


PUBLIC inline
void
Irq_sender::hit()
{
  // We're entered holding the kernel lock, which also means irqs are
  // disabled on this CPU (XXX always correct?).  We never enable irqs
  // in this stack frame (except maybe in a nonnested invocation of
  // switch_exec() -> switchin_context()) -- they will be re-enabled
  // once we return from it (iret in entry.S:all_irqs) or we switch to
  // a different thread.

  // LOG_MSG_3VAL(current(), "IRQ", dbg_id(), 0, _queued);

  assert (cpu_lock.test());
  pin()->mask_and_ack();

  if (EXPECT_FALSE (!_irq_thread))
    return;
  else if (EXPECT_FALSE (_irq_thread == (void*)-1))
    {
      // debugger attached to IRQ
#if defined(CONFIG_KDB) || defined(CONFIG_JDB)
      if (pin()->check_debug_irq())
        kdb_ke("IRQ ENTRY");
#endif
      pin()->unmask();
      return;
    }


  Smword old;
  do
    old = _queued;
  while (!mp_cas(&_queued, old, old + 1));

  if (EXPECT_TRUE (old == 0))	// increase hit counter
    {
      if (EXPECT_FALSE(_irq_thread->cpu() != current_cpu()))
	_irq_thread->drq(&_drq, handle_remote_hit, this, 0,
	                 Context::Drq::Target_ctxt, Context::Drq::No_wait);
      else
	send_msg(_irq_thread);
    }
}


PRIVATE
L4_msg_tag
Irq_sender::sys_attach(L4_msg_tag const &tag, Utcb const *utcb, Syscall_frame * /*f*/,
                Obj_space *o_space)
{
  L4_snd_item_iter snd_items(utcb, tag.words());

  Receiver *thread = 0;
  unsigned mode = utcb->values[0] >> 16;

  if (tag.items() == 0)
    {
      // detach
      if (mode & Set_irq_mode)
	printf("DEPRECATED SET IRQ MODE\n");
	//pin()->set_mode(mode);
      else
	{
	  free(_irq_thread);
	  _irq_id = ~0UL;
	}
      return commit_result(0);
    }

  if (tag.items() && snd_items.next())
    {
      L4_fpage bind_thread(snd_items.get()->d);
      if (EXPECT_FALSE(!bind_thread.is_objpage()))
	return commit_error(utcb, L4_error::Overflow);

      thread = Kobject::dcast<Thread_object*>(o_space->lookup_local(bind_thread.obj_index()));
    }

  if (!thread)
    thread = current_thread();

  if (alloc(thread))
    {
      if (mode & Set_irq_mode)
	printf("DEPRECATED SET IRQ MODE\n");
      _irq_id = utcb->values[1];
      return commit_result(0);
    }

  return commit_result(-L4_err::EInval);
}

PUBLIC explicit
Irq_muxer::Irq_muxer(Ram_quota *q = 0)
: Kobject_h<Irq_muxer, Irq>(q), _mask_cnt(0)
{}

PUBLIC
void
Irq_muxer::destroy(Kobject ***rl)
{
  // FIXME: unchain IRQs

  Irq::destroy(rl);
}

PRIVATE
L4_msg_tag
Irq_muxer::sys_attach(L4_msg_tag const &tag, Utcb const *utcb, Syscall_frame * /*f*/,
                Obj_space *o_space)
{
  L4_snd_item_iter snd_items(utcb, tag.words());

  Irq *irq = 0;
  unsigned mode = utcb->values[0] >> 16;

  if (tag.items() == 0)
    return commit_result(-L4_err::EInval);

  if (tag.items() && snd_items.next())
    {
      L4_fpage bind_irq(snd_items.get()->d);
      if (EXPECT_FALSE(!bind_irq.is_objpage()))
	return commit_error(utcb, L4_error::Overflow);

      irq = Kobject::dcast<Irq*>(o_space->lookup_local(bind_irq.obj_index()));
    }

  if (!irq)
    return commit_result(-L4_err::EInval);

  if (mode & Set_irq_mode)
    printf("DEPRECATED SET IRQ MODE\n");
    //pin()->set_mode(mode);

  irq->pin()->unbind_irq();

  if (!irq->pin()->masked())
    {
      Smword old;
      do
	old = _mask_cnt;
      while (!mp_cas(&_mask_cnt, old, old + 1));
    }

  irq->pin()->replace<Chain_irq_pin>(this);

  irq->Irq_base::_next = Irq_base::_next;
  Irq_base::_next = irq;

  return commit_result(0);
}

PUBLIC
L4_msg_tag
Irq_muxer::kinvoke(L4_obj_ref, Mword /*rights*/, Syscall_frame *f,
                   Utcb const *utcb, Utcb *)
{
  register Context *const c_thread = ::current();
  register Space *const c_space = c_thread->space();
  register Obj_space *const o_space = c_space->obj_space();

  L4_msg_tag tag = f->tag();

  if (EXPECT_FALSE(tag.proto() != L4_msg_tag::Label_irq))
    return commit_result(-L4_err::EBadproto);

  if (EXPECT_FALSE(tag.words() < 1))
    return commit_result(-L4_err::EInval);

  switch ((utcb->values[0] & 0xffff))
    {
    case Op_chain:
      return sys_attach(tag, utcb, f, o_space);
    case Op_trigger:
      hit();
      Irq::log_irq(this, 0);
      return no_reply();
    default:
      return commit_result(-L4_err::EInval);
    }
}

PUBLIC
L4_msg_tag
Irq_sender::kinvoke(L4_obj_ref, Mword /*rights*/, Syscall_frame *f,
                    Utcb const *utcb, Utcb *)
{
  register Context *const c_thread = ::current();
  register Space *const c_space = c_thread->space();
  register Obj_space *const o_space = c_space->obj_space();

  L4_msg_tag tag = f->tag();

  if (EXPECT_FALSE(tag.proto() != L4_msg_tag::Label_irq))
    return commit_result(-L4_err::EBadproto);

  if (EXPECT_FALSE(tag.words() < 1))
    return commit_result(-L4_err::EInval);

  switch ((utcb->values[0] & 0xffff))
    {
    case Op_eoi_1:
    case Op_eoi_2:
      pin()->unmask();
      return no_reply();
    case Op_attach: /* ATTACH, DETACH */
      return sys_attach(tag, utcb, f, o_space);
    case Op_trigger:
      hit();
      Irq::log_irq(this, 0);
      return no_reply();
    default:
      return commit_result(-L4_err::EInval);
    }
}

PUBLIC
Mword
Irq_sender::obj_id() const
{ return _irq_id; }


// --------------------------------------------------------------------------
IMPLEMENTATION [debug]:

#include "logdefs.h"

PUBLIC
char const *
Chain_irq_pin::pin_type() const
{ return "CHAIN IRQ"; }

IMPLEMENT
unsigned
Irq::irq_log_fmt(Tb_entry *e, int maxlen, char *buf)
{
  Irq_log *l = e->payload<Irq_log>();
  return snprintf(buf, maxlen, "0x%x/%u D:%lx userip=%lx",
                  l->irq_number, l->irq_number,
                  l->irq_obj, l->user_ip);
}

PUBLIC static inline
void
Irq::log_irq(Irq *irq, int nr)
{
  Context *c = current();
  LOG_TRACE("IRQ-Object triggers", "irq", c, __irq_log_fmt,
      Irq::Irq_log *l = tbe->payload<Irq::Irq_log>();
      l->irq_number = nr;
      l->user_ip    = c->regs()->ip();
      l->irq_obj    = irq ? irq->dbg_id() : ~0UL;
  );
}

PUBLIC static inline NEEDS["config.h"]
void
Irq::log_timer_irq(int nr)
{
  Context *c = current();
  LOG_TRACE("Timer IRQs (kernel scheduling)", "timer", c, __irq_log_fmt,
      Irq::Irq_log *l = tbe->payload<Irq::Irq_log>();
      l->irq_number = nr;
      l->user_ip    = c->regs()->ip(),
      l->irq_obj    = ~0UL;
  );
}

// --------------------------------------------------------------------------
IMPLEMENTATION [!debug]:

PUBLIC static inline
void
Irq::log_irq(Irq *, int)
{}

PUBLIC static inline
void
Irq::log_timer_irq(int)
{}
