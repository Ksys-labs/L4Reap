-- This is a configuration snippet for PCI device selection

Io.add_vbusses
{
  pciclient = Vi.System_bus(function ()
    PCI = Vi.PCI_bus(function ()
      pci_mm      = wrap(hw:match("PCI/CC_04"));
      pci_net     = wrap(hw:match("PCI/CC_02"));
      pci_storage = wrap(hw:match("PCI/CC_01"));
    end)
  end)
}
