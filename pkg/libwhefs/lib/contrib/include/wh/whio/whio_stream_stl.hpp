#ifndef WANDERINGHORSE_NET_WHEFS_HPP_INCLUDED
#define WANDERINGHORSE_NET_WHEFS_HPP_INCLUDED
/** @file whio_stream_stl.hpp

This file contains code for proxying whio_stream objects via the C++ STL
std::ostream and std::istream interfaces.


Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

License: Public Domain

*/
#include <wh/whio/whio_stream.h>
#include <iostream>

namespace whio
{
    /**
       ostreambuf_whio is a helper to divert output intended for a
       std::ostream to a whio_stream instead.

       This object is intended to be used as a proxy, like:

       \code
       whio_stream * outfile = whio_stream_for_file("my.file", "w+");
       std::ostringstream dummy;
       ostreambuf_whio sentry( dummy, out );
       ... output to dummy. It goes to outfile instead ...
       // For example:
       s11nlite::save( dummy, myS11nableObject );
       ...
       }
       \endcode

       Limitations: only write is supported. No lookback, no re-get,
       etc.
    */
    class ostreambuf_whio : public std::streambuf
    {
    private:
	struct Impl;
	Impl * impl;
    public:
	typedef std::streambuf::char_type char_type;
	typedef std::streambuf::traits_type traits_type;

	/**
	   Sets in.rdbuf(this) and sets up to divert all output to
	   whio_out.

           If whio_out does not appear to be opened for read mode, a
           std::exception is thrown.

           std_out must outlive this object.

           If ownsWh is true then this object takes over ownership of
           whio_out and will destroy it when this object is destroyed.
           If ownsWh is false then the caller is responsible for
           ensuring that both streams outlive this object.
	*/
	ostreambuf_whio( std::ostream & std_out,
                         whio_stream * whio_out,
                         bool ownsWh );

	/**
	   If the ostream still has this object as its
	   rdbuf() then this function re-sets the rdbuf
	   to its original state.

	   This does not close the associated streams.
	*/
	virtual ~ostreambuf_whio();

	/**
	   If !traits_type::not_eof(c) then it writes c to the
	   whio_stream and returns traits_type::not_eof(c), else is
	   does nothing and returns traits_type::eof().
	*/
	virtual int overflow(int c);
    private:
	//! Copying not allowed.
	ostreambuf_whio( const ostreambuf_whio & );
	//! Copying not allowed.
	ostreambuf_whio & operator=( const ostreambuf_whio & );
    };

    /**
       istreambuf_whio is a helper to redirect input aimed at std::istreams to
       a whio_stream instead.

       This object is intended to be used as a proxy, like:

       \code
       whio_stream * infile = whio_stream_for_filename("my.file","r");
       std::istreamstream dummy;
       ostreambuf_whio sentry( dummy, infile );
       ... input from dummy, but it will actually come from infile ...
       // For example
       s11nlite::node_type * node = s11nlite::load_node(dummy);
       ...
       }
       \endcode

       Limitations: only readahead is supported. No lookahead, no
       putback, etc.
    */
    class istreambuf_whio : public std::streambuf
    {
    private:
	struct Impl;
	Impl * impl;
    public:
	typedef std::streambuf::char_type char_type;
	typedef std::streambuf::traits_type traits_type;
	/**
	   Sets std_in.rdbuf(this) and sets up to divert
	   all output to whio_in.

           If 

	   If whio_in does not appear to be opened for read mode,
           a std::exception is thrown.

           std_in must outlive this object.

           If ownsWh is true then this object takes over ownership of
           whio_in and will destroy it when this object is destroyed.
           If ownsWh is false then the caller is responsible for
           ensuring that both streams outlive this object.
	*/
	istreambuf_whio( std::istream & std_in,
                         whio_stream * whio_in,
                         bool ownsWh );
	/**
	   If the istream still has this object as its
	   rdbuf() then this function re-sets the rdbuf
	   to its original state.

	   This does not close the associated streams.
	*/
	virtual ~istreambuf_whio();

    protected:
	/**
	   Fetches the next character(s) from the whio_stream.
	*/
	virtual int_type underflow();
    private:
	//! Copying not allowed.
	istreambuf_whio( const istreambuf_whio & );
	//! Copying not allowed.
	istreambuf_whio & operator=( const istreambuf_whio & );
    };


    /**
       A proxy class to allow passing a whio_stream to a
       routine requiring a std::ostream. All output to this object will end
       up going to the whio_stream.

       Example usage:

       \code
       whio_stream * file = whio_stream_for_filename( "my.file", "r+"));
       ostream_whio str(file);
       some_routine_taking_std_stream( str );
       \endcode

    */
    class ostream_whio : public std::ostream
    {
    public:
	/**
	   Sets up all output to go to proxy.

	   If proxy does not appear to be opened in write mode then a
	   std::exception is thrown.

           If ownsWh is true then this object takes over ownership of
           proxy and will destroy it when this object is destroyed.

           If ownsWh is false then the caller is responsible for
           ensuring that the proxy stream outlives this object.
	*/
	ostream_whio( whio_stream * proxy, bool ownsWh );
	/**
	   Detaches this object from the constructor-specified proxy.
	*/
	virtual ~ostream_whio();
    private:
	//! Copying not allowed.
	ostream_whio( ostream_whio const & );
	//! Copying not allowed.
	ostream_whio & operator=( ostream_whio const & );
	ostreambuf_whio m_buf;
    };

    /**
       A proxy class to allow passing a QIODevice (indirectly) to a
       routine requiring a std::istream. All input to this object will end
       up going to the QIODevice.

       Example usage:

       \code
       whio_stream * file = whio_stream_for_filename("my.file","r");
       istream_whio str(file);
       some_routine_taking_std_stream( str );
       \endcode
    */
    class istream_whio : public std::istream
    {
    public:
	/**
	   Sets up all output input to go to proxy.

	   If proxy does not appear to be opened in read mode then a
	   std::exception is thrown.

           If ownsWh is true then this object takes over ownership of
           proxy and will destroy it when this object is destroyed.

           If ownsWh is false then the caller is responsible for
           ensuring that the proxy stream outlives this object.
	*/
	istream_whio( whio_stream * proxy, bool ownsWh );
	virtual ~istream_whio();
    private:
	//! Copying not allowed.
	istream_whio( istream_whio const & );
	//! Copying not allowed.
	istream_whio & operator=( istream_whio const & );
	istreambuf_whio m_buf;
    };


} // namespace whio

#endif /* WANDERINGHORSE_NET_WHEFS_HPP_INCLUDED */
