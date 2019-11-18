#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main( int argc, char **argv ) {
	if( argc <= 1 ) {
		fprintf( stderr, "Usage: %s filename\n",
				argv[ 0 ] );
		exit( EXIT_FAILURE );
	}

	char *filename = argv[ 1 ];
	int fd = open(filename, O_RDWR);
	if( fd == -1 ) {
		perror( "File opening failure" );
		exit( EXIT_FAILURE );
	}

	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;

	if( fcntl( fd, F_SETLK, &fl ) == -1 ) {
		perror( "Lock acquisition failure" );
		close( fd );
		exit( EXIT_FAILURE );
	} else {
		char cmd[1024] = "/bin/vim ";
		strcpy( cmd + 9, filename );
		cmd[ 9 + strlen( filename ) ] = '\0';

		system( cmd );
	}

	close( fd );
	exit( EXIT_SUCCESS );
}
