IMPLEMENTATION[16550]:

IMPLEMENT
bool Kernel_uart::startup(unsigned port, int irq)
{
  unsigned io_port;
  switch(port)
    {
     case 0: return false;
     case 1: if (irq == -1) irq = 4; io_port = 0x3f8; break;
     case 2: if (irq == -1) irq = 3; io_port = 0x2f8; break;
     case 3:                         io_port = 0x3e8; break;
     case 4:                         io_port = 0xe28; break;
    default:                         io_port = port;  break;
    }

  return Uart::startup(io_port, irq);
}
