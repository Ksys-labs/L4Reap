
/*
  Test/demo app for libwhefs.

  Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)

  License: Public Domain
*/
#ifdef NDEBUG
#  undef NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
//#include <unistd.h> /* sleep() */
#include <wh/whefs/whefs.h>
#include <wh/whefs/whefs_client_util.h>
#include <wh/whio/whio_devs.h>
#include <wh/whio/whio_encode.h>

#include "WHEFSApp.c"
#include "../src/whefs_encode.h"

struct app_data {
    whefs_fs_options fsopts;
} ThisApp;

typedef uint32_t whio_avl_id_type;
typedef uint64_t whio_avl_key_type;

struct whio_avl_link_info
{
    whio_avl_id_type left;
    whio_avl_id_type right;
    uint16_t avl_height;
};
typedef struct whio_avl_link_info whio_avl_link_info;
#define whio_avl_link_info_empty_m {0,0,0}
const whio_avl_link_info whio_avl_link_info_empty = whio_avl_link_info_empty_m;

struct whio_avl_node
{
    whio_avl_id_type id;
    whio_avl_id_type value;
    whio_avl_key_type key;
    whio_avl_link_info link;
};
typedef struct whio_avl_node whio_avl_node;
#define whio_avl_node_empty_m {0,0,0,whio_avl_link_info_empty_m}
const whio_avl_node whio_avl_node_empty = whio_avl_node_empty_m;




enum whio_sizeof_encoded_avl {
    whio_sizeof_encoded_avl_id = whio_sizeof_encoded_uint32,
    whio_sizeof_encoded_avl_key = whio_sizeof_encoded_uint64,
    whio_sizeof_encoded_avl_link = 1 /* tag byte */
    + whio_sizeof_encoded_avl_id /* left*/
    + whio_sizeof_encoded_avl_id /* right*/
    + whio_sizeof_encoded_uint16 /* avl_height*/,

    whio_sizeof_encoded_avl_node = 1 /* tag byte */
    + whio_sizeof_encoded_avl_id /* id*/
    + whio_sizeof_encoded_avl_key /* key*/
    + whio_sizeof_encoded_avl_id /* value*/
    + whio_sizeof_encoded_avl_link
};

static const unsigned char whio_avl_link_tag_char = 0x80 | 'L';
static whio_size_t whio_avl_link_encode( unsigned char * dest, const whio_avl_link_info * src )
{
    if( ! dest || ! src ) return 0;
    unsigned char * pos = dest;
    *pos = whio_avl_link_tag_char;
    ++pos;
    pos += whio_size_t_encode( pos, src->left );
    pos += whio_size_t_encode( pos, src->right );
    pos += whio_encode_uint16( pos, src->avl_height );
    return pos - dest;
}

static const unsigned char whio_avl_node_tag_char = 0x80 | 'N';
whio_size_t whio_avl_node_encode( unsigned char * dest, const whio_avl_node * src )
{
    if( ! dest || ! src ) return 0;
    unsigned char * pos = dest;
    *(pos++) = whio_avl_node_tag_char;
    pos += whio_encode_uint32( pos, src->id );
    pos += whio_encode_uint64( pos, src->key );
    pos += whio_encode_uint32( pos, src->value );
    pos += whio_avl_link_encode( pos, &src->link );
    //MARKER("node encode returning %u\n",(pos-dest));
    return pos - dest;
}



static whio_size_t whio_avl_node_pos( whio_avl_id_type n )
{
    return n * whio_sizeof_encoded_avl_node;
}

whio_size_t whio_avl_node_write( whio_dev * dev, const whio_avl_node * src )
{
    if( ! dev || !src ) return 0;
    unsigned char buf[whio_sizeof_encoded_avl_node];
    const whio_size_t pos = whio_avl_node_pos(src->id);
    if( pos != dev->api->seek(dev, pos, SEEK_SET ) ) return 0;
    if(0)
    {
        MARKER("write node [#%"PRIu32"] seek pos=%"WHIO_SIZE_T_PFMT"\n",
               src->id, pos );
    }
    return ( whio_sizeof_encoded_avl_node == whio_avl_node_encode( buf, src ) )
        ? dev->api->write( dev, buf, whio_sizeof_encoded_avl_node )
        : 0;
}

static whio_size_t whio_avl_link_decode( unsigned char const * src, whio_avl_link_info * dest )
{
    if( ! src || !dest ) return 0;
    unsigned char const * pos = src;
    if( *(pos++) != whio_avl_link_tag_char ) return whio_rc.ConsistencyError;
    int rc;
#define RC if( rc != whio_rc.OK ) return rc
    rc = whio_decode_uint32( pos, &dest->left );
    RC;
    pos += whio_sizeof_encoded_avl_id;
    rc = whio_decode_uint32( pos, &dest->right );
    RC;
    pos += whio_sizeof_encoded_avl_id;
    rc = whio_decode_uint16( pos, &dest->avl_height );
#undef RC    
    return rc;
}

int whio_avl_node_decode( unsigned char const * src, whio_avl_node * dest )
{
    if( ! src || ! dest ) return whio_rc.ArgError;
    unsigned char const * pos = src;
    if( *(pos++) != whio_avl_node_tag_char ) return whio_rc.ConsistencyError;
    int rc;
#define RC if( rc != whio_rc.OK ) return rc
    rc = whio_decode_uint32( pos, &dest->id );
    RC;
    pos += whio_sizeof_encoded_avl_id;
    rc = whio_decode_uint64( pos, &dest->key );
    RC;
    pos += whio_sizeof_encoded_avl_key;
    rc = whio_decode_uint32( pos, &dest->value );
    RC;
    pos += whio_sizeof_encoded_avl_id;
#undef RC
    return whio_avl_link_decode( pos, &dest->link );
}

int whio_avl_node_read( whio_dev * src, whio_avl_node * dest )
{
    if( ! src || ! dest ) return whefs_rc.ArgError;
    unsigned char buf[whio_sizeof_encoded_avl_node];
    const whio_size_t sz = src->api->read( src, buf, whio_sizeof_encoded_avl_node );
    if( whio_sizeof_encoded_avl_node != sz ) return whio_rc.IOError;
    return whio_avl_node_decode( buf, dest );
}

int whio_avl_node_read_seek( whio_dev * dev, whio_avl_id_type id, whio_avl_node * dest )
{
    if( ! dev || ! dest ) return whefs_rc.ArgError;
    whio_size_t const pos = whio_avl_node_pos( id );
    if(1) MARKER("read id [#%"PRIu32"] seek pos=%"WHIO_SIZE_T_PFMT"\n",
                 id, pos );
    if( pos != dev->api->seek( dev, pos, SEEK_SET ) )
    {
        return whio_rc.IOError;
    }
    return whio_avl_node_read( dev, dest );
}

static int whio_avl_node_read_link( whio_dev * dev, whio_avl_node const * p, whio_avl_node * ch, bool isLeft  )
{
#define NID(N) (isLeft ? (N)->link.left : (N)->link.right )
    if( 0 == NID(p) )
    {
        *ch = whio_avl_node_empty;
        return whio_rc.OK;
    }
    return whio_avl_node_read_seek( dev, NID(p), ch );
#undef NID
 
}

int whio_avl_node_read_links( whio_dev * dev, whio_avl_node const * n, whio_avl_node * nL, whio_avl_node * nR )
{
    int rc = whio_avl_node_read_link( dev, n, nL, true );
    return ( whio_rc.OK == rc )
        ? whio_avl_node_read_link( dev, n, nR, false )
        : rc;
}

int whio_avl_node_read_block( whio_dev * dev, whio_avl_node * n, whio_avl_node * nL, whio_avl_node * nR )
{
    whio_size_t const orig = dev->api->tell(dev);
    if( whio_rc.SizeTError == orig ) return whio_rc.IOError;
    int rc = whio_avl_node_read( dev, n );
    if( whio_rc.OK != rc ) return rc;
    if( n->link.left )
    {
        rc = whio_avl_node_read_link( dev, n, nL, true );
        if( whio_rc.OK != rc ) return rc;
    }
    if( n->link.right )
    {
        rc = whio_avl_node_read_link( dev, n, nR, false );
        if( whio_rc.OK != rc ) return rc;
    }
    if( orig != dev->api->seek( dev, orig, SEEK_SET ) )
    {
        return whio_rc.IOError;
    }
    return whio_rc.OK;
}


#if 0
enum { whio_avl_delta_max = 1 };
#define AVL_DELTA(n,nl,nr)                              \
    (( (((n)->link.left) ? (nl)->link.avl_height  : 0))	\
     - (((n)->link.right) ? (nr)->link.avl_height : 0))

static whio_avl_node * whio_avl_balance( whio_dev * dev, whio_avl_node * n )
{
    whio_avl_node nl, nr;
    whio_avl_node_read_links( dev, n, &nl, &nr );
    int delta = AVL_DELTA(n,&nl,&nr);
    if( delta < -whio_avl_delta_max)
    {
        if( AVL_DELTA(&nr) > 0 )
        {
            //fuck, this is gonna be difficult.
        }
    }
    return n;
}
#endif


int whio_avl_node_cmp_one( whio_avl_node const * lhs, whio_avl_node const * rhs )
{
    if( lhs && !rhs ) return 1;
    else if( !lhs && rhs ) return -1;
    else if( lhs == rhs ) return 0;
    else if( lhs->key < rhs->key ) return -1;
    else if( lhs->key > rhs->key ) return 1;
    else return 0;
}

whio_avl_key_type whio_avl_node_hash_inode_name( whio_avl_node const * node )
{

    return 0;
}

struct whio_avl_tree_impl
{
    whio_dev * dev;
    whio_avl_id_type next_free;
    whio_avl_id_type max_block_count;
    whio_size_t block_size;
    void * buffer;
    void (*buffer_dtor)( void * );
};
typedef struct whio_avl_tree_impl whio_avl_tree_impl;
#define whio_avl_tree_impl_empty_m { \
       0 /*dev*/, \
       0 /*next_free*/,                     \
       0 /*max_block_count*/,           \
       0 /*block_size*/,            \
       0 /*buffer*/,            \
       free /*buffer_dtor*/ \
}

const whio_avl_tree_impl whio_avl_tree_impl_empty = whio_avl_tree_impl_empty_m;

struct whio_avl_tree_api
{
    whio_avl_key_type (*hash)( whio_avl_node const * node );
};
typedef struct whio_avl_tree_api whio_avl_tree_api;
#define whio_avl_tree_api_empty_m { \
        whio_avl_node_hash_inode_name \
    }
static const whio_avl_tree_api whio_avl_tree_api_default = whio_avl_tree_api_empty_m;

const whio_avl_tree_api whio_avl_tree_api_empty = whio_avl_tree_api_empty_m;

struct whio_avl_tree
{
    whio_avl_node head;
    whio_avl_tree_api const * api;
    whio_avl_tree_impl impl;
};
typedef struct whio_avl_tree whio_avl_tree;
#define whio_avl_tree_empty_m {                  \
        whio_avl_node_empty_m,                   \
        &whio_avl_tree_api_default,         \
        whio_avl_tree_impl_empty_m           \
    }

const whio_avl_tree whio_avl_tree_empty = whio_avl_tree_empty_m;

whio_avl_tree * whio_avl_tree_alloc()
{
    whio_avl_tree * x = (whio_avl_tree *)malloc(sizeof(whio_avl_tree));
    if( x ) *x = whio_avl_tree_empty;
    return x;
}

void whio_avl_tree_free(whio_avl_tree * x)
{
    if( x )
    {
        if( x->impl.buffer && (0 != x->impl.buffer_dtor) )
        {
            (*x->impl.buffer_dtor)( x->impl.buffer );
        }
        *x = whio_avl_tree_empty;
        free(x);
    }
}

void dump_avl_node( char const * lbl, whio_avl_node const * n )
{
    printf("avl node: %s: id=%"PRIu32", key=%"PRIu64", value=%"PRIu32", left=%"PRIu32", right=%"PRIu32"\n",
           lbl, n->id, n->key, n->value, n->link.left, n->link.right );
}

int test_one()
{
    char const * fname = "avl.iodev";
    whio_dev * d = whio_dev_for_filename( fname, "w+" );
    assert(d && "could not open file!");

    MARKER("whio_sizeof_encoded_avl_node=%u\n",whio_sizeof_encoded_avl_node);
    MARKER("sizeof(whio_avl_node)=%u\n",sizeof(whio_avl_node));

    whio_avl_node n1, n2, n3;
    n1 = n2 = n3 = whio_avl_node_empty;
    n1.id = 1; n1.key = 11; n1.value = 1111;
    n2.id = 2; n2.key = 22; n2.value = 44;
    n3.id = 3; n3.key = 33; n3.value = 33;
    n1.link.left = n2.id;
    n1.link.right = n3.id;
    //n2.link.right = n3.id;
    whio_avl_node_write( d, &n1 );
    whio_avl_node_write( d, &n2 );
    whio_avl_node_write( d, &n3 );

    whio_dev_rewind(d);
    whio_avl_node nR = whio_avl_node_empty;
    int rc = whio_avl_node_read_seek( d, n1.id, &nR );
    MARKER("node read rc=%d\n",rc);
#define D(N) dump_avl_node(# N, &(N))
    D(n1);
    D(n2);
    D(n3);
    D(nR);

    whio_dev_rewind(d);
    n1 = n2 = n3 = whio_avl_node_empty;
    d->api->seek( d, whio_avl_node_pos(1), SEEK_SET );
    rc = whio_avl_node_read_block( d, &n1, &n2, &n3 );
    MARKER("read block rc=%d\n",rc);
    D(n1);
    D(n2);
    D(n3);
    d->api->finalize(d);
    return whefs_rc.OK;
#undef D
}

int main( int argc, char const ** argv )
{
    WHEFSApp.usageText = "[flags]";
    WHEFSApp.helpText =
	"Basic test/sanity-check program for whefs."
	;
    int rc = 0;
    WHEFSApp_init( argc, argv, WHEFSApp_NoOpen, 0, 0 ); // ignore return.
    ThisApp.fsopts = whefs_fs_options_default;
    if(!rc) rc =  test_one();
    printf("Done rc=%d=[%s].\n",rc,
	   (0==rc)
	   ? "You win :)"
	   : "You lose :(");
    return rc;
}
