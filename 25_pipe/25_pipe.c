#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define PARBUFSIZE ( 10 )
#define CHIBUFSIZE ( 10 )

int parent( int *pipefd ) {
	close( pipefd[ 0 ] );
	int fd = pipefd[ 1 ];

	char input[ PARBUFSIZE ];

	while( 1 ) {
		int read_status = read( 0, input, PARBUFSIZE );
		if( read_status == -1 ) {
			perror( "read() from stdin failure in parent" );
			close( fd );
			return -1;
		} else if( read_status == 0 ) {
			close( fd );
			return 0;
		} else {
			int write_status = write( fd, input, read_status );
			if( write_status == -1 ) {
				perror( "write() to pipe failure in parent" );
				close( fd );
				return -2;
			}
		}
	}
}

int child( int *pipefd ) {
	close( pipefd[ 1 ] );
	int fd = pipefd[ 0 ];

	char input[ CHIBUFSIZE ];
	char uppercase[ CHIBUFSIZE ];
	while( 1 ) {
		int read_status = read( fd, input, CHIBUFSIZE );
		if( read_status == -1 ) {
			perror( "read() from pipe failure in child" );
			close( fd );
			return -1;
		} else if( read_status == 0 ) {
			close( fd );
			return 0;
		} else {
			for( int i = 0; i < read_status; ++i )
				uppercase[ i ] = toupper( input[ i ] );

			int write_status = write( 1, uppercase, read_status );
			if( write_status == -1 ) {
				perror( "write() to stdout failure in child" );
				close( fd );
				return -2;
			}
		}
	}
}

int main() {
	int pipefd[2];
	
	if( pipe( pipefd ) == -1 ) {
		perror( "pipe() failure" );
		exit( EXIT_FAILURE );
	}

	int iamparent = fork();
	if( iamparent == -1 ) {
		perror( "fork() failure" );
		close( pipefd[ 0 ] );
		close( pipefd[ 1 ] );
	}
	else if( iamparent )
		return parent( pipefd );
	else
		return child( pipefd );
}
