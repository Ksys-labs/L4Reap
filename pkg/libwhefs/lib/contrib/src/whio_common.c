#include <wh/whio/whio_common.h>
#include <string.h> /* strchr() */

const whio_client_data whio_client_data_empty = whio_client_data_empty_m;
const whio_impl_data whio_impl_data_empty = whio_impl_data_empty_m;

short whio_mode_to_iomode( char const * mode )
{
    if( ! mode ) return -1;
    else if( (0 != strchr( mode, 'w' )) )
    { 
        return 1;
    }
    else if( 0 != strchr( mode, 'r' ) )
    {
	if( 0 != strchr( mode, '+' ) )
        {
            return 1;
        }
	else return 0;
    }
    else return -1;
}
