#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_LEN ( 50 )

int main( int argc, char** argv ) {
	if( argc < 2 ) {
		fprintf( stderr, "Usage: %s file\n", argv[ 0 ] );
		exit( EXIT_FAILURE );
	}
	const char *filename = argv[ 1 ];
	
	size_t command_size = BASE_LEN + strlen( filename );
	char *command = malloc( sizeof( char ) * command_size );
	if( command == NULL ) {
		fprintf( stderr, "malloc() failed\n" );
		exit( EXIT_FAILURE );
	}

	sprintf( command, "grep -E \"^$\" %s | wc -l", filename );

	FILE *sh = popen( command, "r" );
	if( sh == NULL ) {
		fprintf( stderr, "popen() failed\n" );
		perror( NULL );
		free( command );
		exit( EXIT_FAILURE );
	}

	int empty_lines;
	fscanf( sh, "%d", &empty_lines );
	printf( "%s contains %d empty lines\n", filename, empty_lines );

	free( command );
	exit( pclose( sh ) );
}
