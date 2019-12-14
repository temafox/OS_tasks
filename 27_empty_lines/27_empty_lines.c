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

	sprintf( command, "set -o pipefail ; grep -E '^$' %s | wc -l", filename );

	FILE *sh = popen( command, "r" );
	if( sh == NULL ) {
		fprintf( stderr, "popen() failed\n" );
		perror( NULL );
		free( command );
		exit( EXIT_FAILURE );
	}

	int empty_lines;
	fscanf( sh, "%d", &empty_lines );

	int shstatus = pclose( sh );
	if( shstatus == -1 )
		perror( "pclose() error\n" );
	else if( !WIFEXITED( shstatus ) )
		fprintf( stderr, "Abnormal termination of shell\n" );
	else if( WEXITSTATUS( shstatus ) > 1 )
		fprintf( stderr, "Shell returned an error\n" );
	else // Everything OK _or_ grep returned 1 = no empty lines
		printf( "%d empty lines in %s\n", empty_lines, filename );

	free( command );
	exit( EXIT_SUCCESS );
}
