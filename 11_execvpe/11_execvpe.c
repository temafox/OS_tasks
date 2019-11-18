#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

extern char **environ;

int execvpe( const char *file, char *const argv[], char *const envp[] ) {
	for( int i = 0; envp[i] != NULL; ++i ) {
		putenv( envp[ i ] );
	}

	return execvp( file, argv );
}

int main() {
	char *argv[] = { "/bin/sh", "-c", "./prog", NULL };
	char *envp[] = { "V=myval", NULL };

	if( execvpe( argv[0], argv, envp ) == -1 ) {
		perror( "execvpe() failure" );
		exit( EXIT_FAILURE );
	}

	exit( EXIT_SUCCESS );
}
