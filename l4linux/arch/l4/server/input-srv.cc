#include <l4/re/event>
#include <l4/re/util/event_svr>
#include <l4/re/util/event_buffer>
#include <l4/re/util/meta>
#include <l4/cxx/ipc_server>
#include <l4/log/log.h>

#include <asm/server/server.h>
#include <asm/server/input-srv.h>

class Input : public L4Re::Util::Event_svr<Input>,
              public l4x_srv_object
{
public:
	l4x_input_srv_ops *ops;

	int init();
	int get_num_streams();
	int get_stream_info(int idx, L4Re::Event_stream_info *);
	int get_stream_info_for_id(l4_umword_t id, L4Re::Event_stream_info *);
	int get_axis_info(l4_umword_t id, unsigned naxes, unsigned *axis,
	                  L4Re::Event_absinfo *i);

	int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
	void add(struct l4x_input_event const *);
	void trigger() const { _irq.trigger(); }
	L4::Cap<L4::Kobject> rcv_cap() { return l4x_srv_rcv_cap(); }

private:
	L4Re::Util::Event_buffer _evbuf;
};

int
Input::init()
{
	L4Re::Util::Auto_cap<L4Re::Dataspace>::Cap b
		= L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
	if (!b.is_valid())
		return -L4_ENOMEM;

	int r;
	if ((r = L4Re::Env::env()->mem_alloc()->alloc(L4_PAGESIZE, b.get())) < 0)
		return r;

	if ((r = _evbuf.attach(b.get(), L4Re::Env::env()->rm())) < 0)
		return r;

	memset(_evbuf.buf(), 0, b.get()->size());

	_ds  = b.release();

	l4x_srv_object::dispatch = &l4x_srv_generic_dispatch<Input>;
	return 0;
}

void
Input::add(struct l4x_input_event const *e)
{
	_evbuf.put(*reinterpret_cast<L4Re::Event_buffer::Event const *>(e));
}

int
Input::get_num_streams()
{
	return ops->num_streams();
}

int
Input::get_stream_info(int idx, L4Re::Event_stream_info *si)
{
	return ops->stream_info(idx, si);
}

int
Input::get_stream_info_for_id(l4_umword_t id, L4Re::Event_stream_info *si)
{
	return ops->stream_info_for_id(id, si);
}

int
Input::get_axis_info(l4_umword_t id, unsigned naxes, unsigned *axis,
                     L4Re::Event_absinfo *i)
{
	return ops->axis_info(id, naxes, axis, i);
}

int
Input::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios)
{
	l4_msgtag_t tag;
	ios >> tag;

	switch (tag.label()) {
		case L4::Meta::Protocol:
			return L4::Util::handle_meta_request<L4Re::Event>(ios);
		case L4Re::Protocol::Event:
		case L4_PROTO_IRQ:
			return L4Re::Util::Event_svr<Input>::dispatch(obj, ios);
		default:
			return -L4_EBADPROTO;
	}
}

static Input _input;

static Input *input_obj()
{
	return &_input;
}

extern "C" void
l4x_srv_input_init(l4_cap_idx_t thread, struct l4x_input_srv_ops *ops)
{
	L4::Cap<void> c;
	int err;

	if ((err = input_obj()->init()) < 0) {
		LOG_printf("l4x-srv: Input server object initialization failed (%d)\n",
		           err);
		return;
	}


	input_obj()->ops = ops;
	c = l4x_srv_register_name(input_obj(),
	                          L4::Cap<L4::Thread>(thread), "ev");
	if (!c)
		LOG_printf("l4x-srv: Input object registration failed.\n");
}

extern "C" void
l4x_srv_input_add_event(struct l4x_input_event *e)
{
	input_obj()->add(e);
}

extern "C" void
l4x_srv_input_trigger()
{
	if (input_obj()->icu_get_irq(0)->cap())
		input_obj()->trigger();
}

