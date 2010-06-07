# This is a configuration snippet for PCI device selection

pciclient => new System_bus()
{
  pci_storage[] => wrap(hw-root.match("PCI/CC_01"));
  pci_net[] => wrap(hw-root.match("PCI/CC_02"));
  pci_mm[] => wrap(hw-root.match("PCI/CC_04"))
}
