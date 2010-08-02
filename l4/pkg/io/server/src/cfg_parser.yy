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
#include "hw_device.h"
#include <vector>
#include <l4/cxx/string>
#include "vbus_factory.h"

struct Const_expression
{
  unsigned type;
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
  } val;
};


typedef std::vector<Hw::Device*> Dev_list;
typedef std::vector<Const_expression> Const_expression_list;

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

%error-verbose

%union {
    l4_uint64_t num;
    struct {
      char const *s;
      char const *e;
    } str;
    Vi::Device *device;
    Hw::Device *hw_device;
    Dev_list *dev_list;
    Adr_resource *resource;
    Const_expression const_expression;
    Const_expression_list *const_expr_list;
}

%token		END	0	"end of file"
//%token		EOL		"end of line"
%token		NEW		"new operator"
%token		RESOURCE	"new-res operator"
%token		HWROOT		"hw-root operator"
%token		WRAP		"wrap operator"
%token		INSTANCE	"instantiation oprtator '=>'"
%token<str>	IDENTIFIER	"identifier"
%token<num>	INTEGER		"integer"
%token<str>	STRING		"string"
%token		RANGE		"range operator '..'"
%token		COMMA		"comma"

%type<hw_device> hw_device
%type<dev_list> hw_device_list
%type<device> device_def device_body expression expression_statement
%type<device> statement_list top_level_list top_level_statement
%type<str>    param_list
%type<resource> hw_resource_def
%type<hw_device> hw_device_def hw_device_body_statement
%type<const_expression> const_expression
%type<const_expr_list> parameter_list

%code {

#include <algorithm>
#include "cfg_scanner.h"
#include "hw_device.h"

#undef yylex
#define yylex _lexer->lex

static
void wrap(Device *h, Vi::Device **first)
{
  Hw::Device *hd = dynamic_cast<Hw::Device *>(h);

  Vi::Device *vd = Vi::Dev_factory::create(hd);
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
  Dev_list *l;

  match_by_cid(cxx::String const &cid, Dev_list *l) : cid(cid), l(l)
  {}

  void operator () (Hw::Device *dev) const
  {
    Hw::Device *hd = dev;
    cxx::String h = cid;

    for (cxx::String::Index n = h.find(","); !h.empty();
	 h = h.substr(n+1), n = h.find(","))
      if (hd->match_cid(h.head(n)))
        {
	  l->push_back(hd);
	  return;
	}
  }
};

struct match_by_name
{
  enum { DEPTH = 0 };
  cxx::String cid;
  Dev_list *l;

  match_by_name(cxx::String const &cid, Dev_list *l) : cid(cid), l(l)
  {}

  void operator () (Hw::Device *dev) const
  {
    Hw::Device *hd = dev;
    if (cid == hd->name())
      l->push_back(hd);
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
Dev_list *
dev_list_from_dev(Hw::Device *in, cxx::String const & /*op*/,
                  cxx::String const &arg, LOC const &l)
{
  Dev_list *nl = new Dev_list();

  std::for_each(in->begin(Match::DEPTH), in->end(),
                Match(arg, nl));
  if (nl->empty())
    std::cerr << l << ": warning: could not find '" << arg << "'"
              << std::endl;

  return nl;
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
Vi::Device *
wrap_hw_devices(Dev_list *hw_devs)
{
  Vi::Device *vd = 0;
  for (Dev_list::iterator i = hw_devs->begin(); i != hw_devs->end(); ++i)
    wrap(*i, &vd);

  delete hw_devs;
  return vd;
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
std::ostream &operator << (std::ostream &s, cxx::String const &str)
{
  s.write(str.start(), str.len());
  return s;
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
Adr_resource *
create_resource(cfg::Parser::location_type const &l, cxx::String const &type,
                Const_expression_list *args)
{

  unsigned flags = 0;

  if (type == "Io")
    flags = Adr_resource::Io_res;
  else if (type == "Irq")
    flags = Adr_resource::Irq_res;
  else if (type == "Mmio")
    flags = Adr_resource::Mmio_res;
  else if (type == "Mmio_ram")
    flags = Adr_resource::Mmio_res | Adr_resource::Mmio_data_space;
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
    case Adr_resource::Io_res:
    case Adr_resource::Irq_res:
    case Adr_resource::Mmio_res:
      if (args->size() < 1)
        {
	  std::cerr << l << ": too few arguments to constructor of resource '" << type << "'" << std::endl;
	  delete args;
	  return 0;
	}
      if (args->size() > 2)
        {
	  std::cerr << l << ": too many arguments to constructor of resource '" << type << "'" << std::endl;
	  delete args;
	  return 0;
	}
      switch ((*args)[0].type)
        {
	case 0:
	  s = (*args)[0].val.num;
	  e = (*args)[0].val.num;
	  break;
	case 1:
	  s = (*args)[0].val.range.s;
	  e = (*args)[0].val.range.e;
	  break;
	default:
	  s = 0;
	  e = 0;
	  break;
	}

      if (args->size() == 2)
        {
	  if ((*args)[1].type != 0)
	    {
	      std::cerr << l << ": argument 2 must by of type integer in constructor of resource '" << type << "'" << std::endl;
	      delete args;
	      return 0;
	    }
	  if (s > e)
	    {
	      std::cerr << l << ": start of range bigger than end" << std::endl;
	      delete args;
	      return 0;
	    }
          flags |= (*args)[1].val.num & 0x3fff00;
	}

      delete args;

      return new Adr_resource(flags, s , e);

    case (Adr_resource::Mmio_res |  Adr_resource::Mmio_data_space):
      if (args->size() < 2)
        {
	  std::cerr << l << ": too few arguments to constructor of resource '" << type << "'" << std::endl;
	  delete args;
	  return 0;
	}
      if (args->size() > 2)
        {
	  std::cerr << l << ": too many arguments to constructor of resource '" << type << "'" << std::endl;
	  delete args;
	  return 0;
	}
      s = (*args)[0].val.num;
      e = (*args)[1].val.num;
      delete args;

      return new Mmio_data_space(s, e);
    }

  delete args;
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

}

%%

param_list
/*	: *empty* { $$.s = "(noname)"; $$.e = $$.s + strlen($$.s); } */
	: '(' ')' { $$.s = "(noname)"; $$.e = $$.s + strlen($$.s); }
/*	| '(' STRING ')' { $$=$2; ++$$.s; --$$.e; } */

device_def
	: NEW IDENTIFIER param_list device_body
	  {
	    $$ = _vbus_factory->create(std::string($2.s, $2.e));
	    add_children($$, $4);
	    $$->finalize_setup();
	  }
	| WRAP '(' hw_device_list ')' ';'
	  { $$ = wrap_hw_devices($3); }

expression
	: device_def { $$ = $1; }
	| IDENTIFIER INSTANCE device_def
	  { $$ = $3; set_device_names($3, cxx::String($1.s, $1.e)); }
	| IDENTIFIER '[' ']' INSTANCE device_def
	  { $$ = $5; set_device_names($5, cxx::String($1.s, $1.e), true); }

expression_statement : expression

statement_list
	: /*empty*/ { $$ = 0; }
	| expression_statement statement_list
	  { $$ = concat($1, $2); }

device_body
	: ';' { $$ = 0; }
	| '{' statement_list '}' { $$ = $2; }

hw_device_list
	: hw_device { $$ = new Dev_list(); $$->push_back($1); }
	| hw_device ',' hw_device_list { $$ = $3; $$->push_back($1); }
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

const_expression
	: INTEGER
	{ $$.type = 0; $$.val.num = $1; }
	| INTEGER RANGE INTEGER
	{ $$.type = 1; $$.val.range.s = $1; $$.val.range.e = $3; }
	| IDENTIFIER
	{ $$.type = 2; $$.val.str.s = $1.s; $$.val.str.e = $1.e; }

parameter_list
	: /* empty */ { $$ = 0; }
	| const_expression
	{ $$ = new Const_expression_list(); $$->push_back($1); }
	| parameter_list COMMA const_expression
	{ $$ = $1; $$->push_back($3); }

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
	| hw_resource_def {$$ = $<hw_device>0; $$->add_resource($1); }
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
	: expression_statement
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


