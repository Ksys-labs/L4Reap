
#line 1 "cfg_scan.rl"
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


#line 118 "cfg_scan.rl"



#line 24 "cfg_scan.cc"
static const char _cfg_lang_actions[] = {
	0, 1, 1, 1, 2, 1, 3, 1, 
	8, 1, 9, 1, 10, 1, 11, 1, 
	12, 1, 13, 1, 14, 1, 16, 1, 
	17, 1, 18, 1, 19, 1, 20, 1, 
	21, 1, 22, 1, 23, 1, 24, 1, 
	25, 1, 26, 2, 0, 6, 2, 0, 
	7, 2, 0, 15, 2, 3, 4, 2, 
	3, 5
};

static const unsigned char _cfg_lang_key_offsets[] = {
	0, 0, 3, 4, 5, 11, 12, 13, 
	14, 15, 16, 17, 18, 49, 52, 53, 
	54, 55, 58, 60, 66, 67, 74, 82, 
	90, 98, 106, 114, 122, 130
};

static const char _cfg_lang_trans_keys[] = {
	10, 34, 92, 10, 10, 48, 57, 65, 
	70, 97, 102, 114, 111, 111, 116, 114, 
	101, 115, 9, 10, 13, 32, 34, 35, 
	44, 46, 47, 48, 61, 95, 104, 110, 
	119, 33, 38, 40, 45, 49, 57, 58, 
	64, 65, 90, 91, 96, 97, 122, 123, 
	126, 9, 13, 32, 10, 46, 47, 120, 
	48, 57, 48, 57, 48, 57, 65, 70, 
	97, 102, 62, 95, 48, 57, 65, 90, 
	97, 122, 95, 119, 48, 57, 65, 90, 
	97, 122, 45, 95, 48, 57, 65, 90, 
	97, 122, 95, 101, 48, 57, 65, 90, 
	97, 122, 95, 119, 48, 57, 65, 90, 
	97, 122, 45, 95, 48, 57, 65, 90, 
	97, 122, 95, 114, 48, 57, 65, 90, 
	97, 122, 95, 97, 48, 57, 65, 90, 
	98, 122, 95, 112, 48, 57, 65, 90, 
	97, 122, 0
};

static const char _cfg_lang_single_lengths[] = {
	0, 3, 1, 1, 0, 1, 1, 1, 
	1, 1, 1, 1, 15, 3, 1, 1, 
	1, 1, 0, 0, 1, 1, 2, 2, 
	2, 2, 2, 2, 2, 2
};

static const char _cfg_lang_range_lengths[] = {
	0, 0, 0, 0, 3, 0, 0, 0, 
	0, 0, 0, 0, 8, 0, 0, 0, 
	0, 1, 1, 3, 0, 3, 3, 3, 
	3, 3, 3, 3, 3, 3
};

static const unsigned char _cfg_lang_index_offsets[] = {
	0, 0, 4, 6, 8, 12, 14, 16, 
	18, 20, 22, 24, 26, 50, 54, 56, 
	58, 60, 63, 65, 69, 71, 76, 82, 
	88, 94, 100, 106, 112, 118
};

static const char _cfg_lang_indicies[] = {
	1, 2, 1, 0, 5, 4, 7, 6, 
	9, 9, 9, 8, 11, 10, 12, 10, 
	13, 10, 14, 10, 16, 15, 17, 15, 
	18, 15, 19, 20, 19, 19, 0, 22, 
	23, 24, 25, 26, 28, 29, 30, 31, 
	32, 21, 21, 27, 21, 29, 21, 29, 
	21, 1, 19, 19, 19, 33, 5, 4, 
	35, 34, 6, 34, 37, 27, 36, 27, 
	36, 9, 9, 9, 38, 39, 34, 29, 
	29, 29, 29, 40, 29, 42, 29, 29, 
	29, 41, 43, 29, 29, 29, 29, 41, 
	29, 44, 29, 29, 29, 41, 29, 45, 
	29, 29, 29, 41, 47, 29, 29, 29, 
	29, 46, 29, 48, 29, 29, 29, 41, 
	29, 49, 29, 29, 29, 41, 29, 50, 
	29, 29, 29, 41, 0
};

static const char _cfg_lang_trans_targs[] = {
	1, 0, 12, 12, 2, 12, 3, 12, 
	12, 19, 12, 6, 7, 8, 12, 12, 
	10, 11, 12, 13, 12, 12, 14, 12, 
	15, 16, 17, 18, 20, 21, 22, 24, 
	27, 12, 12, 12, 12, 4, 12, 12, 
	12, 12, 23, 5, 25, 26, 12, 9, 
	28, 29, 21
};

static const char _cfg_lang_trans_actions[] = {
	0, 0, 19, 33, 0, 46, 0, 43, 
	39, 0, 37, 0, 0, 0, 17, 35, 
	0, 0, 15, 0, 49, 13, 5, 11, 
	0, 5, 5, 0, 0, 55, 0, 0, 
	0, 27, 21, 9, 29, 0, 31, 7, 
	41, 25, 5, 0, 0, 5, 23, 0, 
	0, 0, 52
};

static const char _cfg_lang_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

static const char _cfg_lang_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 3, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

static const unsigned char _cfg_lang_eof_trans[] = {
	0, 0, 4, 4, 9, 11, 11, 11, 
	11, 16, 16, 16, 0, 34, 35, 35, 
	35, 37, 37, 39, 35, 41, 42, 42, 
	42, 42, 47, 42, 42, 42
};

static const int cfg_lang_start = 12;
static const int cfg_lang_error = 0;

static const int cfg_lang_en_main = 12;


#line 121 "cfg_scan.rl"


void
cfg::Scanner::init()
{
  
#line 158 "cfg_scan.cc"
	{
	cs = cfg_lang_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 127 "cfg_scan.rl"
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

  
#line 183 "cfg_scan.cc"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _cfg_lang_actions + _cfg_lang_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 2:
#line 1 "NONE"
	{ts = p;}
	break;
#line 204 "cfg_scan.cc"
		}
	}

	_keys = _cfg_lang_trans_keys + _cfg_lang_key_offsets[cs];
	_trans = _cfg_lang_index_offsets[cs];

	_klen = _cfg_lang_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _cfg_lang_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _cfg_lang_indicies[_trans];
_eof_trans:
	cs = _cfg_lang_trans_targs[_trans];

	if ( _cfg_lang_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _cfg_lang_actions + _cfg_lang_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 19 "cfg_scan.rl"
	{ yylloc->lines(); yylloc->step(); }
	break;
	case 3:
#line 1 "NONE"
	{te = p+1;}
	break;
	case 4:
#line 50 "cfg_scan.rl"
	{act = 10;}
	break;
	case 5:
#line 54 "cfg_scan.rl"
	{act = 11;}
	break;
	case 6:
#line 33 "cfg_scan.rl"
	{te = p+1;}
	break;
	case 7:
#line 34 "cfg_scan.rl"
	{te = p+1;}
	break;
	case 8:
#line 36 "cfg_scan.rl"
	{te = p+1;{ tt = Token::INSTANCE; {p++; goto _out; } }}
	break;
	case 9:
#line 37 "cfg_scan.rl"
	{te = p+1;{ tt = Token::RANGE; {p++; goto _out; } }}
	break;
	case 10:
#line 38 "cfg_scan.rl"
	{te = p+1;{ tt = Token::COMMA; {p++; goto _out; } }}
	break;
	case 11:
#line 42 "cfg_scan.rl"
	{te = p+1;{
		tt = static_cast<Token_type>(ts[0]);
		{p++; goto _out; }
	}}
	break;
	case 12:
#line 48 "cfg_scan.rl"
	{te = p+1;{ tt = Token::RESOURCE; {p++; goto _out; } }}
	break;
	case 13:
#line 49 "cfg_scan.rl"
	{te = p+1;{ tt = Token::HWROOT; {p++; goto _out; } }}
	break;
	case 14:
#line 71 "cfg_scan.rl"
	{te = p+1;{
		tt = Token::STRING;
		yylval->str.s = ts;
		yylval->str.e = te;
		{p++; goto _out; }
	}}
	break;
	case 15:
#line 80 "cfg_scan.rl"
	{te = p+1;}
	break;
	case 16:
#line 42 "cfg_scan.rl"
	{te = p;p--;{
		tt = static_cast<Token_type>(ts[0]);
		{p++; goto _out; }
	}}
	break;
	case 17:
#line 47 "cfg_scan.rl"
	{te = p;p--;{ tt = Token::NEW; {p++; goto _out; } }}
	break;
	case 18:
#line 54 "cfg_scan.rl"
	{te = p;p--;{
		tt = Token::IDENTIFIER;
		yylval->str.s = ts;
		yylval->str.e = te;
		{p++; goto _out; }
	}}
	break;
	case 19:
#line 79 "cfg_scan.rl"
	{te = p;p--;{ yylloc->columns(te-ts); yylloc->step();}}
	break;
	case 20:
#line 84 "cfg_scan.rl"
	{te = p;p--;{
		tt = Token::INTEGER;
		yylval->num = 0;
		for (char const *x = ts; x < te; ++x)
		  yylval->num = yylval->num * 10 + (*x - '0');
		{p++; goto _out; }
	}}
	break;
	case 21:
#line 102 "cfg_scan.rl"
	{te = p;p--;{
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
		{p++; goto _out; }
	}}
	break;
	case 22:
#line 42 "cfg_scan.rl"
	{{p = ((te))-1;}{
		tt = static_cast<Token_type>(ts[0]);
		{p++; goto _out; }
	}}
	break;
	case 23:
#line 47 "cfg_scan.rl"
	{{p = ((te))-1;}{ tt = Token::NEW; {p++; goto _out; } }}
	break;
	case 24:
#line 54 "cfg_scan.rl"
	{{p = ((te))-1;}{
		tt = Token::IDENTIFIER;
		yylval->str.s = ts;
		yylval->str.e = te;
		{p++; goto _out; }
	}}
	break;
	case 25:
#line 84 "cfg_scan.rl"
	{{p = ((te))-1;}{
		tt = Token::INTEGER;
		yylval->num = 0;
		for (char const *x = ts; x < te; ++x)
		  yylval->num = yylval->num * 10 + (*x - '0');
		{p++; goto _out; }
	}}
	break;
	case 26:
#line 1 "NONE"
	{	switch( act ) {
	case 10:
	{{p = ((te))-1;} tt = Token::WRAP; {p++; goto _out; } }
	break;
	case 11:
	{{p = ((te))-1;}
		tt = Token::IDENTIFIER;
		yylval->str.s = ts;
		yylval->str.e = te;
		{p++; goto _out; }
	}
	break;
	}
	}
	break;
#line 432 "cfg_scan.cc"
		}
	}

_again:
	_acts = _cfg_lang_actions + _cfg_lang_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 1:
#line 1 "NONE"
	{ts = 0;}
	break;
#line 445 "cfg_scan.cc"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _cfg_lang_eof_trans[cs] > 0 ) {
		_trans = _cfg_lang_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}

#line 143 "cfg_scan.rl"

  if ( cs == cfg_lang_error )
    {
      std::cerr << *yylloc << ": Parse error!" << std::endl;
      return static_cast<Token_type>(-1);
    }

  yylloc->columns(te-ts);

  return tt;
}


