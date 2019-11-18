#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

int main( int argc, char **argv ) {
	if( argc <= 1 ) {
		fprintf( stderr, "Usage: %s command [arg1 [arg2 ...]]\n", argv[ 0 ] );
		exit( EXIT_FAILURE );
	}

	pid_t child_pid = fork();

	if( child_pid == 0 ) {
		if ( execvp( argv[ 1 ], argv + 1 ) == -1 ) {
			perror( "execvp() failure" );
		}
	} else if( child_pid != -1 ) { 
		siginfo_t waitResult;
		int a;
		scanf( "%d", &a );

	/*	if( waitid( P_PID, child_pid,
			    &waitResult, WEXITED ) == -1 ) {
			perror( "waitid() failure" );
			exit( EXIT_FAILURE );
		}

		switch( waitResult.si_code ) {
			case CLD_EXITED:
				printf( "Exitcode: %d\n", waitResult.si_status );
				break;
			case CLD_KILLED:
				printf( "Killed by signal: %d\n", waitResult.si_status );
				break;
			case CLD_DUMPED:
				printf( "Dumped core, killed by signal: %d\n", waitResult.si_status );
				break;
			case CLD_STOPPED:
				printf( "Stopped by signal: %d\n", waitResult.si_status );
				break;
			case CLD_TRAPPED:
				printf( "Trapped by signal: %d\n", waitResult.si_status );
				break;
			case CLD_CONTINUED:
				printf( "Continued by signal: %d\n", waitResult.si_status );
				break;
			default:
				fprintf( stderr, "I do not know what has happened\n" );
		}*/

		exit( EXIT_SUCCESS );
	} else {
		perror( "fork() failure" );
		exit( EXIT_FAILURE );
	}
}
