/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
/*
 * A mini C-like language scanner.
 */

#include "cfg_scanner.h"

%%{
	machine cfg_lang;

	newline = '\n' @{ yylloc->lines(); yylloc->step(); };
	wspace  = [ \t\r]+;

	main := |*

	# Alpha numberic characters or underscore.
	alnum_u = alnum | '_';

	# Alpha charactres or underscore.
	alpha_u = alpha | '_';

	# Describe both c style comments and c++ style comments. The
	# priority bump on tne terminator of the comments brings us
	# out of the extend* which matches everything.
	'//' [^\n]* newline;
	'#'  [^\n]* newline;

	'=>' { tt = Token::INSTANCE; fbreak; };
	'..' { tt = Token::RANGE; fbreak; };
	','  { tt = Token::COMMA; fbreak; };

	# Symbols. Upon entering clear the buffer. On all transitions
	# buffer a character. Upon leaving dump the symbol.
	( punct - [_'"] ) {
		tt = static_cast<Token_type>(ts[0]);
		fbreak;
	};

	'new' { tt = Token::NEW; fbreak; };
	'new-res' { tt = Token::RESOURCE; fbreak; };
	'hw-root' { tt = Token::HWROOT; fbreak; };
	'wrap' { tt = Token::WRAP; fbreak; };

	# Identifier. Upon entering clear the buffer. On all transitions
	# buffer a character. Upon leaving, dump the identifier.
	alpha_u alnum_u* {
		tt = Token::IDENTIFIER;
		yylval->str.s = ts;
		yylval->str.e = te;
		fbreak;
	};

	# Single Quote.
	#sliteralChar = [^'\\] | newline | ( '\\' . any_count_line );
	#'\'' . sliteralChar* . '\'' {
	#	printf( "single_lit(%i): ", curline );
	#	fwrite( ts, 1, te-ts, stdout );
	#	printf("\n");
	#};

	# Double Quote.
	dliteralChar = [^"\\] - newline;
	'"' . dliteralChar* . '"' {
		tt = Token::STRING;
		yylval->str.s = ts;
		yylval->str.e = te;
		fbreak;
	};

	# Whitespace is standard ws, newlines and control codes.
	wspace { yylloc->columns(te-ts); yylloc->step();};
	newline;

	# Match an integer. We don't bother clearing the buf or filling it.
	# The float machine overlaps with int and it will do it.
	digit+ {
		tt = Token::INTEGER;
		yylval->num = 0;
		for (char const *x = ts; x < te; ++x)
		  yylval->num = yylval->num * 10 + (*x - '0');
		fbreak;
	};

	# Match a float. Upon entering the machine clear the buf, buffer
	# characters on every trans and dump the float upon leaving.
	#digit+ '.' digit+ {
	#	printf( "float(%i): ", curline );
	#	fwrite( ts, 1, te-ts, stdout );
	#	printf("\n");
	#};

	# Match a hex. Upon entering the hex part, clear the buf, buffer characters
	# on every trans and dump the hex on leaving transitions.
	'0x' xdigit+ {
		tt = Token::INTEGER;
		yylval->num = 0;
		for (char const *x = ts; x < te; ++x)
		  {
		    if (*x >= '0' && *x <= '9')
		      yylval->num = yylval->num * 16 + (*x - '0');
		    else if (*x >= 'a' && *x <= 'f')
		      yylval->num = yylval->num * 16 + (*x + 10 - 'a');
		    else if (*x >= 'A' && *x <= 'F')
		      yylval->num = yylval->num * 16 + (*x + 10 - 'A');
		  }
		fbreak;
	};

	*|;
}%%

%% write data nofinal;


void
cfg::Scanner::init()
{
  %% write init;
  p = s;
  pe = e;
}

cfg::Scanner::Token_type
cfg::Scanner::lex(cfg::Parser::semantic_type *yylval, cfg::location *yylloc)
{
  char const * const eof = pe;
  Token_type tt = Token::END;

  if (p == s)
    yylloc->initialize(new std::string(_filename));
  else
    yylloc->step();

  %% write exec;

  if ( cs == cfg_lang_error )
    {
      std::cerr << *yylloc << ": Parse error!" << std::endl;
      return static_cast<Token_type>(-1);
    }

  yylloc->columns(te-ts);

  return tt;
}


