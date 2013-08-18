-- Example configuration for io

-- Configure two platform devices to be known to io
Io.hw_add_devices
{
  FOODEVICE = Io.Hw.Device
  {
    hid = "FOODEVICE";
    Io.Res.irq(17);
    Io.Res.mmio(0x6f000000, 0x6f007fff);
  },

  BARDEVICE = Io.Hw.Device
  {
    hid = "BARDEVICE";
    Io.Res.irq(19);
    Io.Res.irq(20);
    Io.Res.mmio(0x6f100000, 0x6f100fff);
  }
}


Io.add_vbusses
{
-- Create a virtual bus for a client and give access to FOODEVICE
  client1 = Io.Vi.System_bus(function ()
    dev = wrap(hw:match("FOODEVICE"));
  end),

-- Create a virtual bus for another client and give it access to BARDEVICE
  client2 = Io.Vi.System_bus(function ()
    dev = wrap(hw:match("BARDEVICE"));
  end)
}
