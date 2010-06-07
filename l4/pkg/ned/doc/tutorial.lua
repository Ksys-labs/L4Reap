local _doc = [==[

Tutorial lua script for Ned
===========================

Firstly we have a set of definitions available. Some come from 'ned.lua'
and others from the C++ bindings within Ned. The whole L4 stuff is in the
lua module "L4" (use require("L4")).
The L4 module classes and functions to cope with L4 capabilities and
their invocation.  A set of constants and access to the L4Re environment of
the running program.  And a set of classes to start L4 applications and
composing their name spaces.

L4 Capabilities
===============

The L4 module defines a user data type for capabilities.  A capability in lua
carries a typed L4 capability and is accompanied with a set of type specific
methods that may be called on the object behind the capability.  There also
exists a way to cast a capability to a capability to a different type of
object using L4.cast(type, cap).

L4.cast(type, cap)

Returns a cap transformed to a capability of the given type, whereas type
is either the fully qualified C++ name of the class encapsulating the object
or the L4 protocol ID assigned to all L4Re and L4 system objects.
If the type is unknown than nil is returned.

Generic capabilities provide the methods:

is_valid()

  Returns true if the capability is not the invalid cap (L4_INVALID_CAP), and
  false if it is the invalid cap.


L4Re::Namespace object
======================

There is a lua type for name spaces that has the following methods:

query(name), or q(name)

  Returns a closure (function) that initiates a query for the given name
  within the name space.  The function takes no arguments and returns
  a capability if successful or nil if name is not found.


link(name), or l(name)

  Returns a function that can create a link to the given object in the name
  space if put into another name space. The function takes two parameters
  the target name space and the name in the target name space.
  Loader:create_namespace and Loader.fill_namespace calls this function
  when things are really put into an L4Re::Namespace.


register(name, cap), or r(name, cap)

  Registers the given object capability under the given name. cap can
  be either a real capability (note query returns a function), a string, or
  nil.  If it is a capability it is just put into the name space.
  In the case cap is a string a placeholder will be put into the name space
  that will be replaced with a real capability later by some program.
  And if nil is use the name will be deleted from the name space.


L4::Factory object
==================

The factory object provides an interface to the generic create method of a
factory.

create(proto, ...)

  This method calls the factory to create an object of the given type,
  via the L4 protocol number (see L4.Proto table for more) all further
  arguments are passed to the factory.


Access to the L4Re Env capabilities
===================================

The L4 module defines a table L4.Env that contains the capabilities
of the L4Re::Env::env() environment. Namely:

factory   The kernel factory of Ned
log       The log object of Ned
mem_alloc The memory allocator provided to Ned
parent    The parent of Ned
rm        The region map of Ned
scheduler The scheduler of Ned


Some useful constants
=====================

L4.Proto table contains the most important protocol values for
L4 and L4Re objects.

  Namespace
  Goos
  Mem_alloc
  Rm
  Irq
  Sigma0
  Factory
  Log
  Scheduler


Support for starting L4 programs
================================

The L4 module defines two classes that are useful for starting l4 applications.
The class L4.Loader that encapsulates a fairly high level policy for
that is useful for starting a whole set of processes. And the class L4.App_env
that encapsulates a more fine-grained policy.

L4.Loader
---------

The class L4.Loader encapsulates the policy for starting programs with the
basic building blocks for the application comming from a dedicated loader,
such as Moe or a Loader instance.  These building blocks are a region map (Rm),
a name space, a scheduler, a memory allocator, and a logging facility.
A L4.Loader object is typically used to start multiple applications. There
is a L4.default_loader instance of L4.Loader that uses the L4.Env.mem_alloc
factory of the current Ned instance to create the objects for a new program.
However you may also use a more restricted factory for applications and
instanciate a loader for them.  The L4.Loader objects can already be used
to start a program with L4.Loader:start(app_spec, cmd, ...).  Where app_spec
is a table containing some parameters for the new application. cmd is the
command to run and the remaining arguments are the command-line options for
the application.

]==]

L4.default_loader:start({}, "rom/hello");

local _doc = [==[

This statement does the following:
 1. create a new name space for the application
 2. put L4.Env.names:query("rom") into the new name space (thus shares Ned's
    'rom' directory with the new program.
 3. Creates all the building blocks for the new process and starts the
    'l4re' support kernel in the new process which in turn start's 'rom/hello'
    in the new process.

Using the app_spec parameter you can modify the behavior in two ways. There are
two supported options 'ns' for providing a more usefull non-empty name space
for the application. And 'log' for modifying the logger tag and color.

]==]

local my_ns = {
  fb = L4.Env.names:query("vesa");
};

L4.default_loader:start({ns = my_ns, log = {"APP", "blue"}}, "rom/hello");

local _doc = [==[

This snippet creates a name-space template (my_ns) and uses it for the
new process and also sets user-defined log tags.  The L4.Loader:start method,
however, automatically adds the 'rom' directory to the name space if not
already specified in the template.

To use create a new L4.Loader instance you may use a generic factory for all
building blocks or set individual factories.

]==]

l = L4.Loader.new({mem = L4.Env.mem_alloc:create(L4.Proto.Factory, 512*1024)})

local _doc = [==[

Creates a loader instance that uses the newly created 512 Kbyte factory for
all building blocks. To set individual factories use the options:
  'mem'       as memory allocator for the new processes and as
              default factory for all objects not explicitely set to a
              different factory
  'log_fab'   for creating log objects.
  'ns_fab'    for creating name-space objects.
  'rm_fab'    for creating region-map objects.
  'sched_fab' for creating scheduler objects.



L4.App_env
----------

L4.App_env provides a more fine-grained control for a single process or for a
limited number of processes. L4.App_env uses an L4.Loader object as basic
facility. However you can override the memory allocator 'mem' for for the new
process as well as the kernel factory 'factory', the log capability etc.

]==]

local e = L4.App_env.new({
  loader = l,
  mem = L4.Env.mem_alloc:create(L4.Proto.Factory, 128*1024)
});

e:start("rom/hello");

local _doc = [==[

More to come....

]==]
