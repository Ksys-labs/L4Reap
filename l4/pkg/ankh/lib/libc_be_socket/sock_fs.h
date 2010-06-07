#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

/*
 * Socket file API
 */

/*
 * Assign a VFS file descriptor to an lwIP socket
 */
extern int assign_fd_to_socket(int);

/*
 * Lookup socket for file descriptor
 */
extern int socket_for_fd(int);

/*
 * Mark existing VFS fd as connected socket
 */
extern void mark_connected(int,int);

EXTERN_C_END
