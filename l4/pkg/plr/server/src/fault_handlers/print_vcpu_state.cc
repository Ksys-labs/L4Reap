/*
 * print_vcpu_state.cc --
 *
 *     Implementation of the observer that prints the VCPU state upon each
 *     fault.
 *
 * (c) 2011 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "observers.h"

/*****************************************************************
 *                    Print VCPU state                           *
 *****************************************************************/
DEFINE_EMPTY_STARTUP(PrintVCPUStateObserver)

Romain::Observer::ObserverReturnVal
Romain::PrintVCPUStateObserver::notify(Romain::App_instance *,
                                       Romain::App_thread *t,
                                       Romain::App_model *)
{
	t->vcpu()->print_state();
	return Romain::Observer::Continue;
}

void Romain::PrintVCPUStateObserver::status() const { }
