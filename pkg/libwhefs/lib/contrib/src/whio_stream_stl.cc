#include <wh/whio/whio_stream_stl.hpp>
#include <vector>
#include <stdexcept>

namespace whio {

    struct ostreambuf_whio::Impl
    {
	// FIXME? add a buffer?
	std::ostream & in;
	whio_stream * out;
	std::streambuf * oldBuf;
        bool ownsWh;
	Impl(std::ostream & i, whio_stream * o, bool ownsWh ) :
	    in(i),
	    out(o),
	    oldBuf(i.rdbuf()),
            ownsWh(ownsWh)
	{
	    if( !o || (0 == o->api->iomode(o)) )
	    {
		throw std::runtime_error("ostreambuf_whio::ostreambuf_whio() requires that the whio_stream be open write mode.");
	    }
	}
	~Impl()
	{
            if(ownsWh && this->out)
            {
                this->out->api->finalize(this->out);
            }
	}
    };


    ostreambuf_whio::ostreambuf_whio( std::ostream & in,
                                      whio_stream * out,
                                      bool ownsWh )
	: impl(new Impl(in,out,ownsWh))
    {
	this->setp(0,0);
	this->setg(0,0,0);
	in.rdbuf( this );
    }

    ostreambuf_whio::~ostreambuf_whio()
    {
	std::streambuf * rb = impl->in.rdbuf();
	if( rb == this )
	{
	    impl->in.rdbuf( impl->oldBuf );
	}
	delete impl;
    }


    int ostreambuf_whio::overflow(int c)
    {
	typedef traits_type CT;
	if (!CT::eq_int_type(c, CT::eof()))
	{
            char x = static_cast<char>(c);
	    return (1 == impl->out->api->write( impl->out, &x, 1 ))
		? CT::not_eof(c)
		: CT::eof();
	}
	return CT::eof();
    }


    struct istreambuf_whio::Impl
    {
	std::istream & sstr;
	whio_stream * wstr;
	std::streambuf * oldBuf;
        bool ownsWh;
        typedef std::vector<std::istream::char_type> BufferType;
        BufferType bufa;
	static const int bufsize = 512;
	Impl(std::istream & i, whio_stream * o, bool ownsWh ) :
	    sstr(i),
	    wstr(o),
	    oldBuf(i.rdbuf()),
            ownsWh(ownsWh),
	    bufa(bufsize,0)
	{
	    if( !o || (o->api->iomode(o) > 0) )
	    {
		throw std::runtime_error("istreambuf_whio::istreambuf_whio() requires that the whio_stream be open in read mode.");
	    }
	}
	~Impl()
	{
            if( this->ownsWh && this->wstr )
            {
                this->wstr->api->finalize( this->wstr );
            }
	}
    };


    istreambuf_whio::istreambuf_whio( std::istream & in,
                                      whio_stream * out,
                                      bool ownsWh )
	: impl(new Impl(in,out,ownsWh))
    {
	this->setp(0,0);
	this->setg(0,0,0);
	in.rdbuf( this );
    }

    istreambuf_whio::~istreambuf_whio()
    {
	std::streambuf * rb = impl->sstr.rdbuf();
	if( rb == this )
	{
	    impl->sstr.rdbuf( impl->oldBuf );
	}
	delete impl;
    }

    int istreambuf_whio::underflow()
    {
	char * dest = &impl->bufa[0];
	whio_size_t rd = impl->wstr->api->read( impl->wstr, dest, Impl::bufsize );
	if( rd < 1 )
	{
	    return traits_type::eof();
	}
	this->setg(dest,dest,dest+rd);
	return traits_type::not_eof(*dest);
    }


    ostream_whio::ostream_whio( whio_stream * proxy, bool ownsWh )
	: m_buf( *this, proxy, ownsWh )
    {
    }
    ostream_whio::~ostream_whio()
    {
    }

    istream_whio::istream_whio( whio_stream * proxy, bool ownsWh )
	: m_buf( *this, proxy, ownsWh )
    {
    }
    istream_whio::~istream_whio()
    {
    }


} // namespace whio
