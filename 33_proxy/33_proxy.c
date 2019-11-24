#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include "defines.h"
#include "parse.h"
#include "dns.h"
#include "tools.h"
#include "conn_manip.h"

int main( int argc, char **argv ) {
	printf( "***     Proxy server      ***\n" );

	int src_port, dst_port;
	char *program_call_name	= argv[ 0 ];
	char *src_port_name	= argv[ 1 ];
	char *dst_ip_name	= argv[ 2 ];
	char *dst_port_name	= argv[ 3 ];

	if( parse_argv( argc, argv,
			&src_port, &dst_port ) == -1 ) {
		print_usage_stderr( program_call_name );
		exit_abortmsg();
	}

	struct addrinfo *destination;
	int resolve_return = resolve_host_tcp_ipv4( dst_ip_name, dst_port_name, &destination );
	switch( resolve_return ) {
		case 0:
			print_server_settings( src_port, dst_ip_name, dst_port, destination );
			break;
		case EAI_SYSTEM:
			perror( "A system error while resolving host" );
			break;
		default:
			fprintf( stderr, "%s\n", gai_strerror( resolve_return ) );
	}
	if( resolve_return != 0 )
		exit_abortmsg();
	
	int listener = socket( AF_INET, SOCK_STREAM, 0 );
	if( listener == -1 ) {
		perror( "Unable to create a listener socket" );
		exit_abortmsg();
	}

	struct sockaddr_in listener_addr;
	listener_addr.sin_family       = AF_INET;
	listener_addr.sin_port         = htons( src_port );
	listener_addr.sin_addr.s_addr  = htonl( INADDR_ANY );

	if( bind( listener, ( struct sockaddr const * )&listener_addr, sizeof( struct sockaddr_in ) ) == -1 ) {
		perror( "Unable to bind the listener socket to an address" );
		close( listener );
		exit_abortmsg();
	}

	if( listen( listener, MAX_BACKLOG ) == -1 ) {
		perror( "Unable to listen on the listener socket" );
		close( listener );
		exit_abortmsg();
	}

	// From this moment on, we cannot drop errors
	// nor simply abort the whole server

	// [    0        1         2            2n-1        2n      ]
	// [ listener client1 destination1 ... clientn destinationn ]
	struct pollfd		pollfds[ TOTAL_SOCKET_COUNT ];
	struct sockaddr_in	address_info[ TOTAL_SOCKET_COUNT ];

	// There will always be the one listening socket, so it does not count
	int nclients = 0;
	
	// #0 is the listening connection
	address_info[ 0 ] = listener_addr;
	pollfds[ 0 ].fd = listener;
	pollfds[ 0 ].events = POLLIN;

	for ever {
		// We don't wait for more than a minute
		int poll_status = poll( pollfds, 2 * nclients + 1, 60000 /* millisec */ );
		if( poll_status == 0 && nclients == 0 ) {
			fprintf( stderr, "Timeout exceeded - no one connected\n" );
			break;
		} else if( poll_status == -1 ) {
			perror( "Error during a poll() call" );
		} else {
			// There is a connection to accept
			if( pollfds[ 0 ].revents & POLLIN ) {
				add_connection( pollfds, address_info, &nclients, destination );
				continue;
			}

			// Existing connections have changed state
			for( int i = 1; i <= 2 * nclients; ++i ) {
				int src = i, dst = ( i%2 == 0 ? i-1 : i+1 );

				if( pollfds[ src ].revents & POLLIN ) {
					if( has_errors_print( &( pollfds[ src ] ), src,
								&( address_info[ src ] ) ) == -1 )
						disconnect_pair( src, pollfds, address_info, &nclients );
					else if( pollfds[ src ].revents & POLLIN )
						resend_data_print_errors( pollfds, src, dst );
				}
			}
		}
	}

	for( int i = 0; i <= nclients; ++i )
		close( pollfds[ 2 * i - 1 ].fd );
	freeaddrinfo( destination );
	printf( "*** Successfully finished ***\n" );
	exit( EXIT_SUCCESS );
}
