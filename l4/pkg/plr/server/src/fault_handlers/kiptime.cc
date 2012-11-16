/*
 * kiptime.cc --
 *
 * (c) 2011-2012 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "../log"
#include "../app_loading"
#include "../configuration"
//#include "../emulation"

#include "observers.h"

#include <vector>
#include <stdlib.h>

namespace Romain {
class KipTimeObserver_priv : public KIPTimeObserver
{
	private:
		std::vector<Breakpoint*> _breakpoints;

		void read_target_list(char *list)
		{
			char *end = list + strlen(list); // end of list
			char *c = list;                  // iterator

			while (c < end) {
				char *c2 = c;

				/* find next separator or end char */
				while ((*c2 != ',') && (c2 < end)) ++c2;

				*c2 = 0;

				/* convert the address found */
				errno = 0;
				l4_addr_t a = strtol(c, NULL, 16); // XXX check return
				if (errno) {
					ERROR() << "Conversion error.";
				} else {
					_breakpoints.push_back(new Breakpoint(a));
				}

				c = c2+1;
			}
		}

	DECLARE_OBSERVER("kip time");
	KipTimeObserver_priv();
};
}

Romain::KIPTimeObserver* Romain::KIPTimeObserver::Create()
{
	return new Romain::KipTimeObserver_priv();
}

Romain::KipTimeObserver_priv::KipTimeObserver_priv()
{
	char *list = strdup(ConfigStringValue("kip-time:target", "none"));
	if (list) {
		read_target_list(list);
	}
	free(list);
}


void Romain::KipTimeObserver_priv::status() const { }

/*****************************************************************
 *                      Debugging stuff                          *
 *****************************************************************/
void Romain::KipTimeObserver_priv::startup_notify(Romain::App_instance *inst,
                                                  Romain::App_thread *,
                                                  Romain::Thread_group *,
                                                  Romain::App_model *am)
{
	for (std::vector<Breakpoint*>::iterator i = _breakpoints.begin();
		 i != _breakpoints.end(); ++i) {
		DEBUG() << std::hex << (*i)->address();
		(*i)->activate(inst, am);
	}
}

Romain::Observer::ObserverReturnVal
Romain::KipTimeObserver_priv::notify(Romain::App_instance *i,
                                     Romain::App_thread *t,
                                     Romain::Thread_group *,
                                     Romain::App_model *am)
{
	if (!entry_reason_is_int3(t->vcpu(), i, am) &&
		!entry_reason_is_int1(t->vcpu()))
		return Romain::Observer::Ignored;

	for (std::vector<Breakpoint*>::const_iterator it = _breakpoints.begin();
		 it != _breakpoints.end(); ++it) {
		if ((*it)->was_hit(t)) {
			l4_cpu_time_t time = l4re_kip()->clock;

			t->vcpu()->r()->si = time & 0xFFFFFFFF;
			t->vcpu()->r()->di = (time >> 32) & 0xFFFFFFFF;
			t->vcpu()->r()->ip += 11;

			return Romain::Observer::Replicatable;
		}
	}

	return Romain::Observer::Ignored;
}
