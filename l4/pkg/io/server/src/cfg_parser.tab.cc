/* A Bison parser, made by GNU Bison 2.6.2.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */
/* "%code top" blocks.  */
/* Line 271 of lalr1.cc  */
#line 10 "cfg_parser.yy"

#include "vbus_factory.h"
#include <cstring>



/* Line 271 of lalr1.cc  */
#line 43 "cfg_parser.tab.cc"

// Take the name prefix into account.
#define yylex   cfglex

/* First part of user declarations.  */

/* Line 278 of lalr1.cc  */
#line 51 "cfg_parser.tab.cc"


#include "cfg_parser.tab.hh"

/* User implementation prologue.  */

/* Line 284 of lalr1.cc  */
#line 59 "cfg_parser.tab.cc"
/* Unqualified %code blocks.  */
/* Line 285 of lalr1.cc  */
#line 91 "cfg_parser.yy"


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


/* Line 285 of lalr1.cc  */
#line 476 "cfg_parser.tab.cc"


# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_REDUCE_PRINT(Rule)
# define YY_STACK_PRINT()

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace cfg {
/* Line 352 of lalr1.cc  */
#line 571 "cfg_parser.tab.cc"

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              /* Fall through.  */
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  Parser::Parser (class Scanner *_lexer_yyarg, Vi::Dev_factory *_vbus_factory_yyarg, Vi::Device *&_glbl_vbus_yyarg, Hw::Device *_hw_root_yyarg, int &_errors_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      _lexer (_lexer_yyarg),
      _vbus_factory (_vbus_factory_yyarg),
      _glbl_vbus (_glbl_vbus_yyarg),
      _hw_root (_hw_root_yyarg),
      _errors (_errors_yyarg)
  {
  }

  Parser::~Parser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  Parser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    std::ostream& yyo = debug_stream ();
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    switch (yytype)
      {
         default:
	  break;
      }
  }


  void
  Parser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  Parser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
  
	default:
	  break;
      }
  }

  void
  Parser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif

  inline bool
  Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    /* State.  */
    int yyn;
    int yylen = 0;
    int yystate = 0;

    /* Error handling.  */
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    semantic_type yylval;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[3];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
	YYCDEBUG << "Reading a token: ";
	yychar = yylex (&yylval, &yylloc);
      }


    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yy_table_value_is_error_ (yyn))
	  goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
	  case 2:
/* Line 661 of lalr1.cc  */
#line 507 "cfg_parser.yy"
    { (yyval.str).s = "(noname)"; (yyval.str).e = (yyval.str).s + strlen((yyval.str).s); }
    break;

  case 3:
/* Line 661 of lalr1.cc  */
#line 511 "cfg_parser.yy"
    { (yyval.param) = new Tagged_parameter(cxx::String((yysemantic_stack_[(3) - (1)].str).s, (yysemantic_stack_[(3) - (1)].str).e), (yysemantic_stack_[(3) - (3)].expr)); }
    break;

  case 4:
/* Line 661 of lalr1.cc  */
#line 512 "cfg_parser.yy"
    { (yyval.param) = new Tagged_parameter(cxx::String((yysemantic_stack_[(5) - (1)].str).s, (yysemantic_stack_[(5) - (1)].str).e), (yysemantic_stack_[(5) - (4)].expr)); }
    break;

  case 6:
/* Line 661 of lalr1.cc  */
#line 516 "cfg_parser.yy"
    { (yyval.param) = (yysemantic_stack_[(3) - (3)].param)->prepend((yysemantic_stack_[(3) - (1)].param)); }
    break;

  case 7:
/* Line 661 of lalr1.cc  */
#line 519 "cfg_parser.yy"
    { (yyval.param) = 0; }
    break;

  case 9:
/* Line 661 of lalr1.cc  */
#line 524 "cfg_parser.yy"
    {
	    Vi::Device *d = _vbus_factory->create(std::string((yysemantic_stack_[(4) - (2)].str).s, (yysemantic_stack_[(4) - (2)].str).e));
	    if (d)
	      {
	        add_children(d, (yysemantic_stack_[(4) - (4)].device));
	        d->finalize_setup();
		(yyval.expr) = new Expression(d);
	      }
	  }
    break;

  case 10:
/* Line 661 of lalr1.cc  */
#line 534 "cfg_parser.yy"
    { (yyval.expr) = wrap_hw_devices((yylocation_stack_[(5) - (3)]), (yysemantic_stack_[(5) - (3)].expr), 0); }
    break;

  case 11:
/* Line 661 of lalr1.cc  */
#line 536 "cfg_parser.yy"
    { (yyval.expr) = wrap_hw_devices((yylocation_stack_[(8) - (6)]), (yysemantic_stack_[(8) - (6)].expr), (yysemantic_stack_[(8) - (3)].param)); check_parameter_list((yylocation_stack_[(8) - (3)]), (yysemantic_stack_[(8) - (3)].param)); Tagged_parameter::del_all((yysemantic_stack_[(8) - (3)].param)); }
    break;

  case 13:
/* Line 661 of lalr1.cc  */
#line 541 "cfg_parser.yy"
    { (yyval.device) = check_dev_expr((yylocation_stack_[(1) - (1)]), (yysemantic_stack_[(1) - (1)].expr)); }
    break;

  case 14:
/* Line 661 of lalr1.cc  */
#line 543 "cfg_parser.yy"
    { (yyval.device) = check_dev_expr((yylocation_stack_[(3) - (3)]), (yysemantic_stack_[(3) - (3)].expr)); set_device_names((yyval.device), cxx::String((yysemantic_stack_[(3) - (1)].str).s, (yysemantic_stack_[(3) - (1)].str).e)); }
    break;

  case 15:
/* Line 661 of lalr1.cc  */
#line 545 "cfg_parser.yy"
    { (yyval.device) = check_dev_expr((yylocation_stack_[(5) - (5)]), (yysemantic_stack_[(5) - (5)].expr)); set_device_names((yyval.device), cxx::String((yysemantic_stack_[(5) - (1)].str).s, (yysemantic_stack_[(5) - (1)].str).e), true); }
    break;

  case 16:
/* Line 661 of lalr1.cc  */
#line 547 "cfg_parser.yy"
    { (yyval.device) = check_dev_expr((yylocation_stack_[(3) - (3)]), (yysemantic_stack_[(3) - (3)].expr)); set_device_names((yyval.device), cxx::String((yysemantic_stack_[(3) - (1)].str).s + 1, (yysemantic_stack_[(3) - (1)].str).e - 1)); }
    break;

  case 17:
/* Line 661 of lalr1.cc  */
#line 549 "cfg_parser.yy"
    { (yyval.device) = check_dev_expr((yylocation_stack_[(5) - (5)]), (yysemantic_stack_[(5) - (5)].expr)); set_device_names((yyval.device), cxx::String((yysemantic_stack_[(5) - (1)].str).s + 1, (yysemantic_stack_[(5) - (1)].str).e - 1), true); }
    break;

  case 18:
/* Line 661 of lalr1.cc  */
#line 552 "cfg_parser.yy"
    { (yyval.device) = 0; }
    break;

  case 19:
/* Line 661 of lalr1.cc  */
#line 554 "cfg_parser.yy"
    { (yyval.device) = concat((yysemantic_stack_[(2) - (1)].device), (yysemantic_stack_[(2) - (2)].device)); }
    break;

  case 20:
/* Line 661 of lalr1.cc  */
#line 557 "cfg_parser.yy"
    { (yyval.device) = 0; }
    break;

  case 21:
/* Line 661 of lalr1.cc  */
#line 558 "cfg_parser.yy"
    { (yyval.device) = (yysemantic_stack_[(3) - (2)].device); }
    break;

  case 22:
/* Line 661 of lalr1.cc  */
#line 561 "cfg_parser.yy"
    { (yyval.expr) = new Expression((yysemantic_stack_[(1) - (1)].hw_device)); }
    break;

  case 23:
/* Line 661 of lalr1.cc  */
#line 562 "cfg_parser.yy"
    { (yyval.expr) = (new Expression((yysemantic_stack_[(3) - (1)].hw_device)))->prepend((yysemantic_stack_[(3) - (3)].expr)); }
    break;

  case 24:
/* Line 661 of lalr1.cc  */
#line 564 "cfg_parser.yy"
    { (yyval.expr) = dev_list_from_dev<match_by_cid>((yysemantic_stack_[(6) - (1)].hw_device), cxx::String((yysemantic_stack_[(6) - (3)].str).s, (yysemantic_stack_[(6) - (3)].str).e), cxx::String((yysemantic_stack_[(6) - (5)].str).s + 1, (yysemantic_stack_[(6) - (5)].str).e - 1), (yylocation_stack_[(6) - (5)])); }
    break;

  case 25:
/* Line 661 of lalr1.cc  */
#line 568 "cfg_parser.yy"
    { (yyval.hw_device) = _hw_root; }
    break;

  case 26:
/* Line 661 of lalr1.cc  */
#line 570 "cfg_parser.yy"
    {
	    (yyval.hw_device) = find_by_name((yysemantic_stack_[(3) - (1)].hw_device), cxx::String((yysemantic_stack_[(3) - (3)].str).s, (yysemantic_stack_[(3) - (3)].str).e));
	    if (!(yyval.hw_device))
	      {
	        error((yylocation_stack_[(3) - (3)]), "device '" + std::string((yysemantic_stack_[(3) - (1)].hw_device)->name()) + "' does not have a member named '" + std::string((yysemantic_stack_[(3) - (3)].str).s, (yysemantic_stack_[(3) - (3)].str).e) + "'");
	        YYERROR;
	      }
	  }
    break;

  case 35:
/* Line 661 of lalr1.cc  */
#line 599 "cfg_parser.yy"
    { (yyval.expr) = new Expression((yysemantic_stack_[(1) - (1)].num)); }
    break;

  case 36:
/* Line 661 of lalr1.cc  */
#line 601 "cfg_parser.yy"
    { (yyval.expr) = new Expression((yysemantic_stack_[(3) - (1)].num), (yysemantic_stack_[(3) - (3)].num)); }
    break;

  case 37:
/* Line 661 of lalr1.cc  */
#line 603 "cfg_parser.yy"
    { (yyval.expr) = new Expression((yysemantic_stack_[(1) - (1)].str).s, (yysemantic_stack_[(1) - (1)].str).e); }
    break;

  case 38:
/* Line 661 of lalr1.cc  */
#line 606 "cfg_parser.yy"
    { (yyval.expr) = 0; }
    break;

  case 39:
/* Line 661 of lalr1.cc  */
#line 608 "cfg_parser.yy"
    { (yyval.expr) = (yysemantic_stack_[(1) - (1)].expr); }
    break;

  case 40:
/* Line 661 of lalr1.cc  */
#line 610 "cfg_parser.yy"
    { (yyval.expr) = (yysemantic_stack_[(3) - (1)].expr)->prepend((yysemantic_stack_[(3) - (3)].expr)); }
    break;

  case 41:
/* Line 661 of lalr1.cc  */
#line 614 "cfg_parser.yy"
    { (yyval.expr) = create_resource((yylocation_stack_[(6) - (2)]), cxx::String((yysemantic_stack_[(6) - (2)].str).s, (yysemantic_stack_[(6) - (2)].str).e), (yysemantic_stack_[(6) - (4)].expr)); }
    break;

  case 42:
/* Line 661 of lalr1.cc  */
#line 618 "cfg_parser.yy"
    { hw_device_set_property((yylocation_stack_[(5) - (2)]), (yysemantic_stack_[(5) - (0)].hw_device), cxx::String((yysemantic_stack_[(5) - (2)].str).s, (yysemantic_stack_[(5) - (2)].str).e), cxx::String((yysemantic_stack_[(5) - (4)].str).s + 1, (yysemantic_stack_[(5) - (4)].str).e - 1)); }
    break;

  case 43:
/* Line 661 of lalr1.cc  */
#line 620 "cfg_parser.yy"
    { hw_device_set_property((yylocation_stack_[(5) - (2)]), (yysemantic_stack_[(5) - (0)].hw_device), cxx::String((yysemantic_stack_[(5) - (2)].str).s, (yysemantic_stack_[(5) - (2)].str).e), (yysemantic_stack_[(5) - (4)].num)); }
    break;

  case 44:
/* Line 661 of lalr1.cc  */
#line 624 "cfg_parser.yy"
    {
	    (yyval.hw_device) = (yysemantic_stack_[(1) - (0)].hw_device);
	    (yyval.hw_device)->add_child((yysemantic_stack_[(1) - (1)].hw_device));
	    if ((yyval.hw_device)->parent() || (yyval.hw_device) == _hw_root)
	      (yysemantic_stack_[(1) - (1)].hw_device)->plugin();
	  }
    break;

  case 45:
/* Line 661 of lalr1.cc  */
#line 630 "cfg_parser.yy"
    {(yyval.hw_device) = (yysemantic_stack_[(1) - (0)].hw_device); (yyval.hw_device)->add_resource(check_resource_expr((yylocation_stack_[(1) - (1)]), (yysemantic_stack_[(1) - (1)].expr))); }
    break;

  case 46:
/* Line 661 of lalr1.cc  */
#line 631 "cfg_parser.yy"
    { (yyval.hw_device) = (yysemantic_stack_[(1) - (0)].hw_device); }
    break;

  case 49:
/* Line 661 of lalr1.cc  */
#line 639 "cfg_parser.yy"
    {
	    Hw::Device *d = Hw::Device_factory::create(cxx::String((yysemantic_stack_[(7) - (4)].str).s, (yysemantic_stack_[(7) - (4)].str).e));
	    if (!d)
	      {
	        error((yylocation_stack_[(7) - (4)]), "unknown device type '" + std::string((yysemantic_stack_[(7) - (4)].str).s, (yysemantic_stack_[(7) - (4)].str).e) + "'");
	        YYERROR;
	      }
	    d->set_name(std::string((yysemantic_stack_[(7) - (1)].str).s, (yysemantic_stack_[(7) - (1)].str).e));
	    (yyval.hw_device) = d;
	  }
    break;

  case 50:
/* Line 661 of lalr1.cc  */
#line 648 "cfg_parser.yy"
    { (yyval.hw_device) = (yysemantic_stack_[(10) - (8)].hw_device); }
    break;

  case 51:
/* Line 661 of lalr1.cc  */
#line 651 "cfg_parser.yy"
    { (yyval.hw_device) = (yysemantic_stack_[(2) - (1)].hw_device); }
    break;

  case 54:
/* Line 661 of lalr1.cc  */
#line 655 "cfg_parser.yy"
    { (yyval.device) = 0; }
    break;

  case 55:
/* Line 661 of lalr1.cc  */
#line 656 "cfg_parser.yy"
    { (yyval.device) = 0; }
    break;

  case 56:
/* Line 661 of lalr1.cc  */
#line 659 "cfg_parser.yy"
    { (yyval.device) = 0; }
    break;

  case 57:
/* Line 661 of lalr1.cc  */
#line 661 "cfg_parser.yy"
    { (yyval.device) = concat((yysemantic_stack_[(2) - (1)].device), (yysemantic_stack_[(2) - (2)].device)); }
    break;

  case 58:
/* Line 661 of lalr1.cc  */
#line 663 "cfg_parser.yy"
    { _glbl_vbus = (yysemantic_stack_[(1) - (1)].device); }
    break;


/* Line 661 of lalr1.cc  */
#line 1159 "cfg_parser.tab.cc"
	default:
          break;
      }
    /* User semantic actions sometimes alter yychar, and that requires
       that yytoken be updated with the new translation.  We take the
       approach of translating immediately before every use of yytoken.
       One alternative is translating here after every semantic action,
       but that translation would be missed if the semantic action
       invokes YYABORT, YYACCEPT, or YYERROR immediately after altering
       yychar.  In the case of YYABORT or YYACCEPT, an incorrect
       destructor might then be invoked immediately.  In the case of
       YYERROR, subsequent parser actions might lead to an incorrect
       destructor call or verbose syntax error message before the
       lookahead is translated.  */
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = yytranslate_ (yychar);

    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	if (yychar == yyempty_)
	  yytoken = yyempty_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[1] = yylloc;
    if (yyerrstatus_ == 3)
      {
	/* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

	if (yychar <= yyeof_)
	  {
	  /* Return failure if at end of input.  */
	  if (yychar == yyeof_)
	    YYABORT;
	  }
	else
	  {
	    yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
	    yychar = yyempty_;
	  }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;

    yyerror_range[1] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (!yy_pact_value_is_default_ (yyn))
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	YYABORT;

	yyerror_range[1] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[2] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      {
        /* Make sure we have latest lookahead translation.  See comments
           at user semantic actions for why this is necessary.  */
        yytoken = yytranslate_ (yychar);
        yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval,
                     &yylloc);
      }

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (yystate_stack_.height () != 1)
      {
	yydestruct_ ("Cleanup: popping",
		   yystos_[yystate_stack_[0]],
		   &yysemantic_stack_[0],
		   &yylocation_stack_[0]);
	yypop_ ();
      }

    return yyresult;
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (int yystate, int yytoken)
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yychar.
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            /* Stay within bounds of both yycheck and yytname.  */
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULL;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char Parser::yypact_ninf_ = -90;
  const signed char
  Parser::yypact_[] =
  {
        22,    -1,   -90,     2,     3,     4,   -90,   -90,   -90,    12,
     -90,    22,   -90,    14,    13,    36,    10,    30,    27,    25,
     -90,    30,    29,   -90,    41,   -90,   -90,    44,    32,    45,
      31,    48,    52,   -90,    42,   -90,    53,    54,    59,   -90,
      60,     1,   -90,   -90,   -90,    34,   -90,    55,    36,    61,
      37,    10,    57,    27,    56,    30,    30,    64,    66,    68,
     -90,   -90,     1,    58,   -90,     5,    34,    62,   -90,   -90,
      67,   -90,    63,    46,   -90,   -90,    36,   -90,    30,   -90,
     -90,   -90,    70,    74,    69,   -90,   -90,   -90,   -90,    71,
      76,    75,    65,    72,   -90,    73,    77,    46,    82,    47,
      78,   -90,    46,   -90,    79,    30,   -90,    80,    83,    84,
      85,   -90,   -90,   -90,   -90,    86,    89,   -90,   -90,   -90,
      87,   -90,     1,    88,   -90
  };

  /* YYDEFACT[S] -- default reduction number in state S.  Performed when
     YYTABLE doesn't specify something else to do.  Zero means the
     default is an error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
        56,     0,    25,     0,     0,     0,    12,    13,    53,     0,
      55,    56,    58,     0,     0,     0,     7,     0,    27,     0,
      54,     0,     0,    51,     0,    57,     1,     0,     0,     0,
      22,     0,     5,     8,     0,    14,    28,     0,     0,    16,
       0,    47,    26,     2,    20,    18,     9,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
      45,    46,    47,     0,    44,     0,    18,     0,    10,    23,
      26,    37,    35,    38,     3,     6,     0,    29,    31,    34,
      15,    17,     0,     0,     0,    48,    52,    19,    21,     0,
       0,    39,     0,     0,    30,     0,     0,    38,     0,     0,
       0,    36,    38,     4,     0,    31,    33,     0,     0,     0,
       0,    24,    40,    11,    32,     0,     0,    43,    42,    41,
       0,    49,    47,     0,    50
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -90,   -90,   -90,    40,   -90,   -90,   -17,   -42,    33,   -90,
     -47,     6,    51,   -90,   -12,   -90,   -90,    81,   -89,   -90,
     -90,   -90,   -60,   -90,   -90,   -90,   -90,   -90,    94,   -90
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,    28,    32,    33,    34,     6,     7,     8,    67,    46,
      29,    30,    37,    95,    96,    79,    20,    91,    92,    60,
      61,    62,    63,    64,   122,    10,    41,    11,    12,    13
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF_, syntax error.  */
  const signed char Parser::yytable_ninf_ = -1;
  const unsigned char
  Parser::yytable_[] =
  {
        35,    69,    85,    66,    39,    57,     9,    14,   107,    58,
      17,    21,    17,   112,    26,    15,    18,     9,    31,    16,
      19,    22,    19,    59,    66,     1,    27,     2,     3,    93,
       4,    23,     5,     1,    24,    36,     3,     1,    80,    81,
       3,     2,    65,    38,     5,    71,    72,    40,    44,    42,
      73,    45,    48,    49,    71,    72,   109,   110,    43,    47,
      52,    94,   123,    50,    51,    53,    55,    56,    54,    70,
      76,    68,    82,    83,    90,    78,    84,    98,    86,   103,
      89,   100,    88,    97,    99,   101,   104,   102,    94,   105,
     108,    75,   111,   114,   115,   113,   116,   106,     0,    87,
     117,   118,   119,   120,    77,    25,   121,     0,   124,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74
  };

  /* YYCHECK.  */
  const signed char
  Parser::yycheck_[] =
  {
        17,    48,    62,    45,    21,     4,     0,     8,    97,     8,
       7,     7,     7,   102,     0,    13,    13,    11,     8,    17,
      17,    17,    17,    22,    66,     3,    13,     5,     6,    76,
       8,    19,    10,     3,    22,     8,     6,     3,    55,    56,
       6,     5,     8,    18,    10,     8,     9,    18,    16,     8,
      13,    19,    21,    22,     8,     9,     9,    10,    14,    14,
      18,    78,   122,    15,    12,    12,     7,     7,    14,     8,
      13,    16,     8,     7,    11,    19,     8,     3,    20,    14,
      13,    10,    20,    13,    15,     9,    14,    12,   105,    16,
       8,    51,    14,   105,    14,    16,    13,    20,    -1,    66,
      16,    16,    16,    14,    53,    11,    19,    -1,    20,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,     3,     5,     6,     8,    10,    28,    29,    30,    34,
      48,    50,    51,    52,     8,    13,    17,     7,    13,    17,
      39,     7,    17,    19,    22,    51,     0,    13,    24,    33,
      34,     8,    25,    26,    27,    29,     8,    35,    18,    29,
      18,    49,     8,    14,    16,    19,    32,    14,    21,    22,
      15,    12,    18,    12,    14,     7,     7,     4,     8,    22,
      42,    43,    44,    45,    46,     8,    30,    31,    16,    33,
       8,     8,     9,    13,    40,    26,    13,    35,    19,    38,
      29,    29,     8,     7,     8,    45,    20,    31,    20,    13,
      11,    40,    41,    33,    29,    36,    37,    13,     3,    15,
      10,     9,    12,    14,    14,    16,    20,    41,     8,     9,
      10,    14,    41,    16,    37,    14,    13,    16,    16,    16,
      14,    19,    47,    45,    20
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  Parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,    40,    41,    61,    59,    91,    93,   123,
     125,    44,    46
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  Parser::yyr1_[] =
  {
         0,    23,    24,    25,    25,    26,    26,    27,    27,    28,
      28,    28,    29,    30,    30,    30,    30,    30,    31,    31,
      32,    32,    33,    33,    33,    34,    34,    35,    35,    35,
      36,    37,    37,    38,    39,    40,    40,    40,    41,    41,
      41,    42,    43,    43,    44,    44,    44,    45,    45,    47,
      46,    49,    48,    50,    50,    50,    51,    51,    52
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     2,     3,     5,     1,     3,     0,     1,     4,
       5,     8,     1,     1,     3,     5,     3,     5,     0,     2,
       1,     3,     1,     3,     6,     1,     3,     0,     1,     3,
       1,     0,     3,     3,     4,     1,     3,     1,     0,     1,
       3,     6,     5,     5,     1,     1,     1,     0,     2,     0,
      10,     0,     5,     1,     2,     1,     0,     2,     1
  };


  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const Parser::yytname_[] =
  {
    "\"end of file\"", "error", "$undefined", "\"new operator\"",
  "\"new-res operator\"", "\"hw-root operator\"", "\"wrap operator\"",
  "\"instantiation oprtator '=>'\"", "\"identifier\"", "\"integer\"",
  "\"string\"", "\"range operator '..'\"", "\"comma\"", "'('", "')'",
  "'='", "';'", "'['", "']'", "'{'", "'}'", "','", "'.'", "$accept",
  "param_list", "tagged_parameter", "tagged_parameter_list_ne",
  "tagged_parameter_list", "device_def_expr", "expression",
  "device_def_statement", "statement_list", "device_body",
  "hw_device_list", "hw_device", "macro_parameter_list", "macro_statement",
  "macro_statement_list", "macro_body", "macro_def", "const_expression",
  "parameter_list", "hw_resource_def", "hw_device_set_property",
  "hw_device_body_statement", "hw_device_body", "hw_device_def", "@1",
  "hw_device_ext", "@2", "top_level_statement", "top_level_list", "start", YY_NULL
  };

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const Parser::rhs_number_type
  Parser::yyrhs_[] =
  {
        52,     0,    -1,    13,    14,    -1,     8,    15,    40,    -1,
       8,    15,    13,    41,    14,    -1,    25,    -1,    25,    12,
      26,    -1,    -1,    26,    -1,     3,     8,    24,    32,    -1,
       6,    13,    33,    14,    16,    -1,     6,    17,    27,    18,
      13,    33,    14,    16,    -1,    28,    -1,    29,    -1,     8,
       7,    29,    -1,     8,    17,    18,     7,    29,    -1,    10,
       7,    29,    -1,    10,    17,    18,     7,    29,    -1,    -1,
      30,    31,    -1,    16,    -1,    19,    31,    20,    -1,    34,
      -1,    34,    21,    33,    -1,    34,    22,     8,    13,    10,
      14,    -1,     5,    -1,    34,    22,     8,    -1,    -1,     8,
      -1,     8,    12,    35,    -1,    29,    -1,    -1,    36,    16,
      37,    -1,    19,    37,    20,    -1,    13,    35,    14,    38,
      -1,     9,    -1,     9,    11,     9,    -1,     8,    -1,    -1,
      40,    -1,    40,    12,    41,    -1,     4,     8,    13,    41,
      14,    16,    -1,    22,     8,    15,    10,    16,    -1,    22,
       8,    15,     9,    16,    -1,    46,    -1,    42,    -1,    43,
      -1,    -1,    44,    45,    -1,    -1,     8,     7,     3,     8,
      13,    14,    19,    47,    45,    20,    -1,    -1,    34,    19,
      49,    45,    20,    -1,    30,    -1,     8,    39,    -1,    48,
      -1,    -1,    50,    51,    -1,    51,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  Parser::yyprhs_[] =
  {
         0,     0,     3,     6,    10,    16,    18,    22,    23,    25,
      30,    36,    45,    47,    49,    53,    59,    63,    69,    70,
      73,    75,    79,    81,    85,    92,    94,    98,    99,   101,
     105,   107,   108,   112,   116,   121,   123,   127,   129,   130,
     132,   136,   143,   149,   155,   157,   159,   161,   162,   165,
     166,   177,   178,   184,   186,   189,   191,   192,   195
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  Parser::yyrline_[] =
  {
         0,   507,   507,   511,   512,   515,   516,   519,   520,   523,
     533,   535,   539,   541,   542,   544,   546,   548,   552,   553,
     557,   558,   561,   562,   563,   568,   569,   579,   581,   582,
     585,   587,   589,   592,   595,   598,   600,   602,   606,   607,
     609,   613,   617,   619,   623,   630,   631,   633,   635,   639,
     638,   651,   651,   654,   655,   656,   659,   660,   663
  };

  // Print the state stack on the debug stream.
  void
  Parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  Parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  Parser::token_number_type
  Parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      13,    14,     2,     2,    21,     2,    22,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    16,
       2,    15,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    17,     2,    18,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    19,     2,    20,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int Parser::yyeof_ = 0;
  const int Parser::yylast_ = 131;
  const int Parser::yynnts_ = 30;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 26;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 23;

  const unsigned int Parser::yyuser_token_number_max_ = 267;
  const Parser::token_number_type Parser::yyundef_token_ = 2;


} // cfg
/* Line 1106 of lalr1.cc  */
#line 1749 "cfg_parser.tab.cc"
/* Line 1107 of lalr1.cc  */
#line 665 "cfg_parser.yy"


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


