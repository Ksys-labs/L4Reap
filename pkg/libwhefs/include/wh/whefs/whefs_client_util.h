#if !defined(WANDERINGHORSE_NET_WHEFS_CLIENT_UTIL_INCLUDED)
#define WANDERINGHORSE_NET_WHEFS_CLIENT_UTIL_INCLUDED 1

#include "whefs.h"
#include "whefs_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
   Like the 'ls' Unix program, this function fetches a list of
   filenames matching the given pattern. If no matches are found, null
   is returned, otherwise a string list is returned and count (if it
   is not null) is set to the number of items in the list.

   A pattern of null or an empty string is equivalent to a pattern of
   "*" (but much faster, since it doesn't need to be compared).

   The returned list must be freed by calling whefs_string_finalize()
   and passing true as the second parameter.

   Example usage:

   @code
    whefs_string * ls = whefs_ls( myFS, "*.c", 0 );
    whefs_string * head = ls;
    while( ls )
    {
        printf("%s\n", ls->string );
        ls = ls->next;
    }
    whefs_string_finalize( head, true );
   @endcode

   BUG:

   This routine has no way of reporting an error. If some sort of I/O
   error happens after an entry has been fetched, the entries matched
   so far are returned but there is no way of knowing how many entries
   would have been returned if all could have been read. That said, if
   the first entry is read successfully, it is "reasonable to assume"
   that the remaining entries could be read as well. In fact, since
   only "used" entries are considered for matching, any entries
   returned here must have been read previously (to build the
   used-items cache), and only a true I/O error (or corruption of the
   VFS container) should cause a failure here.
*/
whefs_string * whefs_ls( struct whefs_fs * fs, char const * pattern, whefs_id_type * count );


/**
   Imports all contents from src into the VFS pseudofile named fname.
   If overwrite is true, any existing entry will be overwritten,
   otherwise an existing file with that name will cause this function
   to return whefs_rc.AccessError.

   Ownership of src is not changed. On success, this routine will
   re-set src's cursor to its pre-call position before returning.

   On success, whefs_rc.OK is returned.

   On error, any number of different codes might be returned, both
   from whefs_rc and whio_rc (some of which overlap or conflict).
   If the import fails and the entry did not exist before the import
   started, it is removed from the filesystem. If it did exist then
   exactly what happens next depends on a few factors:

   - If the import failed because the existing file could not be
   expanded to its new size then it is kept intact, with its old
   size. This step happens first, before any writing is done.

   - If the import failed during the copy process then the destination
   file is left in an undefined state and it should probably be
   unlinked.

*/
int whefs_import_dev( whefs_fs * fs, whio_dev * src, char const * fname, bool overwrite );


/**
   A type for reporting certain vfs metrics.
*/
typedef struct whefs_fs_stats
{
    /**
       Size of the vfs container.
    */
    size_t size;
    /**
       Number of uses inodes.
    */
    size_t used_inodes;
    /**
       Number of used blocks.
    */
    size_t used_blocks;
    /**
       Number of used bytes (not necessarily whole blocks).
    */
    size_t used_bytes;
} whefs_fs_stats;

/**
   Not yet implemented.

   Calculates some statistics for fs and returns them via
   the st parameter.

   Returns whefs_rc.OK on success. On error some other value
   is returned and st's contents are in an undefined state.
*/
int whefs_fs_stats_get( whefs_fs * fs, whefs_fs_stats * st );

/**
   Test/debug routine which inserts some entries into the inode table.
*/
int whefs_test_insert_dummy_files( whefs_fs * fs );

/** @struct whefs_fs_entry

   Experimental!

   whefs_fs_entry is the public interface into filesystem entries.
   They are intended to be fetched and interated over using
   whefs_fs_entry_foreach().
*/
struct whefs_fs_entry
{
    /** I-node ID of the entry. */
    whefs_id_type inode_id;

    /** ID of the first data block used by this entry. */
    whefs_id_type block_id;

    /** Last modification time. */
    uint32_t mtime;

    /** The size of the pseudofile, in bytes. */
    whio_size_t size;
    /**
       Entry's name. Be careful with deallocation! When objects of
       this type are passed to a callback function from
       whefs_fs_entry_foreach() the bytes used by this member are
       owned by whefs_fs_entry_foreach() and become
       invalidated/overwritten on each iteration. Thus if client code
       needs this name it must be copied in the foreach callback
       function. the third argument to whefs_fs_entry_foreach() may
       contain a client-side object for storing such copies.

       Also note that objects of this type MUST be initialized to 0
       values if they are to be used properly. Failing to do so can
       result in attempts to allocate huge amounts of memory or
       reallocate/re-use memory which doesn't belong to the string
       object. To avoid such problems, always initialize
       whefs_fs_entry objects using the whefs_fs_entry_empty object or
       whefs_fs_entry_empty_m macro.
     */
    whefs_string name;
};
typedef struct whefs_fs_entry whefs_fs_entry;

/** Empty initialization object for whefs_fs_entry. */
#define whefs_fs_entry_empty_m { 0U/*inode_id*/, 0U/*block_id*/, 0U/*mtime*/, 0U/*size*/, whefs_string_empty_m }

/** Empty initialization object for whefs_fs_entry. */
extern const whefs_fs_entry whefs_fs_entry_empty;

/**
   whefs_fs_entry_foreach_f describes a callback for use with
   whefs_fs_entry_foreach().  It is called from whefs_fs_entry_foreach(),
   passing:

   - fs is the whefs_fs object to which the entry belongs.

   - ent is the current entry being iterated over.

   - clientData is the client-determined argument passed to
   whefs_fs_entry_foreach().

   The string bytes belonging to ent->name will become invalid after
   this function returns (and control returns to
   whefs_fs_entry_foreach()), so they must be copied if they are
   needed for later.
*/

typedef int (*whefs_fs_entry_foreach_f)( whefs_fs * fs, whefs_fs_entry const * ent, void * clientData );

/**
   Walks each inode entry in fs, starting at inode #2 (#1 is reserved
   for internal use as the root node). For each in-use inode entry
   forEach(fs,entry,forEachData) is called. If forEach() returns any
   value other than whefs_rc.OK then looping stops and that return
   code is returned. Note that forEach() is NOT called for unused
   inodes.

   The forEach function may not be null. The forEachData pointer may
   be anything - it is passed on as-is to the forEach function.

   On success, whefs_rc.OK is returned. The other failure cases are:

   - fs or forEach are null: whefs_rc.ArgError

   - reading an inode fails: some propagated error code.

   - If forEach() returns non-OK, then that result is returned.

   ACHTUNG: this bypasses the opened-inodes cache (because lookup time
   would grow exponentially as the number of opened inodes grew). Thus
   the data passed to forEach() may not reflect unflushed state of
   inodes which are currently opened for write access.
*/
int whefs_fs_entry_foreach( whefs_fs * fs, whefs_fs_entry_foreach_f forEach, void * forEachData );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // WANDERINGHORSE_NET_WHEFS_CLIENT_UTIL_INCLUDED
