/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#define LWIP_COMPAT_SOCKETS 0

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <string.h>
#include "sock_fs.h"

int socket(int domain, int type, int protocol);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen);
int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen);
int listen(int sockfd, int backlog);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, unsigned int flags);
int shutdown(int sockfd, int how);
struct hostent *gethostbyname(const char *name);
int gethostname(char *name, size_t len);


/* XXX
 * BD: Actually, we only need this, if we intermix uclibc and lwip
 *     socket functions which seems to be a bad idea anyway?
 */
static void __attribute__((unused))
uclibc_to_lwip_sockaddr(struct sockaddr *addr)
{
  unsigned char *a = (unsigned char *)addr;
  unsigned char x = a[0];
  a[0] = a[1];
  a[1] = x;
}


/* Wrappers */
int socket(int domain, int type, int protocol)
{
  int sockfd = lwip_socket(domain, type, protocol);
  return assign_fd_to_socket(sockfd);
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  int sock = socket_for_fd(sockfd);
  int ret = lwip_connect(sock, addr, addrlen);
  if (ret == 0)
    mark_connected(sockfd, 1);

  return ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  int sock = socket_for_fd(sockfd);
  int ret = lwip_accept(sock, addr, addrlen);
  return ret;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  int sock = socket_for_fd(sockfd);
  int ret = lwip_bind(sock, addr, addrlen);
  if (ret == 0)
    mark_connected(sockfd, 1);
  return ret;
}

int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen)
{
  int sock = socket_for_fd(sockfd);
  return lwip_getsockopt(sock, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen)
{
  int sock = socket_for_fd(sockfd);
  return lwip_setsockopt(sock, level, optname, optval, optlen);
}

int listen(int sockfd, int backlog)
{
  int sock = socket_for_fd(sockfd);
  return lwip_listen(sock, backlog);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
  int sock = socket_for_fd(sockfd);
  return lwip_recv(sock, buf, len, flags);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
  int sock = socket_for_fd(sockfd);
  return lwip_send(sock, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
  int sock = socket_for_fd(sockfd);
  return lwip_sendto(sock, buf, len, flags, dest_addr, addrlen);
}

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  int sock = socket_for_fd(sockfd);
  return lwip_getsockname(sock, addr, addrlen);
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  int sock = socket_for_fd(sockfd);
  return lwip_getpeername(sock, addr, addrlen);
}

int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, unsigned int flags)
{
  printf("Unimplemented: %s(%p, %d, %p, %d, %p, %d, %d)\n", __func__,
         sa, salen, host, hostlen, serv, servlen, flags);
  return -4;
}

int shutdown(int sockfd, int how)
{
  printf("Unimplemented: %s(%d, %d)\n", __func__, sockfd, how);
  errno = -EBADF;
  return -1;
}

struct hostent *gethostbyname(const char *name)
{
  return lwip_gethostbyname(name);
}

int gethostname(char *name, size_t len)
{
  const char const *my_fine_hostname = "l4re-host";
  strncpy(name, my_fine_hostname, len);
  name[len - 1] = 0;
  return 0;
}
