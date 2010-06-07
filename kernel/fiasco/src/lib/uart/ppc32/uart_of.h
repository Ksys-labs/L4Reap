#ifndef L4_CXX_UART_OF_H__
#define L4_CXX_UART_OF_H__

#include "uart_base.h"
#include <stdarg.h>
#include <string.h>
#include "of1275.h"

namespace L4
{
	class Uart_of : public Uart, public Of
	{
	private:
		ihandle_t  _serial;

	public:
		Uart_of(unsigned long prom)
		  : Uart(0, 0), Of(prom), _serial(0) {}
		bool startup(unsigned long base);

		void shutdown();
		bool enable_rx_irq(bool enable = true);
		bool enable_tx_irq(bool enable = true);
		bool change_mode(Transfer_mode m, Baud_rate r);
		int get_char(bool blocking = true) const;
		int char_avail() const;
		void out_char(char c) const;
		int write(char const *s, unsigned long count) const;
	};
};

#endif
