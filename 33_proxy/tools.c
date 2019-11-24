#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "defines.h"
#include "tools.h"

void print_server_settings( int src_port, char const *dst_ip_name, int dst_port, struct addrinfo const *dst ) {
	char ip_address_dotted[ INET_ADDRSTRLEN + 1 ]; // max = xxx.xxx.xxx.xxx
	inet_ntop( AF_INET, ( char * )&( ( ( struct sockaddr_in * )( dst->ai_addr ) )->sin_addr.s_addr ), ip_address_dotted, INET_ADDRSTRLEN + 1 );

	printf( "Server settings:\n" );
	printf( "\tSource port: %d\n", src_port );
	printf( "\tDestination host name: %s\n", dst_ip_name );
	printf( "\tDestination host IP: %s\n", ip_address_dotted );
	printf( "\tDestination port: %d\n", dst_port );
}

void exit_abortmsg() {
	printf( "***        Aborted        ***\n" );
	exit( EXIT_FAILURE );
}

void print_usage_stderr( char const *program_call_name ) {
	fprintf( stderr, "Usage: %s src_port ( dst_ip | dst_name ) dst_port\n", program_call_name );
}
