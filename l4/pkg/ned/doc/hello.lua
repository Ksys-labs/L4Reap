-- Simplest way to start a program.
--
-- This uses the global memory allocator and factory of Ned
-- for everything (the name space, the region map, the memory allocator,
-- and the logger). Hello gets a new name space containing only the 'rom'
-- directory from Ned.
L4.default_loader:start({}, "rom/hello");

-- The first parameter of start is a table of options for the startup.
-- Those options are:
--   ns:  to set a template table for the name space for the program.
--        NOTE: if there is not already an element with name 'rom' in the
--        table start will add rom = L4.Env.names:q("rom") automatically.
--   log: a two element table for specifying a custom log tag. The first
--        element is the textual tag and the second the color.
-- The second parameter is the file to start. And the third and further
-- arguments are passed as the 'argv' vector to the program.
-- In the case that the last argument is a table it is interpreted as POSIX
-- environment for the program.
