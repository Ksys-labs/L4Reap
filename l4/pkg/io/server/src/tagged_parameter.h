#pragma once

#include "expression.h"

class Tagged_parameter
{
public:
  Expression *get(char const *tag)
  {
    for (Tagged_parameter *c = this; c; c = c->_next)
      {
        if (c->_tag == tag)
	  {
	    c->mark_used();
	    return c->_val;
	  }
      }
    return 0;
  }

  Tagged_parameter(cxx::String const &tag, Expression *val)
  : _next(0), _tag(tag), _val(val), _used(false)
  {}

  Tagged_parameter *prepend(Tagged_parameter *n)
  { n->_next = this; return n; }

  static void del_all(Tagged_parameter *f, bool del_expr = true)
  {
    while (f)
      {
        Tagged_parameter *c = f;
        f = c->_next;

	if (del_expr)
	  Expression::del_all(c->_val);

	delete c;
      }
  }

  Tagged_parameter *next() { return _next; }
  cxx::String const &tag() const { return _tag; }
  Expression *val() const { return _val; }

  void mark_used() { _used = true; }

  bool used() const { return _used; }

private:
  Tagged_parameter *_next;
  cxx::String const _tag;
  Expression *_val;
  bool _used;
};


