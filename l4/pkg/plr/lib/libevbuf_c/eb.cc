#include <l4/plr/measurements.h>

extern "C" l4_uint64_t evbuf_get_time(void *eb, unsigned local)
{
	return reinterpret_cast<Measurements::EventBuf*>(eb)->getTime(local == 1);
}


extern "C" struct GenericEvent* evbuf_next(void *eb)
{
	return (GenericEvent*)reinterpret_cast<Measurements::EventBuf*>(eb)->next();
}
