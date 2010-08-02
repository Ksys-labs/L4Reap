
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008 Free Software
   Foundation, Inc.
   
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

/* Line 304 of lalr1.cc  */
#line 10 "cfg_parser.yy"

#include "vbus_factory.h"
#include <cstring>




/* Line 304 of lalr1.cc  */
#line 47 "cfg_parser.tab.cc"

// Take the name prefix into account.
#define yylex   cfglex

/* First part of user declarations.  */


/* Line 311 of lalr1.cc  */
#line 56 "cfg_parser.tab.cc"


#include "cfg_parser.tab.hh"

/* User implementation prologue.  */


/* Line 317 of lalr1.cc  */
#line 65 "cfg_parser.tab.cc"
/* Unqualified %code blocks.  */

/* Line 318 of lalr1.cc  */
#line 109 "cfg_parser.yy"


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




/* Line 318 of lalr1.cc  */
#line 372 "cfg_parser.tab.cc"

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

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


/* Line 380 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

namespace cfg {

/* Line 380 of lalr1.cc  */
#line 441 "cfg_parser.tab.cc"
#if YYERROR_VERBOSE

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

#endif

  /// Build a parser object.
  Parser::Parser (class Scanner *_lexer_yyarg, Vi::Dev_factory *_vbus_factory_yyarg, Vi::Device *&_glbl_vbus_yyarg, Hw::Device *_hw_root_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      _lexer (_lexer_yyarg),
      _vbus_factory (_vbus_factory_yyarg),
      _glbl_vbus (_glbl_vbus_yyarg),
      _hw_root (_hw_root_yyarg)
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
    location_type yyerror_range[2];

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
    if (yyn == yypact_ninf_)
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
	if (yyn == 0 || yyn == yytable_ninf_)
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

/* Line 678 of lalr1.cc  */
#line 413 "cfg_parser.yy"
    { (yyval.str).s = "(noname)"; (yyval.str).e = (yyval.str).s + strlen((yyval.str).s); }
    break;

  case 3:

/* Line 678 of lalr1.cc  */
#line 418 "cfg_parser.yy"
    {
	    (yyval.device) = _vbus_factory->create(std::string((yysemantic_stack_[(4) - (2)].str).s, (yysemantic_stack_[(4) - (2)].str).e));
	    add_children((yyval.device), (yysemantic_stack_[(4) - (4)].device));
	    (yyval.device)->finalize_setup();
	  }
    break;

  case 4:

/* Line 678 of lalr1.cc  */
#line 424 "cfg_parser.yy"
    { (yyval.device) = wrap_hw_devices((yysemantic_stack_[(5) - (3)].dev_list)); }
    break;

  case 5:

/* Line 678 of lalr1.cc  */
#line 427 "cfg_parser.yy"
    { (yyval.device) = (yysemantic_stack_[(1) - (1)].device); }
    break;

  case 6:

/* Line 678 of lalr1.cc  */
#line 429 "cfg_parser.yy"
    { (yyval.device) = (yysemantic_stack_[(3) - (3)].device); set_device_names((yysemantic_stack_[(3) - (3)].device), cxx::String((yysemantic_stack_[(3) - (1)].str).s, (yysemantic_stack_[(3) - (1)].str).e)); }
    break;

  case 7:

/* Line 678 of lalr1.cc  */
#line 431 "cfg_parser.yy"
    { (yyval.device) = (yysemantic_stack_[(5) - (5)].device); set_device_names((yysemantic_stack_[(5) - (5)].device), cxx::String((yysemantic_stack_[(5) - (1)].str).s, (yysemantic_stack_[(5) - (1)].str).e), true); }
    break;

  case 9:

/* Line 678 of lalr1.cc  */
#line 436 "cfg_parser.yy"
    { (yyval.device) = 0; }
    break;

  case 10:

/* Line 678 of lalr1.cc  */
#line 438 "cfg_parser.yy"
    { (yyval.device) = concat((yysemantic_stack_[(2) - (1)].device), (yysemantic_stack_[(2) - (2)].device)); }
    break;

  case 11:

/* Line 678 of lalr1.cc  */
#line 441 "cfg_parser.yy"
    { (yyval.device) = 0; }
    break;

  case 12:

/* Line 678 of lalr1.cc  */
#line 442 "cfg_parser.yy"
    { (yyval.device) = (yysemantic_stack_[(3) - (2)].device); }
    break;

  case 13:

/* Line 678 of lalr1.cc  */
#line 445 "cfg_parser.yy"
    { (yyval.dev_list) = new Dev_list(); (yyval.dev_list)->push_back((yysemantic_stack_[(1) - (1)].hw_device)); }
    break;

  case 14:

/* Line 678 of lalr1.cc  */
#line 446 "cfg_parser.yy"
    { (yyval.dev_list) = (yysemantic_stack_[(3) - (3)].dev_list); (yyval.dev_list)->push_back((yysemantic_stack_[(3) - (1)].hw_device)); }
    break;

  case 15:

/* Line 678 of lalr1.cc  */
#line 448 "cfg_parser.yy"
    { (yyval.dev_list) = dev_list_from_dev<match_by_cid>((yysemantic_stack_[(6) - (1)].hw_device), cxx::String((yysemantic_stack_[(6) - (3)].str).s, (yysemantic_stack_[(6) - (3)].str).e), cxx::String((yysemantic_stack_[(6) - (5)].str).s + 1, (yysemantic_stack_[(6) - (5)].str).e - 1), (yylocation_stack_[(6) - (5)])); }
    break;

  case 16:

/* Line 678 of lalr1.cc  */
#line 452 "cfg_parser.yy"
    { (yyval.hw_device) = _hw_root; }
    break;

  case 17:

/* Line 678 of lalr1.cc  */
#line 454 "cfg_parser.yy"
    {
	    (yyval.hw_device) = find_by_name((yysemantic_stack_[(3) - (1)].hw_device), cxx::String((yysemantic_stack_[(3) - (3)].str).s, (yysemantic_stack_[(3) - (3)].str).e));
	    if (!(yyval.hw_device))
	      {
	        error((yylocation_stack_[(3) - (3)]), "device '" + std::string((yysemantic_stack_[(3) - (1)].hw_device)->name()) + "' does not have a member named '" + std::string((yysemantic_stack_[(3) - (3)].str).s, (yysemantic_stack_[(3) - (3)].str).e) + "'");
	        YYERROR;
	      }
	  }
    break;

  case 18:

/* Line 678 of lalr1.cc  */
#line 465 "cfg_parser.yy"
    { (yyval.const_expression).type = 0; (yyval.const_expression).val.num = (yysemantic_stack_[(1) - (1)].num); }
    break;

  case 19:

/* Line 678 of lalr1.cc  */
#line 467 "cfg_parser.yy"
    { (yyval.const_expression).type = 1; (yyval.const_expression).val.range.s = (yysemantic_stack_[(3) - (1)].num); (yyval.const_expression).val.range.e = (yysemantic_stack_[(3) - (3)].num); }
    break;

  case 20:

/* Line 678 of lalr1.cc  */
#line 469 "cfg_parser.yy"
    { (yyval.const_expression).type = 2; (yyval.const_expression).val.str.s = (yysemantic_stack_[(1) - (1)].str).s; (yyval.const_expression).val.str.e = (yysemantic_stack_[(1) - (1)].str).e; }
    break;

  case 21:

/* Line 678 of lalr1.cc  */
#line 472 "cfg_parser.yy"
    { (yyval.const_expr_list) = 0; }
    break;

  case 22:

/* Line 678 of lalr1.cc  */
#line 474 "cfg_parser.yy"
    { (yyval.const_expr_list) = new Const_expression_list(); (yyval.const_expr_list)->push_back((yysemantic_stack_[(1) - (1)].const_expression)); }
    break;

  case 23:

/* Line 678 of lalr1.cc  */
#line 476 "cfg_parser.yy"
    { (yyval.const_expr_list) = (yysemantic_stack_[(3) - (1)].const_expr_list); (yyval.const_expr_list)->push_back((yysemantic_stack_[(3) - (3)].const_expression)); }
    break;

  case 24:

/* Line 678 of lalr1.cc  */
#line 480 "cfg_parser.yy"
    { (yyval.resource) = create_resource((yylocation_stack_[(6) - (2)]), cxx::String((yysemantic_stack_[(6) - (2)].str).s, (yysemantic_stack_[(6) - (2)].str).e), (yysemantic_stack_[(6) - (4)].const_expr_list)); }
    break;

  case 25:

/* Line 678 of lalr1.cc  */
#line 484 "cfg_parser.yy"
    { hw_device_set_property((yylocation_stack_[(5) - (2)]), (yysemantic_stack_[(5) - (0)].hw_device), cxx::String((yysemantic_stack_[(5) - (2)].str).s, (yysemantic_stack_[(5) - (2)].str).e), cxx::String((yysemantic_stack_[(5) - (4)].str).s + 1, (yysemantic_stack_[(5) - (4)].str).e - 1)); }
    break;

  case 26:

/* Line 678 of lalr1.cc  */
#line 486 "cfg_parser.yy"
    { hw_device_set_property((yylocation_stack_[(5) - (2)]), (yysemantic_stack_[(5) - (0)].hw_device), cxx::String((yysemantic_stack_[(5) - (2)].str).s, (yysemantic_stack_[(5) - (2)].str).e), (yysemantic_stack_[(5) - (4)].num)); }
    break;

  case 27:

/* Line 678 of lalr1.cc  */
#line 490 "cfg_parser.yy"
    {
	    (yyval.hw_device) = (yysemantic_stack_[(1) - (0)].hw_device);
	    (yyval.hw_device)->add_child((yysemantic_stack_[(1) - (1)].hw_device));
	    if ((yyval.hw_device)->parent() || (yyval.hw_device) == _hw_root)
	      (yysemantic_stack_[(1) - (1)].hw_device)->plugin();
	  }
    break;

  case 28:

/* Line 678 of lalr1.cc  */
#line 496 "cfg_parser.yy"
    {(yyval.hw_device) = (yysemantic_stack_[(1) - (0)].hw_device); (yyval.hw_device)->add_resource((yysemantic_stack_[(1) - (1)].resource)); }
    break;

  case 29:

/* Line 678 of lalr1.cc  */
#line 497 "cfg_parser.yy"
    { (yyval.hw_device) = (yysemantic_stack_[(1) - (0)].hw_device); }
    break;

  case 32:

/* Line 678 of lalr1.cc  */
#line 505 "cfg_parser.yy"
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

  case 33:

/* Line 678 of lalr1.cc  */
#line 514 "cfg_parser.yy"
    { (yyval.hw_device) = (yysemantic_stack_[(10) - (8)].hw_device); }
    break;

  case 34:

/* Line 678 of lalr1.cc  */
#line 517 "cfg_parser.yy"
    { (yyval.hw_device) = (yysemantic_stack_[(2) - (1)].hw_device); }
    break;

  case 37:

/* Line 678 of lalr1.cc  */
#line 521 "cfg_parser.yy"
    { (yyval.device) = 0; }
    break;

  case 38:

/* Line 678 of lalr1.cc  */
#line 524 "cfg_parser.yy"
    { (yyval.device) = 0; }
    break;

  case 39:

/* Line 678 of lalr1.cc  */
#line 526 "cfg_parser.yy"
    { (yyval.device) = concat((yysemantic_stack_[(2) - (1)].device), (yysemantic_stack_[(2) - (2)].device)); }
    break;

  case 40:

/* Line 678 of lalr1.cc  */
#line 528 "cfg_parser.yy"
    { _glbl_vbus = (yysemantic_stack_[(1) - (1)].device); }
    break;



/* Line 678 of lalr1.cc  */
#line 998 "cfg_parser.tab.cc"
	default:
          break;
      }
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
    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[0] = yylloc;
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

    yyerror_range[0] = yylocation_stack_[yylen - 1];
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
	if (yyn != yypact_ninf_)
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

	yyerror_range[0] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[1] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
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
      yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval, &yylloc);

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
  Parser::yysyntax_error_ (int yystate, int tok)
  {
    std::string res;
    YYUSE (yystate);
#if YYERROR_VERBOSE
    int yyn = yypact_[yystate];
    if (yypact_ninf_ < yyn && yyn <= yylast_)
      {
	/* Start YYX at -YYN if negative to avoid negative indexes in
	   YYCHECK.  */
	int yyxbegin = yyn < 0 ? -yyn : 0;

	/* Stay within bounds of both yycheck and yytname.  */
	int yychecklim = yylast_ - yyn + 1;
	int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
	int count = 0;
	for (int x = yyxbegin; x < yyxend; ++x)
	  if (yycheck_[x + yyn] == x && x != yyterror_)
	    ++count;

	// FIXME: This method of building the message is not compatible
	// with internationalization.  It should work like yacc.c does it.
	// That is, first build a string that looks like this:
	// "syntax error, unexpected %s or %s or %s"
	// Then, invoke YY_ on this string.
	// Finally, use the string as a format to output
	// yytname_[tok], etc.
	// Until this gets fixed, this message appears in English only.
	res = "syntax error, unexpected ";
	res += yytnamerr_ (yytname_[tok]);
	if (count < 5)
	  {
	    count = 0;
	    for (int x = yyxbegin; x < yyxend; ++x)
	      if (yycheck_[x + yyn] == x && x != yyterror_)
		{
		  res += (!count++) ? ", expecting " : " or ";
		  res += yytnamerr_ (yytname_[x]);
		}
	  }
      }
    else
#endif
      res = YY_("syntax error");
    return res;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char Parser::yypact_ninf_ = -43;
  const signed char
  Parser::yypact_[] =
  {
         5,    -3,   -43,     2,     0,   -43,   -43,   -43,    -1,   -43,
       5,   -43,    30,    13,    32,    18,    21,   -43,    31,   -43,
     -43,    27,    10,    28,    11,   -43,    36,    -2,   -43,   -43,
     -43,     6,   -43,    29,    32,    37,    18,    38,    40,    41,
     -43,   -43,    -2,    33,   -43,     6,    34,   -43,   -43,    35,
     -43,    42,    47,    39,   -43,   -43,   -43,   -43,    44,    25,
      43,    26,    45,   -43,    46,   -43,    15,    49,    48,    50,
     -43,    51,    25,    52,    54,   -43,   -43,   -43,   -43,   -43,
      53,   -43,    -2,    55,   -43
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
        38,     0,    16,     0,     0,     5,     8,    36,     0,    37,
      38,    40,     0,     0,     0,     0,     0,    34,     0,    39,
       1,     0,     0,     0,    13,     6,     0,    30,    17,     2,
      11,     9,     3,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    30,     0,    27,     9,     0,     4,    14,    17,
       7,     0,     0,     0,    31,    35,    10,    12,     0,    21,
       0,     0,     0,    20,    18,    22,     0,     0,     0,     0,
      15,     0,     0,     0,     0,    26,    25,    19,    23,    24,
       0,    32,    30,     0,    33
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -43,   -43,   -14,   -43,   -27,    19,   -43,    22,   -11,    -6,
     -43,   -43,   -43,   -43,   -42,   -43,   -43,   -43,   -43,   -43,
      59,   -43
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,    22,     5,     6,     7,    46,    32,    23,     8,    65,
      66,    40,    41,    42,    43,    44,    82,     9,    27,    10,
      11,    12
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char Parser::yytable_ninf_ = -1;
  const unsigned char
  Parser::yytable_[] =
  {
        54,    25,    37,    24,    45,    13,    38,    15,     1,     1,
       2,     3,     3,     4,     4,    14,    16,    17,    45,    39,
      18,     1,    50,    24,     3,    30,    21,    72,    31,    73,
      20,    34,    35,    63,    64,    68,    69,     2,    26,    28,
      83,    29,    33,    36,    47,    49,    51,    52,    58,    53,
      60,    67,    55,    57,    62,    59,    48,    71,     0,    70,
      77,    61,    74,    75,    56,    76,    78,    79,    80,    19,
       0,    81,     0,     0,    84
  };

  /* YYCHECK.  */
  const signed char
  Parser::yycheck_[] =
  {
        42,    15,     4,    14,    31,     8,     8,     7,     3,     3,
       5,     6,     6,     8,     8,    13,    16,    18,    45,    21,
      21,     3,    36,    34,     6,    15,    13,    12,    18,    14,
       0,    20,    21,     8,     9,     9,    10,     5,    17,     8,
      82,    14,    14,     7,    15,     8,     8,     7,    13,     8,
       3,     8,    19,    19,    10,    13,    34,    11,    -1,    14,
       9,    22,    13,    15,    45,    15,    72,    15,    14,    10,
      -1,    18,    -1,    -1,    19
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,     3,     5,     6,     8,    25,    26,    27,    31,    40,
      42,    43,    44,     8,    13,     7,    16,    18,    21,    43,
       0,    13,    24,    30,    31,    25,    17,    41,     8,    14,
      15,    18,    29,    14,    20,    21,     7,     4,     8,    21,
      34,    35,    36,    37,    38,    27,    28,    15,    30,     8,
      25,     8,     7,     8,    37,    19,    28,    19,    13,    13,
       3,    22,    10,     8,     9,    32,    33,     8,     9,    10,
      14,    11,    12,    14,    13,    15,    15,     9,    32,    15,
      14,    18,    39,    37,    19
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  Parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,    40,    41,    59,    91,    93,   123,   125,
      44,    46,    61
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  Parser::yyr1_[] =
  {
         0,    23,    24,    25,    25,    26,    26,    26,    27,    28,
      28,    29,    29,    30,    30,    30,    31,    31,    32,    32,
      32,    33,    33,    33,    34,    35,    35,    36,    36,    36,
      37,    37,    39,    38,    41,    40,    42,    42,    43,    43,
      44
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     2,     4,     5,     1,     3,     5,     1,     0,
       2,     1,     3,     1,     3,     6,     1,     3,     1,     3,
       1,     0,     1,     3,     6,     5,     5,     1,     1,     1,
       0,     2,     0,    10,     0,     5,     1,     1,     0,     2,
       1
  };

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const Parser::yytname_[] =
  {
    "\"end of file\"", "error", "$undefined", "\"new operator\"",
  "\"new-res operator\"", "\"hw-root operator\"", "\"wrap operator\"",
  "\"instantiation oprtator '=>'\"", "\"identifier\"", "\"integer\"",
  "\"string\"", "\"range operator '..'\"", "\"comma\"", "'('", "')'",
  "';'", "'['", "']'", "'{'", "'}'", "','", "'.'", "'='", "$accept",
  "param_list", "device_def", "expression", "expression_statement",
  "statement_list", "device_body", "hw_device_list", "hw_device",
  "const_expression", "parameter_list", "hw_resource_def",
  "hw_device_set_property", "hw_device_body_statement", "hw_device_body",
  "hw_device_def", "@1", "hw_device_ext", "@2", "top_level_statement",
  "top_level_list", "start", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const Parser::rhs_number_type
  Parser::yyrhs_[] =
  {
        44,     0,    -1,    13,    14,    -1,     3,     8,    24,    29,
      -1,     6,    13,    30,    14,    15,    -1,    25,    -1,     8,
       7,    25,    -1,     8,    16,    17,     7,    25,    -1,    26,
      -1,    -1,    27,    28,    -1,    15,    -1,    18,    28,    19,
      -1,    31,    -1,    31,    20,    30,    -1,    31,    21,     8,
      13,    10,    14,    -1,     5,    -1,    31,    21,     8,    -1,
       9,    -1,     9,    11,     9,    -1,     8,    -1,    -1,    32,
      -1,    33,    12,    32,    -1,     4,     8,    13,    33,    14,
      15,    -1,    21,     8,    22,    10,    15,    -1,    21,     8,
      22,     9,    15,    -1,    38,    -1,    34,    -1,    35,    -1,
      -1,    36,    37,    -1,    -1,     8,     7,     3,     8,    13,
      14,    18,    39,    37,    19,    -1,    -1,    31,    18,    41,
      37,    19,    -1,    27,    -1,    40,    -1,    -1,    42,    43,
      -1,    43,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  Parser::yyprhs_[] =
  {
         0,     0,     3,     6,    11,    17,    19,    23,    29,    31,
      32,    35,    37,    41,    43,    47,    54,    56,    60,    62,
      66,    68,    69,    71,    75,    82,    88,    94,    96,    98,
     100,   101,   104,   105,   116,   117,   123,   125,   127,   128,
     131
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  Parser::yyrline_[] =
  {
         0,   413,   413,   417,   423,   427,   428,   430,   433,   436,
     437,   441,   442,   445,   446,   447,   452,   453,   464,   466,
     468,   472,   473,   475,   479,   483,   485,   489,   496,   497,
     499,   501,   505,   504,   517,   517,   520,   521,   524,   525,
     528
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
      13,    14,     2,     2,    20,     2,    21,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    15,
       2,    22,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    16,     2,    17,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    18,     2,    19,     2,     2,     2,     2,
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
  const int Parser::yylast_ = 74;
  const int Parser::yynnts_ = 22;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 20;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 23;

  const unsigned int Parser::yyuser_token_number_max_ = 267;
  const Parser::token_number_type Parser::yyundef_token_ = 2;


/* Line 1054 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

} // cfg

/* Line 1054 of lalr1.cc  */
#line 1488 "cfg_parser.tab.cc"


/* Line 1056 of lalr1.cc  */
#line 530 "cfg_parser.yy"


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



