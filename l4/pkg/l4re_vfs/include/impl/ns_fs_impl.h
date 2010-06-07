/*
 * (c) 2010 Technische Universität Dresden
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
#include "ns_fs.h"
#include "vfs_api.h"
#include "ro_file.h"

#include <l4/re/dataspace>
#include <l4/re/util/env_ns>
#include <cstring>

namespace L4Re { namespace Core {

Simple_store_sz<Env_dir::Size> Ns_base_dir::store;

void *
Ns_base_dir::operator new(size_t s) throw()
{
  if (s > Size)
    return 0;

  return store.alloc();
}

void
Ns_base_dir::operator delete(void *b) throw()
{
  store.free(b);
}


int
Ns_dir::get_ds(const char *path, L4Re::Auto_cap<L4Re::Dataspace>::Cap *ds) throw()
{
  L4Re::Auto_cap<L4Re::Dataspace>::Cap file(cap_alloc()->alloc<L4Re::Dataspace>(), cap_alloc());

  if (!file.is_valid())
    return -ENOMEM;

  int err = _ns->query(path, file.get());

  if (err < 0)
    return -ENOENT;

  *ds = file;
  return err;
}

int
Ns_dir::get_entry(const char *path, int flags, mode_t mode,
                  Ref_ptr<L4Re::Vfs::File> *f) throw()
{
  (void)mode; (void)flags;
  if (!*path)
    {
      *f = this;
      return 0;
    }

  L4Re::Auto_cap<Dataspace>::Cap file;
  int err = get_ds(path, &file);

  if (err < 0)
    return -ENOENT;

  // FIXME: should check if it is a dataspace, somehow
  L4Re::Vfs::File *fi = 0;

  L4::Cap<L4Re::Namespace> nsc
    = L4::cap_dynamic_cast<L4Re::Namespace>(file.get());

  if (!nsc.is_valid())
    fi = new Ro_file(file.get());
  else // use mat protocol here!!
    fi = new Ns_dir(nsc);

  if (!fi)
    return -ENOMEM;

  file.release();
  *f = fi;
  return 0;
}

int
Ns_dir::faccessat(const char *path, int mode, int flags) throw()
{
  (void)flags;
  L4Re::Auto_cap<void>::Cap tmpcap(cap_alloc()->alloc<void>(), cap_alloc());

  if (!tmpcap.is_valid())
    return -ENOMEM;

  if (_ns->query(path, tmpcap.get()))
    return -ENOENT;

  if (mode & W_OK)
    return -EACCES;

  return 0;
}

int
Env_dir::get_ds(const char *path, L4Re::Auto_cap<L4Re::Dataspace>::Cap *ds) throw()
{
  Vfs::Path p(path);
  Vfs::Path first = p.strip_first();

  if (first.empty())
    return -ENOENT;

  L4::Cap<L4Re::Namespace>
    c = _env->get_cap<L4Re::Namespace>(first.path(), first.length());

  if (!c.is_valid())
    return -ENOENT;

  if (p.empty())
    {
      *ds = L4Re::Auto_cap<L4Re::Dataspace>::Cap(L4::cap_reinterpret_cast<L4Re::Dataspace>(c));
      return 0;
    }

  L4Re::Auto_cap<L4Re::Dataspace>::Cap file(cap_alloc()->alloc<L4Re::Dataspace>(), cap_alloc());

  if (!file.is_valid())
    return -ENOMEM;

  int err = c->query(p.path(), p.length(), file.get());

  if (err < 0)
    return -ENOENT;

  *ds = file;
  return err;
}

int
Env_dir::get_entry(const char *path, int flags, mode_t mode,
                   Ref_ptr<L4Re::Vfs::File> *f) throw()
{
  (void)mode; (void)flags;
  if (!*path)
    {
      *f = this;
      return 0;
    }

  L4Re::Auto_cap<Dataspace>::Cap file;
  int err = get_ds(path, &file);

  if (err < 0)
    return -ENOENT;

  // FIXME: should check if it is a dataspace, somehow
  L4Re::Vfs::File *fi = 0;

  L4::Cap<L4Re::Namespace> nsc
    = L4::cap_dynamic_cast<L4Re::Namespace>(file.get());

  if (!nsc.is_valid())
    fi = new Ro_file(file.get());
  else // use mat protocol here!!
    fi = new Ns_dir(nsc);

  if (!fi)
    return -ENOMEM;

  file.release();
  *f = fi;
  return 0;
}

int
Env_dir::faccessat(const char *path, int mode, int flags) throw()
{
  (void)flags;
  Vfs::Path p(path);
  Vfs::Path first = p.strip_first();

  if (first.empty())
    return -ENOENT;

  L4::Cap<L4Re::Namespace>
    c = _env->get_cap<L4Re::Namespace>(first.path(), first.length());

  if (!c.is_valid())
    return -ENOENT;

  if (p.empty())
    {
      if (mode & W_OK)
	return -EACCES;

      return 0;
    }

  L4Re::Auto_cap<void>::Cap tmpcap(cap_alloc()->alloc<void>(), cap_alloc());

  if (!tmpcap.is_valid())
    return -ENOMEM;

  if (c->query(p.path(), p.length(), tmpcap.get()))
    return -ENOENT;

  if (mode & W_OK)
    return -EACCES;

  return 0;
}

}}
