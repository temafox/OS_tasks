#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>

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

	pid_t child_pid = fork();

	if( child_pid == 0 ) {
		if( execlp( "/bin/cat", "/bin/cat",
			    filename, NULL ) == -1 ) {
			perror( "execlp() failure" );
		}
	} else if( child_pid != -1 ) { 
		siginfo_t waitResult;

		if( waitid( P_PID, child_pid,
			    &waitResult, WEXITED ) == -1 ) {
			perror( "waitid() failure" );
			close( fd );
			exit( EXIT_FAILURE );
		}
		printf( "I am the parent\n" );
	} else {
		perror( "fork() failure" );
		close( fd );
		exit( EXIT_FAILURE );
	}

	close( fd );
	exit( EXIT_SUCCESS );
}
