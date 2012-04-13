#!/bin/bash
########################################################################
# A quick hack to generate C code for static tables of free objects
# for use as a malloc() substitute.
#
# Run it without arguments to see the Usage text.
# Author: Stephan Beal (http://wanderinghorse.net/home/stephan/)
# License: Public Domain
########################################################################

class="$1"
count="${2-10}"
if [[ x = "${class}x" ]]; then
    cat <<EOF
Usage:
	$0 ClassName [static_table_size]

The output is C code providing allocation and deallocation
routines semantically equivalent to malloc() and free(), but
(A) specialized for objects of type ClassName and (B) they
use a statically-allocated list (with fairly good performance
characteristics) of ClassName objects, falling back to
malloc() and free() if the list fills up.

Warning: the generated allocation code is not thread-safe
(the deallocation code is).
EOF
exit 1
fi


malloc="${class}_alloc"
free="${class}_free"
#guard="$(echo ${class} | tr '[a-z]' '[A-Z]')_USE_STATIC_ALLOC"
guard="WHEFS_CONFIG_ENABLE_STATIC_MALLOC"
slotsobj="${malloc}_slots"
countvar="${malloc}_count"
protomacro="${slotsobj}_${class}_INIT"

########################################################################
# Create the API docs and function declarations...
cat <<EOF

/**
   Works like malloc(), but beware...

   Creates an empty, non-functional ${class} object and returns it.
   The object must be populated by the caller and must eventually
   be destroyed by calling ${free}() AFTER the private parts
   of the object are cleaned up (see ${free}() for details).

   Clients of ${class} objects SHOULD NOT use this routine - it is
   intended for use by ${class} factory/initialization routines for
   reasons explained below.

   This function is not required to be either thread safe nor reentrant,
   and may use static data as storage. How the object is allocated by
   this routine is technically undefined, but we will say this:

   It is configured to statically allocate some unpublished number
   of objects. Each call hands out one of these objects. If they
   are all used up, it falls back to malloc().

   A side effect of the allocation rules is that objects returned from
   this function are not guaranteed to be valid if used after main()
   exits (e.g. via the C++ destructor of a statically allocated
   object, or possibly even atexit()) because the underlying objects
   might have been cleaned up already.

   If the static allocation pool is enabled (it is a compile-time
   decision), this operation is guaranteed to be O(1) until the
   compiled-in maximum number of objects is allocated. If the routine
   has to fall back to malloc() then it has the same performance as
   malloc() (making this determination is O(1)). As long as the list
   has at least 1 slot free, operation will be (at best) O(1) and O(N)
   at worst, where N is (at most) the number of pre-allocated slots
   which are currently in use minus 1 (but it can average much better
   than that, depending on the usage patterns of ${malloc}()
   and ${free}()).

   @see ${free}()
*/
${class} * ${malloc}();

/**
   Deallocates ${class} objects, but...

   BIG FAT HAIRY WARNING: this function DOES NOT do any type-specific
   cleanup.  Instead, it is intended to be called from a
   ${class}-specific cleanup routine after any private data owned by
   the object has been freed.

   This function effectively passes the given object to free(), but
   the implementation is free to use a different alloc/dealloc
   method. In any case, clients should treat obj as invalid after this
   call.

   This is an O(1) operation unless obj was internally allocated by
   malloc() (see ${malloc}()), in which case it has the same performance
   as free().

   @see ${malloc}()
*/
void ${free}( ${class} * obj );

EOF

########################################################################
# Create the implementation code...
cat <<EOF
#if !defined(${guard})
/**
   If ${guard} is true then we statically allocate
   ${countvar} ${class} objects to dole out via
   ${malloc}(), falling back to malloc() if the list is full.

   Depending on sizeof(${class}), we may not actually save much
   dynamic memory, but we potentially save many calls to malloc().
   That depends on the application, of course, but this idea
   was implemented for a library where saving calls to malloc()
   was a high-priority goal.
*/
#define ${guard} 1
#endif



#define ${protomacro} {0 /* FILL THIS OUT FOR ${class} OBJECTS! */}
#if ${guard}
enum {
/**
   The number of elements to statically allocate
   in the ${slotsobj} object.
*/
${countvar} = ${count}
};
static struct
{
    ${class} objs[${countvar}];
    char used[${countvar}];
    size_t next;
    const size_t count;
} ${slotsobj} = { { ${protomacro} }, {0}, 0, ${countvar} };
#endif
static const ${class} ${malloc}_prototype = ${protomacro};
#undef ${protomacro}

${class} * ${malloc}()
{
    ${class} * obj = 0;
#if ${guard}
    size_t i = ${slotsobj}.next;
    for( ; i < ${slotsobj}.count; ++i )
    {
	if( ${slotsobj}.used[i] ) continue;
	${slotsobj}.used[i] = 1;
	${slotsobj}.next = i+1;
	obj = &${slotsobj}.objs[i];
	break;
    }
#endif /* ${guard} */
    if( ! obj ) obj = (${class} *) malloc( sizeof(${class}) );
    if( obj ) *obj = ${malloc}_prototype;
    return obj;
}

void ${free}( ${class} * obj )
{
    if( ! obj ) return;
#if ${guard}
    if( (obj < &${slotsobj}.objs[0]) ||
	(obj > &${slotsobj}.objs[${slotsobj}.count-1]) )
    { /* it does not belong to us */
        *obj = ${malloc}_prototype;
	free(obj);
	return;
    }
    else
    {
	const size_t ndx = (obj - &${slotsobj}.objs[0]);
	${slotsobj}.used[ndx] = 0;
	if( ndx < ${slotsobj}.next ) ${slotsobj}.next = ndx;
	return;
    }
#else
    *obj = ${malloc}_prototype;
    free(obj);
#endif /* ${guard} */
}

EOF