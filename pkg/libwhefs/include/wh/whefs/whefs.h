#ifndef WANDERINGHORSE_NET_WHEFS_H_INCLUDED
#define WANDERINGHORSE_NET_WHEFS_H_INCLUDED

#include <wh/whefs/whefs_config.h>
#include <stddef.h>
#include <stdio.h>
#include <wh/whio/whio_dev.h>
#include <wh/whio/whio_stream.h>

//Doxygen won't allow us to have both @page and @mainpage, which is problematic
//when we re-use headers in different projects which also use doxygen.
//@page page_whefs_main whefs: Embedded/Virtual Filesystem
/** 
    @mainpage whefs: Embedded/Virtual Filesystem

   ACHTUNG: this is beta software!

   whefs is a C library implementing an embedded/virtual filesystem
   (EFS or VFS).  It works by creating a so-called container file. This API
   can then treat that container similarly to a filesystem. In essence
   this is similar to conventional archives (e.g. zip files), except
   that this library provides random-access read/write support to the
   fs via an API similar to fopen(), fread(), fwrite() and
   friends. Also, an EFS container is more like a real filesystem than
   (e.g.) a zip file because the container's size is fixed when it is
   created.

   Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

   License: Public Domain

   You can find MUCH more information about this library on its
   home page:

   http://code.google.com/p/whefs

*/

/** @page whefs_naming_conventions whefs API Naming conventions

The whefs API uses the following API naming conventions:

Functions and types all begin with the prefix "whefs_". Function
are always (or nearly always) declared in the form:

whefs_NOUN_VERB( NOUN theObject, ... )

The NOUN part(s) refer to an object or a member of an object, and the
VERB part is the action (or actions) to perform on the given object.

For example, the function to set an inode's name is
whefs_inode_name_set().  This is arguably less intuitive than the more
conventional-sounding whefs_inode_set_name() or
whefs_set_inode_name(), but this approach was chosen because:

- The ordering is natural if one thinks in an OO
mindset. e.g. inode.name.set(foo).

- When the API docs are generated in sorted order, it's easy to find
all operations for a given noun (object type). e.g. whefs_inode_name_set()
and whefs_inode_name_get() will be grouped together.

- Experience has shown it to be easily and consistently applicable,
and simple to remember because it follows a simple rule. "Was that
foo_set_name() or set_foo_name()?" Just remember that the noun comes
first. When there are multiple nouns (e.g. 'foo' and 'name'), they are
in descending order based on their parentage/ownership/API
significance.

The public whefs API is all declared in whefs.h, but the library
internally uses several headers which declare the private API -
functions and types which should never be used by client code. These
include all functions or types with the following prefixes:

- whefs_inode
- whefs_block
- whefs_file or whefs_fs, UNLESS it is declared in whefs.h (in which case it is public)

The internal API is subject to change at any time, and should never
be relied upon as a stable interface.

whefs is based off of another library, whio, and some of those details
"leak through" the whefs abstraction and are visible in the public
whefs API. While whio can be considered to be a private implementation
detail, it is nonetheless fully open to public use because it is an
independent API. whefs' usage of whio is compatible with the whio
public API and conventions, and there is no problem in mixing the APIs
in client code. For example, it is possible to use whio routines which
are not part of the whefs API on whefs-supplied i/o devices (e.g. the
whio gzip compression routines can be used on whefs-supplied i/o
devices).

*/
#ifdef __cplusplus
extern "C" {
#endif

/** @page page_whefs_file_format whefs container file format

The file format is described briefly (and informally) below...

(Begin file format...)

[CORE_MAGIC_BYTES] 4 integer values which must match the magic bytes expected
by a particular version. The 4th value of these bytes tells us whether we
use 16- or 32-bit inode and block IDs. Libraries compiled with a different
ID bit size will not be compatible.

[FILE_SIZE] 1 integer value. This provides a good sanity check when
opening an existing vfs.

[CLIENT_MAGIC_LENGTH] length (in bytes) of the following sequence...

[CLIENT_MAGIC_BYTES] "magic cookie" for this vfs. Determined by the client.

FS OPTIONS:
    - [BLOCK_SIZE] byte size of each block
    - [BLOCK_COUNT] number of blocks
    - [INODE_COUNT] number of "inodes" (filesystem entries)
    - [FILE_NAME_LENGTH]

[INODE_NAMES_TABLE]

Each inode name is composed of:

    - [TAG_BYTE] consistency-checking byte
    - [ID] stored only for consistency/error checking
    - [STRING_SIZE] number of used bytes encoded as a uint16_t.
    - The string data. Encoding is left to the client.

[INODE_LIST]

The inode list, 1 node per entry, starting with ID 1. ID 0 is
reserved for invalid/uninitialized inodes. ID 1 is reserved for a root
directory entry.

Each inode is stored as:

    - [TAG_BYTE] a consistency-checking byte
    - [ID]
    - [FIRST_BLOCK_ID]
    - [FLAGS]
    - [MODIFICATION_TIME]
    - [DATA_SIZE] the size of the associated pseudofile

[DATA BLOCKS] fixed-length blocks. Each block is stored as:

   - [TAG_BYTE] a sanity-checking byte.

   - [BLOCK_ID] not strictly necessary (we can calculate it), but it may
   be useful for error checking/correction. Starts at 1, not 0, as ID 0
   is reserved for invalid blocks.

   - [FLAGS] internal flags

   - [NEXT_BLOCK_ID] id of the next data block. Updated when a write()
   overflows a block or a file shrinks (thereby dropping blocks).

   - [BLOCK OF BYTES], a block for the data bytes of a pseudofile.
   The size is a property of the vfs object.

[EOF]

(...end file format)

For portability, numbers are stored in a platform-independent
manner. They are stored as char arrays, with a tag byte as the first
byte and the rest of the number stored in the following N bytes in
big-endian format, where N is the number of bytes in the numeric type
(e.g. uint32_t is stored as (1+4)=5 bytes and uint8_t is stored as two
bytes). The tag byte is used for data consistency checking, and each
encoded type has its own tag byte identifier which, by convention,
has a value greater than 127 (to help avoid false hits on ASCII input).

Reminder to self: storing the numbers little-ending might actually
make more sense because then we could use the decoding routines for
smaller data types on encoded data from larger data without having to
do any byte skipping or extra range checking. This is similar to the reason
which little-endian machines have fewer problems porting up to higher
bitness than big-endian machines have.


Limitations:

- We are stuck with 32 bit support only for now. i do not have a 64-bit
machine to play on. Also, we are currently using fseek() in some code,
and that is restricted to signed long values, so we have got a 2GB
container size limitation.

- File names are internally (char const *). Encoding/code
page is up to the user.

*/


/** @struct whefs_rc_t
   Functions in the api which return an int almost always return a
   value from the whefs_rc object. All members of this type must have
   a non-zero value, except for the OK member, which must be 0. The
   values and signedness for all error values is unspecified except
   for SizeTError, which is -1.
*/
typedef struct whefs_rc_t
{
    /**
       The non-error value, always equals 0.
    */
    int OK;

    /**
       Error in argument handling (e.g. unexpected arg type, count,
       etc.).
     */
    int ArgError;

    /**
       Read or write error.
    */
    int IOError;

    /**
       Memory allocation error.
    */
    int AllocError;

    /**
       Error parsing vfs magic bytes.
    */
    int BadMagicError;

    /**
       An internal error in the API.
    */
    int InternalError;

    /**
       An out-of-range error. e.g. wrong size or value. Also often
       used for "no item found", as in "not found in the current range
       of possibilities."
    */
    int RangeError;

    /**
       Signals that the EFS appears to be filled, either it has run
       out of inodes or free blocks.
    */
    int FSFull;

    /**
       The requested resource could not be accessed, or write
       permissions are required but denied.
    */
    int AccessError;

    /**
       The data in the vfs appears to be corrupted (or a bug
       in a read routine makes the data look corrupted even
       though it is not).
    */
    int ConsistencyError;

    /**
       Indicates that the called routine is not yet implemented.
    */
    int NYIError;

    /**
       Indicates that some unsupported operation was requested.
    */
    int UnsupportedError;
    /**
       This is equivalent to (whio_size_t)-1, and is used by routines which
       need an error value for a whio_size_t object.
    */
    whio_size_t SizeTError;

    /**
       This is equivalent to (whefs_id_type)-1, and is used by routines which
       need an error value for a whefs_id_type object. Some types/routines
       can use 0 for an error value, but search routines cannot because 0 might
       be a valid search result index.

       Remember that -1 is a different binary value depending on the
       number of bits in whefs_id_type.
    */
    whefs_id_type IDTypeEnd;
} whefs_rc_t;
/**
   A shared instance of whefs_rc_t which contains the "official"
   values of the common error codes for the whefs API.
*/
extern const whefs_rc_t whefs_rc;



/**
   Each vfs is tagged with a "magic cookie". This cookie is normally
   a descriptive string, such as "my vfs version 0.0.1", but may be
   arbitrary bytes.

   The value of having a custom magic cookie is arguable. i cannot
   personally see a real need for it, but it's easy enough to
   support and may be useful later. Theoretically, client apps
   could use the cookie space to store app-specific data, within
   the limitation that the cookie must have a constant number of
   bytes.
*/
typedef struct whefs_magic
{
    /**
       The length of this object's data member. The length
       must be greater than 0, and is conventionally quite
       small (e.g. 10-20 bytes).
    */
    uint16_t length;
    /**
       Must point to at least length bytes of data. The first
       length bytes are used as a magic cookie header.
    */
    unsigned char const * data;
} whefs_magic;

/**
   The default magic cookie used by whefs.
*/
extern const whefs_magic whefs_magic_default;


/** @struct whefs_fs_options

   whefs_fs_options defines the major parameters of an EFS. Once they
   are set, they must not be changed for the life of the EFS, as doing
   so invalidates the EFS and will lead to data corruption. (The public
   API provides no way for the client to change these options after
   a EFS is initialized.)

   Normally this type is used by clients only when creating a new EFS
   container file (e.g. via whefs_mkfs()). When opening an existing
   container, the options will be read in from the container.
*/
struct whefs_fs_options
{
    /**
       The magic cookie of the vfs.
    */
    whefs_magic magic;
    /**
       The size of each data block in the vfs.
    */
    whio_size_t block_size;
    //^^^ maintenance reminder: this cannot be bigger than whio_size_t.

    /**
       The number of blocks in the VFS. The implementation
       requires that this number be at least as large as
       inode_count. Future versions may allow this value to
       grow during the operation of the VFS.
    */
    whefs_id_type block_count;
    /**
       Number of "inodes" in the vfs. That is, the maximum number
       of files or directories. Each non-0-byte file/directory also
       takes up at least one data block.
    */
    whefs_id_type inode_count;
    /**
       The maximum filename length for pseudofiles in the EFS,
       including.  This cannot be changed after mkfs. It must be
       greater than 0 and less than or equal to
       WHEFS_MAX_FILENAME_LENGTH.
    */
    uint16_t filename_length;
};
typedef struct whefs_fs_options whefs_fs_options;

/**
   Initializer macro for inlined whefs_magic objects. Sets the magic
   to the given string.
*/
#define WHEFS_MAGIC_INIT(STR) { sizeof(STR) -1, (unsigned char const *)STR }

/**
   The default magic cookie used by the library.
*/
#define WHEFS_MAGIC_DEFAULT WHEFS_MAGIC_INIT(WHEFS_MAGIC_STRING)

/**
   A static initializer for an empty whefs_magic object.
*/
#define WHEFS_MAGIC_NIL { 0, (unsigned char const *)"\0" }

/**
   A static initializer for whefs_fs_options objects. It defaults
   the block_count to INODE_COUNT, which means to use the same value as
   inode_count.
*/
#define WHEFS_FS_OPTIONS_INIT(BLOCK_SIZE,INODE_COUNT,FN_LEN) \
    { WHEFS_MAGIC_DEFAULT, BLOCK_SIZE, INODE_COUNT, INODE_COUNT, FN_LEN }
/**
   Static initializer for whefs_fs_options object, using
   some rather arbitrary defaults.
*/
#define WHEFS_FS_OPTIONS_DEFAULT { \
    WHEFS_MAGIC_DEFAULT, \
    1024 * 8, /* block_size */ \
    128, /* block_count */ \
    128, /* node_count */ \
    64 /* filename_length */ \
    }
/**
   Static initializer for whefs_fs_options object, with
   all values set to 0.
*/
#define WHEFS_FS_OPTIONS_NIL { \
    WHEFS_MAGIC_NIL, \
    0, /* block_size */ \
    0, /* block_count */ \
    0, /* node_count */ \
    0 /* filename_length */ \
    }

/**
   A default configuration for whefs_fs_options. Can be used as a
   starting point for custom options.
*/
extern const whefs_fs_options whefs_fs_options_default;

/**
   A default configuration for whefs_fs_options with all values
   set to 0.
*/
extern const whefs_fs_options whefs_fs_options_nil;

/** @struct whefs_fs

   An opaque handle type for a virtual filesystem. This is the master
   handle to a vfs container.

   These objects are created using whefs_mkfs() or whefs_openfs().
   They are destroyed using whefs_fs_finalize().

   It is illegal to use any given whefs_fs object from multiple
   threads, whether concurrently or not.
*/
struct whefs_fs;
typedef struct whefs_fs whefs_fs;

/** @struct whefs_file

   whefs_file is an opaque handle type to a pseudo-file within
   a vfs container. They are created using whefs_fopen() and
   destroyed using whefs_fclose().

   Example:

   @code
   whefs_file * f = whefs_fopen( myFS, "my.file", "r+" );
   whefs_fwritef( f, "Hi, %s!", "world" );
   whefs_fclose( f );
   @endcode

*/
struct whefs_file;
typedef struct whefs_file whefs_file;


/**
   Opens a "pseudofile" within the given filesystem.

   mode is similar to the fopen() mode parameter, only the following
   are currently recognized:

   - "r" = read-only mode

   - "r+" = read/write mode, positioned at the start of the file.
   Currently, 'r+' will cause the file to be created if it does
   not exist, but this behaviour may change to more closely match
   that of fopen() (where 'w+' takes that role).

   Any other characters are currently ignored. There are no plans
   to support the various "append" options.

   To avoid potential "misinteractions", the VFS will not allow any
   given file to be opened in write mode more than once at a
   time. That is, once a file is opened in write mode, all other
   attempts to open it in write mode will fail. Multiple readers are
   allowed, even if the file is already opened for writing.

   On success, a new file handle is returned. On error, 0 is returned,
   but to discover the nature of the problem you'll have to use a
   debugger. (Hint: it's likely that the requested file wasn't found,
   could not be created (e.g. VFS full), is already opened for write
   mode, or any of 37 other potential errors. Just pick one.)

   Potential errors are:

   - allocation error while setting up the internal data.
   - (mode=="r") and no such entry is found
   - (mode=="r+"), the file is not found, and no free inode could be
   found (FS is full), or the file is already opened in write mode.
   - A general i/o error
   - The filesystem is opened read-only and read/write access was requested.

   Design notes:

   The main purpose of whefs_file is to provide an API which is close
   enough to the C FILE API that we can easily swap them out. That
   said, whefs_file is basically a very thin wrapper around a whio_dev
   specialization which is designed to work with whefs_fs. See whefs_fdev()
   for more information on that.

   Bugs:

   This routine is exceedingly inefficient - the lack of inode name
   caching means it must do an O(N) search, where N =
   whefs_fs_options_get(fs)->inode_count.

   @see whefs_fdev()
   @see whefs_dev_open()
*/
whefs_file * whefs_fopen( whefs_fs * restrict fs, char const * name, char const * mode );

/**
   Similar to whefs_fopen(), but returns a whio_dev instead of a
   whefs_file.  On success the caller owns the returned object, which
   must be destroyed by calling dev->finalize(dev). The device will be
   positioned at the beginning of the file.

   The returned object has the same overall behaviours as a whefs_file,
   and can be used in place of a whefs_file wherever it is convenient
   to do so (personally i prefer the whio_dev interface).

   If the filesystem is opened read-only then this operation will fail
   if writeMode is true.

   Peculiarities of the returned whio_dev object, vis-a-vis the whio_dev interface
   specifications, include:

   - write() updates the mtime and size of the associated inode, but
   does not flush that info to disk until flush() is called on the i/o
   device. Closing the device will automatically flush it. A read-only
   device will never update the on-disk state of the inode and shares
   a copy of the inode with all other files/devices opened for that
   inode (so it will see changes in the inode's size).

   - write()ing past EOF will allocate new VFS data blocks, if
   needed. If such an operation fails because it fills the filesystem,
   the device may have been partially expanded and may need to be
   manually truncated to free up VFS space.  This could convievably be
   considered to be a bug.

   - To keep pseudofiles from hammering each other's virtual cursor
   positions, all seek() requests are deferred until the next read or
   write. Thus a seek will only fail if the combination of arguments
   is bogus (e.g. a numeric overflow or a request to move before the
   start of the file). Because of this behaviour, seek() and tell()
   are O(1).

   @see whefs_fdev()
   @see whefs_fopen()
   @see whefs_dev_close()
*/
whio_dev * whefs_dev_open( whefs_fs * restrict fs, char const * name, bool writeMode );

/**
   Similar to whefs_dev_open(), but returns a whio_stream object
   instead of a whio_object. If writeMode is true and append is true
   then the pseudofile's cursor is placed at the end of the file. If
   writeMode is true and append is false then the file is truncated to
   zero bytes. If writeMode is false then the append argument is
   ignored.

   Unlike whefs_fopen(), there is no way to get a handle to the
   underlying random-access i/o device. If you need access to both a
   streaming and random-access for a given pseudofile, you can open it
   using whefs_dev_open() or whefs_fopen(), then pass the device
   object to whio_stream_for_dev(). Note, however, that mixing random
   and sequential access that way may lead to confusing or incorrect
   results, as the objects share an underlying file cursor.
*/
whio_stream * whefs_stream_open( whefs_fs * fs, char const * name, bool writeMode, bool append );

/**
   Returns the i/o device associated with f, or 0 if !f. 

   The returned object is owned by f and will be destroyed when f is
   closed via whefs_fclose().

   Each whefs_file has an associated i/o device to abstract away the
   interaction with the underlying whefs_fs object. This is a
   "pseudodevice" which indirectly uses the underlying storage of the
   vfs but has its own storage range (internal to the vfs) and is
   designed to not read or write outside of the bounds of that
   range. In general, reading and writing and such are simpler (IMO)
   via the whio_dev interface, but the whefs_fXXXX() API will be more
   familiar to long-time C users. Porting code from the standard FILE
   API to this API is normally trivial.

   @see whefs_dev_open()
   @see whefs_fopen().
*/
whio_dev * whefs_fdev( whefs_file * restrict f );


/**
   Returns the file's current cursor position, or whefs_rc.SizeTError
   if !f or the combination of pos and whence is not legal. See
   whio_dev::seek() for the full documentation and possible error
   conditions.

   Note that it is generally legal to seek past EOF, but the size of
   the file will not be changed unless a write actually happens past
   EOF. This behaviour is technically device-dependent, but all
   current device implementations work that way.
*/
whio_size_t whefs_fseek( whefs_file * restrict f, size_t pos, int whence );

/**
   "Rewinds" the file pointer back to its starting position.
   Returns whefs_rc.OK on success, else some other value.
*/
int whefs_frewind( whefs_file * restrict f );

/**
   Returns the current size of the given file, or whefs_rc.SizeTError
   if !f or on an internal error (which "shouldn't happen").

   It is actually more efficient (faster) to use
   whio_dev_size(whefs_fdev(f)), but this implementation is different
   in that it respects the constness of f.
*/
whio_size_t whefs_fsize( whefs_file const * restrict f );

/**
   Truncates the given pseudofile to the given length.

   On success, whefs_rc.OK is returned, otherwise some other value is
   returned. Read-only files cannot be truncated and will result in
   whio_rc.AccessError being returned.

   If the file grows then all bytes between the file's previous EOF
   (not its current position) and the new EOF are zeroed out.

   If the file shrinks then all data after the new EOF, but which
   previously belonged to the file, are also zeroed out.
*/
int whefs_ftrunc( whefs_file * restrict f, size_t newLen );

/**
   Closes f, freeing its resources. After calling this, f is an invalid object.
   Returns whefs_rc.OK on success, or whefs_rc.ArgError if (!f).
 */
int whefs_fclose( whefs_file * restrict f );

/**
   Similar to whefs_fclose() but closes a device opened with whefs_dev_open().

   DO NOT pass this function a device which was fetched using
   whefs_fdev()! Use whefs_fclose() to close the file instead. Failing
   to follow this rule may lead to undefined results when the
   associated file handle dangles around and a double-delete if the
   file is closed afterwards.

   ONLY use this for devices opened with whefs_dev_open().

   For example:

   @code
   // The proper way:
   whio_dev * dev1 = whefs_dev_open(...);
   ...
   whefs_dev_close( dev1 ); // Correct!

   // And when using whefs_file handles:
   whefs_file * f = whefs_fopen(...);
   whio_dev * dev2 = whefs_fdev(f);
   ...
   //whefs_dev_close(dev2); // WRONG!!! This will lead to problems! Instead do:
   whefs_fclose( f ); // CORRECT! dev2 is owned by f and is cleaned up by this.
   @endcode

   @see whefs_dev_open()
*/
int whefs_dev_close( whio_dev * restrict dev );

/**
   Unlinks (deletes) the given file and its associated inode. It there
   are open handles to the file the deletion will fail.

   For data security/integrity reasons, this also zeroes out all data
   on all blocks owned by the inode, which makes unlinking an O(N)
   operation, with N being a function of the block count and their
   size.

   On success whefs_rc.OK is returned. If the file is opened then
   whefs_rc.AccessError is returned. Any other error would be i/o
   related or caused by inconsistent internal VFS state.

   Bugs:

   The choice to not allow deletion when there are open handles is to
   avoid some potential errors. We should (maybe) simply defer the
   unlink until all handles are closed. The problem with that is if
   the handles are not properly closed (e.g. due to a crash or the
   client doesn't close them for some reason), the deferred unlink
   won't ever happen. (Unless we store it in a journal of some time,
   but that's a whole other can of worms.)
*/
int whefs_unlink_filename( whefs_fs * restrict fs, char const * fname );

/**
   Equivalent the fwrite() C function, but works on whefs_file handles.

   Writes n objects from src, each of bsize bytes, to f. Returns the number
   of successfully written, which will e less than n on error.

   When writing a single block of memory (e.g. a string), it is
   significantly more efficient to pass the data length as the bsize
   parameter and n to 1 than it is to set n to the data length and
   bsize to 1. The end effect is the same, but the overhead of the
   latter is much higher than the former.

   @see whefs_fread()
   @see whefs_file_write()
   @see whefs_file_read()
*/
size_t whefs_fwrite( whefs_file * restrict f, size_t bsize, size_t n, void const * src );

/**
   Writes a printf-style formatted string to f. Returs the number of
   bytes written. It is, in general case, impossible to know if all
   data was written correctly.
*/
size_t whefs_fwritev( whefs_file * restrict f, char const * fmt, va_list vargs );

/**
   Equivalent to whefs_fwritev() except that it takes elipsis arguments
   instead of a va_list.
 */
size_t whefs_fwritef( whefs_file * restrict f, char const * fmt, ... );

/**
   Reads n objects, each of bsize bytes, from f and writes them to dest.
   Returns the number of objects successfully written, which will be
   less than n on error.

   @see whefs_fwrite()
   @see whefs_file_write()
   @see whefs_file_read()
*/
size_t whefs_fread( whefs_file * restrict f, size_t bsize, size_t n, void * dest );

/**
   Similar to whefs_fwrite(), but takes arguments in the same
   convention as the write() system call (which incidentally is the
   same convention used by whio_dev). Returns the number of bytes
   written, which will be less than n on error.

   @see whefs_fwrite()
   @see whefs_file_read()
*/
size_t whefs_file_write( whefs_file * restrict f, void const * src, size_t n  );

/**
   Similar to whefs_fread(), but takes arguments in the same convention
   as the read() system call (which incidentally is the same convention
   used by whio_dev). Returns the number of bytes read, which will be
   less than n on error.

   @see whefs_file_write()
   @see whefs_fread()
*/
size_t whefs_file_read( whefs_file * restrict f, void * dest, size_t n  );

/**
   Returns the vfs associated with the given file, or 0 if !f.
*/
whefs_fs * whefs_file_fs( whefs_file * restrict f );

/**
   Flushes the underlying i/o device. Flushing a whefs_file will also
   flush the VFS, so calling this is normally not needed.

   Returns whefs_rc.ArgError if !fs or fs has no backing store,
   whefs_rc.AccessError if fs is read-only, else the result is the
   same as calling whio_dev::flush() on the underlying i/o device.

   If/when the VFS caches any information (e.g. to synchronize inode
   use across multiple open handles) then this function will also
   write that cache out to disk.
*/
int whefs_fs_flush( whefs_fs * restrict fs );

/**
   Flushes the file's data to disk, if necessary. A read-only
   file will not flush but a success code is returned in that case.

   Note that the underlying implementation does not buffering, so syncing
   is not required for the sake of f itself. Instead, the flush will
   propogate down the i/o device chain until it gets to the underlying
   storage, which may very well be buffering.

   On success, whefs_rc.OK is returned, otherwise whefs_rc.ArgError
   (if (!f)) or a device-specific error code from the underlying call
   to flush().

   In practice, one doesn't normally need to manually flush whefs_file
   objects.  The underlying i/o process must flush changes made to
   inodes and data blocks, and that flushing will be going on behing
   the scenes often enough to not need to use it from client code.

*/
int whefs_fflush( whefs_file * restrict f );

/**
   Creates a new vfs using the given filename and options. The resulting
   vfs handle is assigned to tgt.

   The contents of filename will be mercilessly overwritten. As a
   special case, the filename ":memory:" (case-sensitive) will instead
   cause an in-memory buffer to be allocated. In that case, we must be
   able to allocate whefs_fs_calculate_size() bytes for the backing
   storage (plus memory for the vfs-related objects) or this routine
   will fail.

   On success, whefs_rc.OK is returned and tgt is assigned to the new
   whefs_fs object, which the caller must eventually destroy by calling
   whefs_fs_finalize().

   The new vfs object uses a copy of the opt object, so the lifetime
   of the opt object is not significant, nor will changes to that
   object affect the vfs.

   On error, tgt is not modified and one of these whefs_rc error codes
   is returned:

   - ArgError = either filename, opt, or tgt are 0.

   - RangeError = the values in the opt object are out of range. See
   below.

   - AllocError = an allocation error occured (out of memory).

   - AccessError = the specifies file could not be opened for writing.

   - IOError = an i/o error happened while writing the vfs. The
   resulting file is almost certainly corrupt (or incomplete) and
   should be deleted (this code does not delete it, however).

   A RangeError signifies that some value in the opt object is out of range.
   The currently allowed ranges are:

   - block_size >= 32 (which was arbitrarily chosen). Except when
   storing only very small data, a block size of 4k-32k is more
   realistic. Very small block sizes are probably only useful for
   testing the file-pos-to-data-block mapping in the i/o code.

   - inode_count >= 2, as 1 inode is reserved for the root directory, and
   a filesystem without at least 1 free inode is effectively full.

   - block_count must be at least as big as inode_count, because inodes
   which can never be mapped to a block are fairly useless (they can only
   be used as 0-byte files).

   All other values must be non-0.

   It is possible to calculate the size of a vfs container before creating
   it by calling whefs_fs_calculate_size() and passing it the whefs_fs_options
   object which you intend to pass to whefs_mkfs().
*/
int whefs_mkfs( char const * filename, whefs_fs_options const * opt, whefs_fs ** );

/**
   Equivalent to whefs_mkfs() but takes an i/o device as the
   target. The device must be writable and must be able to grow to the
   required size. The existing contents will be destroyed.

   If takeDev is true then on success ownership of dev is passed to
   the object assigned to to tgt, and dev will be destroyed when tgt
   is destroyed. On failure, ownership of dev does not change and tgt
   is not changed.
*/
int whefs_mkfs_dev( whio_dev * dev, whefs_fs_options const * opt, whefs_fs ** tgt, bool takeDev );

/**
   Opens an existing vfs container file.

   On success, whefs_rc.OK is returned and tgt is assigned to the new
   whefs_fs object, which the caller must eventually destroy by calling
   whefs_fs_finalize().

   If writeMode is false then the underlying file is opened read-only.
*/
int whefs_openfs( char const * filename, whefs_fs ** tgt, bool writeMode );

/**
   Similar to whefs_openfs(), but works on an existing i/o device. The
   device must contain a whefs filesystem.

   On success, whefs_rc.OK will be returned and tgt is assigned to the
   newly-created whefs_fs object. Ownership of tgt is then transfered to
   the caller, who must eventually finalize it by passing it to
   whefs_fs_finalize().

   On error, non-whefs_rc.OK wil be returned, tgt will not be
   modified, and ownership of dev does not change.

   If takeDev is true then on success ownership of dev is transfered to
   tgt, such that dev will be finalized when tgt is destroyed.

   Error conditions:

   - !dev or !tgt

   - dev does not contain a readable whefs.

   If dev shares storage with any other objects (be they whio_dev
   objects or otherwise), in particular if more than one have write
   access, the filesystem will in all likelyhood be quickly corrupted.
*/
int whefs_openfs_dev( whio_dev * restrict dev, whefs_fs ** tgt, bool takeDev );

/**
   Cleans up all resources associated with fs. After calling this,
   fs will be an invalid pointer.

   If file/device handles are opened via whefs_fopen(),
   whefs_dev_open(), etc. and they are not closed by the time this is
   called, then they will be closed by this routine. If that happens,
   the client must NOT close them again after calling this - doing so
   will lead to stepping on null pointers or otherwise invalid objects.
*/
void whefs_fs_finalize( whefs_fs * restrict fs );

/**
   Returns the options associated with fs, or 0 if !fs.
*/
whefs_fs_options const * whefs_fs_options_get( whefs_fs const * restrict fs );

/**
   A shorter name for whefs_fs_options_get().
*/
whefs_fs_options const * whefs_fs_opt( whefs_fs const * restrict fs );

/**
   A debug-only function for showing some information about a
   vfs. It's only for my debugging use, not client-side use.
*/
void whefs_fs_dump_info( whefs_fs const * restrict fs, FILE * out );

/**
   Calculates the size of a vfs container based on the given
   options. This does not account for any internal bookkeeping memory,
   just the size of the container image. This can be used, e.g., to
   allocate a buffer for an in-memory whio_dev implementation, or to
   ensure that enough filesystem space (or memory) is available.

   The size of a container is based on:

   - the number of inodes
   - the maximum length of inode names
   - the size of each data block
   - the size of the magic cookie
   - a few internal bookkeeping and consistency checking details

   Once a container is created its size must stay constant. If it is
   changed it will effectively corrupt the vfs.

   If !fs then 0 is returned (0 is never a valid size of a vfs).
*/
whio_size_t whefs_fs_calculate_size( whefs_fs_options const * opt );

/**
   A debuggering routine which dumps fs to the given FILE. This is
   mainly intended for dumping in-memory vfs containers to a file for
   examination or persistency purposes.

   On success it returns whefs_rc.OK. On error may return:

   whefs_rc.ArgError = !fs or !outstr, or fs is not attached to a device.

   whefs_rc.IOError = copying to outstr failed.

   Ownership of outstr is not changed, and this routine does not close
   the stream.
*/
int whefs_fs_dump_to_FILE( whefs_fs * restrict fs, FILE * outstr );

/**
   Equivalent to whefs_fs_dump_to_FILE() except that it takes a file name
   instead of a FILE handle. In addition to the return codes from that
   routine, it may return whefs_rc.AccessError if it cannot open the
   file for writing.
*/
int whefs_fs_dump_to_filename( whefs_fs * restrict fs, char const * filename );


/**
   A type for reporting certain whefs_file statistics.

   @see whefs_fstat().
*/
typedef struct whefs_file_stats
{
    /**
       Size of the file, in bytes.
    */
    whio_size_t bytes;

    /**
       inode number.
    */
    whefs_id_type inode;

    /**
       Number of blocks used by the file.
    */
    whefs_id_type blocks;
} whefs_file_stats;


/**
   Populates st (which may not be null) with some statistics for f
   (which also may not be null but may be read-only).

   Returns whefs_rc.OK on success. On error some other value
   is returned and st's contents are in an undefined state.

   Example usage:

   @code
   whefs_file_stats st;
   if( whefs_rc.OK == whefs_fstat( myFile, &st ) ) {
       ... use st ...
   }
   @endcode
*/
int whefs_fstat( whefs_file const * f, whefs_file_stats * st );

/**
   Renames the given file, which must be opened in read/write mode, to
   the new name. On success, whefs_rc.OK is returned, else some other value
   is returned.

   Errors include:

   - whefs_rc.ArgError = either f or newName are null or newName is
   empty.

   - whefs_rc.AccessError = f is not opened in write mode.

   - whefs_rc.RangeError = newName is longer than the smaller of
   whefs_fs_options_get(whefs_file_fs(f))->filename_length or
   WHEFS_MAX_FILENAME_LENGTH.

   - Some other error = something weird happened.
*/
int whefs_file_name_set( whefs_file * restrict f, char const * newName );

/**
   Returns the given file's current name, or 0 if !f or on some weird
   internal error. The memory for the string is owned by the efs
   internals and will be deallocated when the last handle to the file
   is closed (or possibly before then, if the internals are
   reallocated for some reason). The caller is advised to copy it if
   there is any question at all about its lifetime.

   The returned string is guaranteed to be no longer than
   whefs_fs_options_get(whefs_file_fs(f))->filename_length
   characters (not including the trailing null).


   Maintenance reminder: f is only non-const for very silly
   internal reasons (namely whefs_file::name, which needs
   to go away).
*/
char const * whefs_file_name_get( whefs_file * restrict f );

/**
   Returns a static string with the URL of the whefs home page.
*/
char const * whefs_home_page_url();

/**
   Returns a static string containing the file format version
   supported by this version of the library.
*/
char const * whefs_data_format_version_string();

/**
   Sets the library's debugging stream (which may be 0). This only works if
   the lib is compiled with the debugging support enabled.

   Pass a flags val of 0 to disable all debug output, otherwise pass
   a value used by the internal debugging routines. Pass -1 to use
   the default debugging flags, wh
*/
void whefs_setup_debug( FILE * ostream, unsigned int flags );

/**
   Identical whefs_setup_debug(), but takes the debug flags as a
   string, for use primarily in main() argument handling. Each letter
   (case-sensitive) represents one category of debugging message:

    'a' = All messages.

    'c' = Caching messages.

    'd' = Default log level. Typically warnings and errors.

    'e' = Error messages.

    'f' = FIXME messages.

    'h' = Hacker-level messages. Turns on several other options.

    'l' = Locking messages.

    'n' = NYI (Not Yet Implemented) messages.

    'w' = Warning messages.


    Unknown characters are ignored.
   
*/
void whefs_setup_debug_arg( FILE * ostream, char const * arg );


/**
   Returns an array of numbers which represent this library's file
   format version number. The bytes are conventionally { YYYY, MM, DD,
   WHEFS_ID_TYPE_BITS }, where YYYY, MM, and DD are the year/month/day
   of the format change (and WHEFS_ID_TYPE_BITS is documented
   elsewhere).

   The returned result is guaranteed to not be null and to have a value
   of 0 as the end-of-array entry.
*/
const uint32_t * whefs_get_core_magic();

/**
   Experimental and only lightly tested.

   Tries to add count blocks to the given fs, appending them to the
   end of the storage device. On failure, it tries to restore the fs
   to its previous state, but it may not be able to do so 100%
   correctly.

   On success whefs_rc.OK is returned and fs is expanded by the given
   number of blocks.

   If !fs or !count then whefs_rc.ArgError is returned.

   If fs is not opened for read/write whefs_rc.AccessError
   is returned.

   Other errors may be returned if something goes wrong during the
   i/o.
*/
int whefs_fs_append_blocks( whefs_fs * restrict fs, whefs_id_type count );

/**
   Returns true if fs was opened in read/write mode, else false.
*/
bool whefs_fs_is_rw( whefs_fs const * restrict fs );

/**
   Removes the least-visited items from the inode names cache,
   possibly freeing some memory, using a simple heuristic: it sorts by
   how often the item was searched for, lops off the bottom half, and
   re-sorts on the internal lookup key.

   This normally isn't needed. This *might* be useful if an EFS has a
   huge number of *used* inodes (thousands) and only a few of them are
   actually needed by the application. Some memory *might* be released
   by shrinking the list, but not enough for most applications to be
   concerned about (none is leaked, in any case). The search speed
   improves because the cache shrinks, and new i/o won't be caused due
   to lookups by name until an item which was removed from the cache
   is re-added.
*/
int whefs_inode_hash_cache_chomp_lv( whefs_fs * fs );

/**
   This function allows one to toggle the "inode names hash cache".
   If on is true then the cache is enabled, else it is disabled.  If
   on is true and loadNow is true then the cache is fully populated
   immediately. If loadNow is false then the cache is built up
   incrementally as inodes are searched for by name. If on is false then
   loadNow is ignored and any current cache is deallocated.

   This cache costs approximately 8 bytes per object, assuming
   WHEFS_ID_TYPE_BITS==16, so it is not recommended for huge
   EFSes. For small ones it doesn't cost much.

   When the cache is enabled, searches for inodes by their name
   will be reduced from O(N) (N=number of inodes) to O(log N)
   once a given name has been cached (by being traversed once,
   perhaps during the search for another inode).

   On success whefs_rc.OK is returned. The only error conditions are:

   - If !fs then whefs_rc.ArgError is returned.

   - If the cache is enabled and memory cannot be allocated or population
   of the cache fails due to an i/o error, that error is propagated back.

   If an error happens then the cache will be disabled, under the assumption
   that it could not be properly initialized. This is not a fatal error,
   but may cause a performance hit on inode searches by name.

   If WHEFS_CONFIG_ENABLE_STRINGS_HASH_CACHE (defined in
   whefs_config.h) is set to true then this cache is enabled by
   default but not pre-loaded, otherwise it is disabled by default.
*/
int whefs_fs_setopt_hash_cache( whefs_fs * fs, bool on, bool loadNow );

/**
   By default if a whefs_fs object is closed while pseudofile handles
   are still opened then they will be properly closed at that time to
   avoid memory leaks. However, if the client properly closes such
   handles after the owning whefs_fs has been destroyed, undefined
   behaviour will result and a segfault is certain.

   If this option is turned off, opened handles are not closed
   automatically when the whefs_fs has closed. After that, the objects
   MUST NOT be destroyed via the normal means (and their memory is
   leaked).

   The original intention of toggling this off was to assist in
   certain cases when binding whefs to scripting engines where a
   garbage collector is involved and can complicate proper destruction
   order. Whether or not it's really necessary is a whole other
   question, though.
*/
int whefs_fs_setopt_autoclose_files( whefs_fs * fs, bool on );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WANDERINGHORSE_NET_WHEFS_H_INCLUDED */
