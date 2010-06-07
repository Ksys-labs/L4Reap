/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "cfg_scanner.h"
#include <cstring>

namespace cfg {

Scanner::Scanner(char const *zts)
: s(zts), e(zts ? strlen(zts) + zts : zts)
{
  init();
}

Scanner::Scanner(char const *s, char const *e, const char *filename)
: s(s), e(e), _filename(filename)
{
  init();
}


Scanner::~Scanner()
{
}

}
