hw-root
{
  sys => new Device()
  {
    .hid = "sys";
    new-res Mmio(0x48002000 .. 0x480047ff);
    new-res Mmio(0x48004800 .. 0x4800ffff);
  }

  dmamem => new Device()
    {
        .hid = "dmamem";
		new-res Mmio_ram(0xa00000,0);
    }


    omap_gpio => new Device()
  {
	.hid = "omap_gpio";
    new-res Mmio(0x48310000 .. 0x48310fff);
    new-res Mmio(0x49050000 .. 0x49050fff);
    new-res Mmio(0x49052000 .. 0x49052fff);
    new-res Mmio(0x49054000 .. 0x49054fff);
    new-res Mmio(0x49056000 .. 0x49056fff);
    new-res Mmio(0x49058000 .. 0x49058fff);
	new-res Irq(29);
	new-res Irq(30);
	new-res Irq(31);
	new-res Irq(32);
	new-res Irq(33);
	new-res Irq(34);
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

  NIC2 => new Device()
  {
    .hid = "dm9000";
    new-res Mmio(0x2c000000 .. 0x2c000003);
    new-res Mmio(0x2c000400 .. 0x2c000403);
    new-res Irq(185);
  }

  prcm2 => new Device()
  {
    .hid = "prcm2";

    new-res Mmio(0x48056000 .. 0x48056fff);#DMA

    new-res Mmio(0x48060000 .. 0x4806007f);#i2c
    new-res Mmio(0x48062000 .. 0x48062fff);#usb
    new-res Mmio(0x48062000 .. 0x48063fff);#usb
    new-res Mmio(0x48064000 .. 0x480643ff);#usb
    new-res Mmio(0x48064400 .. 0x480647ff);#usb
    new-res Mmio(0x48064800 .. 0x4806ffff);#usb

    new-res Mmio(0x48072000 .. 0x4807207f);

    new-res Mmio(0x480AB000 .. 0x480ACfff);


    new-res Mmio(0x48200000 .. 0x4820ffff);#ICs


    new-res Mmio(0x48306800 .. 0x48309fff);#PRCM
    new-res Mmio(0x4830A000 .. 0x4830ffff);#tap

    new-res Mmio(0x68000000 .. 0x6800ffff);#
    new-res Mmio(0x4809c000 .. 0x4809c1ff);#mmc2
    new-res Mmio(0x480b4000 .. 0x480b41ff);#mmc3
    new-res Mmio(0x480ad000 .. 0x480ad1ff);#mmc3

    new-res Mmio(0x6C000000 .. 0x6Cffffff);#sms
    new-res Mmio(0x6D000000 .. 0x6Dffffff);#sdrc

    new-res Irq(12);
    new-res Irq(13);
    new-res Irq(14);
    new-res Irq(15);

    new-res Irq(56);
    new-res Irq(61);
    new-res Irq(57);
    new-res Irq(7);
    new-res Irq(92);
    new-res Irq(83);
    new-res Irq(86);

    new-res Irq(93);
    new-res Irq(94);

    new-res Irq(76); #ohci
    new-res Irq(77);#ehci
    new-res Irq(20);#gpmc

  }

  plat => new Device()
  {
    .hid = "plat";
    new-res Irq(76); #ohci

  }

  prcm3 => new Device()
  {
    .hid = "prcm3";
    new-res Mmio(0x6e000000 .. 0x6e000fff); #gmpc
    new-res Mmio(0x30000000 .. 0x30000fff); #nand
  }

  tsc => new Device()
  {
    .hid = "OMAP_TSC";
    new-res Mmio(0x48098000 .. 0x48098fff);
    new-res Irq(32);
  }

}


l4linux => new System_bus()
{
#	omap_gpio => wrap(hw-root.omap_gpio);
	dmamem => wrap(hw-root.dmamem);
    sys => wrap(hw-root.sys);
    i2c1 => wrap(hw-root.i2c1);
    prcm2 => wrap(hw-root.prcm2);
    plat =>  wrap(hw-root.plat);
    prcm3 => wrap(hw-root.prcm3);

}