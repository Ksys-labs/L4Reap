function handle_event(ev)
  local device = ev[6];
  local stream = ev[5];
  local ddh = Mag.devices[device];
  if not ddh then
    Mag.devices[device] = {};
    ddh =  Mag.devices[device];
  end

  local dh = ddh[stream];
  if not dh then
    dh = Mag.create_input_device(device, stream);
  end

  -- print ("EV: ", ev[1], ev[2], ev[3], ev[4], ev[5], ev[6]);
  return dh:process(ev);
end


module("Mag", package.seeall);

Event = {
  Type   = 1,
  Code   = 2,
  Value  = 3,
  Time   = 4,
  Device = 5,
  Stream = 6,

  SYN = 0;
  KEY = 1;
  REL = 2;
  ABS = 3;
  MSC = 4;
  SW  = 5;

  Syn = {
    REPORT    = 0,
    CONFIG    = 1,
    MT_REPORT = 2,
  };

  Key = {
    RESERVED         = 0,
    ESC              = 1,
    KEY_1            = 2,
    KEY_2            = 3,
    KEY_3            = 4,
    KEY_4            = 5,
    KEY_5            = 6,
    KEY_6            = 7,
    KEY_7            = 8,
    KEY_8            = 9,
    KEY_9            = 10,
    KEY_0            = 11,
    MINUS            = 12,
    EQUAL            = 13,
    BACKSPACE        = 14,
    TAB              = 15,
    Q                = 16,
    W                = 17,
    E                = 18,
    R                = 19,
    T                = 20,
    Y                = 21,
    U                = 22,
    I                = 23,
    O                = 24,
    P                = 25,
    LEFTBRACE        = 26,
    RIGHTBRACE       = 27,
    ENTER            = 28,
    LEFTCTRL         = 29,
    A                = 30,
    S                = 31,
    D                = 32,
    F                = 33,
    G                = 34,
    H                = 35,
    J                = 36,
    K                = 37,
    L                = 38,
    SEMICOLON        = 39,
    APOSTROPHE       = 40,
    GRAVE            = 41,
    LEFTSHIFT        = 42,
    BACKSLASH        = 43,
    Z                = 44,
    X                = 45,
    C                = 46,
    V                = 47,
    B                = 48,
    N                = 49,
    M                = 50,
    COMMA            = 51,
    DOT              = 52,
    SLASH            = 53,
    RIGHTSHIFT       = 54,
    KPASTERISK       = 55,
    LEFTALT          = 56,
    SPACE            = 57,
    CAPSLOCK         = 58,
    F1               = 59,
    F2               = 60,
    F3               = 61,
    F4               = 62,
    F5               = 63,
    F6               = 64,
    F7               = 65,
    F8               = 66,
    F9               = 67,
    F10              = 68,
    NUMLOCK          = 69,
    SCROLLLOCK       = 70,
    KP7              = 71,
    KP8              = 72,
    KP9              = 73,
    KPMINUS          = 74,
    KP4              = 75,
    KP5              = 76,
    KP6              = 77,
    KPPLUS           = 78,
    KP1              = 79,
    KP2              = 80,
    KP3              = 81,
    KP0              = 82,
    KPDOT            = 83,
    ZENKAKUHANKAKU   = 85,
    KEY_102ND        = 86,
    F11              = 87,
    F12              = 88,
    RO               = 89,
    KATAKANA         = 90,
    HIRAGANA         = 91,
    HENKAN           = 92,
    KATAKANAHIRAGANA = 93,
    MUHENKAN         = 94,
    KPJPCOMMA        = 95,
    KPENTER          = 96,
    RIGHTCTRL        = 97,
    KPSLASH          = 98,
    SYSRQ            = 99,
    RIGHTALT         = 100,
    LINEFEED         = 101,
    HOME             = 102,
    UP               = 103,
    PAGEUP           = 104,
    LEFT             = 105,
    RIGHT            = 106,
    END              = 107,
    DOWN             = 108,
    PAGEDOWN         = 109,
    INSERT           = 110,
    DELETE           = 111,
    MACRO            = 112,
    MUTE             = 113,
    VOLUMEDOWN       = 114,
    VOLUMEUP         = 115,
    POWER            = 116,
    KPEQUAL          = 117,
    KPPLUSMINUS      = 118,
    PAUSE            = 119,
    KPCOMMA          = 121,
    HANGEUL          = 122,
    HANGUEL          = 122,
    HANJA            = 123,
    YEN              = 124,
    LEFTMETA         = 125,
    RIGHTMETA        = 126,
    COMPOSE          = 127,
    STOP             = 128,
    AGAIN            = 129,
    PROPS            = 130,
    UNDO             = 131,
    FRONT            = 132,
    COPY             = 133,
    OPEN             = 134,
    PASTE            = 135,
    FIND             = 136,
    CUT              = 137,
    HELP             = 138,
    MENU             = 139,
    CALC             = 140,
    SETUP            = 141,
    SLEEP            = 142,
    WAKEUP           = 143,
    FILE             = 144,
    SENDFILE         = 145,
    DELETEFILE       = 146,
    XFER             = 147,
    PROG1            = 148,
    PROG2            = 149,
    WWW              = 150,
    MSDOS            = 151,
    COFFEE           = 152,
    DIRECTION        = 153,
    CYCLEWINDOWS     = 154,
    MAIL             = 155,
    BOOKMARKS        = 156,
    COMPUTER         = 157,
    BACK             = 158,
    FORWARD          = 159,
    CLOSECD          = 160,
    EJECTCD          = 161,
    EJECTCLOSECD     = 162,
    NEXTSONG         = 163,
    PLAYPAUSE        = 164,
    PREVIOUSSONG     = 165,
    STOPCD           = 166,
    RECORD           = 167,
    REWIND           = 168,
    PHONE            = 169,
    ISO              = 170,
    CONFIG           = 171,
    HOMEPAGE         = 172,
    REFRESH          = 173,
    EXIT             = 174,
    MOVE             = 175,
    EDIT             = 176,
    SCROLLUP         = 177,
    SCROLLDOWN       = 178,
    KPLEFTPAREN      = 179,
    KPRIGHTPAREN     = 180,
    NEW              = 181,
    REDO             = 182,
    F13              = 183,
    F14              = 184,
    F15              = 185,
    F16              = 186,
    F17              = 187,
    F18              = 188,
    F19              = 189,
    F20              = 190,
    F21              = 191,
    F22              = 192,
    F23              = 193,
    F24              = 194,
    PLAYCD           = 200,
    PAUSECD          = 201,
    PROG3            = 202,
    PROG4            = 203,
    SUSPEND          = 205,
    CLOSE            = 206,
    PLAY             = 207,
    FASTFORWARD      = 208,
    BASSBOOST        = 209,
    PRINT            = 210,
    HP               = 211,
    CAMERA           = 212,
    SOUND            = 213,
    QUESTION         = 214,
    EMAIL            = 215,
    CHAT             = 216,
    SEARCH           = 217,
    CONNECT          = 218,
    FINANCE          = 219,
    SPORT            = 220,
    SHOP             = 221,
    ALTERASE         = 222,
    CANCEL           = 223,
    BRIGHTNESSDOWN   = 224,
    BRIGHTNESSUP     = 225,
    MEDIA            = 226,
    SWITCHVIDEOMODE  = 227,
    KBDILLUMTOGGLE   = 228,
    KBDILLUMDOWN     = 229,
    KBDILLUMUP       = 230,
    SEND             = 231,
    REPLY            = 232,
    FORWARDMAIL      = 233,
    SAVE             = 234,
    DOCUMENTS        = 235,
    UNKNOWN          = 240,
    OK               = 0x160,
    SELECT           = 0x161,
    GOTO             = 0x162,
    CLEAR            = 0x163,
    POWER2           = 0x164,
    OPTION           = 0x165,
    INFO             = 0x166,
    TIME             = 0x167,
    VENDOR           = 0x168,
    ARCHIVE          = 0x169,
    PROGRAM          = 0x16a,
    CHANNEL          = 0x16b,
    FAVORITES        = 0x16c,
    EPG              = 0x16d,
    PVR              = 0x16e,
    MHP              = 0x16f,
    LANGUAGE         = 0x170,
    TITLE            = 0x171,
    SUBTITLE         = 0x172,
    ANGLE            = 0x173,
    ZOOM             = 0x174,
    MODE             = 0x175,
    KEYBOARD         = 0x176,
    SCREEN           = 0x177,
    PC               = 0x178,
    TV               = 0x179,
    TV2              = 0x17a,
    VCR              = 0x17b,
    VCR2             = 0x17c,
    SAT              = 0x17d,
    SAT2             = 0x17e,
    CD               = 0x17f,
    TAPE             = 0x180,
    RADIO            = 0x181,
    TUNER            = 0x182,
    PLAYER           = 0x183,
    TEXT             = 0x184,
    DVD              = 0x185,
    AUX              = 0x186,
    MP3              = 0x187,
    AUDIO            = 0x188,
    VIDEO            = 0x189,
    DIRECTORY        = 0x18a,
    LIST             = 0x18b,
    MEMO             = 0x18c,
    CALENDAR         = 0x18d,
    RED              = 0x18e,
    GREEN            = 0x18f,
    YELLOW           = 0x190,
    BLUE             = 0x191,
    CHANNELUP        = 0x192,
    CHANNELDOWN      = 0x193,
    FIRST            = 0x194,
    LAST             = 0x195,
    AB               = 0x196,
    NEXT             = 0x197,
    RESTART          = 0x198,
    SLOW             = 0x199,
    SHUFFLE          = 0x19a,
    BREAK            = 0x19b,
    PREVIOUS         = 0x19c,
    DIGITS           = 0x19d,
    TEEN             = 0x19e,
    TWEN             = 0x19f,
    DEL_EOL          = 0x1c0,
    DEL_EOS          = 0x1c1,
    INS_LINE         = 0x1c2,
    DEL_LINE         = 0x1c3,
    FN               = 0x1d0,
    FN_ESC           = 0x1d1,
    FN_F1            = 0x1d2,
    FN_F2            = 0x1d3,
    FN_F3            = 0x1d4,
    FN_F4            = 0x1d5,
    FN_F5            = 0x1d6,
    FN_F6            = 0x1d7,
    FN_F7            = 0x1d8,
    FN_F8            = 0x1d9,
    FN_F9            = 0x1da,
    FN_F10           = 0x1db,
    FN_F11           = 0x1dc,
    FN_F12           = 0x1dd,
    FN_1             = 0x1de,
    FN_2             = 0x1df,
    FN_D             = 0x1e0,
    FN_E             = 0x1e1,
    FN_F             = 0x1e2,
    FN_S             = 0x1e3,
    FN_B             = 0x1e4,
    MAX              = 0x1ff,
  };

  Rel = {
    X      = 0x00,
    Y      = 0x01,
    Z      = 0x02,
    RX     = 0x03,
    RY     = 0x04,
    RZ     = 0x05,
    HWHEEL = 0x06,
    DIAL   = 0x07,
    WHEEL  = 0x08,
    MISC   = 0x09,
    MAX    = 0x0f,
  };

  Abs = {
    X          = 0x00,
    Y          = 0x01,
    Z          = 0x02,
    RX         = 0x03,
    RY         = 0x04,
    RZ         = 0x05,
    THROTTLE   = 0x06,
    RUDDER     = 0x07,
    WHEEL      = 0x08,
    GAS        = 0x09,
    BRAKE      = 0x0a,
    HAT0X      = 0x10,
    HAT0Y      = 0x11,
    HAT1X      = 0x12,
    HAT1Y      = 0x13,
    HAT2X      = 0x14,
    HAT2Y      = 0x15,
    HAT3X      = 0x16,
    HAT3Y      = 0x17,
    PRESSURE   = 0x18,
    DISTANCE   = 0x19,
    TILT_X     = 0x1a,
    TILT_Y     = 0x1b,
    TOOL_WIDTH = 0x1c,
    VOLUME     = 0x20,
    MISC       = 0x28,
    MT_TOUCH_MAJOR = 0x30,
    MT_TOUCH_MINOR = 0x31,
    MT_WIDTH_MAJOR = 0x32,
    MT_WIDTH_MINOR = 0x33,
    MT_ORIENTATION = 0x34,
    MT_POSITION_X  = 0x35,
    MT_POSITION_Y  = 0x36,
    MT_TOOL_TYPE   = 0x37,
    MT_BLOB_ID     = 0x38,
    MT_TRACKING_ID = 0x39,
    MAX        = 0x3f,
  };

  Btn = {
    MISC           = 0x100,
    BTN_0          = 0x100,
    BTN_1          = 0x101,
    BTN_2          = 0x102,
    BTN_3          = 0x103,
    BTN_4          = 0x104,
    BTN_5          = 0x105,
    BTN_6          = 0x106,
    BTN_7          = 0x107,
    BTN_8          = 0x108,
    BTN_9          = 0x109,
    MOUSE          = 0x110,
    LEFT           = 0x110,
    RIGHT          = 0x111,
    MIDDLE         = 0x112,
    SIDE           = 0x113,
    EXTRA          = 0x114,
    FORWARD        = 0x115,
    BACK           = 0x116,
    TASK           = 0x117,
    JOYSTICK       = 0x120,
    TRIGGER        = 0x120,
    THUMB          = 0x121,
    THUMB2         = 0x122,
    TOP            = 0x123,
    TOP2           = 0x124,
    PINKIE         = 0x125,
    BASE           = 0x126,
    BASE2          = 0x127,
    BASE3          = 0x128,
    BASE4          = 0x129,
    BASE5          = 0x12a,
    BASE6          = 0x12b,
    DEAD           = 0x12f,
    GAMEPAD        = 0x130,
    A              = 0x130,
    B              = 0x131,
    C              = 0x132,
    X              = 0x133,
    Y              = 0x134,
    Z              = 0x135,
    TL             = 0x136,
    TR             = 0x137,
    TL2            = 0x138,
    TR2            = 0x139,
    SELECT         = 0x13a,
    START          = 0x13b,
    MODE           = 0x13c,
    THUMBL         = 0x13d,
    THUMBR         = 0x13e,
    DIGI           = 0x140,
    TOOL_PEN       = 0x140,
    TOOL_RUBBER    = 0x141,
    TOOL_BRUSH     = 0x142,
    TOOL_PENCIL    = 0x143,
    TOOL_AIRBRUSH  = 0x144,
    TOOL_FINGER    = 0x145,
    TOOL_MOUSE     = 0x146,
    TOOL_LENS      = 0x147,
    TOUCH          = 0x14a,
    STYLUS         = 0x14b,
    STYLUS2        = 0x14c,
    TOOL_DOUBLETAP = 0x14d,
    TOOL_TRIPLETAP = 0x14e,
    WHEEL          = 0x150,
    GEAR_DOWN      = 0x150,
    GEAR_UP        = 0x151,
  };
};

-- stores all kown input devices

devices = {};
Input_device = {
  Devs = {};
  quirks = {};
  Bus = {
    [0x01] = "pci",
    [0x02] = "isapnp",
    [0x03] = "usb",
    [0x04] = "hil",
    [0x05] = "bluetooth",
    [0x06] = "virtual",

    [0x10] = "isa",
    [0x11] = "i8042",
    [0x12] = "xtkbd",
    [0x13] = "rs232",
    [0x14] = "gameport",
    [0x15] = "parport",
    [0x16] = "amiga",
    [0x17] = "adb",
    [0x18] = "i2c",
    [0x19] = "host",
    [0x1A] = "gsc",
    [0x1B] = "atari",

    PCI		= 0x01,
    ISAPNP	= 0x02,
    USB		= 0x03,
    HIL		= 0x04,
    BLUETOOTH	= 0x05,
    VIRTUAL	= 0x06,
    
    ISA		= 0x10,
    I8042	= 0x11,
    XTKBD	= 0x12,
    RS232	= 0x13,
    GAMEPORT	= 0x14,
    PARPORT	= 0x15,
    AMIGA	= 0x16,
    ADB		= 0x17,
    I2C		= 0x18,
    HOST	= 0x19,
    GSC		= 0x1A,
    ATARI	= 0x1B,
  };
};

function Input_device:drop_event(e)
  return true;
end

-- ----------------------------------------------------------------------
--  Generic pointer device handling.
--
--  Handles absolute and relative motion events and the buttons
--  corresponding to a pointer device, suahc as a mouse.
-- ----------------------------------------------------------------------
Input_device.Devs.pointer = {};

function Input_device.Devs.pointer:init()
  -- raw position of an absolute device
  self.raw_pos       = { 0, 0 };
  self.motion        = { 0, 0 };
  -- transformation of raw coordinates to screen coordinates
  self.transform_pos = self:get_pointer_transform_func();
  self.drag_focus    = user_state:create_view_proxy();
  self.pointed_view  = user_state:create_view_proxy();

  if (self.info:get_absbit(0) and self.info:get_absbit(1)) then
    local res, x, y = self.device:get_abs_info(self.stream, 0, 1);
    if res == 0 then
      x.delta = x.max - x.min;
      y.delta = y.max - y.min;
      self.abs_info = { x, y };
    end
  end
end

-- generic transformationof absolute pointer events to screen coordinates
function Input_device:get_pointer_transform_func()
  -- transformation of raw coordinates to screen coordinates
  return function(self, x, y)
    if self.abs_info then
      return x * user_state.width / self.abs_info[1].delta,
             y * user_state.height / self.abs_info[2].delta;
    else
      return x, y;
    end
  end;
end


function Input_device.Devs.pointer:motion(e)
  local t = e[1];
  local c = e[2];
  if t == Event.REL and (c == 0 or c == 1) then
    self.motion[c + 1] = e[3];
    self.core_motion = 1;
    return true; -- cosnumed event
  elseif t == Event.ABS and (c == 0 or c == 1) then
    self.raw_pos[c + 1] = e[3];
    self.core_motion = 2;
    return true; -- consumed event
  end
  return false; -- did not consume event (pass on)
end

function Input_device.Devs.pointer:button(e)
  local c = e[2];
  local v = e[3];
  if v == 1 then
    if self.pressed_keys == 0 then
      self.set_focus = true;
      self.start_drag = true;
    end
    self.pressed_keys = self.pressed_keys + 1;

    if c == 70 then
      self.toggle_mode = 1;
    elseif c == 210 or c == 99 then
      self.toggle_mode = 2;
    end
  elseif v == 0 then
    self.pressed_keys = self.pressed_keys - 1;
    if self.pressed_keys == 0 then
      self.stop_drag = true;
    end
  end
end

-- ----------------------------------------------------------------
-- sync a set of events from a pointer device
--
-- - converts motion events to the core mouse pointer of mag
-- - manages dragging and keyboard focus

function Input_device.Devs.pointer:sync(ev)
  local update = false;
  local sink;

  if self.stop_drag or self.pressed_keys > 0 then
    sink = self.drag_focus;
  else
    sink = self.pointed_view;
  end

  if self.toggle_mode ~= 0 then
    user_state:toggle_mode(self.toggle_mode);
    update = true;
  end
  if self.core_motion == 2 then
    local x, y = self:transform_pos(self.raw_pos[1], self.raw_pos[2]);
    user_state:set_pointer(x, y);
  elseif self.core_motion == 1 then
    user_state:move_pointer(self.motion[1], self.motion[2]);
    self.motion = { 0, 0 };
  end

  user_state:find_pointed_view(self.pointed_view);

  if self.start_drag then
    self.drag_focus:set(self.pointed_view);
    self.start_drag = false;
  end

  -- print ("core_motion", self.core_motion);
  if self.core_motion and self.core_motion ~= 0 then
    if self.set_focus then
      update = user_state:set_kbd_focus(self.pointed_view) or update;
      self.set_focus = false;
    end
    user_state:post_pointer_event(sink, self.stream, ev[4]);
    self.core_motion = 0;
  elseif self.set_focus then
    update = user_state:set_kbd_focus(self.pointed_view) or update;
    self.set_focus = false;
  end

  for index = 1, #self.events do
    local e = self.events[index];
    user_state:post_event(sink, self.stream, ev[4], e[1], e[2], e[3]);
  end

  -- post syn event
  user_state:post_event(sink, self.stream, ev[4], ev[1], ev[2], ev[3], update);

  self.stop_drag = false;
  self.toggle_mode = 0;
  self.events = {}; -- clear old events
end

function Input_device.Devs.pointer:process(ev)
--  print ("got event:", time, device.stream, "f", device.can_set_focus, "f", t, c, v);

  if ev[1] == 0 and ev[2] == 0 then
    self.handler.sync(self, ev);
    return;
  end

  if ev[1] == 1 then
    if self.handler.button(self, ev) then
      return;
    end
  elseif self.handler.motion(self, ev) then
    return;
  end

  self.events[#self.events + 1] = ev;
end


Input_device.Devs.touchscreen = {};
setmetatable(Input_device.Devs.touchscreen, { __index = Input_device.Devs.pointer});

function Input_device.Devs.touchscreen:init()
  Input_device.Devs.pointer.init(self);
end

function Input_device.Devs.touchscreen:button(e)
  local c = e[2];
  local v = e[3];
  if c == Event.Btn.TOUCH then
    e[2] = Event.Btn.LEFT;
  end
  Input_device.Devs.pointer.button(self, e);
end

-- ----------------------------------------------------------------------
--  Touchpad pointing device handling
--  Inherits from pointer device and adds functionality to:
--  - transform absolute finger positions to relative movements
--  - tapping button emulation (missing)
--  - scroll wheel emulation etc (missing)
-- ----------------------------------------------------------------------
Input_device.Devs.touchpad = {};
setmetatable(Input_device.Devs.touchpad, { __index = Input_device.Devs.pointer});

function Input_device.Devs.touchpad:init()
  Input_device.Devs.pointer.init(self);
  self.touchdev = {
    touch      = false;
    release    = false;
    touch_drag = false;
    tap_time   = 150 * 1000; -- 150 ms
  };
  local tdev = self.touchdev;
  tdev.p    = { Mag.Axis_buf(4), Mag.Axis_buf(4) };
  tdev.pkts = 0;
end

function Input_device.Devs.touchpad:button(e)
  local c = e[2];
  local v = e[3];
  Input_device.Devs.pointer.button(self, e);
  if c == Event.Btn.TOUCH and v ~= 2 then
    local time = e[4];
    local p = self.touchdev;
    if v == 1 then
      if p.release and p.release + p.tap_time > time then
        p.touch_drag = true;
        self:process({ Event.KEY, Event.Btn.LEFT, 1, p.touch, e[5], e[6] });
      end
      p.touch = time;
      p.release = false;
    else
      if p.touch_drag then
        p.touch_drag = false;
        self:process({ Event.KEY, Event.Btn.LEFT, 0, time, e[5], e[6] });
      elseif p.touch and p.touch + p.tap_time > time then
        p.release = time;
        self:process({ Event.KEY, Event.Btn.LEFT, 1, p.touch, e[5], e[6] });
        self:process({ Event.KEY, Event.Btn.LEFT, 0, time, e[5], e[6] });
      end
      p.pkts = 0;
      p.touch = false;
    end
  end
end

function Input_device.Devs.touchpad:motion(e)
  local t = e[1];
  local c = e[2];
  local p = self.touchdev;
  if not p.touch then
    return true; -- drop event, if not touched
  end

  if t == Event.REL and (c == 0 or c == 1) then
    self.motion[c + 1] = e[3];
    self.core_motion = 1;
    return true; -- cosnumed event
  elseif t == Event.ABS and (c == 0 or c == 1) then
    c = c + 1;
    local v = e[3];
    local cnt = p.pkts;
    p.p[c][cnt] = v;
    if cnt >= 2 then
      local size = self.abs_info[c].delta;
      self.motion[c] = (v - p.p[c]:get(cnt - 2)) * 256 / size;
      self.core_motion = 1;
    end
    return true; -- cosnumed event
  end
  return false; -- did not consume event (pass on)
end

function Input_device.Devs.touchpad:sync(ev)
  local p = self.touchdev;
  if p.touch then
    local nc = p.pkts + 1;
    p.pkts = nc;
    p.p[1]:copy(nc, nc - 1);
    p.p[2]:copy(nc, nc - 1);
  end
  Input_device.Devs.pointer.sync(self, ev);
end

-- ----------------------------------------------------------------------
--  Keyboard device handling
-- ----------------------------------------------------------------------
Input_device.Devs.keyboard = {};

function Input_device.Devs.keyboard:process(ev)
  local c = ev[2];
  local v = ev[3];
  local toggle_mode = 0;
  if v == 1 then
    self.pressed_keys = self.pressed_keys + 1;
    if c == 70 then
      toggle_mode = 1;
    elseif c == 210 or c == 99 then
      toggle_mode = 2;
    end
  elseif v == 0 then
    self.pressed_keys = self.pressed_keys - 1;
  end
  local update = false;
  if toggle_mode ~= 0 then
    user_state:toggle_mode(toggle_mode);
    update = true;
  end
  user_state:post_event(nil, self.stream, ev[4], ev[1], ev[2], ev[3], update);
end

function Input_device:find_device_type()
  local bus, vend, prod, ver = self.info:get_device_id();
  self.dev_id = { bus = bus, vendor = vend, product = prod, version = ver };
  local id = bus .. "_" .. vend .. "_" .. prod;

  self.dev_type = "unknown";

  if self.quirks[id] and self.quirks[id](self, id) then
    return;
  end

  id = id .. "_" .. ver;

  if self.quirks[id] and self.quirks[id](self, id) then
    return;
  end

  local have_keys = self.info:get_evbit(1);
  if (have_keys and self.info:get_evbit(2)) then
    -- we have relative axes, can be a mouse
    if (self.info:get_relbit(0) and self.info:get_relbit(1)
        and self.info:get_keybit(0x110)) then
      -- x and y axis and at least the left mouse button found
      self.dev_type = "pointer";
    end
  end
  if (have_keys and self.info:get_evbit(3)) then
    -- we have absolute axes, can ba a mouse
    if self.info:get_absbit(0) and self.info:get_absbit(1) then
      if self.info:get_keybit(0x110) then
        -- x and y axis and at least the left mouse button found
        self.dev_type = "pointer";
      end
      if self.info:get_keybit(Event.Btn.TOUCH)
         and not self.info:get_keybit(0x110) then
        -- x and y axis and at least the left mouse button found
        self.dev_type = "touchscreen";
      end
    end
  end

  if self.dev_type == "pointer" then
    if self.info:get_keybit(Event.Btn.TOOL_FINGER) then
      self.dev_type = "touchpad";
    end
  end

  if (have_keys) then
    local k;
    for k = 0, 199 do
      if (self.info:get_keybit(k)) then
        self.dev_type = "keyboard";
        return;
      end
    end
  end
end


-- -----------------------------------------------------------------------
--  Create a input device (stream) object.
--  A single input stream is managed by a device specific set of
--  methods and data structures.
-- -----------------------------------------------------------------------
function create_input_device(device, stream)
  local dh = {
    -- the device handle itself
    device        = device,
    stream        = stream,

    -- number of keys currently pressed on this device
    pressed_keys  = 0,

    -- data according to the current bundle of events (one comound event)
    -- need to set the focus to the pointed view
    set_focus     = false,
    -- the current events stop dragging mode
    stop_drag     = false,
    -- the current events toggle kill and x-ray mode
    toggle_mode   = 0,
    -- the events to pass to the applications
    events        = {},
    -- event handler for the device
    process       = Input_device.drop_event
  };

  local r;
  r, dh.info = device:get_stream_info(stream);

  local meta_table = meta_table or { __index = Input_device };
  setmetatable(dh, meta_table);
  devices[device][stream] = dh;

  dh:find_device_type();
  dh.handler = Input_device.Devs[dh.dev_type] or {};
  dh.process = dh.handler.process or Input_device.drop_event;
  if dh.handler.init then dh.handler.init(dh); end

  local pdev;
  function pdev(dh)
    local a, b, c, d = dh.info:get_device_id();
    if Input_device.Bus[a] then
      return Input_device.Bus[a], b, c, d;
    end
    return "bus " .. a, b, c, d;
  end

  print (string.format([==[Input: new %s device (src='%s' stream='%s')
                           bus='%s' vendor=0x%x product=0x%x version=%d]==],
                       dh.dev_type, tostring(dh.device), tostring(dh.stream),
                       pdev(dh)));
  return dh;
end
