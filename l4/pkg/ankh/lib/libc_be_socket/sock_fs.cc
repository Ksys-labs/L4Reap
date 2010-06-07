#include <l4/l4re_vfs/vfs.h>
#include <l4/l4re_vfs/backend>
#include <l4/util/util.h>
#include <l4/sys/kdebug.h>
#include <l4/cxx/ref_ptr>
#include <errno.h>
#include <cstdio>

EXTERN_C_BEGIN
#include "lwip/sockets.h"
EXTERN_C_END

/*
 * lwIP file backend for integrating lwIP sockets into L4Re VFS
 *
 * lwip maintains its own internal socket numbers and clients expect these to
 * be normal file descriptors from which to read/write. L4Re's VFS however has
 * a different view on fds and so we need an indirection that translates
 * between VFS fds and lwIP socket IDs. This translation is used by the BSD
 * socket API wrappers in this library to convert between these two
 * abstractions.
 *
 * Every time a client application creates a socket using the socket() wrapper
 * in this library, we register a new Socket_file with VFS. This allows us to
 * intercept readv/writev calls to this fd and map them to lwip_read(),
 * lwip_write().
 */
class Socket_file : public L4Re::Vfs::Be_file
{
	private:
		int _lwip_fd; // underlying lwIP file descriptor
		bool _connected; // socket connected?

	public:
		/* _connected is initially false, as we cannot read/write
		 * from a socket until it has been connected/bound
		 */
		explicit Socket_file(int lwip_fd = -1) throw()
		    : L4Re::Vfs::Be_file(),
		      _lwip_fd(lwip_fd),
		      _connected(false)
		{ }


		~Socket_file() throw()
		{
			if (_lwip_fd > 0)
				lwip_close(_lwip_fd);
		}

		bool connected() { return _connected; }
		void connected(bool t) { _connected = t; }

		int lwip_fd() { return _lwip_fd; }

		ssize_t readv(const struct iovec*, int) throw();
		ssize_t writev(const struct iovec*, int) throw();
		
		// should be in L4Re::Vfs::Be_file ?
		int fstat64(struct stat64 *) const throw()
		{ return -EINVAL; }
};


ssize_t Socket_file::readv(const struct iovec* iov,
                           int iov_cnt) throw()
{
	ssize_t read_cnt = 0;

	if (!connected()) {
		errno = EINVAL;
		return -1;
	}

	for (int i = 0; i < iov_cnt; ++i)
		read_cnt += lwip_read(_lwip_fd, iov[i].iov_base,
							  iov[i].iov_len);

	return read_cnt;
}


ssize_t Socket_file::writev(const struct iovec* iov,
                            int iov_cnt) throw()
{
	ssize_t write_cnt = 0;

	if (!connected()) {
		errno = EINVAL;
		return -1;
	}

	for (int i = 0; i < iov_cnt; ++i)
		write_cnt += lwip_write(_lwip_fd, iov[i].iov_base,
								iov[i].iov_len);

	return write_cnt;
}


EXTERN_C int assign_fd_to_socket(int sockfd)
{
	using cxx::Ref_ptr;

	Ref_ptr<Socket_file> f(new Socket_file(sockfd));
	int vfs_fd = L4Re::Vfs::vfs_ops->alloc_fd(f);

	return vfs_fd;
}


EXTERN_C int socket_for_fd(int sock)
{
	using namespace cxx;

	// XXX: why doesn't ref_ptr_static_cast() work?
	Ref_ptr<L4Re::Vfs::File> vf = L4Re::Vfs::vfs_ops->get_file(sock);
	Ref_ptr<Socket_file> f = ref_ptr(static_cast<Socket_file*>(vf.ptr()));
	return f->lwip_fd();
}


EXTERN_C void mark_connected(int sock, int val)
{
	using namespace cxx;
	Ref_ptr<L4Re::Vfs::File> vf = L4Re::Vfs::vfs_ops->get_file(sock);
	Ref_ptr<Socket_file> f = ref_ptr(static_cast<Socket_file*>(vf.ptr()));
	f->connected(val == 1);
}
