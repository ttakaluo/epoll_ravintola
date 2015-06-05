#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "make_socket_non_blocking.h"

//static int make_socket_non_blocking(int sfd){
int make_socket_non_blocking(int sfd){

	int flags, s;

	// int fcntl(int fd, int cmd, ... /* arg */ );
	flags = fcntl (sfd, F_GETFL, 0);			//manipulate file descriptor
									//F_GETFL = get file access mode and
									//		file status flags
	if (flags == -1){
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);			//F_SETFL	= set file status flags to "arg"
									//in this case "arg" is O_NONBLOCK
	if (s == -1){
		perror ("fcntl");
		return -1;
	}

	return 0;
}
