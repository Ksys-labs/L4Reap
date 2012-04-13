/**
   Code which is not part of libwhefs but may be shared between vfs-related tools.
   It provides a very minimal app framework for whefs applications, like 'ls',
   'cp', 'mkfs' and friends.

   This file is intended to be #include'd by the tool code.

   Author: Stephan Beal

   License: Public Domain
*/
#include "../src/whefs_details.c"
#include "whargv.h"
#include <wh/whglob.h>
#ifdef NDEBUG
#  undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h> // free()
#include <string.h> // strcmp()
#include <stdio.h>

enum ArgSpecTypes {
ArgTypeIgnore,
ArgTypeSizeT,
ArgTypeIOSizeT,
ArgTypeUInt16,
ArgTypeUInt32,
ArgTypeIDType,
ArgTypeInt,
ArgTypeBool,
ArgTypeBoolInvert,
ArgTypeCString
};
typedef struct ArgSpec
{
    /**
       Name of the command-line argument, e.g. -f or --foo.
    */
    char const * name;
    /** MUST be a value from ArgSpecTypes enum. */
    int type;
    /** Parsed argument will be stored here. Exact requirements for
        this value depend on the value of the type member. */
    void * target;
    /** Help text for this option. */
    char const * help;
    /**
       Called when WHEFSApp_init() parsed command line arguments. If a match is found
       for this argument, this callback is called. If it returns non-zero
       then args processing stops with an error.
    */
    int (*callback)( char const * key, char const * val, void const * data );
    /**
       The data parameter passed as the 3rd argument to callback().
    */
    void const * cbData;
} ArgSpec;
//#define ARGSPEC_INIT(KEY,TYPE,TGT,CB) { KEY,TYPE,TGT,CB }


/**
   File entry list. Stores the non-flag, non-EFS command-line
   arguments, which may be names of local files or EFS-side wildcards,
   depending on the application.
*/
typedef struct WHEFSApp_fe
{
    /** Argument as passed on the command line. Might be a file name or wildcard. */
    char const * name;
    /** Currently unusued. */
    size_t size;
    struct WHEFSApp_fe * next;
} WHEFSApp_fe;

static const WHEFSApp_fe WHEFSApp_fe_init =
    {
    0, /* name */
    0, /* size */
    0  /* next */
    };

static struct WHEFSApp_
{
    char const * appName;
    char const * usageText;
    char const * helpText;
    char const * fsName;
    char const * debugFlags;
    whefs_fs * fs;
    WHEFSApp_fe * fe;
    bool verbose;
    ArgSpec * argsSpec;
} WHEFSApp =
{
0, /* appName */
"fill out WHEFSApp.usageText!",  /* usageText */
"fill out WHEFSApp.helpText!", /* helpText */
0, /* fsName */
0, /* debugFlags*/
0, /* fs */
0,  /* fe */
false, /* verbose */
0 /* appArgSpec */
};

#define WHEFS_VFSNAME (WHEFSApp.fsName ? WHEFSApp.fsName : "No EFS opened")
#define MARKER printf("MARKER: %s:%d:%s():\n",__FILE__,__LINE__,__func__); printf
#define APPMSG printf("%s [%s]: ",WHEFSApp.appName, WHEFS_VFSNAME ); printf
#define APPERR printf("%s [%s]: ERROR: ",WHEFSApp.appName, WHEFS_VFSNAME ); printf
#define APPWARN printf("%s [%s]: WARNING: ",WHEFSApp.appName, WHEFS_VFSNAME ); printf
#define ERROR printf("ERROR: %s:%d:%s():\n",WHEFSApp.appName,__LINE__,__func__); printf
#define VERBOSE if(WHEFSApp.verbose) printf("%s [%s]: ",WHEFSApp.appName, WHEFS_VFSNAME); if(WHEFSApp.verbose) printf

/**
   Shows formated help text for the given arguments array,
   which must be terminated by an item with a name of 0.
*/
void ArgSpec_show_help( ArgSpec * args )
{
    char const * dash = 0;
    char const * tail = 0;
    char const * eq = 0;
    for( ; args && args->name; ++args )
    {
        if( args->name[1] )
        {
            dash = "--";
            eq = "=";
        }
        else
        {
            dash = "-";
            eq = "";
        }
        switch( args->type )
	{
	  case ArgTypeInt:
              tail = "<#signed integer>";
              break;
	  case ArgTypeIDType:
#if 8 == WHEFS_ID_TYPE_BITS
              tail = "<#uint8>";
#elif 16 == WHEFS_ID_TYPE_BITS
              tail = "<#uint16>";
#elif 32 == WHEFS_ID_TYPE_BITS
              tail = "<#uint32>";
#elif 64 == WHEFS_ID_TYPE_BITS
              tail = "<#uint64>";
#endif
              break;
	  case ArgTypeUInt32: 
              tail = "<#uint32>";
              break;
	  case ArgTypeUInt16: 
              tail = "<#uint16>";
              break;
	  case ArgTypeIOSizeT:
#if 8 == WHIO_SIZE_T_BITS
              tail = "<#uint8>";
#elif 16 == WHIO_SIZE_T_BITS
              tail = "<#uint16>";
#elif 32 == WHIO_SIZE_T_BITS
              tail = "<#uint32>";
#elif 64 == WHIO_SIZE_T_BITS
              tail = "<#uint64>";
#endif
              break;
	  case ArgTypeSizeT:
              tail = "<#size_t>";
              break;
	  case ArgTypeCString:
              tail = "<string>";
              break;
	  default:
	      tail = "";
              eq = "";
	};
	printf("\t%s%s%s%s\n\t\t%s\n", dash, args->name, eq, tail, args->help );
    }
}

/**
   Returns true if val is NULL or the first char is one of [tTyY1],
   else false.
*/
static bool ArgSpec_val_to_bool( char const * val )
{
    return (!val /* special case so -X is the same as -X=1 */)
	|| (*val == 't')
	|| (*val == 'T')
	|| (*val == '1')
	|| (*val == 'y')
	|| (*val == 'Y')
	;
}

/**
   Converts a single ArgSpec to a value. It convert val to the type
   specified by src->type and sets src->target to that converted
   value. src->type and src->target's type MUST match up or behaviour
   is undefined.

   The requirements are:

   - ArgTypeBool: src->target must be a pointer to a bool, which
   simply gets assigned to true.

   - ArgTypeBoolInvert: same as ArgTypeBool, but it inverts the value.

   - ArgTypeSizeT: must be a pointer to a size_t.

   - ArgTypeIOSizeT: must be a pointer to a whio_size_t.

   - ArgTypeUInt16: must be a pointer to a uint16_t.

   - ArgTypeUInt32: must be a pointer to a uint32_t.

   - ArgTypeIDType: must be a pointer to a whefs_id_type,

   - ArgTypeInt: must be a pointer to an int.

   - ArgTypeCString: must be a pointer to a (char const *).

   Any other value will cause an error code to be returned. On success,
   whefs_rc.OK is returned.
*/
static int ArgSpec_convert( ArgSpec * src, char const * val )
{
    if( ! src ) return whefs_rc.ArgError;
    int rc = whefs_rc.ArgError;
    if( (ArgTypeIgnore != src->type) && !src->target  )
    {
	APPERR("Argument '%s' is declared as taken a value but no target was passed for value assigment!\n", src->name );
	return rc;
    }
#if 0
    if( (ArgTypeIgnore != src->type) && !val  )
    {
	APPERR("Argument '%s' requires a value!\n", src->name );
	return rc;
    }
#endif
    size_t sz = 0;
    whefs_id_type id = 0;
    int d = 0;
    uint16_t iu16 = 0;
    uint32_t iu32 = 0;
    whio_size_t iosz = 0;
    switch( src->type )
    {
      case ArgTypeIgnore:
	  rc = whefs_rc.OK;
	  break;
      case ArgTypeBool:
      case ArgTypeBoolInvert:
	  rc = whefs_rc.OK;
	  *((bool*)src->target) = (src->type == ArgTypeBool)
	      ? ArgSpec_val_to_bool( val )
	      : (ArgSpec_val_to_bool( val ) ? false : true);
	  break;
      case ArgTypeSizeT:
	  if( ! val )
	  {
	      APPERR("Argument '%s' requires an unsigned numeric value!\n",
		     src->name );
	      return whefs_rc.ArgError;
	  }
	  if( 1 != sscanf( val, "%u", &sz ) )
	  {
	      APPERR("Could not parse value '%s' of option '%s' as a numeric value!\n",
		     val, src->name );
	      return whefs_rc.RangeError;
	  }
	  *((size_t*)src->target) = sz;
	  rc = whefs_rc.OK;
	  break;
      case ArgTypeIOSizeT:
	  if( ! val )
	  {
	      APPERR("Argument '%s' requires an unsigned numeric value!\n",
		     src->name );
	      return whefs_rc.ArgError;
	  }
	  if( 1 != sscanf( val, "%"WHIO_SIZE_T_SFMT, &iosz ) )
	  {
	      APPERR("Could not parse value '%s' of option '%s' as a numeric value!\n",
		     val, src->name );
	      return whefs_rc.RangeError;
	  }
	  *((whio_size_t*)src->target) = iosz;
	  rc = whefs_rc.OK;
	  break;
      case ArgTypeUInt32:
	  if( ! val )
	  {
	      APPERR("Argument '%s' requires an unsigned numeric value!\n",
		     src->name );
	      return whefs_rc.ArgError;
	  }
	  if( 1 != sscanf( val, "%u", &iu32 ) )
	  {
	      APPERR("Could not parse value '%s' of option '%s' as a numeric value!\n",
		     val, src->name );
	      return whefs_rc.RangeError;
	  }
	  *((uint32_t*)src->target) = iu32;
	  rc = whefs_rc.OK;
	  break;
      case ArgTypeUInt16:
	  if( ! val )
	  {
	      APPERR("Argument '%s' requires an unsigned numeric value!\n",
		     src->name );
	      return whefs_rc.ArgError;
	  }
	  if( 1 != sscanf( val, "%hu", &iu16 ) )
	  {
	      APPERR("Could not parse value '%s' of option '%s' as a numeric value!\n",
		     val, src->name );
	      return whefs_rc.RangeError;
	  }
	  *((uint16_t*)src->target) = iu16;
	  rc = whefs_rc.OK;
	  break;
      case ArgTypeIDType:
	  if( ! val )
	  {
	      APPERR("Argument '%s' requires an unsigned numeric value!\n",
		     src->name );
	      return whefs_rc.ArgError;
	  }
	  if( 1 != sscanf( val, "%"WHEFS_ID_TYPE_SFMT, &id ) )
	  {
	      APPERR("Could not parse value '%s' of option '%s' as a numeric value!\n",
		     val, src->name );
	      return whefs_rc.RangeError;
	  }
	  *((whefs_id_type*)src->target) = id;
	  //printf("Parsed whefs_id_type val as: ");printf(fmt,id);printf("\n");
	  rc = whefs_rc.OK;
	  break;
      case ArgTypeInt:
	  if( ! val )
	  {
	      APPERR("Argument '%s' requires an integer value!\n",
		     src->name );
	      return whefs_rc.ArgError;
	  }
	  if( 1 != sscanf( val, "%d", &d ) )
	  {
	      APPERR("Could not parse value '%s' of option '%s' as a numeric value!\n",
		     val, src->name );
	      return whefs_rc.RangeError;
	  }
	  *((int*)src->target) = d;
	  rc = whefs_rc.OK;
	  break;
      case ArgTypeCString:
	  rc = whefs_rc.OK;
	  do {
	      char const ** x = (char const **)src->target;
	      *x = val;
	  } while(0);
	  break;
      default:
	  rc = whefs_rc.NYIError;
	  break;
    };
    return rc;
}

char const * WHEFSApp_errno_to_string( int n )
{

    /* whio and whefs... */
    if( whefs_rc.OK == n ) return "whefs_rc.OK";
    if( -1 == n ) return "whefs_rc.SizeTError";

    /* whefs... */
    if( whefs_rc.ArgError == n ) return "whefs_rc.ArgError";
    if( whefs_rc.IOError == n ) return "whefs_rc.IOError";
    if( whefs_rc.AllocError == n ) return "whefs_rc.AllocError";
    if( whefs_rc.BadMagicError == n ) return "whefs_rc.BadMagicError";
    if( whefs_rc.InternalError == n ) return "whefs_rc.InternalError";
    if( whefs_rc.RangeError == n ) return "whefs_rc.RangeError";
    if( whefs_rc.FSFull == n ) return "whefs_rc.FSFull";
    if( whefs_rc.AccessError == n ) return "whefs_rc.AccessError";
    if( whefs_rc.ConsistencyError == n ) return "whefs_rc.ConsistencyError";
    if( whefs_rc.NYIError == n ) return "whefs_rc.NYIError";
    if( whefs_rc.UnsupportedError == n ) return "whefs_rc.UnsupportedError";

    /* whio... */
    if( whio_rc.ArgError == n ) return "whio_rc.ArgError";
    if( whio_rc.IOError == n ) return "whio_rc.IOError";
    if( whio_rc.AllocError == n ) return "whio_rc.AllocError";
    if( whio_rc.InternalError == n ) return "whio_rc.InternalError";
    if( whio_rc.RangeError == n ) return "whio_rc.RangeError";
    if( whio_rc.AccessError == n ) return "whio_rc.AccessError";
    if( whio_rc.ConsistencyError == n ) return "whio_rc.ConsistencyError";
    if( whio_rc.NYIError == n ) return "whio_rc.NYIError";
    if( whio_rc.UnsupportedError == n ) return "whio_rc.UnsupportedError";
    if( whio_rc.TypeError == n ) return "whio_rc.TypeError";

    return "Unknown Error Code";
}


/**
   If !WHEFSApp.fe, false is returned. Otherwise...  For each entry in
   WHEFSApp.fe, str is compared to it using whglob_matches(
   entry->name, str ).  If a match is found, true is returned.
*/
bool WHEFSApp_matches_fe( char const * str )
{
    if( ! WHEFSApp.fe ) return false;
    WHEFSApp_fe const * x = WHEFSApp.fe;
    for( ; x ; x = x->next )
    {
	if( whglob_matches( x->name, str ) ) return true;
    }
    return false;
}

/** Cleans up WHEFSApp.fs */
void WHEFSApp_fe_destroy()
{
    if( ! WHEFSApp.fe ) return;
    WHEFSApp_fe * x = WHEFSApp.fe;
    WHEFSApp.fe = 0;
    WHEFSApp_fe * e = x;
    while( e )
    {
	//MARKER("Freeing file entry @0x%p [%s]\n", (void const *)e, e->name );
	x = e->next;
	//MARKER("->next entry = @0x%p\n", (void const *)x );
	free(e);
	e = x;
    }
}

/**
   Adds fname to WHEFSApp.fe.
*/
int WHEFSApp_fe_append( char const * fname )
{
    if( ! fname || !*fname ) return whefs_rc.ArgError;
    WHEFSApp_fe * x = malloc(sizeof(WHEFSApp_fe));
    if( !x ) return whefs_rc.AllocError;
    *x = WHEFSApp_fe_init;
    x->name = fname;
    WHEFSApp_fe * h = WHEFSApp.fe;
    if( ! h ) WHEFSApp.fe = x;
    else 
    {
	while( h->next ) h = h->next;
	h->next = x;
    }
    return whefs_rc.OK;
}

/**
   Shared arguments for all WHEFSApp apps.
*/
static ArgSpec WHEFSApp_SharedArgs[] = {
{"?",  ArgTypeIgnore, 0, "Show help text.", 0, 0 },
{"help",  ArgTypeIgnore, 0, "Show help text.", 0, 0 },

{"v",  ArgTypeBool, &WHEFSApp.verbose, "Enables verbose mode.", 0, 0 },
{"verbose",  ArgTypeBool, &WHEFSApp.verbose, "Enables verbose mode.", 0, 0 },

{"D",  ArgTypeCString, &WHEFSApp.debugFlags, "Same as --debug-flags", 0, 0 },
{"debug-flags",  ArgTypeCString, &WHEFSApp.debugFlags, "Enables certain libwhefs debug flags. See whefs_setup_debug_arg() in the API docs.", 0, 0 },

{"V",  ArgTypeIgnore, 0, "Same as --version.", 0, 0 },
{"version",  ArgTypeIgnore, 0, "Show whefs version information.", 0, 0 },
{0}
};


static void WHEFSApp_atexit()
{
    WHEFSApp_fe_destroy();
    if( WHEFSApp.fs )
    {
	whefs_fs_finalize( WHEFSApp.fs );
	WHEFSApp.fs = 0;
    }
}

void WHEFSApp_show_help()
{
    printf("Usage: %s %s\n\n",
	   WHEFSApp.appName,
	   WHEFSApp.usageText );
    printf( "%s\n\n", WHEFSApp.helpText );
    puts("This program is for use with libwhefs:\n");
    printf("\t\t%s\n\n", whefs_home_page_url());
    puts("The first non-flag argument must be a EFS container file.\n");

    puts("Shared arguments supported by the core whefs tools:\n");
    ArgSpec_show_help( WHEFSApp_SharedArgs );

    if( WHEFSApp.argsSpec && WHEFSApp.argsSpec[0].name )
    {
	printf("\nArguments specific to %s:\n\n", WHEFSApp.appName);
	ArgSpec_show_help( WHEFSApp.argsSpec );
    }
    puts("");
}

enum WHEFSApp_OpenModes {

/**
   Tells WHEFSApp_init() to open the FS read-only.
*/
WHEFSApp_OpenRO = -1,
/**
   Tells WHEFSApp_init() not to open the FS.
*/
WHEFSApp_NoOpen = 0,
/**
   Tells WHEFSApp_init() to open the FS read/write.
*/
WHEFSApp_OpenRW = 1

};

/**
   Opens WHEFSApp.fs using the filename WHEFSApp.fsName. openMode must
   come from whe WHEFSApp_OpenModes enum.

   Returns whefs_rc.OK on success.
*/
int WHEFSApp_openfs( int openMode )
{
    if( WHEFSApp.fs || !WHEFSApp.fsName ) return whefs_rc.ArgError;
    if( (openMode != WHEFSApp_OpenRO) && (openMode != WHEFSApp_OpenRW) ) return whefs_rc.RangeError;
    VERBOSE("Opening EFS [%s]\n", WHEFSApp.fsName);
    int rc = whefs_openfs( WHEFSApp.fsName, &WHEFSApp.fs, (openMode == WHEFSApp_OpenRW) );
    if( whefs_rc.OK != rc )
    {
	APPERR("Error code #%d while opening EFS file [%s].\n", rc, WHEFSApp.fsName );
    }
    return rc;
}

void WHEFSApp_show_version()
{
    printf("libwhefs with file format version: [%s]\n",
	   whefs_data_format_version_string() );
    printf("Magic string of the linked-in libwhefs: [%s]\n",
	   whefs_fs_options_default.magic.data );
    printf("Magic string this app was compiled again: [%s]\n",
	   WHEFS_MAGIC_STRING );

    printf("libwhefs home page: %s\n", whefs_home_page_url() );

}
/**
   Initializes the app. argc and argv should come directly from main.

   openMode must be one of the WHEFSApp_OpenModes values.

   If gotHelp is not null then it will be set to true if this function
   thinks that the user needs help. That is: (A) if no arguments are
   given or (B) if -? or --help is given. If it sets this value to
   true then it calls the built-in help system. When it shows help it
   returns whefs_rc.ArgError but the application should conventionally
   quit with code 0 i that case.

   argspec is an array with a terminating entry which has a NULL
   name. argspec may be null, in which case it is ignored (but an
   internal/common args list is still checked). If it is not null,
   then argv is compared against it to look for application arguments.
*/
int WHEFSApp_init( int argc,
		   char const ** argv,
		   int openMode /* should be from WHEFSApp_OpenModes enum */,
		   bool * gotHelp,
		   ArgSpec * argspec )
{
    if( ! argc || !argv ) return whefs_rc.ArgError;
    WHEFSApp.appName = argv[0];
    WHEFSApp.argsSpec = argspec;
    WHEFSApp.debugFlags = 0;
    atexit( WHEFSApp_atexit );
    whefs_setup_debug( stderr, (unsigned int)-1 );
    //whefs_setup_debug_arg( stderr, "" );
    if( gotHelp ) *gotHelp = false;

    if( 1 == argc )
    {
        if( gotHelp )
        {
            *gotHelp = true;
            WHEFSApp_show_help();
        }
        return whefs_rc.ArgError;
    }
    else
    {
        whargv_global_parse( argc - 1, argv + 1 );
    }

    const size_t gac = whargv_global.argc;
    whargv_entry * garg = whargv_global.argv;
    size_t i = 0;
    int rc = whefs_rc.OK;
    bool setDebug = false;
#define RC if ( whefs_rc.OK != rc ) return rc
    for( ; (i < gac) && garg; ++i, ++garg )
    {
#if 0
	MARKER("arg entry: key=[%s] val=[%s] is_long=%d is_dash=%d is_numflag=%d\n",
	       gargs->key, gargs->val, gargs->is_long, gargs->is_dash, gargs->is_numflag );
#endif

	if( garg->is_help )
	{
	    if( gotHelp ) *gotHelp = true;
	    WHEFSApp_show_help();
	    return 0;
	}
        if( !setDebug && WHEFSApp.debugFlags )
        {
            VERBOSE("Setting debug flags to [%s]\n",WHEFSApp.debugFlags);
            whefs_setup_debug_arg( stderr, WHEFSApp.debugFlags );
            setDebug = true;
        }

	if( garg->is_nonflag )
	{
	    if( ! WHEFSApp.fsName )
	    {
		WHEFSApp.fsName = garg->val;
		if( WHEFSApp_NoOpen != openMode )
		{
		    rc = WHEFSApp_openfs( openMode );
		    RC;
		}
		continue;
	    }
	    else
	    {
		//VERBOSE("Appending file [%s] to list\n", garg->val );
		rc = WHEFSApp_fe_append( garg->val );
		RC;
		continue;
	    }
	}
	if( (0 == strcmp("V", garg->key )
	     || (0 == strcmp("version", garg->key )) ) )
	{
	    *gotHelp = true;
	    WHEFSApp_show_version();
	    return 0;
	}
	if( (0 == strcmp("v", garg->key )
	     || (0 == strcmp("verbose", garg->key )) ) )
	{
	    WHEFSApp.verbose = true;
	    VERBOSE("Verbose mode activated.\n");
	    continue;
	}
        ArgSpec * arr[2] = {0,0};
        arr[0] = &WHEFSApp_SharedArgs[0];
        arr[1] = argspec;
        bool gotSpec = false;
        int i = 0;
        for( i = 0; i < 2; ++i )
	{
            ArgSpec * as = arr[i];
	    for( ; as && as->name; ++as )
	    {
		gotSpec = (0 == strcmp( as->name, garg->key ));
                //APPMSG("Trying arg [%s] vs [%s] rc=%d\n",garg->key,as->name,gotSpec);
		if( ! gotSpec ) continue;
                //APPMSG("Matched arg [%s] vs [%s] rc=%d. val=[%s]\n",garg->key,as->name,rc,garg->val);
		rc = ArgSpec_convert( as, garg->val );
		if( whefs_rc.OK != rc )
		{
		    APPERR("Error parsing flag '%s'!\n", garg->key );
		    return rc;
		}
		if( as->callback )
		{
		    int rc = as->callback( as->name, garg->val, as->cbData );
		    if( 0 != rc ) return rc;
		}
                break;
	    }
            if( gotSpec ) break;
	}
        if( ! gotSpec )
        {
            APPERR("Unknown argument flag '%s'\n", garg->key );
            return whefs_rc.ArgError;
        }

    }
#undef RC
    if( !setDebug && WHEFSApp.debugFlags )
    {
        VERBOSE("Setting debug flags to [%s]\n",WHEFSApp.debugFlags);
        whefs_setup_debug_arg( stderr, WHEFSApp.debugFlags );
        setDebug = true;
    }

    if( !WHEFSApp.fsName  )
    {
        if( (openMode != WHEFSApp_NoOpen) && ! WHEFSApp.fs)
        {
            APPERR("No EFS file specified! The EFS file must be the first non-flag argument.\n");
            return whefs_rc.ArgError;
        }
    }
    return whefs_rc.OK;
}

