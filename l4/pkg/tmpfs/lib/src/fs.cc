/**
 */
/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/l4re_vfs/backend>
#include <l4/cxx/string>
#include <l4/cxx/avl_tree>

#include <sys/stat.h>
#include <errno.h>

#include <cstdio>


namespace {

using namespace L4Re::Vfs;
using cxx::Ref_ptr;

class File_data
{
public:
  File_data() : _buf(0), _size(0) {}

  unsigned long put(unsigned long offset,
                    unsigned long bufsize, void *srcbuf);
  unsigned long get(unsigned long offset,
                    unsigned long bufsize, void *dstbuf);

  unsigned long size() const { return _size; }

  ~File_data() throw() { free(_buf); }

private:
  void *_buf;
  unsigned long _size;
};

unsigned long
File_data::put(unsigned long offset, unsigned long bufsize, void *srcbuf)
{
  if (offset + bufsize > _size)
    {
      _size = offset + bufsize;
      _buf = realloc(_buf, _size);
    }

  if (!_buf)
    return 0;

  memcpy((char *)_buf + offset, srcbuf, bufsize);
  return bufsize;
}

unsigned long
File_data::get(unsigned long offset, unsigned long bufsize, void *dstbuf)
{
  unsigned long s = bufsize;

  if (offset > _size)
    return 0;

  if (offset + bufsize > _size)
    s = _size - offset;

  memcpy(dstbuf, (char *)_buf + offset, s);
  return s;
}


class Node : public cxx::Avl_tree_node
{
public:
  Node(const char *path, mode_t mode)
    : _ref_cnt(0), _path(strdup(path))
  { _info.st_mode = mode; }

  const char *path() const { return _path; }
  struct stat64 *info() { return &_info; }

  void add_ref() throw() { ++_ref_cnt; }
  int remove_ref() throw() { return --_ref_cnt; }

  bool is_dir() const { return S_ISDIR(_info.st_mode); }

  virtual ~Node() { free(_path); }

private:
  int           _ref_cnt;
  char         *_path;
  struct stat64 _info;
};

struct Node_get_key
{
  typedef cxx::String Key_type;
  static Key_type key_of(Node const *n)
  { return n->path(); }
};

struct Path_avl_tree_compare
{
  bool operator () (const char *l, const char *r) const
  { return strcmp(l, r) < 0; }
  bool operator () (const cxx::String l, const cxx::String r) const
  { return strncmp(l.start(), r.start(), cxx::min(l.len(), r.len())) < 0; }
};

class Pers_file : public Node
{
public:
  Pers_file(const char *name, mode_t mode)
    : Node(name, (mode & 0777) | __S_IFREG) {}
  File_data const &data() const { return _data; }
  File_data &data() { return _data; }
private:
  File_data     _data;
};

class Pers_dir : public Node
{
public:
  Pers_dir(const char *name, mode_t mode)
    : Node(name, (mode & 0777) | __S_IFDIR) {}
  Ref_ptr<Node> find_path(cxx::String);
  int add_node(Ref_ptr<Node> const &);

private:
  typedef cxx::Avl_tree<Node, Node_get_key, Path_avl_tree_compare> Tree;
  Tree _tree;
};

Ref_ptr<Node> Pers_dir::find_path(cxx::String path)
{
  return cxx::ref_ptr(_tree.find_node(path));
}

int Pers_dir::add_node(Ref_ptr<Node> const &n)
{
  int e;
  e = _tree.insert(n.ptr()).second;
  if (!e)
    n->add_ref();
  return e;
}

class Tmpfs_dir : public Be_file
{
public:
  explicit Tmpfs_dir(Ref_ptr<Pers_dir> const &d) throw()
    : _dir(d) {}
  int get_entry(const char *, int, mode_t, Ref_ptr<File> *) throw();
  int fstat64(struct stat64 *buf) const throw();
  int mkdir(const char *, mode_t) throw();
  int unlink(const char *) throw();
  int rename(const char *, const char *) throw();

private:
  int walk_path(cxx::String const &_s,
                Ref_ptr<Node> *ret, cxx::String *remaining = 0);

  Ref_ptr<Pers_dir> _dir;
};

class Tmpfs_file : public Be_file
{
public:
  explicit Tmpfs_file(Ref_ptr<Pers_file> const &f) throw()
    : Be_file(), _file(f), _pos(0) {}

  ssize_t readv(const struct iovec*, int iovcnt) throw();
  ssize_t writev(const struct iovec*, int iovcnt) throw();
  off64_t lseek64(off64_t, int) throw();
  int fstat64(struct stat64 *buf) const throw();

private:
  Ref_ptr<Pers_file> _file;
  off64_t _pos;
};

ssize_t Tmpfs_file::readv(const struct iovec *v, int iovcnt) throw()
{
  if (iovcnt < 0)
    return -EINVAL;


  ssize_t sum = 0;
  for (int i = 0; i < iovcnt; ++i)
    {
      sum  += _file->data().get(_pos, v[i].iov_len, v[i].iov_base);
      _pos += v[i].iov_len;
    }
  return sum;
}

ssize_t Tmpfs_file::writev(const struct iovec *v, int iovcnt) throw()
{
  if (iovcnt < 0)
    return -EINVAL;

  ssize_t sum = 0;
  for (int i = 0; i < iovcnt; ++i)
    {
      sum  += _file->data().put(_pos, v[i].iov_len, v[i].iov_base);
      _pos += v[i].iov_len;
    }
  return sum;
}

int Tmpfs_file::fstat64(struct stat64 *buf) const throw()
{
  memcpy(buf, _file->info(), sizeof(*buf));
  return 0;
}


off64_t Tmpfs_file::lseek64(off64_t offset, int whence) throw()
{
  switch (whence)
    {
    case SEEK_SET: _pos = offset; break;
    case SEEK_CUR: _pos += offset; break;
    case SEEK_END: _pos = _file->data().size() + offset; break;
    default: return -EINVAL;
    };

  if (_pos < 0)
    return -EINVAL;

  return _pos;
}


int
Tmpfs_dir::get_entry(const char *name, int flags, mode_t mode,
                     Ref_ptr<File> *file) throw()
{
  Ref_ptr<Node> path;
  if (!*name)
    {
      *file = this;
      return 0;
    }

  cxx::String n = name;

  int e = walk_path(n, &path, &n);

  if (e == -ENOTDIR)
    return e;

  if (!(flags & O_CREAT) && e < 0)
    return e;

  if ((flags & O_CREAT) && e == -ENOENT)
    {
      Ref_ptr<Node> node(new Pers_file(n.start(), mode));
      // when ENOENT is return, path is always a directory
      int e = cxx::ref_ptr_static_cast<Pers_dir>(path)->add_node(node);
      if (e)
        return e;
      path = node;
    }

  if (path->is_dir())
    *file = new Tmpfs_dir(cxx::ref_ptr_static_cast<Pers_dir>(path));
  else
    *file = new Tmpfs_file(cxx::ref_ptr_static_cast<Pers_file>(path));

  if (!*file)
    return -ENOMEM;


  return 0;
}

int
Tmpfs_dir::fstat64(struct stat64 *buf) const throw()
{
  memcpy(buf, _dir->info(), sizeof(*buf));
  return 0;
}

int
Tmpfs_dir::walk_path(cxx::String const &_s,
                     Ref_ptr<Node> *ret, cxx::String *remaining)
{
  Ref_ptr<Pers_dir> p = _dir;
  cxx::String s = _s;
  Ref_ptr<Node> n;

  while (1)
    {
      cxx::String::Index sep = s.find("/");

      n = p->find_path(s.head(sep - s.start()));

      if (!n)
        {
          *ret = p;
          if (remaining)
            *remaining = s.head(sep - s.start());
          return -ENOENT;
        }


      if (sep == s.end())
        {
          *ret = n;
          return 0;
        }

      if (!n->is_dir())
        return -ENOTDIR;

      s = s.substr(sep + 1);

      p = cxx::ref_ptr_static_cast<Pers_dir>(n);
    }

  *ret = n;

  return 0;
}

int
Tmpfs_dir::mkdir(const char *name, mode_t mode) throw()
{
  Ref_ptr<Node> node = _dir;
  cxx::String p = cxx::String(name);
  cxx::String path, last = p;
  cxx::String::Index s = p.rfind("/");
  if (s != p.end())
    {
      path = p.head(s);
      last = p.substr(s + 1, p.end() - s);

      int e = walk_path(path, &node);
      if (e < 0)
        return e;
    }

  if (!node->is_dir())
    return -ENOTDIR;

  Ref_ptr<Pers_dir> dnode = cxx::ref_ptr_static_cast<Pers_dir>(node);

  Ref_ptr<Pers_dir> dir(new Pers_dir(last.start(), mode));
  return dnode->add_node(dir);
}

int
Tmpfs_dir::unlink(const char *name) throw()
{
  printf("Unimplemented: %s(%s)\n", __func__, name); 
  return -ENOMEM;
}

int
Tmpfs_dir::rename(const char *old, const char *newn) throw()
{
  printf("Unimplemented: %s(%s, %s)\n", __func__, old, newn); 
  return -ENOMEM;
}



class Tmpfs_fs : public Be_file_system
{
public:
  Tmpfs_fs() : Be_file_system("tmpfs") {}
  int mount(char const *source, unsigned long mountflags,
            void const *data, cxx::Ref_ptr<File> *dir) throw()
  {
    (void)mountflags;
    (void)source;
    (void)data;
    *dir = cxx::ref_ptr(new Tmpfs_dir(cxx::ref_ptr(new Pers_dir("root", 0777))));
    if (!*dir)
      return -ENOMEM;
    return 0;
  }
};

static Tmpfs_fs _tmpfs;

}
