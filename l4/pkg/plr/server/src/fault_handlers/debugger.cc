/*
 * debugger.cc --
 *
 *     Implementation of a tiny debugger without any GDB features.
 *
 *     Currently this debugger allows to configure an address in the Romain ini file,
 *     (simplegdb:singlestep). It will then place a breakpoint (0xCC) on this address
 *     and once this BP is hit, start single-stepping from this point on.
 *
 * (c) 2011 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "../log"
#include "../app_loading"
#include "../emulation"
#include "../configuration"

#include "debugging.h"
#include "observers.h"

Romain::SimpleDebugObserver::SimpleDebugObserver()
	: _int1_seen(0)
{
	_bp = new Breakpoint(determine_address());
}

/*****************************************************************
 *                      Debugging stuff                          *
 *****************************************************************/
void Romain::SimpleDebugObserver::startup_notify(Romain::App_instance *i,
                                               Romain::App_thread *,
                                               Romain::App_model *am)
{
	_bp->activate(i, am);
	DEBUG() << "Activated breakpoint in instance " << i->id()
	        << " @ address " << std::hex << _bp->address();
	enter_kdebug();
}


void Romain::SimpleDebugObserver::status() const { }

Romain::Observer::ObserverReturnVal
Romain::SimpleDebugObserver::notify(Romain::App_instance *i,
                                    Romain::App_thread *t,
                                    Romain::App_model *am)
{
	switch(t->vcpu()->r()->trapno) {
		case 1:
			Romain::InstructionPrinter(t->vcpu()->r()->ip, 0).print_instruction();
			++_int1_seen;
			t->vcpu()->print_state();
			INFO() << "INT1 seen: " << _int1_seen;
			enter_kdebug("single step exception");
			break;
		case 3:
			DEBUG() << "INT3 @ " << std::hex << t->vcpu()->r()->ip
				    << " addr = " << _bp->address();
			if (_bp->was_hit(t)) {
				DEBUG() << "refs: " << _bp->refs();
				t->vcpu()->r()->ip -= 1; // revert int3
				t->vcpu()->r()->flags |= TrapFlag;
				_bp->deactivate(i, am);
				if (_bp->refs() == 0)
					delete _bp;
			}
			break;
		default:
			break;
	}

	return Romain::Observer::Continue;
}
