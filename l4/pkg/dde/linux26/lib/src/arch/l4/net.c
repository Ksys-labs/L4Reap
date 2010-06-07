/******************************************************************************
 * DDELinux networking utilities.                                             *
 *                                                                            *
 * Bjoern Doebel <doebel@tudos.org>                                           *
 *                                                                            *
 * (c) 2005 - 2007 Technische Universitaet Dresden                            *
 * This file is part of DROPS, which is distributed under the terms of the    *
 * GNU General Public License 2. Please see the COPYING file for details.     *
 ******************************************************************************/

#include <l4/dde/linux26/dde26_net.h>

#include <linux/kernel.h>
#include <linux/skbuff.h>

#include "local.h"


/* Callback function to be called if a network packet arrives and needs to
 * be handled by netif_rx() or netif_receive_skb()
 */
linux_rx_callback l4dde26_rx_callback = NULL;


/* Register a netif_rx callback function.
 *
 * \return pointer to old callback function
 */
linux_rx_callback l4dde26_register_rx_callback(linux_rx_callback cb)
{
	linux_rx_callback old = l4dde26_rx_callback;
	l4dde26_rx_callback = cb;
	DEBUG_MSG("New rx callback @ %p.", cb);

	return old;
}
