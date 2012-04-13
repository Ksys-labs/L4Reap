/**
   Implementations for the whefs_fs_closer_list-related API.
*/
#include "whefs_details.c"
#include <stdlib.h> /* malloc() and co. */

const whefs_fs_closer_list whefs_fs_closer_list_empty = whefs_fs_closer_list_empty_m;


/**
   Unlinks x from its neighbors.
*/
static void whefs_fs_closer_list_unlink( whefs_fs_closer_list * x )
{
    if( x->prev ) { x->prev->next = x->next; x->prev = 0; }
    if( x->next ) { x->next->prev = x->prev; x->next = 0; }
}


#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
enum {
/**
   The number of elements to statically allocate
   in the whefs_fs_closer_list_alloc_slots object.
*/
whefs_fs_closer_list_alloc_count = 6
};
static struct
{
    whefs_fs_closer_list objs[whefs_fs_closer_list_alloc_count];
    char used[whefs_fs_closer_list_alloc_count];
    size_t next;
    const size_t count;
} whefs_fs_closer_list_alloc_slots = { { whefs_fs_closer_list_empty_m }, {0}, 0, whefs_fs_closer_list_alloc_count };
#endif
/**
   Allocates a new zero-initialized object. Ownership is passed
   to the caller.
*/
whefs_fs_closer_list * whefs_fs_closer_list_alloc()
{
    whefs_fs_closer_list * obj = 0;
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    size_t i = whefs_fs_closer_list_alloc_slots.next;
    for( ; i < whefs_fs_closer_list_alloc_slots.count; ++i )
    {
	if( whefs_fs_closer_list_alloc_slots.used[i] ) continue;
	whefs_fs_closer_list_alloc_slots.used[i] = 1;
	whefs_fs_closer_list_alloc_slots.next = i+1;
	obj = &whefs_fs_closer_list_alloc_slots.objs[i];
	break;
    }
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
    if( ! obj ) obj = (whefs_fs_closer_list *) malloc( sizeof(whefs_fs_closer_list) );
    if( obj ) *obj = whefs_fs_closer_list_empty;
    return obj;
}

/**
   Unlinks obj from its neighbors and deallocates it. Ownership of
   obj is transfered to this function.
*/
void whefs_fs_closer_list_free( whefs_fs_closer_list * obj )
{
    if( ! obj ) return;
#if WHEFS_CONFIG_ENABLE_STATIC_MALLOC
    if( (obj < &whefs_fs_closer_list_alloc_slots.objs[0]) ||
	(obj > &whefs_fs_closer_list_alloc_slots.objs[whefs_fs_closer_list_alloc_slots.count-1]) )
    { /* it does not belong to us */
        whefs_fs_closer_list_unlink( obj );
        *obj = whefs_fs_closer_list_empty;
	free(obj);
	return;
    }
    else
    {
        whefs_fs_closer_list_unlink( obj );
        *obj = whefs_fs_closer_list_empty;
	const size_t ndx = (obj - &whefs_fs_closer_list_alloc_slots.objs[0]);
	whefs_fs_closer_list_alloc_slots.used[ndx] = 0;
	if( ndx < whefs_fs_closer_list_alloc_slots.next ) whefs_fs_closer_list_alloc_slots.next = ndx;
	return;
    }
#else
    whefs_fs_closer_list_unlink( obj );
    *obj = whefs_fs_closer_list_empty;
    free(obj);
#endif /* WHEFS_CONFIG_ENABLE_STATIC_MALLOC */
}


int whefs_fs_closer_list_close( whefs_fs_closer_list * head, bool right )
{
    if( ! head ) return whefs_rc.ArgError;
    int rc = whefs_rc.OK;
    whefs_fs_closer_list * x = head;
    for( ; x; head = x )
    {
        x = head->next;
        // We unlink head before closing b/c the close routines may update the list indirectly.
        whefs_fs_closer_list_unlink(head);
        switch( head->type )
        {
          case WHEFS_CLOSER_TYPE_FILE:
              whefs_fclose( head->item.file );
              break;
          case WHEFS_CLOSER_TYPE_DEV:
              head->item.dev->api->finalize( head->item.dev );
              break;
          case WHEFS_CLOSER_TYPE_STREAM:
              head->item.stream->api->finalize( head->item.stream );
              break;
          default:
              WHEFS_DBG_ERR("Internal error whefs_fs_closer_list entry does not have a supported type field. Possibly leaking an object here!");
              rc = whefs_rc.InternalError;
              break;
        };
        whefs_fs_closer_list_free(head);
        if( ! right ) break;
    }
    return rc;
}

/**
   Removes the given object from fs->closers. fs->closers must
   contain an entry where entry->type==type and entry->item.XXX==obj,
   where XXX is dependent on entry->type.

   Returns whefs_rc.OK on sucess.
*/
static int whefs_fs_closer_remove( whefs_fs * fs, char type, void const * obj )
{
    if( ! fs || ! obj ) return whefs_rc.ArgError;
    whefs_fs_closer_list * li = fs->closers;
    if( ! li ) return whefs_rc.RangeError;
    while( li->prev ) li = li->prev;
    int rc = whefs_rc.OK;
    bool foundOne = false;
    for( ; li ; li = li->next )
    {
        if( li->type != type ) continue;
        switch( type )
        {
          case WHEFS_CLOSER_TYPE_FILE:
              if( (void const *)li->item.file != obj ) continue;
              foundOne = true;
              break;
          case WHEFS_CLOSER_TYPE_DEV:
              if( (void const *)li->item.dev != obj ) continue;
              foundOne = true;
              break;
          case WHEFS_CLOSER_TYPE_STREAM:
              if( (void const *)li->item.stream != obj ) continue;
              foundOne = true;
              break;
          default:
              WHEFS_DBG_ERR("Internal error whefs_fs_closer_list entry does not have a supported type field.");
              rc = whefs_rc.InternalError;
              break;
        };
        if( foundOne )
        {
            if( fs->closers == li )
            {
                fs->closers = li->prev ? li->prev : li->next;
            }
            whefs_fs_closer_list_free( li );
            li = 0;
            break;
        }
    }
    return rc;
}


/**
   Creates a new, empty whefs_fs_closer_list object and
   appends it to fs->closers.
*/
static whefs_fs_closer_list * whefs_fs_closer_add( whefs_fs * fs )
{
    if( ! fs ) return NULL;
    whefs_fs_closer_list * li = whefs_fs_closer_list_alloc();
    if( ! li ) return NULL;
    if( ! fs->closers )
    {
        fs->closers = li;
    }
    else
    {
        whefs_fs_closer_list * tail = fs->closers;
        while( tail->next ) tail = tail->next;
        tail->next = li;
        li->prev = tail;
    }
    return li;
}

int whefs_fs_closer_file_add( whefs_fs * fs, whefs_file * f )
{
    if( ! fs || ! f ) return whefs_rc.ArgError;
    whefs_fs_closer_list * li = fs->closers;
    if( li )
    { /** If we find f->dev in the list then we
          promote the entry to type WHEFS_CLOSER_TYPE_FILE.
      */
        while( li->prev ) li = li->prev;
        for( ; li ; li = li->next )
        {
            if( li->type != WHEFS_CLOSER_TYPE_DEV ) continue;
            if( li->item.dev != f->dev ) continue;
            li->type = WHEFS_CLOSER_TYPE_FILE;
            li->item.file = f;
            break;
        }
    }
    else
    {
        whefs_fs_closer_list * li = whefs_fs_closer_add(fs);
        if( ! li ) return whefs_rc.AllocError /* that's a guess */;
        li->type = WHEFS_CLOSER_TYPE_FILE;
        li->item.file = f;
    }
    return whefs_rc.OK;
}

int whefs_fs_closer_file_remove( whefs_fs * fs, whefs_file const * f )
{
    return whefs_fs_closer_remove( fs, WHEFS_CLOSER_TYPE_FILE, f );
}

int whefs_fs_closer_dev_add( whefs_fs * fs, whio_dev * d )
{
    if( ! fs || ! d ) return whefs_rc.ArgError;
    whefs_fs_closer_list * li = whefs_fs_closer_add(fs);
    if( ! li ) return whefs_rc.AllocError /* that's a guess */;
    li->type = WHEFS_CLOSER_TYPE_DEV;
    li->item.dev = d;
    return whefs_rc.OK;
}

int whefs_fs_closer_dev_remove( whefs_fs * fs, whio_dev const * d )
{
    return whefs_fs_closer_remove( fs, WHEFS_CLOSER_TYPE_DEV, d );
}

int whefs_fs_closer_stream_add( whefs_fs * fs, whio_stream * s, whio_dev const * d )
{
    if( ! fs || ! s || !d ) return whefs_rc.ArgError;
    whefs_fs_closer_list * li = fs->closers;
    if( li )
    { /** If we find f->dev in the list then we
          promote the entry to type WHEFS_CLOSER_TYPE_STREAM.
      */
        while( li->prev ) li = li->prev;
        for( ; li ; li = li->next )
        {
            if( li->type != WHEFS_CLOSER_TYPE_DEV ) continue;
            if( d != li->item.dev ) continue;
            li->type = WHEFS_CLOSER_TYPE_STREAM;
            li->item.stream = s;
            break;
        }
        if( ! li ) return whefs_rc.RangeError;
    }
    else
    { /* re-evaluate this. Is this sane? */
        whefs_fs_closer_list * li = whefs_fs_closer_add(fs);
        if( ! li ) return whefs_rc.AllocError /* that's a guess */;
        li->type = WHEFS_CLOSER_TYPE_STREAM;
        li->item.stream = s;
    }
    return whefs_rc.OK;
}

int whefs_fs_closer_stream_remove( whefs_fs * fs, whio_stream const * s )
{
    return whefs_fs_closer_remove( fs, WHEFS_CLOSER_TYPE_STREAM, s );
}
