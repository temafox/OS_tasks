#include <bits/types/__sigset_t.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int sigint_count = 0;

void sigint_handler( int sigint ) {
	write( STDOUT_FILENO, "\a", sizeof( char ) * 2 );
	++sigint_count;
}

void sigquit_handler( int sigquit ) {
	char msg[50] = "\nSIGINT has been received 00000 times\n";
	for( int i = 30; i >= 26; --i ) {
		msg[ i ] = '0' + sigint_count % 10;
		sigint_count /= 10;
	}

	write( STDOUT_FILENO, msg, sizeof( char ) * 38 );
	exit( EXIT_FAILURE );
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

	while( 1 );
}
