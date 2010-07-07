/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "cfg_parser.tab.hh"

namespace cfg {

class Scanner
{
public:
  typedef cfg::Parser::token Token;
  typedef cfg::Parser::token::yytokentype Token_type;

  Token_type lex(cfg::Parser::semantic_type*, cfg::location*);

  explicit Scanner(char const *zts);
  explicit Scanner(char const *s, char const *e, const char *filename);
  ~Scanner();

private:
  void init();

  int cs, act;
  char const *ts, *te;
  char const *p, *pe;

  char const *s, *e;
  char const *_filename;
};

}

#define YY_EXTRA_TYPE cfg::Scanner *
