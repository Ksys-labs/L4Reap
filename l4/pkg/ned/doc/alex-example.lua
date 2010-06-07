-- vim:set ft=lua:

package.path = "rom/?.lua";

require("L4");
require("Aw");

l = L4.default_loader;

Aw.io({}, "-vvvv", "rom/x86-legacy.devs", "rom/vbus-config.vbus");

Aw.rtc();
Aw.hid();

local gui = {
  -- put the factory caps for both interfaces in rws mode here
  -- 'mag' is the factory for fully flexible mag-sessions with multiple
  -- views and buffers and so on. 'svc' is the factory for fixed frame
  -- buffers.
  mag = l:new_channel():m("rws");
  svc = l:new_channel():m("rws");
}

Aw.gui(gui, Aw.fb("0x117"));

ex1_caps = {
  fb = gui.mag:create(L4.Proto.Goos);
};

local ldr = {
  loader = l:new_channel():svr()
};

l:start({ caps = ldr }, "rom/loader" );
l:start({ caps = ex1_caps, log = {"sflex"}}, "rom/scout");
l:start({ caps = {fb = gui.mag:create(L4.Proto.Goos) }}, "rom/l4_intro");

local xl = L4.Loader.new({
  mem = L4.Env.mem_alloc,
  loader = ldr.loader:m("rws")
});

ex2_caps = {
  fb = gui.svc:create(L4.Proto.Goos, "530x660");
};

xl:start({caps = ex2_caps, log={"sfix"}}, "rom/scout");

