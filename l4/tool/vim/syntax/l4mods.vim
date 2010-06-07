" Vim syntax file for modules.list configuration language
" Language:    modules.list configuration language
" Maintainer:  Adam Lackorzynski <adam@os.inf.tu-dresden.de>
" Last Change: 2010 Apr 27

if exists("b:current_syntax")
  finish
endif

syn clear
syn case match

setlocal iskeyword+=-
syn keyword l4modsStatement       modaddr module kernel sigma0 roottask module-group module-glob module-perl module-shell bootstrap
syn keyword l4modsStatementTitle  entry group contained
syn match   l4modsTitle           /^ *\(entry\|group\).*/ contains=l4modsStatementTitle
syn match   l4modsComment         /#.*/

hi def link l4modsStatement      Statement
hi def link l4modsStatementTitle Type
hi def link l4modsComment        Comment
hi def link l4modsTitle          String

let b:current_syntax = "l4mods"
