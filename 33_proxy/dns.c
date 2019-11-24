#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "defines.h"
#include "dns.h"

int resolve_host_tcp_ipv4( char const *ip_name, char const *port_service, struct addrinfo **result ) {
	struct addrinfo hints;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	return getaddrinfo( ip_name, port_service, ( struct addrinfo * )&hints, result );
}


