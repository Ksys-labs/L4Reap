# Example configuration for io

# Configure 2 platform device to be known to io
hw-root
{
  FOODEVICE => new Device()
  {
    .hid = "FOODEVICE";
    new-res Irq(17);
    new-res Mmio(0x6f000000 .. 0x6f007fff);
  }

  BARDEVICE => new Device()
  {
    .hid = "BARDEVICE";
    new-res Irq(19);
    new-res Irq(20);
    new-res Mmio(0x6f100000 .. 0x6f100fff);
  }
}

# Create a virtual bus for a client and give access to FOODEVICE
client1 => new System_bus()
{
  dev => wrap(hw-root.FOODEVICE);
}

# Create a virtual bus for another client and give it access to
# BARDEVICE
client2 => new System_bus()
{
  dev => wrap(hw-root.BARDEVICE);
}
