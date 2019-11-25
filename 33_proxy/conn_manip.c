#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "defines.h"
#include "conn_manip.h"

int add_connection( struct pollfd *pollfds, struct sockaddr_in *address_info, int *nclients, struct addrinfo const *dest ) {
	// If we have MAX_CLIENTS clients, let pending
	// connections wait till a client disconnects (i.e. do nothing)
	if( *nclients == MAX_CLIENTS )
		return 0;

	struct pollfd *pollrecord_listener = &( pollfds[ 0 ] );
	struct pollfd *pollrecord_client = &( pollfds[ 2 * *nclients + 1 ] ),
		      *pollrecord_dest   = &( pollfds[ 2 * *nclients + 2 ] );
	struct sockaddr_in *addrinfo_client = &( address_info[ 2 * *nclients + 1 ] ),
			   *addrinfo_dest   = &( address_info[ 2 * *nclients + 2 ] );
	socklen_t addr_len = sizeof( struct sockaddr_in );
	pollrecord_client->fd = accept( pollrecord_listener->fd,
			( struct sockaddr * )addrinfo_client, &addr_len );
	pollrecord_client->events = POLLIN;

	if( pollrecord_client->fd == -1 ) {
		perror( "Error while accepting a connection" );
		return -1;
	}

	pollrecord_dest->fd = socket( AF_INET, SOCK_STREAM, 0 );
	pollrecord_dest->events = POLLIN;
	if( pollrecord_dest->fd == -1 ) {
		perror( "Error while creating a socket for the destination" );
		close( pollrecord_client->fd );
		return -1;
	}

	*addrinfo_dest = *( struct sockaddr_in * )( dest->ai_addr );
	if( connect( pollrecord_dest->fd,
			( struct sockaddr * )addrinfo_dest, addr_len ) == -1 ) {
		perror( "Error while connecting to the destination" );
		close( pollrecord_client->fd );
		close( pollrecord_dest->fd );
		return -1;
	}

	++( *nclients );
	printf( "Client #%d connected on port %d\n",
			*nclients, addrinfo_client->sin_port );
	return 0;
}

int has_errors_print( struct pollfd const *pollfd, int number, struct sockaddr_in const *address_info ) {
	number = ( number%2 == 0 ? number/2 : ( number + 1 )/2 );

	if( pollfd->revents & POLLHUP )
		fprintf( stderr, "Client #%d hang up on port %d\n", number, address_info->sin_port );
	else if( pollfd->revents & POLLNVAL )
		fprintf( stderr, "Client #%d invalid request on port %d\n", number, address_info->sin_port );
	else if( pollfd->revents & POLLERR )
		fprintf( stderr, "Client #%d error on port %d\n", number, address_info->sin_port );
	else
		return 0;

	return -1;
}

int send_nbytes( int fd, void *buf, ssize_t n ) {
	ssize_t position = 0;
	while( position < n ) {
		ssize_t sent = write( fd, ( char * )buf + position, n - position );
		if( sent == -1 )
			return -1;
		position += sent;
	}
	return 0;
}

int transmit_data_print_errors( struct pollfd *pollfds, int src, int dst ) {
	int src_fd = pollfds[ src ].fd;
	int dst_fd = pollfds[ dst ].fd;

	struct pollfd write_request;
	write_request.fd = dst_fd;
	write_request.events = POLLOUT;

	int output_state = poll( &write_request, 1, 0 );
	if( ! ( ( output_state > 0 ) && ( write_request.revents & POLLOUT ) ) ) {
		fprintf( stderr, "Cannot send data from entry %d to entry %d\n", src, dst );
		return -1;
	}

	char buf[ RECV_BUFFER_SIZE ];
	ssize_t received;

	received = recv( src_fd, buf, sizeof( char ) * RECV_BUFFER_SIZE, 0 );
	if( received == -1 ) {
		perror( "Data receipt error" );
		return -1;
	} else if( received > 0 ) {
		if( send_nbytes( dst_fd, buf, received ) == -1 ) {
			fprintf( stderr, "Transmission error\n" );
			return -1;
		}
		printf( "%ld bytes have been transmitted\n", received );
	}

	return 0;
}

int disconnect_pair( int one_of_pair, struct pollfd *pollfds, struct sockaddr_in *address_info, int *nclients ) {
	int retval = 0;

	int client, remote;
	int clientid;
	
	if( one_of_pair % 2 == 0 ) {
		remote = one_of_pair;
		client = remote - 1;
	} else {
		client = one_of_pair;
		remote = client + 1;
	}
	clientid = ( client + 1 ) / 2;
	
	if( close( pollfds[ client ].fd ) == -1 ) {
		perror( "Closing failure on client connection" );
		retval = -1;
	}
	if( close( pollfds[ remote ].fd ) == -1 ) {
		perror( "Closing failure on remote connection" );
		retval = -1;
	}

	printf( "Client #%d on port %d disconnected from remote server on port %d\n",
		clientid, address_info[ client ].sin_port, address_info[ remote ].sin_port );
	
	if( *nclients == 1 || clientid == *nclients )
		--( *nclients );
	else {
		// There would be a hole in the array of clients,
		// but we move the last client to make up for it
		
		pollfds[ client ] = pollfds[ 2 * *nclients - 1 ];
		pollfds[ remote ] = pollfds[ 2 * *nclients ];

		address_info[ client ] = address_info[ 2 * *nclients - 1 ];
		address_info[ remote ] = address_info[ 2 * *nclients ];

		printf( "Client #%d moved to #%d\n", *nclients, clientid );

		--( *nclients );
	}

	return retval;
}
