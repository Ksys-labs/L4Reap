#!/usr/bin/python

import struct

class Event:
    """Event base class"""

    HEADSIZE = 13
    EVENTSIZE = 32

    SYSCALL_TYPE = 1
    PAGEFAULT_TYPE = 2
    SWIFI_TYPE = 3
    FOO_TYPE = 4
    TRAP_TYPE = 5
    THREAD_START_TYPE = 6
    THREAD_STOP_TYPE = 7

    @staticmethod
    def eventName(ev):
        """Get name for event type"""
        syscall_names = ["INV", "SYS", "PF",
                         "SWIFI", "FOO", "TRAP",
                         "START", "STOP"]
        return syscall_names[ev]

    def __init__(self, time=0, typ=0, utcb=0, uid=None):
        self.ts = time
        self.type = typ
        self.utcb = utcb
        if uid is None:
        	self.id = self.utcb
        else:
        	self.id = uid

    def uid(self):
    	return self.id

    def __repr__(self):
        return "%d [%8x|%5s]" % (self.ts, self.utcb,
                                 Event.eventName(self.type))


class SyscallEvent(Event):
    def __init__(self, raw, time=0, utcb=0, uid=None):
        Event.__init__(self, time, Event.SYSCALL_TYPE, utcb, uid)
        (self.ip, self.label, ) = struct.unpack_from("II",
                                                     raw[Event.HEADSIZE:])

    def __repr__(self):
        return Event.__repr__(self) + " SYSCALL %08x, ret to %08x" % \
            (self.label, self.ip)

    def pretty(self, cols=20):
        return ["SYSCALL %08x" % (self.label),
                " ret -> %08x" % (self.ip)]


class PagefaultEvent(Event):
    def __init__(self, raw, time=0, utcb=0, uid=None):
        Event.__init__(self, time, Event.PAGEFAULT_TYPE, utcb, uid)
        (self.writepf, ) = struct.unpack_from("B", raw[Event.HEADSIZE:])
        (self.pfa,
         self.local,
         self.remote) = struct.unpack_from("III", raw[Event.HEADSIZE + 1:])
        #print hex(self.pfa)

    def __repr__(self):
        r = Event.__repr__(self)
        if (self.writepf):
            r += " w"
        r += "r pf @ %08x -> %08x" % (self.local, self.remote)
        return r

    def pretty(self, cols=20):
        r = []
        if self.writepf:
            r += ["wr pf @ 0x%x" % self.pfa]
        else:
            r += ["r pf @ 0x%x" % self.pfa]
        r += ["%x -> %x" % (self.local, self.remote)]
        return r


class SwifiEvent(Event):
    def __init__(self, raw, time=0, utcb=0, uid=None):
        Event.__init__(self, time, Event.SWIFI_TYPE, utcb, uid)

    def pretty(self, cols=20):
        return ["SWIFI"]


class FooEvent(Event):
    def __init__(self, raw, time=0, utcb=0, uid=None):
        Event.__init__(self, time, Event.FOO_TYPE, utcb, uid)
        (self.is_start, ) = struct.unpack_from("I", raw[Event.HEADSIZE:])

    def __repr__(self):
        r = Event.__repr__(self)
        if self.is_start == 0:
            r += " STOP"
        else:
            r += " START"
        return r

    def pretty(self, cols=20):
        return ["FOO"]


class TrapEvent(Event):

    counters = {}

    def __init__(self, raw, time=0, utcb=0, uid=None):
        Event.__init__(self, time, Event.TRAP_TYPE, utcb, uid)
        (self.is_start, ) = struct.unpack_from("B", raw[Event.HEADSIZE:])
        (self.trapaddr, self.trapno, ) = \
            struct.unpack_from("II", raw[Event.HEADSIZE + 1:])

        #print "S %d T %d" % (self.is_start, self.trapno)

    def __repr__(self):
        r = Event.__repr__(self)
        if self.is_start == 1:
            r += " start, trapno %x" % self.trapno
        else:
            r += " done"
        return r

    def pretty(self, cols=20):
        if self.is_start:
            return ["TRAP %x @ %08x" % (self.trapno, self.trapaddr)]
        else:
            return ["--- TRAP END ---"]


class ThreadStartEvent(Event):

    def __init__(self, raw, time=0, utcb=0, uid=None):
        Event.__init__(self, time, Event.THREAD_START_TYPE, utcb, uid)

    def __repr__(self):
        r = Event.__repr__(self)
        r += "Thread::Start"
        return r

    def pretty(self, cols=20):
        return ["Thread::Start"]


class ThreadStopEvent(Event):

    def __init__(self, raw, time=0, utcb=0, uid=None):
        Event.__init__(self, time, Event.THREAD_STOP_TYPE, utcb, uid)

    def __repr__(self):
        r = Event.__repr__(self)
        r += "Thread::Exit"
        return r

    def pretty(self, cols=20):
        return ["Thread::Exit"]
