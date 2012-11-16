/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
%code top {
#include "vbus_factory.h"
#include <cstring>

}

%code requires {

#include "vdevice.h"
#include "device.h"
#include "expression.h"
#include "hw_device.h"
#include <vector>
#include <l4/cxx/string>
#include "vbus_factory.h"
#include "tagged_parameter.h"

}

%require "2.3"

%debug

%start start

%defines

%skeleton "lalr1.cc"

%name-prefix="cfg"

%define "parser_class_name" "Parser"

%locations

%parse-param { class Scanner *_lexer }
%parse-param { Vi::Dev_factory *_vbus_factory }
%parse-param { Vi::Device *&_glbl_vbus }
%parse-param { Hw::Device *_hw_root }
%parse-param { int &_errors }

%error-verbose

%union {
    l4_uint64_t num;
    struct {
      char const *s;
      char const *e;
    } str;
    Tagged_parameter *param;
    Expression *expr;
    Vi::Device *device;
    Hw::Device *hw_device;
}

%token		END	0	"end of file"
//%token		EOL		"end of line"
%token		NEW		"new operator"
//%token		RETURN		"return operator"
%token		RESOURCE	"new-res operator"
%token		HWROOT		"hw-root operator"
%token		WRAP		"wrap operator"
%token		INSTANCE	"instantiation oprtator '=>'"
%token<str>	IDENTIFIER	"identifier"
%token<num>	INTEGER		"integer"
%token<str>	STRING		"string"
%token		RANGE		"range operator '..'"
%token		COMMA		"comma"

%type<device> device_body device_def_statement
%type<expr> expression device_def_expr
%type<expr> hw_device_list
%type<hw_device> hw_device
%type<device> statement_list top_level_list top_level_statement
%type<str>    param_list
%type<expr> hw_resource_def
%type<hw_device> hw_device_def hw_device_body_statement
%type<expr> const_expression
%type<expr> parameter_list
%type<param> tagged_parameter tagged_parameter_list_ne tagged_parameter_list

%code {

#include <algorithm>
#include "cfg_scanner.h"
#include "hw_device.h"
#include "tagged_parameter.h"

#undef yylex
#define yylex _lexer->lex

static inline
std::ostream &operator << (std::ostream &s, cxx::String const &str)
{
  s.write(str.start(), str.len());
  return s;
}


static
void wrap(Device *h, Vi::Device **first, Tagged_parameter *filter)
{
  Hw::Device *hd = dynamic_cast<Hw::Device *>(h);

  Vi::Device *vd = Vi::Dev_factory::create(hd, filter);
  for (Tagged_parameter *c = filter; c; c = c->next())
    for (Expression *x = c->val(); x; x = x->next())
      {
        Value v = x->eval();
        int r;
        switch (v.type)
          {
          case Value::String:
            r = vd->add_filter(c->tag(), cxx::String(v.val.str.s, v.val.str.e));
            break;
          case Value::Num:
            r = vd->add_filter(c->tag(), v.val.num);
            break;
          case Value::Range:
            r = vd->add_filter(c->tag(), v.val.range.s, v.val.range.e);
            break;
          default:
            d_printf(DBG_ERR, "ERROR: Filters expressions must be of type string, number, or range.\n");
            r = -ENODEV;
            break;
          }

        if (r >= 0)
          c->mark_used();
        if (r == -EINVAL)
          d_printf(DBG_ERR, "ERROR: filter '%*.s' has unsupoorted value\n", c->tag().len(), c->tag().start());
      }

  if (vd)
    {
      vd->add_sibling(*first);
      *first = vd;
    }
}

struct match_by_cid
{
  enum { DEPTH = L4VBUS_MAX_DEPTH };
  cxx::String cid;
  mutable Expression **tail;

  match_by_cid(cxx::String const &cid, Expression *&l) : cid(cid), tail(&l)
  {}

  void operator () (Hw::Device *dev) const
  {
    Hw::Device *hd = dev;
    cxx::String h = cid;

    for (cxx::String::Index n = h.find(","); !h.empty();
	 h = h.substr(n+1), n = h.find(","))
      if (hd->match_cid(h.head(n)))
        {
	  tail = Expression::append(tail, new Expression(hd));
	  return;
	}
  }
};

struct match_by_name
{
  enum { DEPTH = 0 };
  cxx::String cid;
  mutable Expression **tail;

  match_by_name(cxx::String const &cid, Expression *&l) : cid(cid), tail(&l)
  {}

  void operator () (Hw::Device *dev) const
  {
    Hw::Device *hd = dev;
    if (cid == hd->name())
      tail = Expression::append(tail, new Expression(hd));
  }
};

template< typename LI >
static
LI *concat(LI *l, LI *r)
{
  if (!l)
    return r;
  else if (r)
    {
      LI *x = l;
      while (x->next())
	x = x->next();

      x->add_sibling(r);
    }
  return l;
}

template<typename Match, typename LOC>
static
Expression *
dev_list_from_dev(Hw::Device *in, cxx::String const & /*op*/,
                  cxx::String const &arg, LOC const &l)
{
  Expression *nl = 0;

  std::for_each(in->begin(Match::DEPTH), in->end(),
                Match(arg, nl));
  if (!nl)
    std::cerr << l << ": warning: could not find '" << arg << "'"
              << std::endl;

  return nl;
}

static void check_parameter_list(cfg::location const &l, Tagged_parameter *t)
{
  for (; t; t = t->next())
    {
      if (!t->used())
        std::cerr << l << ": warning: unused parameter '" << t->tag() << "'" << std::endl;
    }
}

static
Hw::Device *
find_by_name(Hw::Device *p, cxx::String const &arg);

static
void
add_children(Vi::Device *p, Vi::Device *cld_list)
{
  if (!p)
    return;

  for (Vi::Device *c = cld_list; c;)
    {
      Vi::Device *n = c->next();
      c->add_sibling(0);
      p->add_child(c);
      c = n;
    }
}

static
Expression *
wrap_hw_devices(cfg::location const &l, Expression *hw_devs, Tagged_parameter *filter)
{
  Vi::Device *vd = 0;
  Expression *i = hw_devs;
  while (i)
    {
      Value v = i->eval();
      if (v.type == Value::Hw_dev)
        wrap(v.val.hw_dev, &vd, filter);
      else
        std::cerr << l << ": error: 'wrap' takes a list of hardware devices" << std::endl;

      i = Expression::del_this(i);
    }

  if (vd)
    return new Expression(vd);
  else
    return 0;
}

static
void
set_device_names(Device *devs, cxx::String const &name, bool array = false)
{
  int num = 0;
  for (Device *c = devs; c; c = c->next())
    {
      if (array)
        {
	  char buf[40];
	  snprintf(buf, sizeof(buf) - 1, "%.*s[%04d]", name.len(), name.start(), num++);
	  c->name(buf);
	}
      else
        c->name(name);
    }
}

static inline
std::ostream &operator << (std::ostream &s, Hw::Device::Prop_val const &v)
{
  if (v.type == Hw::Device::Prop_val::String)
    s.write(v.val.str.s, v.val.str.e - v.val.str.s);
  else if (v.type == Hw::Device::Prop_val::Int)
    s << (int)v.val.integer;

  return s;
}


static
Expression *
create_resource(cfg::Parser::location_type const &l, cxx::String const &type,
                Expression *args)
{

  unsigned flags = 0;
  Value a;

  if (type == "Io")
    flags = Resource::Io_res;
  else if (type == "Irq")
    flags = Resource::Irq_res;
  else if (type == "Mmio")
    flags = Resource::Mmio_res;
  else if (type == "Mmio_ram")
    flags = Resource::Mmio_res | 0x80000;
  else
    {
      std::cerr << l << ": unknown resource type '" << type << "', expected Io, Irq, Mmio, or Mmio_ram" << std::endl;

      return 0;
    }

  if (!args)
    {
      std::cerr << l << ": missing arguments for new resource '" << type << "'" << std::endl;
      return 0;
    }

  l4_uint64_t s, e;
  switch (flags)
    {
    case Resource::Io_res:
    case Resource::Irq_res:
    case Resource::Mmio_res:
      if (!args)
        {
	  std::cerr << l << ": too few arguments to constructor of resource '" << type << "'" << std::endl;
	  return 0;
	}

      a = args->eval();
      switch (a.type)
        {
	case Value::Num:
	  s = a.val.num;
	  e = a.val.num;
	  break;
	case Value::Range:
	  s = a.val.range.s;
	  e = a.val.range.e;
	  break;
	default:
	  std::cerr << l
	    << ": error: first argument for a resource definition must be a numer or a range"
	    << std::endl;
	  s = 0;
	  e = 0;
	  break;
	}
      if (s > e)
	{
	  std::cerr << l << ": start of range bigger than end" << std::endl;
	  Expression::del_all(args);
	  return 0;
	}

      args = Expression::del_this(args);

      if (args)
        {
	  a = args->eval();
	  if (a.type != Value::Num)
	    {
	      std::cerr << l
	        << ": argument 2 must by of type integer in constructor of resource '"
	        << type << "'" << std::endl;
	      Expression::del_all(args);
	      return 0;
	    }
          flags |= a.val.num & 0x3fff00;

	  args = Expression::del_this(args);
	}

      if (args)
        {
	  std::cerr << l << ": too many arguments to constructor of resource '" << type << "'" << std::endl;
	  Expression::del_all(args);
	}

      return new Expression(new Resource(flags, s , e));

    case (Resource::Mmio_res |  0x80000):
      if (!args)
        {
	  std::cerr << l << ": too few arguments to constructor of resource '" << type << "'" << std::endl;
	  Expression::del_all(args);
	  return 0;
	}

      a = args->eval();
      if (a.type != Value::Num)
        {
	  std::cerr << l << ": error: expected number argument to constructor of resource '" << type << "'" << std::endl;
	  Expression::del_all(args);
	  return 0;
	}

      s = a.val.num;
      args = Expression::del_this(args);

      a = args->eval();
      if (a.type != Value::Num)
        {
	  std::cerr << l << ": error: expected number argument to constructor of resource '" << type << "'" << std::endl;
	  Expression::del_all(args);
	  return 0;
	}

      e = a.val.num;
      args = Expression::del_this(args);
      if (args)
        {
	  std::cerr << l << ": too many arguments to constructor of resource '" << type << "'" << std::endl;
	  Expression::del_all(args);
	  return 0;
	}

      return new Expression(new Mmio_data_space(s, e));
    }

  Expression::del_all(args);
  return 0;
}

static
void
hw_device_set_property(cfg::Parser::location_type const &l, Hw::Device *dev,
                       cxx::String const &prop, Hw::Device::Prop_val const &val)
{
  int res = dev->set_property(prop, val);
  if (res == Hw::Device::E_ok)
    return;

  std::cerr << l << ": cannot set property '" << prop << "' to '" << val << "'";
  switch (res)
    {
    case -Hw::Device::E_no_prop:
      std::cerr << ", no such property" << std::endl;
      break;
    case -Hw::Device::E_inval:
      std::cerr << ", invalid value" << std::endl;
      break;
    default:
      std::cerr << ", unknown error (" << res << ")" << std::endl;
      break;
    }
}

static
Vi::Device *check_dev_expr(cfg::location const &l, Expression *e)
{
  if (!e)
    return 0;

  Value v = e->eval();
  if (v.type != Value::Vi_dev)
    {
      std::cerr << l << ": error: expected a device definition" << std::endl;
      return 0;
    }

  Expression::del_all(e);
  return v.val.vi_dev;
}

static
Resource *check_resource_expr(cfg::location const &l, Expression *e)
{
  if (!e)
    return 0;

  Value v = e->eval();
  if (v.type != Value::Res)
    {
      std::cerr << l << ": error: expected a device definition" << std::endl;
      return 0;
    }

  Expression::del_all(e);
  return v.val.res;
}
}

%%

param_list
/*	: *empty* { $$.s = "(noname)"; $$.e = $$.s + strlen($$.s); } */
	: '(' ')' { $$.s = "(noname)"; $$.e = $$.s + strlen($$.s); }
/*	| '(' STRING ')' { $$=$2; ++$$.s; --$$.e; } */

tagged_parameter
	: IDENTIFIER '=' const_expression { $$ = new Tagged_parameter(cxx::String($1.s, $1.e), $3); }
	| IDENTIFIER '=' '(' parameter_list ')' { $$ = new Tagged_parameter(cxx::String($1.s, $1.e), $4); }

tagged_parameter_list_ne
	: tagged_parameter
	| tagged_parameter COMMA tagged_parameter_list_ne { $$ = $3->prepend($1); }

tagged_parameter_list
	: /*empty*/ { $$ = 0; }
	| tagged_parameter_list_ne

device_def_expr
	: NEW IDENTIFIER param_list device_body
	  {
	    Vi::Device *d = _vbus_factory->create(std::string($2.s, $2.e));
	    if (d)
	      {
	        add_children(d, $4);
	        d->finalize_setup();
		$$ = new Expression(d);
	      }
	  }
	| WRAP '(' hw_device_list ')' ';'
	  { $$ = wrap_hw_devices(@3, $3, 0); }
	| WRAP '[' tagged_parameter_list ']' '(' hw_device_list ')' ';'
	  { $$ = wrap_hw_devices(@6, $6, $3); check_parameter_list(@3, $3); Tagged_parameter::del_all($3); }

expression
	: device_def_expr

device_def_statement : expression { $$ = check_dev_expr(@1, $1); }
	| IDENTIFIER INSTANCE expression
	  { $$ = check_dev_expr(@3, $3); set_device_names($$, cxx::String($1.s, $1.e)); }
	| IDENTIFIER '[' ']' INSTANCE expression
	  { $$ = check_dev_expr(@5, $5); set_device_names($$, cxx::String($1.s, $1.e), true); }
	| STRING INSTANCE expression
	  { $$ = check_dev_expr(@3, $3); set_device_names($$, cxx::String($1.s + 1, $1.e - 1)); }
	| STRING '[' ']' INSTANCE expression
	  { $$ = check_dev_expr(@5, $5); set_device_names($$, cxx::String($1.s + 1, $1.e - 1), true); }

statement_list
	: /*empty*/ { $$ = 0; }
	| device_def_statement statement_list
	  { $$ = concat($1, $2); }

device_body
	: ';' { $$ = 0; }
	| '{' statement_list '}' { $$ = $2; }

hw_device_list
	: hw_device { $$ = new Expression($1); }
	| hw_device ',' hw_device_list { $$ = (new Expression($1))->prepend($3); }
	| hw_device '.' IDENTIFIER '(' STRING ')'
	  { $$ = dev_list_from_dev<match_by_cid>($1, cxx::String($3.s, $3.e), cxx::String($5.s + 1, $5.e - 1), @5); }


hw_device
	: HWROOT { $$ = _hw_root; }
	| hw_device '.' IDENTIFIER
	  {
	    $$ = find_by_name($1, cxx::String($3.s, $3.e));
	    if (!$$)
	      {
	        error(@3, "device '" + std::string($1->name()) + "' does not have a member named '" + std::string($3.s, $3.e) + "'");
	        YYERROR;
	      }
	  }

macro_parameter_list
	: /* empty */
	| IDENTIFIER
	| IDENTIFIER COMMA macro_parameter_list

macro_statement
	: expression

macro_statement_list
	: /* empty */
	| macro_statement ';' macro_statement_list

macro_body
	: '{' macro_statement_list '}'

macro_def
	: '(' macro_parameter_list ')' macro_body

const_expression
	: INTEGER
	{ $$ = new Expression($1); }
	| INTEGER RANGE INTEGER
	{ $$ = new Expression($1, $3); }
	| IDENTIFIER
	{ $$ = new Expression($1.s, $1.e); }

parameter_list
	: /* empty */ { $$ = 0; }
	| const_expression
	{ $$ = $1; }
	| const_expression COMMA parameter_list
	{ $$ = $1->prepend($3); }

hw_resource_def
	: RESOURCE IDENTIFIER '(' parameter_list ')' ';'
	  { $$ = create_resource(@2, cxx::String($2.s, $2.e), $4); }

hw_device_set_property
	: '.' IDENTIFIER '=' STRING ';'
	  { hw_device_set_property(@2, $<hw_device>0, cxx::String($2.s, $2.e), cxx::String($4.s + 1, $4.e - 1)); }
	| '.' IDENTIFIER '=' INTEGER ';'
	  { hw_device_set_property(@2, $<hw_device>0, cxx::String($2.s, $2.e), $4); }

hw_device_body_statement
	: hw_device_def
	  {
	    $$ = $<hw_device>0;
	    $$->add_child($1);
	    if ($$->parent() || $$ == _hw_root)
	      $1->plugin();
	  }
	| hw_resource_def {$$ = $<hw_device>0; $$->add_resource(check_resource_expr(@1, $1)); }
	| hw_device_set_property { $$ = $<hw_device>0; }

hw_device_body
	: /* empty */
	| hw_device_body_statement hw_device_body

hw_device_def
	: IDENTIFIER INSTANCE NEW IDENTIFIER '(' ')' '{'
	  {
	    Hw::Device *d = Hw::Device_factory::create(cxx::String($4.s, $4.e));
	    if (!d)
	      {
	        error(@4, "unknown device type '" + std::string($4.s, $4.e) + "'");
	        YYERROR;
	      }
	    d->set_name(std::string($1.s, $1.e));
	    $<hw_device>$ = d;
	  } hw_device_body '}' { $$ = $<hw_device>8; }

hw_device_ext
	: hw_device '{' { $<hw_device>$ = $1; } hw_device_body '}'

top_level_statement
	: device_def_statement
	| IDENTIFIER macro_def { $$ = 0; } /* add_macro(cxx::String($1.s, $1.e), $2); }*/
	| hw_device_ext { $$ = 0; }

top_level_list
	: /* empty */ { $$ = 0; }
	| top_level_statement top_level_list
	  { $$ = concat($1, $2); }

start	: top_level_list { _glbl_vbus = $1; }

%%

void cfg::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
  ++_errors;
  std::cerr << l << ": " << m << std::endl;
}

static
Hw::Device *
find_by_name(Hw::Device *p, cxx::String const &arg)
{
  for (Hw::Device::iterator c = p->begin(0);
       c != p->end(); ++c)
    if (arg == (*c)->name())
      return *c;

  return 0;
}


