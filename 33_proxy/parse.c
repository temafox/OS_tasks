#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "parse.h"

int parse_argv( int argc, char **argv, int *src_port, int *dst_port ) {
	if( argc != 4 )
		return -1;
	
	int src_port_res;
	if( ( src_port_res = cvt_str_int( argv[ 1 ] ) )  == -1 )
		return -1;

	int dst_port_res;
	if( ( dst_port_res = cvt_str_int( argv[ 3 ] ) ) == -1 )
		return -1;

	*src_port = src_port_res;
	*dst_port = dst_port_res;
	return 0;
}

int cvt_str_int( char const *str ) {
	// if( str == NULL ) -> null pointer access error
	char *endptr;
	int res = strtol( str, &endptr, 0 );

	if( *str != '\0' && *endptr == '\0' ) // see man strtol(3)
		return res;
	else
	// You won't know if this is a mistake or not!
		return -1;
}
