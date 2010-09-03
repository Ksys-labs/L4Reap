INTERFACE:

#include "context.h"
#include "kobject.h"
#include "l4_types.h"
#include "rcupdate.h"
#include "space.h"
#include "spin_lock.h"

class slab_cache_anon;

/**
 * \brief A task is a protection domain.
 *
 * A is derived from Space, which aggregates a set of address spaces.
 * Additionally to a space, a task provides initialization and
 * destruction functionality for a protection domain.
 * Task is also derived from Rcu_item to provide RCU shutdown of tasks.
 */
class Task :
  public Space,
  public Kobject_iface,
  public Kobject,
  private Rcu_item
{
  FIASCO_DECLARE_KOBJ();

  friend class Jdb_space;

private:

  /// \brief Do host (platform) specific initialization.
  void host_init();

  /// \brief Map the trace buffer to the user address space.
  void map_tbuf();

public:

  enum Operation
  {
    Map         = 0,
    Unmap       = 1,
    Cap_info    = 2,
    Ldt_set_x86 = 0x11,
    Vm_ops      = 0x20,
  };


  /// \brief Destroy it.
  ~Task();

  /**
   * \brief Allocate memory for UTCBs for that task.
   * \return true on success, or false on memory shortage.
   */
  bool alloc_utcbs();

  /**
   * \brief Free the UTCBs allocated with alloc_utcbs()-.
   */
  void free_utcbs();

private:

  enum
  {
    /// Number of Utcbs fitting on a single page
    Utcbs_per_page = Config::PAGE_SIZE / sizeof(Utcb),
  };

private:
  /// map the global utcb pointer page into this task
  void map_utcb_ptr_page();



};


//---------------------------------------------------------------------------
IMPLEMENTATION:

#include "atomic.h"
#include "auto_ptr.h"
#include "config.h"
#include "entry_frame.h"
#include "globals.h"
#include "kdb_ke.h"
#include "kmem.h"
#include "kmem_slab_simple.h"
#include "l4_types.h"
#include "l4_buf_iter.h"
#include "logdefs.h"
#include "map_util.h"
#include "mem_layout.h"
#include "ram_quota.h"
#include "paging.h"
#include "vmem_alloc.h"

FIASCO_DEFINE_KOBJ(Task);


PUBLIC virtual
bool
Task::put()
{
  return dec_ref() == 0;
}


IMPLEMENT
bool
Task::alloc_utcbs()
{
  if (!utcb_area_size())
    {
      set_kern_utcb_area(0);
      return true;
    }

  Mapped_allocator *const alloc = Mapped_allocator::allocator();
  void *utcbs = alloc->q_unaligned_alloc(ram_quota(), utcb_area_size());

  if (EXPECT_FALSE(!utcbs))
    return false;

  // clean up utcbs
  memset(utcbs, 0, utcb_area_size());
  set_kern_utcb_area(Address(utcbs));

  unsigned long page_size = Config::PAGE_SIZE;

  // the following works because the size is a power of two
  // and once we have size larger than a super page we have
  // always multiples of superpages
  if (utcb_area_size() >= Config::SUPERPAGE_SIZE)
    page_size = Config::SUPERPAGE_SIZE;

  for (unsigned long i = 0; i < utcb_area_size(); i += page_size)
    {
      Address kern_va = kern_utcb_area() + i;
      Address user_va = user_utcb_area() + i;
      Address pa = mem_space()->pmem_to_phys(kern_va);

      // must be valid physical address
      assert(pa != ~0UL);

      Mem_space::Status res =
	mem_space()->v_insert(Mem_space::Phys_addr(pa),
	    Mem_space::Addr(user_va), Mem_space::Size(page_size),
	    Mem_space::Page_writable | Mem_space::Page_user_accessible
	    | Mem_space::Page_cacheable);

      switch (res)
	{
	case Mem_space::Insert_ok: break;
	case Mem_space::Insert_err_nomem:
	  free_utcbs();
	  return false;
	default:
	  printf("UTCB mapping failed: va=%p, ph=%p, res=%d\n",
	      (void*)user_va, (void*)kern_va, res);
	  kdb_ke("BUG in utcb allocation");
	  free_utcbs();
	  return false;
	}
    }

  return true;
}

IMPLEMENT
void
Task::free_utcbs()
{
  if (EXPECT_FALSE(!kern_utcb_area() || !mem_space() || !mem_space()->dir()))
    return;

  Mapped_allocator * const alloc = Mapped_allocator::allocator();
  unsigned long page_size = Config::PAGE_SIZE;

  // the following works because the size is a poer of two
  // and once we have size larger than a super page we have
  // always multiples of superpages
  if (utcb_area_size() >= Config::SUPERPAGE_SIZE)
    page_size = Config::SUPERPAGE_SIZE;

  for (unsigned long i = 0; i < utcb_area_size(); i += page_size)
    {
      Address user_va = user_utcb_area() + i;
      mem_space()->v_delete(Mem_space::Addr(user_va),
                            Mem_space::Size(page_size));
    }

  alloc->q_unaligned_free(ram_quota(), utcb_area_size(), (void*)kern_utcb_area());

  set_kern_utcb_area(0);
}



/** Allocate space for the UTCBs of all threads in this task.
 *  @ return true on success, false if not enough memory for the UTCBs
 */
PUBLIC
bool
Task::initialize()
{
  // For UX, map the UTCB pointer page. For ia32, do nothing
  map_utcb_ptr_page();

  return true;
}

/**
 * \brief Create a normal Task.
 * \pre \a parent must be valid and exist.
 */
PUBLIC
template< typename SPACE_FACTORY >
Task::Task(SPACE_FACTORY const &sf, Ram_quota *q, L4_fpage const &utcb_area)
  : Space(sf, q, utcb_area)
{
  host_init();

  inc_ref();

  if (mem_space()->is_sigma0())
    map_tbuf();

  if (alloc_utcbs())
    {
      Lock_guard<Spin_lock> guard(state_lock());
      set_state(Ready);
    }
}


PROTECTED static
slab_cache_anon*
Task::allocator()
{
  static slab_cache_anon* slabs = new Kmem_slab_simple (sizeof (Task), 
							sizeof (Mword),
							"Task");
  return slabs;

  // If Fiasco would kill all tasks even when exiting through the
  // kernel debugger, we could use a deallocating version of the above:
  //
  // static auto_ptr<slab_cache_anon> slabs
  //   (new Kmem_slab_simple (sizeof (Task), sizeof (Mword)))
  // return slabs.get();
}



PROTECTED inline NEEDS["kmem_slab_simple.h"]
void *
Task::operator new (size_t size, void *p)
{
  (void)size;
  assert (size == sizeof (Task));
  return p;
}


PUBLIC //inline NEEDS["kmem_slab_simple.h"]
void
Task::operator delete (void *ptr)
{
  Task *t = reinterpret_cast<Task*>(ptr);
  LOG_TRACE("Kobject delete", "del", current(), __fmt_kobj_destroy,
            Log_destroy *l = tbe->payload<Log_destroy>();
            l->id = t->dbg_id();
            l->obj = t;
            l->type = "Task";
            l->ram = t->ram_quota()->current());

  allocator()->q_free(t->ram_quota(), ptr);
}

PUBLIC template< typename SPACE_FACTORY > inline NEEDS[Task::operator new]
static
Task *
Task::create(SPACE_FACTORY const &sf, Ram_quota *quota,
             L4_fpage const &utcb_area)
{
  if (void *t = allocator()->q_alloc(quota))
    {
      Task *a = new (t) Task(sf, quota, utcb_area);
      if (a->valid())
	return a;

      delete a;
    }

  return 0;
}

PUBLIC inline
bool
Task::valid() const
{ return mem_space()->valid() && state() == Ready; }


PUBLIC
void
Task::initiate_deletion(Kobject ***reap_list)
{
  Kobject::initiate_deletion(reap_list);

  Lock_guard<Spin_lock> guard(state_lock());
  set_state(In_deletion);
}

/**
 * \brief Shutdown the task.
 *
 * Currently:
 * -# Unbind and delete all contexts bound to this task.
 * -# Unmap everything from all spaces.
 * -# Delete child tasks.
 */
PUBLIC
void
Task::destroy(Kobject ***reap_list)
{
  Kobject::destroy(reap_list);

  fpage_unmap(this, L4_fpage::all_spaces(L4_fpage::RWX), L4_map_mask::full(), reap_list);
}

PRIVATE inline NOEXPORT
L4_msg_tag
Task::sys_map(unsigned char rights, Syscall_frame *f, Utcb *utcb)
{
  LOG_TRACE("Task map", "map", ::current(), __task_unmap_fmt,
      Log_unmap *lu = tbe->payload<Log_unmap>();
      lu->id = dbg_id();
      lu->mask  = utcb->values[1];
      lu->fpage = utcb->values[2]);

  if (EXPECT_FALSE(!(rights & L4_fpage::W)))
    return commit_result(-L4_err::EPerm);

  L4_msg_tag const tag = f->tag();

  Obj_space *s = current()->space()->obj_space();
  L4_snd_item_iter snd_items(utcb, tag.words());

  if (EXPECT_FALSE(!tag.items() || !snd_items.next()))
    return commit_result(-L4_err::EInval);

  L4_fpage src_task(snd_items.get()->d);
  if (EXPECT_FALSE(!src_task.is_objpage()))
    return commit_result(-L4_err::EInval);

  Task *from = Kobject::dcast<Task*>(s->lookup_local(src_task.obj_index()));
  if (!from)
    return commit_result(-L4_err::EInval);

  // enforce lock order to prevent deadlocks.
  // always take lock from task with the lower memory address first
  Lock_guard_2<Lock> guard;

  if (!guard.lock(&existence_lock, &from->existence_lock))
    return commit_result(-L4_err::EInval);

  cpu_lock.clear();

  Reap_list rl;

  L4_error ret = fpage_map(from, L4_fpage(utcb->values[2]), this, L4_fpage::all_spaces(), utcb->values[1], &rl);

  rl.del();

  cpu_lock.lock();
  // FIXME: treat reaped stuff
  if (ret.ok())
    return commit_result(0);
  else
    return commit_error(utcb, ret);
}


PRIVATE inline NOEXPORT
L4_msg_tag
Task::sys_unmap(Syscall_frame *f, Utcb *utcb)
{
  Lock_guard<Lock> guard;

  if (!guard.lock(&existence_lock))
    return commit_error(utcb, L4_error::Not_existent);

  LOG_TRACE("Task unmap", "unm", ::current(), __task_unmap_fmt,
            Log_unmap *lu = tbe->payload<Log_unmap>();
            lu->id = dbg_id();
            lu->mask  = utcb->values[1];
            lu->fpage = utcb->values[2]);

  cpu_lock.clear();

  Reap_list rl;
  L4_map_mask m(utcb->values[1]);
  unsigned words = f->tag().words();

  for (unsigned i = 2; i < words; ++i)
    {
      unsigned const flushed = fpage_unmap(this, L4_fpage(utcb->values[i]), m, rl.list());
      utcb->values[i] = (utcb->values[i] & ~0xfUL) | flushed;
    }

  rl.del();

  cpu_lock.lock();
  return commit_result(0, words);
}

PRIVATE inline NOEXPORT
L4_msg_tag
Task::sys_cap_valid(Syscall_frame *, Utcb *utcb)
{
  L4_obj_ref obj(utcb->values[1]);

  if (obj.invalid())
    return commit_result(0);

  Obj_space::Capability cap = obj_space()->lookup(obj.cap());
  if (EXPECT_TRUE(cap.valid()))
    {
      if (!(utcb->values[1] & 1))
	return commit_result(1);
      else
	return commit_result(cap.obj()->map_root()->cap_ref_cnt());
    }
  else
    return commit_result(0);
}

PRIVATE inline NOEXPORT
L4_msg_tag
Task::sys_caps_equal(Syscall_frame *, Utcb *utcb)
{
  L4_obj_ref obj_a(utcb->values[1]);
  L4_obj_ref obj_b(utcb->values[2]);

  if (obj_a == obj_b)
    return commit_result(1);

  if (obj_a.invalid() || obj_b.invalid())
    return commit_result(obj_a.invalid() && obj_b.invalid());
  
  Obj_space::Capability c_a = obj_space()->lookup(obj_a.cap());
  Obj_space::Capability c_b = obj_space()->lookup(obj_b.cap());

  return commit_result(c_a == c_b);
}

PRIVATE inline NOEXPORT
L4_msg_tag
Task::sys_cap_info(Syscall_frame *f, Utcb *utcb)
{
  L4_msg_tag const &tag = f->tag();

  switch (tag.words())
    {
    default: return commit_result(-L4_err::EInval);
    case 2:  return sys_cap_valid(f, utcb);
    case 3:  return sys_caps_equal(f, utcb);
    }
}




PUBLIC
void
Task::invoke(L4_obj_ref, Mword rights, Syscall_frame *f, Utcb *utcb)
{
  if (EXPECT_FALSE(f->tag().proto() != L4_msg_tag::Label_task))
    {
      f->tag(commit_result(-L4_err::EBadproto));
      return;
    }

  switch (utcb->values[0])
    {
    case Map:
      f->tag(sys_map(rights, f, utcb));
      return;
    case Unmap:
      f->tag(sys_unmap(f, utcb));
      return;
    case Cap_info:
      f->tag(sys_cap_info(f, utcb));
      return;
    default:
      L4_msg_tag tag = f->tag();
      if (invoke_arch(tag, utcb))
	f->tag(tag);
      else
        f->tag(commit_result(-L4_err::ENosys));
      return;
    }
}

//---------------------------------------------------------------------------
IMPLEMENTATION [!ux]:

IMPLEMENT inline
void
Task::map_utcb_ptr_page()
{}

IMPLEMENT inline
void
Task::host_init()
{}

IMPLEMENT inline
void
Task::map_tbuf()
{}


//---------------------------------------------------------------------------
IMPLEMENTATION [!(ia32|ux|amd64)]:

IMPLEMENT inline
Task::~Task()
{ free_utcbs(); }



// ---------------------------------------------------------------------------
INTERFACE [debug]:

EXTENSION class Task
{
private:
  struct Log_unmap
  {
    Mword id;
    Mword mask;
    Mword fpage;
  } __attribute__((packed));

  static unsigned unmap_fmt(Tb_entry *, int max, char *buf) asm ("__task_unmap_fmt");
};

// ---------------------------------------------------------------------------
IMPLEMENTATION [debug]:

IMPLEMENT
unsigned
Task::unmap_fmt(Tb_entry *e, int max, char *buf)
{
  Log_unmap *l = e->payload<Log_unmap>();
  L4_fpage fp(l->fpage);
  return snprintf(buf, max, "task=[U:%lx] mask=%lx fpage=[%u/%u]%lx",
                  l->id, l->mask, (unsigned)fp.order(), fp.type(), l->fpage);
}

// ---------------------------------------------------------------------------
IMPLEMENTATION[!ia32 || !svm]:

PRIVATE inline NOEXPORT
L4_msg_tag
Task::sys_vm_run(Syscall_frame *, Utcb *)
{
  return commit_result(-L4_err::ENosys);
}

