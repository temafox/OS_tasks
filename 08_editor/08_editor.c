#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main( int argc, char **argv ) {
	if( argc <= 1 ) {
		fprintf( stderr, "Usage: %s filename", argv[ 0 ] );
		exit( EXIT_FAILURE );
	}

	char *filename = argv[ 1 ];
	;

	exit( EXIT_SUCCESS );
}
