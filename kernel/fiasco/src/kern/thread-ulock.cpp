INTERFACE [ulock]:

#include "prio_list.h"

EXTENSION class Thread
{
private:
  Locked_prio_list *_wait_queue;
};


//------------------------------------------------------------------------
IMPLEMENTATION [ulock]:

PUBLIC inline
Locked_prio_list *
Thread::wait_queue() const
{ return _wait_queue; }


PUBLIC inline
void
Thread::wait_queue(Locked_prio_list *wq)
{ _wait_queue = wq; }


PRIVATE inline NEEDS[Thread::wait_queue]
void
Thread::wait_queue_kill()
{
  while (Locked_prio_list *q = wait_queue())
    {
      Lock_guard<typeof(*q)> g(q);
      if (wait_queue() == q)
	{
          sender_dequeue(q);
	  wait_queue(0);
	  return;
	}
    }
}

//------------------------------------------------------------------------
IMPLEMENTATION [!ulock]:

PRIVATE inline
void
Thread::wait_queue_kill()
{}
