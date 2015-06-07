#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include "create_and_bind_socket.h"

int create_and_bind_socket(char *port){

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s,sockfd;

	memset (&hints, 0, sizeof(struct addrinfo));

	hints.ai_family 	= AF_UNSPEC;				//IPv4 + IPv6
	hints.ai_socktype = SOCK_STREAM;				//TPC socket
	hints.ai_flags	= AI_PASSIVE;				//wildcard IP address
	hints.ai_protocol	= 0;						//any protocol
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo (NULL, port, &hints, &result);
			//internet host 	= (NULL here)
			//service 		= port number
			//hints 		= criteria for socket address structure
			//result		= pointer at start of the list

	if ( s != 0){
		fprintf(stderr, "getaddrinfo %s\n", gai_strerror (s));
		return -1;
	}
	//getaddrinfo returns a list of address structures, try each until bind

	for (rp = result ; rp != NULL; rp = rp -> ai_next){
	
		sockfd = socket(rp -> ai_family, rp -> ai_socktype, rp -> ai_protocol);
		
		if (sockfd == -1)
			continue;

		if ( bind(sockfd, rp -> ai_addr, rp -> ai_addrlen) == 0)
			break;						//Bind succeeded

		close(sockfd);
	}

	if (rp == NULL) {							//No address succeeded
		fprintf(stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo(result);						//No longer needed
	
	return sockfd;
}
