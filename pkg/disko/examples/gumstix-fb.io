hw-root
{
  gpio => new Device()
  {
    .hid = "gpio";
    new-res Mmio(0x48310000 .. 0x48310fff);
    new-res Mmio(0x49050000 .. 0x49050fff);
    new-res Mmio(0x49052000 .. 0x49052fff);
    new-res Mmio(0x49054000 .. 0x49054fff);
    new-res Mmio(0x49056000 .. 0x49056fff);
    new-res Mmio(0x49058000 .. 0x49058fff);
  }

  lcd => new Device()
  {
    .hid = "OMAP_LCD";
    new-res Mmio(0x48050000 .. 0x48050fff);
  }

  i2c1 => new Device()
  {
    .hid = "i2c";
    new-res Mmio(0x48070000 .. 0x48070fff);
  }

  mcspi => new Device()
  {
    .hid = "mcspi";
    new-res Mmio(0x48098000 .. 0x48098fff);
  }

  sys => new Device()
  {
    .hid = "System Control";
    new-res Mmio(0x48002000 .. 0x480047ff);#ctrl
    new-res Mmio(0x48004800 .. 0x4800ffff);#CM
  }


  prcm2 => new Device()
  {
    .hid = "prcm2";

    new-res Mmio(0x48060000 .. 0x4806003f);#i2c
    new-res Mmio(0x48072000 .. 0x4807203f);

    new-res Mmio(0x480AB000 .. 0x480ACfff);


    new-res Mmio(0x48200000 .. 0x4820ffff);#ICs


    new-res Mmio(0x48306800 .. 0x48309fff);#PRCM
    new-res Mmio(0x4830A000 .. 0x4830ffff);#tap

    new-res Mmio(0x6C000000 .. 0x6Cffffff);#sms
    new-res Mmio(0x6D000000 .. 0x6Dffffff);#sdrc

    new-res Irq(56);
    new-res Irq(61);
    new-res Irq(57);
    new-res Irq(7);
    new-res Irq(92);
  }

  tsc => new Device()
  {
    .hid = "OMAP_TSC";
    new-res Mmio(0x48098000 .. 0x48098fff);
    new-res Irq(32);
  }
}

gui => new System_bus()
{
  tsc => wrap(hw-root.tsc);
  gpio => wrap(hw-root.gpio);
  prcm2 => wrap(hw-root.prcm2);
  mcspi => wrap(hw-root.mcspi);
}

fbdrv => new System_bus()
{
  sys => wrap(hw-root.sys);
  lcd => wrap(hw-root.lcd);
  i2c1 => wrap(hw-root.i2c1);
  gpio => wrap(hw-root.gpio);
}

