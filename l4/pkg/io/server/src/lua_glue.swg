%module Io

%include <typemaps.i>
%include <lua_typemap.i>
%include "std_string.i"
%include "lua_typeinfo.i"

%{

#include "vdevice.h"
#include "vbus_factory.h"
#include "hw_device.h"

static int is_lua_propval(lua_State *L, int idx)
{
  if (lua_isnumber(L, idx))
    return 1;
  if (lua_isstring(L, idx))
    return 1;
  return 0;
}

static Hw::Device::Prop_val to_lua_propval(lua_State *L, int idx)
{
  if (lua_isnumber(L, idx))
    return Hw::Device::Prop_val(lua_tointeger(L, idx));
  else if (lua_isstring(L, idx))
    return Hw::Device::Prop_val(lua_tostring(L, idx));
  return 0;
}

int luaopen_Io(lua_State *);
int add_vbus(Vi::Device *dev);
void dump_devs(Device *d);

%}


%apply SWIG_BIGUINT { Resource::Addr }
%apply SWIG_BIGINT { Resource::Size }

%typemap(in,checkfn="is_lua_propval") Hw::Device::Prop_val
%{$1 = to_lua_propval(L, $input); %}

%typemap(in, checkfn="is_lua_propval") Hw::Device::Prop_val const & (Hw::Device::Prop_val tmp)
%{tmp = to_lua_propval(L, $input); $1 = &tmp; %}


%ignore Resource_space;
%ignore Resource_provider;
%ignore Root_resource;
%ignore Vi::Dev_feature;
%ignore Hw::Dev_feature;

%include "resource.h"

%apply SWIGTYPE *DISOWN { Hw::Device *disown };
%apply SWIGTYPE *DISOWN { Vi::Device *disown };
%apply SWIGTYPE *DISOWN { Resource *disown };

%nodefaultctor;
%rename(Vi_dev_factory) Vi::Dev_factory;
%rename(Vi_device) Vi::Device;
%rename(Hw_dev_factory) Hw::Device_factory;
%rename(Hw_device) Hw::Device;

%nodefaultdtor Vi::Dev_factory;
%nodefaultdtor Hw::Device_factory;

class Device
{};

class Generic_device : public Device
{
public:
  void add_resource(Resource *disown);
};

%extend Generic_device
{
  int nresources() const
  {
    return self->resources()->size();
  }

  Resource *resource(int idx) const
  {
    return self->resources()->at(idx);
  }
};

namespace Vi {

class Device : public Generic_device
{
public:
  void add_child(Device *disown);
  void name(char const *);
  char const *name() const;
  Device *parent() const;
  Device *children() const;
  Device *next() const;
  int depth() const;
  int add_filter(cxx::String const &, cxx::String const &);
  int add_filter(cxx::String const &, unsigned long long);
  int add_filter(cxx::String const &, unsigned long long, unsigned long long);
};

class Dev_factory
{
public:
  virtual Device *vcreate() = 0;
  virtual Device *vcreate(Hw::Device *f) = 0;

  static Device *create(std::string const &_class);
  static Device *create(Hw::Device *f, bool warn = true);
};
}

%extend Vi::Device
{
  void set_name(char const *name) { self->name(name); }
};

namespace Hw {
class Device : public Generic_device
{
public:
  struct Prop_val;
  int set_property(char const *prop, Prop_val const &val);
  void add_child(Device *disown);
  void set_name(char const *);
  char const *name() const;
  bool match_cid(char const *cid) const;
  Device *parent() const;
  Device *children() const;
  Device *next() const;
  int depth() const;
  void plugin();
};

class Device_factory
{
public:
  virtual Device *create() = 0;
  static Device *create(char const *name);
};

}

%extend Hw::Device
{
  Hw::Device *find_by_name(std::string const &name) const
  {
    for (Hw::Device::iterator c = self->begin(0);
         c != self->end(); ++c)
      if (name == (*c)->name())
        return *c;

    return 0;
  }
};

Hw::Device *system_bus();
void dump_devs(Device *d);
int add_vbus(Vi::Device *dev);

%luacode {

Io.Res = {}

function Io.Res.io(start, _end, flags)
  local f = flags or 0
  return Io.Resource(Io.Resource_Io_res + f, start, _end or start)
end

function Io.Res.mmio(start, _end, flags)
  local f = flags or 0
  return Io.Resource(Io.Resource_Mmio_res + f, start, _end or start)
end

function Io.Res.irq(start, flags)
  local f = flags or 0
  local s, e
  if type(start) == "table" then
    s = start[1]
    e = start[2]
  else
    s = start
    e = start
  end
  return Io.Resource(Io.Resource_Irq_res + f, s, e)
end

Io.Dt = {}

function Io.Dt.add_child(parent, name, dev, idx)
  parent:add_child(dev)
  if dev.plugin and (parent:parent() or parent == Io.system_bus()) then
    dev:plugin()
  end
  if type(name) == "string" then
    if idx ~= nil then
      name = name .. '[' .. idx ..']'
    end
    dev:set_name(name)
  end
end

function Io.Dt.iterator(dev, max_depth)
  local max_d = (max_depth or 0) + dev:depth()
  local current = dev
  local start = dev
  return function ()
    local c = current
    if c == nil then
      return nil
    end

    local cd = c:depth()
    local cc = c:children()
    if max_d > cd and cc then
      current = cc
      return c
    elseif c:next() then
      current = c:next()
      return c
    else
      local p = c
      while (true) do
        p = p:parent()
        if (not p) or (p == start) then
          current = nil
          return c
        elseif p:next() then
          current = p:next()
          return c
        end
      end
    end
  end
end

function Io.Dt.match_cids(self, cids)
  local r = {}
  for v in string.gmatch(cids, "([^,]+),*") do
    if self:match_cid(v) then
      return true
    end
  end
  return false
end

Io.Dt.PCI_cc =
{
  storage = "CC_01",
  network = "CC_02",
  display = "CC_03",
  media   = "CC_04",
  bridge  = "CC_06",
  com     = "CC_07",
  usb     = "CC_0c",
  wlan    = "CC_0d",
}

Io.Dt.MAX_DEPTH = 1000

function Io.Dt.Range(start, stop)
  return { range = true, start, stop }
end

function Io.Dt.match(self, cids)
  for t,v in pairs(Io.Dt.PCI_cc) do
    cids = cids:gsub("(PCI/"..t..")", "PCI/" .. v)
  end

  local devs = {}
  for d in self:devices(Io.Dt.MAX_DEPTH) do
    if d:match_cids(cids) then
      devs[#devs+1] = d
    end
  end
  return devs
end

function Io.Dt.device(self, path)
  for i in string.gmatch(path, "([^%./]+)%.*") do
    self = self:find_by_name(i)
    if self == nil then
      return nil
    end
  end
  return self
end

function Io.Dt.resources(self)
  local n = self:nresources()
  local c = 0
  return function ()
    if c >= n then return nil end
    c = c + 1
    return self:resource(c - 1)
  end
end

local hwfn = Io.swig_class("Hw_device")[".fn"]
local vifn = Io.swig_class("Vi_device")[".fn"]

local dev_fns =
{
  resources = Io.Dt.resources,
  devices = Io.Dt.iterator,
  match_cids = Io.Dt.match_cids,
  match = Io.Dt.match,
  device = Io.Dt.device
}

for name, func in pairs(dev_fns) do
  hwfn[name] = func
  vifn[name] = func
end

function vifn:add_filter_val(tag, value)
  if type(value) == "table" and value.range then
    return self:add_filter(tag, value[1], value[2])
  elseif type(value) == "table" then
    for _, v in ipairs(value) do
      local res = self:add_filter_val(tag, v)
      if res < 0 then
        return res
      end
    end
    return 0
  else
    return self:add_filter(tag, value)
  end
end

local add_child = Io.Dt.add_child

local handle_device_member = function(dev, val, name)
  if Io.swig_instance_of(val, "Resource *") then
    dev:add_resource(val)
    return
  elseif Io.swig_instance_of(val, "Generic_device *") then
    add_child(dev, name, val)
    return
  elseif type(val) == "table" then
    for i, v in ipairs(val) do
      add_child(dev, name, v, i)
    end
    return
  elseif type(name) == "string" then
    local sp = dev.set_property
    if type(sp) == "function" then
      sp(dev, name, val)
      return
    end
  end
  print("ERROR: cannot handle device member: " .. tostring(name) .. ": " .. tostring(val))
end

function Io.Dt.add_device_data(dev, data)
  local maxi = 0
  for i, v in ipairs(data) do
    handle_device_member(dev, v, i)
    maxi = i
  end
  for k, v in pairs(data) do
    if (type(k) ~= "number") or (k > maxi) then
      handle_device_member(dev, v, k)
    end
  end
end

local set_dev_data = Io.Dt.add_device_data

Io.Hw = {}

setmetatable(Io.Hw, { __index = function (self, t)
  return function (data)
    local b = Io.Hw_dev_factory_create(t)
    if data then
      set_dev_data(b, data)
    end
    return b
  end
end})

function Io.hw_add_devices(data)
  local sb = Io.system_bus()
  for n, dev in pairs(data) do
    if type(n) == "string" then
      dev:set_property("name", n)
    end
    sb:add_child(dev)
    dev:plugin()
  end
  return data
end

Io.Vi = {}

setmetatable(Io.Vi, { __index = function (self, t)
  return function (data)
    local b = Io.Vi_dev_factory_create(t)
    if data then
      set_dev_data(b, data)
    end
    return b
  end
end})

function wrap(devs_, filter)
  local devs = devs_
  if type(devs_) ~= "table" then
    devs = { devs_ }
  end
  local v = {}
  for _, d in ipairs(devs) do
    local vd = Io.Vi_dev_factory_create(d, false)
    if vd then
      if type(filter) == "table" then
        for tag, val in pairs(filter) do
          local res = vd:add_filter_val(tag, val)
          if res < 0 then
            print("ERROR: applying filter expression: "..tag.."=...", debug.traceback(2))
          end
        end
      end
      v[#v + 1] = vd
    end
  end
  if #v == 1 then
    return v[1]
  else
    return v
  end
end

local add_vbus = Io.add_vbus

function Io.add_vbus(name, bus)
  bus:set_name(name)
  add_vbus(bus)
end

function Io.add_vbusses(busses)
  for name, bus in pairs(busses) do
    Io.add_vbus(name, bus)
  end
  return busses
end

}
