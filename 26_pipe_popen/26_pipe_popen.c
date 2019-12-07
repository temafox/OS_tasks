#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE ( 20 )

int main() {
	FILE *rd = popen( "cat", "r" );
	FILE *wr = popen( "cat", "w" );

	char buf[ BUFSIZE ];
	int n_read, n_written;

	while( !feof( rd ) ) {
		fgets( buf, sizeof( char ) * BUFSIZE, rd );
		n_read = strlen( buf );

		for( int i = 0; i <= n_read; ++i )
			buf[ i ] = toupper( buf[ i ] );
		
		n_written = fputs( buf, wr );
		fflush( wr );
		if( n_written == EOF ) {
			fprintf( stderr, "fputs() error\n" );
			break;
		}
	}

	if( pclose( rd ) == -1 )
		exit( EXIT_FAILURE );
	if( pclose( wr ) == -1 )
		exit( EXIT_FAILURE );
	exit( EXIT_SUCCESS );
}
