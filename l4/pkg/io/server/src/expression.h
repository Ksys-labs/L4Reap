#pragma once

#include <l4/sys/l4int.h>
#include "resource.h"

namespace Hw {
  class Device;
}

namespace Vi {
  class Device;
}

struct Value
{
  enum Type
  {
    Void, Num, Range, String, Hw_dev, Vi_dev, Res, Func
  };

  Type type;

  union {
    l4_uint64_t num;

    struct {
      l4_uint64_t s;
      l4_uint64_t e;
    } range;

    struct {
      char const *s;
      char const *e;
    } str;

    Vi::Device *vi_dev;
    Hw::Device *hw_dev;
    Resource *res;
  } val;
};


class Expression
{
private:
  Expression *_next;
  Value v;

public:
  Expression *next() const { return _next; }

  Expression() : _next(0)
  { v.type = Value::Void; }

  explicit Expression(l4_uint64_t n) : _next(0)
  { v.type = Value::Num; v.val.num = n; }

  explicit Expression(l4_uint64_t s, l4_uint64_t e) : _next(0)
  { v.type = Value::Range; v.val.range.s = s; v.val.range.e = e; }

  explicit Expression(char const *s, char const *e) : _next(0)
  { v.type = Value::String; v.val.str.s = s; v.val.str.e = e; }

  explicit Expression(Hw::Device *d) : _next(0)
  { v.type = Value::Hw_dev; v.val.hw_dev = d; }

  explicit Expression(Vi::Device *d) : _next(0)
  { v.type = Value::Vi_dev; v.val.vi_dev = d; }

  explicit Expression(Resource *r) : _next(0)
  { v.type = Value::Res; v.val.res = r; }

  Expression *prepend(Expression *suffix) { _next = suffix; return this; }
  static Expression **append(Expression **tail, Expression *e)
  { e->_next = 0; *tail = e; return &e->_next; }

  static Expression *del_this(Expression *t)
  { Expression *tmp = t->_next; delete t; return tmp; }

  static void del_all(Expression *f)
  {
    while (f)
      f = del_this(f);
  }

  Value const eval() const { return v; }
};


