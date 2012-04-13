#if !defined(WANDERINGHORSE_NET_WHEFS_INODE_H_INCLUDED)
#define WANDERINGHORSE_NET_WHEFS_INODE_H_INCLUDED 1
/*
  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

  License: Public Domain

  This file contains the whefs_inode parts of the whefs
  private/internal API.
*/
#include <wh/whefs/whefs.h>
#include <wh/whefs/whefs_string.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @struct whefs_block

whefs_block holds the metadata for the data blocks of an EFS, but does
not actually hold the client data.
*/
struct whefs_block
{
    /**
       Sequential id number. Value of 0 is reserved for "invalid block".
    */
    whefs_id_type id;
    /** id of next block */
    whefs_id_type next_block;
    /**
       Internal flags.

       An interesting note - i went through the trouble of reducing
       this to a uint8 to save a byte and it turns out it doesn't save
       any memory because sizeof() will still be the next multiple of two.
    */
    uint8_t flags;
};
typedef struct whefs_block whefs_block;

/** @def whefs_block_empty_m

    Empty static initializer for whefs_block objects.
 */
#define whefs_block_empty_m  \
    { \
    0U, /* id */ \
    0U, /* next_block */                     \
    0U /* flags */ \
    }

/** Empty initialization object for whefs_block objects. */
extern const whefs_block whefs_block_empty;

/** @def WHEFS_INODE_RELATIVES

This is going away - don't use it.

WHEFS_INODE_RELATIVES determines whether the underlying support for
directories/subdirectories is turned on. This changes the filesystem
layout and the in-memory size of inode entries.

*/
#define WHEFS_INODE_RELATIVES 0

/** @struct whefs_block_list

Holds the list of blocks for opened inodes.

*/
typedef struct whefs_block_list
{
    /** Array of objects. */
    whefs_block * list;
    /** Number of items allocated. */
    whefs_id_type alloced;
    /** Number of items used. */
    whefs_id_type count;
} whefs_block_list;

/**
   Empty initialization object. Convenience macro for places where a
   whefs_block_list object must be statically initialized.
*/
#define whefs_block_list_empty_m {0,0,0}
/**
   Empty initialization object.
*/
extern const whefs_block_list whefs_block_list_empty;

/** @struct whefs_inode

This type is for internal API use only. Higher-level abstractions
will someday be provided for client-side use.

For most intents and purposes, a whefs_inode object can be considered
to be a file entry in an EFS.

Members documented as being "persistant" are saved in an EFS. Those
marked as "transient" are only used in-memory.
*/
typedef struct whefs_inode
{
    /**
       Sequential id number. Value of 0 is reserved for "invalid
       inode" and ID 1 is reserved for the root node entry of an EFS.
       Persistant.
    */
    whefs_id_type id;

    /**
       Flags from the whefs_flags enum. Persistant.
    */
    uint8_t flags;

    /**
       ID of first block. Persistant.
    */
    whefs_id_type first_block;

    /**
       EOF position (i.e. size of the associated data). Persistant.
    */
    uint32_t data_size;

    /**
       Timestamp of last write/change to the inode. This type may
       change at some point (to uint64_t). Persistant.
     */
    uint32_t mtime;

    /** Used by the open filehandle tracker. Transient. */
    uint16_t open_count;
    /** Is used to mark write ownership of an inode. Transient. */
    void const * writer;
    /**
       This is used by whefs_block_for_pos() (the heart of the i/o
       routines) to keep the block list for an opened inode in
       memory. This saves boatloads of i/o for common use cases.
       Transient.
    */
    whefs_block_list blocks;
    /** Transient string used only by opened nodes. */
    //whefs_string name;
} whefs_inode;

/** Empty inode initialization object. */
#define whefs_inode_empty_m { \
	0, /* id */ \
	WHEFS_FLAG_Unused, /* flags */ \
        0, /* first_block */ \
        0, /* data_size */ \
        0, /* mtime */ \
        0, /* open_count */ \
        0, /* writer */ \
	whefs_block_list_empty_m /*blocks */ \
    }
/** Empty inode initialization object. */
extern const whefs_inode whefs_inode_empty;

/** @struct whefs_inode_list

   whefs_inode_list is a doubly-linked list of inode objects, used for
   holding a list of "opened" inodes.
*/
typedef struct whefs_inode_list
{
    whefs_inode inode;
    struct whefs_inode_list * next;
    struct whefs_inode_list * prev;
} whefs_inode_list;
/** Empty inode_list initialization object. */
#define whefs_inode_list_empty_m { whefs_inode_empty_m, 0, 0 }
/** Empty inode_list initialization object. */
extern const whefs_inode_list whefs_inode_list_empty;


/**
   Reads a whefs_inode's metadata, except for its name, from
   disk. Neither nid nor tgt may be 0. tgt must point to a valid
   object but its contents are irrelevant (they will be
   overwritten). On success, whefs_rc.OK is returned and tgt is
   updated to the on-disk state, otherwise some other value is
   returned and tgt is left in an undefined state (that is, possibly
   partially populated).

   Ownership of tgt is not changed by calling this function.

   Typical usage:

   @code
   whefs_inode ino = whefs_inode_empty;
   whefs_inode_id_read( fs, 42, &ino );
   @endcode

   WARNING: if nid is already opened, then tgt is overwritten with a
   copy of that object (possibly getting older data if the inode
   hasn't been flushed recently). This means that updates to
   tgt->open_count, tgt->blocks (which is NOT copied), etc. will not
   be accurately synced between tgt and the original copy, which can
   lead to bugs if updates are made directly to tgt. If you want to
   update an opened inode, use whefs_inode_search_opened() to see if
   the node is opened.

   The name of the inode is not fetched because it would require (A) a
   malloc() and (B) that the client clean it up (and most callers of
   this routine do not use the name). To fetch the name, use
   whefs_inode_name_get().

   @see whefs_inode_name_get()
*/
int whefs_inode_id_read( whefs_fs * fs, whefs_id_type nid, whefs_inode * tgt );

/**
    Don't use this function - it's probably going away.

   Reads the flags field of the given inode and assigns the flags
   argument to their value. fs may not be null and nid must be a valid
   inode id for fs or the routine will fail. flags may be 0, in which
   case this is simply a very elaborate way to tell if an inode is
   valid.

   On success flags is modified (if it is not null) and whefs_rc.OK is
   returned. On failure flags is not modified and non-OK is returned.
   
   This routine returns the fs i/o device to its original position
   when it is done, so calling this behaves "as if" the cursor has not
   moved. The one exception is if the seek to the correct inode fails,
   in which case the cursor position is in an undefined state and
   error recovery must begin (writing at that point may corrupt the
   vfs).

   If the given inode ID is currently opened, the flags are taken
   from the opened copy and no i/o is necessary.
*/
//int whefs_inode_read_flags( whefs_fs * fs, whefs_id_type nid, uint32_t * flags );

/**
   "Opens" an inode for concurrent (but NOT multi-threaded!) access
   WITHIN ONE PROCESS, such that the node will be shared by open file
   and device handles. That is, two calls to open the same inode will
   both return a handle pointing to the same copy of the inode.

   This should be the only function the remaining API uses to "open
   up" an inode.

   On success whefs_rc.OK is returned tgt is set to a pointer to the
   shared inode. The pointer is owned by fs but must be closed by
   calling whefs_inode_close().

   On error tgt is not modified and some other value is
   returned. Errors can be:
   
   - nodeID is not a valid node.
   - could not allocate space for shared inode.
   - !fs or !tgt

   If the inode is to be opened read-only, pass 0 for the writer argument.
   If the inode is to be opened with write mode enabled, writer must be
   an opaque value which uniquely identifies the writer (e.g. the owning
   object). An attempt to open an inode for a second or subsequent writer
   will fail with whefs_rc.AccessError.

   @see whefs_inode_close()
*/
int whefs_inode_open( whefs_fs * fs, whefs_id_type nodeID, whefs_inode ** tgt, void const * writer );

/**
   The converse of whefs_inode_open(), this unregisters a reference to
   the given inode. src and writer MUST be the same pointers as
   returned from resp. passed to whefs_inode_open() (NOT addresses to
   copied objects) or this operation will fail and client code may end
   up leaking one opened inode handle.

   On success, whefs_rc.OK is returned. The src object was allocated
   by whefs_inode_open() and will be cleaned up here once the open
   count goes to zero.

   The writer argument is an arbitrary client pointer which is used to
   tag who is the write-mode owner of the inode. If (writer != 0) and
   (writer == src->writer) then the inode is flushed to disk as part
   of the closing process. writer may be 0 to signify read-only access,
   but the calling code is required to enforce access.

   @see whefs_inode_open()
*/
int whefs_inode_close( whefs_fs * fs, whefs_inode * src, void const * writer );

/**
   Writes n to disk. If any arguments are 0 then whefs_rc.ArgError
   is returned. If n->id is 0 or more than fs->options.inode_count then
   whefs_rc.RangeError is returned.

   On success, whefs_rc.OK is returned.

   Note that this does not flush the inode's name, as that is handled
   separately. See whefs_inode_name_set().

   WARNING: if n was obtained as a COPY of an opened inode (via
   whefs_inode_read()) then the opened copy will overwrite these
   changes when it is next flushed.

   @see whefs_inode_name_set()
*/
int whefs_inode_flush( whefs_fs * fs, whefs_inode const * n );

/**
   Searches for the next free inode and assigns tgt to that value. If markUsed
   is true then the inode is also marked as used, otherwise a subsequent call
   to this function might return the same inode.

   On success it updates tgt and returns whefs_rc.OK. On error it
   returns some other value and does not update tgt. If it traverses the whole
   list and cannot find a free node it returns whefs_rc.FSFull.
*/
int whefs_inode_next_free( whefs_fs * restrict fs, whefs_inode * restrict tgt, bool markUsed );

/**
   Searches fs for an inode with the given name. On success, tgt is
   updated to (a copy of) that inode's state and whefs_rc.OK is
   returned, otherwise some other value is returned.

   Bugs:

   - Directory structures are not yet supported.
*/
int whefs_inode_by_name( whefs_fs * fs, char const * name, whefs_inode * tgt );


/** @def whefs_inode_id_is_valid_m

   Internal implementation of the more public whefs_inode_id_is_valid().

   Evaluates to true if inode id NID is valid for the given fs. That
   is, it has a non-zero id in a range legal for the given fs object.
 */
#define whefs_inode_id_is_valid_m( FS,NID ) ( \
    ( (FS) && (NID) && \
      ((NID) <= (FS)->options.inode_count)     \
      ) ? true : false)

/** @def whefs_inode_is_valid_m

   Internal implementation of the more public whefs_inode_is_valid().

   Evaluates to true if inode INO (a const pointer to a whefs_inode)
   is valid for the given fs (a const whefs_fs pointer). That is, it
   has a non-zero id in a range legal for the given fs object.
 */
#define whefs_inode_is_valid_m(FS,INO) ((INO) ? (whefs_inode_id_is_valid(FS,(INO)->id)) : false)

#if WHEFS_MACROIZE_SMALL_CHECKS
#define whefs_inode_id_is_valid(FS,NID) whefs_inode_id_is_valid_m(FS,NID)
#define whefs_inode_is_valid(FS,INO) whefs_inode_is_valid_m(FS,INO)
#else
/** @fn bool whefs_inode_id_is_valid( whefs_fs const * restrict fs, whefs_id_type nid )

   Returns true if nid is a valid inode for the given fs. That is, it
   has a non-zero id in a range legal for the given fs object.
*/
bool whefs_inode_id_is_valid( whefs_fs const * restrict fs, whefs_id_type nid );

/** @fn bool whefs_inode_is_valid( whefs_fs const * restrict fs, whefs_inode const * n )

   Returns true if n is "valid" - has a non-zero id in a range legal
   for the given fs object.
*/
bool whefs_inode_is_valid( whefs_fs const * restrict fs, whefs_inode const * n );
#endif

/**
   Creates a new i/o device associated with the given inode (which
   must be a valid inode). The device allows reading and writing
   from/to the inode as if it were a normal i/o device, using
   fs's data blocks as the underlying data store. The capability
   allows such a device to itself be the host of a whefs_fs filesystem
   That is, it makes it possible to embed one VFS with another, to
   an arbitrary depth.

   On success a new device is returned, which eventualy must be
   destroyed via dev->finalize(dev). On error, 0 is returned.

   The given inode must be a fully populated object. The returned
   device will take over doing updates of that object's on-disk
   data. That is, changes to the inode made via the device
   cannot be tracked via the passed-in inode pointer.

   The returned object conforms as closely as possible to the
   whio_dev API specifications, but there may still be an
   outstanding corner case or three.

   whio_dev::ioctl(): the returned object supports the
   whio_dev_ioctl_GENERAL_size ictl to return the current size of the
   device.

   whio_dev::iomode() will return a positive value if writeMode is
   true, 0 if it is false, or -1 if its argument is invalid.
*/
whio_dev * whefs_dev_for_inode( whefs_fs * fs, whefs_id_type nodeID, bool writeMode );

/**
   Returns the on-disk position of the given inode, which must be a
   valid inode id for fs. fs must be opened and initialized. On error
   (!fs or !nid, or nid is out of range), 0 is returned. Does not require
   any i/o.
*/
whio_size_t whefs_inode_id_pos( whefs_fs const * restrict fs, whefs_id_type nid );

/**
   Seeks to the given inode's on-disk position. Returns whefs_rc.OK
   on success.
*/
int whefs_inode_seek( whefs_fs * fs, whefs_inode const * ino );

/**
   Equivalent to whefs_inode_seek(), but takes an inode ID.
*/
int whefs_inode_id_seek( whefs_fs * fs, whefs_id_type id );

/**
   Sets the name of the given inode, updating the on-disk
   record. Returns whefs_rc.OK on success, or some other value on
   error. If the name is longer than the smaller of
   whefs_fs_options_get(fs)->filename_length or
   WHEFS_MAX_FILENAME_LENGTH, whefs_rc.RangeError is returned and n's
   record is not modified.

   This function does NOT change the inode's mtime but does update the on-disk
   name record. If the inode is currently opened (via whefs_inode_open())
   then its in-memory record (whefs_inode::name) is also updated.

   If node_id is opened then this routine will update the opened
   object's copy of the name.
*/
//int whefs_inode_name_set( whefs_fs * fs, whefs_inode const * n, char const * name );
int whefs_inode_name_set( whefs_fs * fs, whefs_id_type node_id, char const * name );

/**
   Loads the name for the given inode id into the given target string
   (which may not be null). If tgt has memory allocated to it, it may
   be re-used (or realloc()'d) by this function, so the caller must
   copy it beforehand if the existing value will be needed later. If the read-in
   string has a length of 0 and tgt->alloced is not 0 (i.e. tgt
   already has content), then the string's memory is kept but is
   zeroed out and tgt->length will be set to 0.

   Returns whefs_rc.OK on success.

   If you want to ensure that no call to malloc() or realloc() is made
   to expand tgt, yet still be assured of having enough memory available
   to store the string, here's a trick:

   @code
   enum { bufSize = WHEFS_MAX_FILENAME_LENGTH + 1 };
   char buf[bufSize] = {0};
   whefs_string str = whefs_string_empty;
   str.string = buf;
   str.alloced = bufSize;
   int rc = whefs_inode_name_get( fs, id, &str );
   assert( (str.string == buf) && "Illegal (re)alloc!" );
   ...
   @endcode

   The internals of the lib won't allow an inode name longer than
   fs->options.filename_length, which must be less than or equal to
   WHEFS_MAX_FILENAME_LENGTH. We add one to the maximum buffer size in
   the above code to accommodate the trailing null.
*/
int whefs_inode_name_get( whefs_fs * fs, whefs_id_type id, whefs_string * tgt );


/**
   If !n then it returns whefs_rc.ArgError, otherwise it returns
   whefs_rc.OK and updates n->mtime to the current time. It does not
   write the changes to disk.
*/
int whefs_inode_update_mtime( whefs_fs * fs, whefs_inode * n );

/**
   Marks the given inode as unused and wipes all associated data
   blocks. The inode object must be fully populated if its associated
   blocks are to be properly freed.

   The the inode is currently opened, unlinking will not be allowed
   and whefs_rc.AccessError will be returned.

   Returns whefs_rc.OK on success. On error, we cannot say how much of
   the unlink worked before we aborted (e.g. not all blocks might have
   been cleared).

   Ownership of inode is not transfered but the inode object will be
   wiped clean of all state except its ID.

   As part of the deletion process, blocks in use by the unlinked
   inode are wiped with zeroes. It is hoped that this feature will
   one day become a toggleable option.
*/
int whefs_inode_unlink( whefs_fs * fs, whefs_inode * inode );

/**
   Equivalent to whefs_inode_unlink(), but takes an inode ID.
   It will fail if the inode is invalid, cannot be read, or
   unlinking fails.
*/
int whefs_inode_id_unlink( whefs_fs * fs, whefs_id_type nid );


/**
   Searches the table of opened inodes for the given ID. If an entry
   is found, tgt is assigned to a pointer to that object and
   whefs_rc.OK is returned. If none is found, or some other error
   happens (e.g. fs or nodeID are invalid) then some other value is
   returned and tgt is not modified.

   WARNING:

   This routine allows the caller to bypass the single-writer
   restriction on inodes, and should be used with care. It is in the
   semi-public API only to support whefs_file_rename() and
   whefs_fsize().
*/
int whefs_inode_search_opened( whefs_fs * fs, whefs_id_type nodeID, whefs_inode ** tgt );

/**
   Encodes src (which may not be null) to dest, which must be valid memory
   of at least whefs_sizeof_encoded_inode bytes long. On succes whefs_rc.OK
   is returned and whefs_sizeof_encoded_inode bytes of dest are written.
   The only error conditions are that neither src nor dest are null, so if
   you are certain you're passing non-nulls then you can ignore the error
   check.
*/
int whefs_inode_encode( whefs_inode const * src, unsigned char * dest );

/**
   Encodes dest (which may not be null) from src, which must be valid memory
   at least whefs_sizeof_encoded_inode bytes long and containing an encoded
   inode (see whefs_inode_encode()). On succes whefs_rc.OK
   is returned and whefs_sizeof_encoded_inode bytes are read from src dest.

   Unlike whefs_inode_encode(), this routine has many potential points
   of failure (as there are several values to be decoded), so checking
   the return value is a must. On success, whefs_rc.OK is returned.
   If src or dest are null then whefs_rc.ArgError is returned. If
   decoding fails then whefs_rc.ConsistencyError is returned.
*/
int whefs_inode_decode( whefs_inode * dest, unsigned char const * src );

/**
   whefs_inode_foreach_f describes a functor for use with
   whefs_inode_foreach().  n is the current inode being iterated
   over. clientData is the client-determined argument passed to
   whefs_inode_foreach().
*/
typedef int (*whefs_inode_foreach_f)( whefs_fs * fs, whefs_inode const * n, void * clientData );
/**
   whefs_inode_predicate_f describes a predicate functor for use with
   whefs_inode_foreach(). n is the current inode being iterated
   over. clientData is the client-determined argument passed to
   whefs_inode_foreach().
*/
typedef bool (*whefs_inode_predicate_f)( whefs_fs * fs, whefs_inode const * n, void * clientData );

/**
   Walks each inode in fs, starting at inode #2 (#1 is reserved for
   internal use as the root node). For each node, whereFunc(node,whereData) is called.
   If it returns true then forEach(fs,n,forEachData) is called. If forEach() returns
   any value other than whefs_rc.OK then looping stops and that code is returned.

   The whereFunc function may be null but forEach may not. If whereFunc is
   null then it is treated as always returning true. The whereData and
   forEachData pointers may be anything - they are passed as-is to the
   whereFunc/forEach functions.

   On success, whefs_rc.OK is returned. The other failure cases are:

   - fs or forEach are null: whefs_rc.ArgError

   - reading an inode fails: some propagated error code.

   ACHTUNG: this bypasses the opened-inodes cache (because lookup time
   would grow exponentially as the number of opened inodes grew). Thus
   care must be taken with the nodes passed on to forEach, lest any
   changes made to them get overwritten by the opened node when it
   flushes.
*/
int whefs_inode_foreach( whefs_fs * fs, whefs_inode_predicate_f whereFunc, void * whereData, whefs_inode_foreach_f forEach, void * forEachData );

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* WANDERINGHORSE_NET_WHEFS_INODE_H_INCLUDED */
