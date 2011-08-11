/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/util/name_space_svr>

namespace Ldr {

namespace Names { using namespace L4Re::Util::Names; }

class Entry : public Names::Entry
{
public:
  Entry(Names::Name const &n, Names::Obj const &o, bool dyn = false)
  : Names::Entry(n, o, dyn) {}

  void * operator new (size_t s) { return ::operator new(s); }
  void operator delete(void *b) { ::operator delete(b); }
};

class Name_space : public L4::Server_object,
                   public Names::Name_space
{
public:

  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
  {
    enum { Max_name = 2048 };
    static char buffer[Max_name];

    return Names::Name_space::dispatch(obj, ios, buffer, Max_name);
  }

  Name_space();
  ~Name_space();

  // server support ----------------------------------------
  int get_capability(L4::Ipc::Snd_fpage const &cap_fp, L4::Cap<void> *cap,
                     L4::Server_object **);
  int save_capability(L4::Cap<void> *cap);
  void free_capability(L4::Cap<void> cap);
  Entry *alloc_dynamic_entry(Names::Name const &n, unsigned flags);
  void free_dynamic_entry(Names::Entry *e);

  // internally used to register bootfs files, name spaces...
  int register_obj(Names::Name const &name, Names::Obj const &o,
                   bool dyn = false)
  {
    Entry *n = new Entry(name, o, dyn);
    int err = insert(n);
    if (err < 0)
      delete n;

    return err;
  }
};

}

inline
L4::BasicOStream &operator << (L4::BasicOStream &s, L4Re::Util::Names::Name const &n)
{
  for (int l = 0; l < n.len(); ++l)
    s << n.name()[l];

  return s;
}
