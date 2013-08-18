/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#include <l4/re/util/name_space_svr>
#include <l4/re/util/debug>

#include <l4/re/namespace-sys.h>
#include <l4/re/namespace>
#include <l4/re/protocols>
#include <l4/re/util/meta>

#include <cassert>
#include <cstring>
#include <typeinfo>


namespace L4Re { namespace Util { namespace Names {

bool
Name::operator < (Name const &r) const
{
  unsigned char l = cxx::min(len(), r.len());
  int v = strncmp(start(), r.start(), l);
  return v < 0 || (v == 0 && len() < r.len());
}


bool
Entry::link(Entry *src)
{
  if (src->obj()->is_replacable() || !src->obj()->is_complete())
    {
      src->add_link(this);
      if (src->obj()->is_complete())
	obj()->set(*src->obj(), obj()->flags());
      return false;
    }

  obj()->set(*src->obj(), obj()->flags());
  return true;
}

void
Entry::set(Obj const &o)
{
  Entry *e = this;
  while (e)
    {
      e->obj()->set(o, e->obj()->flags());
      e = e->next_link();
    }
}

Entry *
Name_space::find_iter(Name const &pname) const
{
  Name name = pname;
  _dbg.printf("resolve '%.*s': ", name.len(), name.start());
  Name_space const *ns = this;
  while (ns)
    {
      cxx::String::Index sep = name.find("/");
      cxx::String part;
      if (!name.eof(sep))
	part = name.head(sep);
      else
	part = name;

      _dbg.cprintf(" '%.*s'", part.len(), part.start());
      Entry *o = ns->find(Name(part.start(), part.len()));

      if (!o)
	{
	  _dbg.cprintf(": resolution failed: '%.*s' remaining\n",
	              name.len(), name.start());
	  return 0;
	}

      ns = dynamic_cast<Name_space const *>(o->obj()->obj());
      if (ns)
	{
	  if (!name.eof(sep))
	    {
	      name = name.substr(sep + 1);
	      continue;
	    }
	}

      _dbg.cprintf(": found object: %p (%s)\n", o->obj()->obj(), o->obj()->obj()?typeid(*(o->obj()->obj())).name():"");

      return o;
    }

  return 0;
}


int
Name_space::query(L4::Ipc::Iostream &ios, char *buffer, size_t max_len)
{
  char const *name = 0;
  unsigned long len = max_len;
  ios >> L4::Ipc::Buf_in<char const>(name, len);
#if 1
  _dbg.printf("query: [%ld] '%.*s'\n", len, (int)len, name);
#endif

  len = cxx::min<unsigned long>(len, max_len);
  char const *sep = (char const*)memchr(name, '/', len);
  unsigned long part;
  if (sep)
    {
      part = sep - name;
      memcpy(buffer, sep + 1, len - part - 1);
    }
  else
    part = len;

  Entry *n = find(Name(name, part));
  if (!n)
    return -L4_ENOENT;
  else if (!n->obj()->is_valid())
    return -L4_EAGAIN;
  else
    {
      if (n->obj()->cap().validate(L4_BASE_TASK_CAP).label() <= 0)
        {
          assert (!n->obj()->is_local());

          free_capability(n->obj()->cap());

          if (n->is_dynamic())
            {
              remove(n->name());
              free_dynamic_entry(n);
            }
          return -L4_ENOENT;
        }

      l4_umword_t result = 0;

      if (part < len)
	{
	  result |= L4Re::Namespace::Partly_resolved;
	  ios << (l4_umword_t)0 << L4::Ipc::Buf_cp_out<char>(buffer, len - part - 1);
	}

      unsigned flags = L4_FPAGE_RO;
      if (n->obj()->is_rw())     flags |= L4_FPAGE_RX;
      if (n->obj()->is_strong()) flags |= L4_FPAGE_RW;

      ios << L4::Ipc::Snd_fpage(n->obj()->cap(), flags );
      _dbg.printf(" result = %lx flgs=%x strg=%d\n",
                  result, flags, (int)n->obj()->is_strong());
      return result;
    }
}

int
Name_space::insert_entry(Name const &name, unsigned flags, Entry **e)
{
  Entry *n = find(name);
  if (n && n->obj()->is_valid())
    {
      if (!(flags & L4Re::Namespace::Overwrite)
	  && n->obj()->cap().validate(L4_BASE_TASK_CAP).label() > 0)
	return -L4_EEXIST;

      if (!n->obj()->is_local())
	free_capability(n->obj()->cap());

      if (n->is_dynamic())
	{
	  remove(n->name());
	  free_dynamic_entry(n);
	  n = 0;
	}
      else
	{
	  n->obj()->reset(Obj::F_rw);
	  if (!n->obj()->is_replacable())
	    return -L4_EEXIST;
	}
    }

  flags &= L4Re::Namespace::Cap_flags;
  if (!n)
    {
      if (!(n = alloc_dynamic_entry(name, flags)))
	return -L4_ENOMEM;
      else
	{
	  int err = insert(n);
	  if (err < 0)
	    {
	      free_dynamic_entry(n);
	      return err;
	    }
	}
    }

  *e = n;
  return 0;
}

int
Name_space::link_entry(L4::Ipc::Iostream &ios, char *buffer, size_t max_len)
{
  char const *name = 0, *src_name = 0;
  unsigned long len, src_len;
  unsigned flags;
  L4::Ipc::Snd_fpage src_cap;
  ios >> flags >> L4::Ipc::Buf_in<char const>(name, len)
      >> L4::Ipc::Buf_in<char const>(src_name, src_len)
      >> src_cap;

  L4::Cap<void> reg_cap(L4::Cap_base::No_init);
  L4::Server_object *src_ns_o = 0;
  // Did we receive something we have handed out ourselves? If yes,
  // register the object under the given name but do not allocate
  // anything more.
  if (int r = get_capability(src_cap, &reg_cap, &src_ns_o))
    return r;

  Name_space *src_ns = dynamic_cast<Name_space*>(src_ns_o);
  if (!src_ns)
    return -L4_EINVAL;

  _dbg.printf("link: '%.*s' flags=%x\n", (int)len, name, flags);

  Name const src_n(src_name, src_len);

  Entry *n = src_ns->find(src_n);
  if (!n)
    {
      if (!(n = src_ns->alloc_dynamic_entry(src_n, 0)))
	return -L4_ENOMEM;
      else
	{
	  int err = src_ns->insert(n);
	  if (err < 0)
	    {
	      src_ns->free_dynamic_entry(n);
	      return err;
	    }
	}
   }

  // got a mapping at Rcv_cap
  len = cxx::min(len, (unsigned long)max_len);
  memcpy(buffer, name, len);
  Name const dst_name(buffer, len);

  Entry *dst;

  if (int r = insert_entry(dst_name, flags, &dst))
    return r;

  dst->link(n);

  return 0;
}


int
Name_space::register_entry(L4::Ipc::Iostream &ios, char *buffer, size_t max_len)
{
  char const *name = 0;
  unsigned long len;
  unsigned flags;
  L4::Ipc::Snd_fpage cap;
  ios >> flags >> L4::Ipc::Buf_in<char const>(name, len);

  L4::Cap<void> reg_cap(L4_INVALID_CAP);
  l4_msgtag_t tag;
  ios >> tag;
  if (tag.items())
    {
      ios >> cap;

      // Did we receive something we have handed out ourselves? If yes,
      // register the object under the given name but do not allocate
      // anything more.
      if (int r = get_capability(cap, &reg_cap))
	return r;
    }

  _dbg.printf("register: '%.*s' flags=%x\n", (int)len, name, flags);

  // got a mapping at Rcv_cap
  len = cxx::min(len, (unsigned long)max_len);
  memcpy(buffer, name, len);
  Name _name(buffer, len);

  Entry *n;
  if (int r = insert_entry(_name, flags, &n))
    return r;

  if (reg_cap.is_valid())
    {
      if (int r = save_capability(&reg_cap))
	return r;

      n->set(Names::Obj(flags & L4Re::Namespace::Cap_flags, reg_cap));
    }

  return 0;
}

int
Name_space::unlink_entry(L4::Ipc::Iostream &ios, char *buffer, size_t max_len)
{
  char const *name = 0;
  unsigned long len = max_len;
  ios >> L4::Ipc::Buf_in<char const>(name, len);
#if 1
  _dbg.printf("unlink: [%ld] '%.*s'\n", len, (int)len, name);
#endif

  len = cxx::min<unsigned long>(len, max_len);
  char const *sep = (char const*)memchr(name, '/', len);
  unsigned long part;
  if (sep)
    {
      part = sep - name;
      memcpy(buffer, sep + 1, len - part - 1);
    }
  else
    part = len;

  Entry *n = find(Name(name, part));
  if (!n || !n->obj()->is_valid())
    return -L4_ENOENT;

  if (!n->obj()->is_local())
    free_capability(n->obj()->cap());

  if (n->is_dynamic())
    {
      remove(n->name());
      free_dynamic_entry(n);
    }
  else
    return -L4_EACCESS;

  return 0;
}



int
Name_space::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios, char *buffer,
                     size_t max_len)
{
  l4_msgtag_t tag;
  ios >> tag;

  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4Re::Util::handle_meta_request<L4Re::Namespace>(ios);

    case L4Re::Protocol::Namespace:
	{
	  L4::Opcode op;
	  ios >> op;


	  int err;
	  switch(op)
	    {
	    case L4Re::Namespace_::Query:
	      return query(ios, buffer, max_len);
	    case L4Re::Namespace_::Link:
	      if (!(obj & 1)) // & L4_FPAGE_X
		return -L4_EPERM;
	      err = link_entry(ios, buffer, max_len);
	      return err;
	    case L4Re::Namespace_::Register:
	      if (!(obj & 1)) // & L4_FPAGE_X
		return -L4_EPERM;
	      err = register_entry(ios, buffer, max_len);
	      return err;
	    case L4Re::Namespace_::Unlink:
	      if (!(obj & 1)) // & L4_FPAGE_X
		return -L4_EPERM;
	      err = unlink_entry(ios, buffer, max_len);
	      return err;
	    default:
	      return -L4_ENOSYS;
	    }
	}
    default:
      return -L4_EBADPROTO;
    }

}

}}}
