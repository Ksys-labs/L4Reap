/**
   Converts input from stdin into an array of binary data for use in C.

   License: Public Domain

   Usage: app VariableName < input > output.c
*/
#include <stdio.h>

static char const * appName = 0;

void usage()
{
    printf("Usage: %s OBJECT_NAME < input > output.c\n\n", appName );
    puts("Converts input from stdin to a C unsigned char array named OBJECT_NAME "
	 "and a constant enum value named OBJECT_NAME_length containing the length "
	 "of the bytes.\n");
}

int main( int argc, char const ** argv )
{
    appName = argv[0];
    if( (argc != 2) || (argv[1][0] == '-') )
    {
	usage();
	return 1;
    }
    char const * varname = argv[1];
    enum { bufSize = 1024 * 8 };
    unsigned char buf[bufSize];
    size_t rd = 0;
    size_t i = 0;
    size_t flip = 0;

    printf( "unsigned char %s[] = {\n\t", varname);
    size_t size = 0;
    while( 0 != (rd = fread( buf, 1, bufSize, stdin ) ) )
    {
	size += rd;
	for(i = 0; i < rd; ++i )
	{
	    printf( "0x%02x", buf[i] );
	    if( !( (rd < bufSize) && (i == rd-1)) ) putchar(',');
	    if( 16 == ++flip )
	    {
		flip = 0;
		printf("\n\t");
	    }
	    else putchar(' ');
	}
    }
    printf("\n\t}; /* end %s */\n", varname );
    printf( "enum { %s_length = %uUL }; ", varname, size);
    //printf("enum { %s_length = sizeof(%s) };\n", varname, varname );
    return 0;
}
