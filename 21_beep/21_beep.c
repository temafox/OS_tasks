#include <bits/types/__sigset_t.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>

int sigpipe[ 2 ];

void sigint_handler( int sigint ) {
	write( STDOUT_FILENO, "!", sizeof( char ) * 2 );
	write( sigpipe[ 1 ], "I", 1 );
}

void sigquit_handler( int sigquit ) {
	write( sigpipe[ 1 ], "Q", 1 );
}

int main() {
	__sigset_t empty_mask;
	sigemptyset( &empty_mask );
	
	struct sigaction sigint_action;
	sigint_action.sa_mask = empty_mask;
	sigint_action.sa_flags = 0;
	sigint_action.sa_handler = &sigint_handler;

	sigaction( SIGINT, &sigint_action, NULL );

	struct sigaction sigquit_action;
	sigquit_action.sa_mask = empty_mask;
	sigquit_action.sa_flags = 0;
	sigquit_action.sa_handler = &sigquit_handler;

	sigaction( SIGQUIT, &sigquit_action, NULL );

	/////////////////////////////////////////////////

	pipe( sigpipe );

	fd_set readfds;
	FD_ZERO( &readfds );
	FD_SET( sigpipe[ 0 ], &readfds );

	int sigint_count = 0;

	while( 1 ) {
		select( 1, &readfds, NULL, NULL, NULL );
		
		char c;
		read( sigpipe[ 0 ], &c, 1 );

		switch( c ) {
			case 'I': ++sigint_count;
				  break;
			case 'Q': printf( "SIGINT has been received %d times\n", sigint_count );
				  exit( EXIT_SUCCESS );
			default: exit( EXIT_FAILURE );
		}
	}
}
