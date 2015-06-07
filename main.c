/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>					//atoi, exit
#include <string.h> 					//memset
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>  					//fork
#include <pthread.h>
#include <errno.h>
#include "print_error.h" 				//error-handling function => void error(char *msg)
#include "talk_to_client.h"  				//child-process read-socket
#include "create_and_bind_socket.h"
#include "make_socket_non_blocking.h"

#define MAX_EVENTS 64

int main(int argc, char *argv[]){

	if (argc < 2)
		error("ERROR, no port provided\n");

	int listen_socket;

	if ((listen_socket = create_and_bind_socket(argv[1])) == -1)
		error("Failed in create_and_bind_socket");

	if (make_socket_non_blocking(listen_socket) == -1)
		error("Error in setting non_block");

     	if (listen(listen_socket,SOMAXCONN) == -1)
		error("Listen failed");


//THREADING INITIALIZATION

      struct sockaddr_in cli_addr;
      socklen_t addrlen;

      struct arg_struct {
      	int conn_socket;
      	pthread_key_t thr_id_key;
      	int * thread_id;
      };

	struct arg_struct args;

	pthread_t thread;
	int * thread_id;
	pthread_key_t thr_id_key;
	thread_id = (int * ) malloc(sizeof(int));
	pthread_key_create(&thr_id_key, NULL);

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	int epollfd, nfds, i, conn_socket;
	struct epoll_event event;
	struct epoll_event events[MAX_EVENTS];
	
	if( (epollfd = epoll_create1 (0)) == -1)
		error ("epoll_create");
	
    //typedef union epoll_data {
    //	void    *ptr;
    //	int      fd;
    //	uint32_t u32;
    //	uint64_t u64;
    //} epoll_data_t;
    //
    //struct epoll_event {
    //	uint32_t     events;    /* Epoll events */
    //	epoll_data_t data;      /* User data variable */
    //	};

	event.events = EPOLLIN;
	event.data.fd = listen_socket;

	if (epoll_ctl (epollfd, EPOLL_CTL_ADD, listen_socket, &event) == -1)
		error ("epoll_ctl fail");
	
	while(1){
		
		printf("Waiting for connections....\n");

		if((nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) == -1)    //Returns number of events waiting
			error("epoll_pwait");

		for (i = 0; i < nfds; i++){

			if (events[i].data.fd == listen_socket) {

				if((conn_socket = accept(listen_socket, (struct sockaddr *) &cli_addr, &addrlen)) == -1)
					error("accept failed");

				if (make_socket_non_blocking(conn_socket) == -1)
					error("Error in setting non_block");
			
				event.events = EPOLLIN | EPOLLET;
				event.data.fd = conn_socket;

				if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_socket, &event)) == -1)
					error("epoll_ctl: conn_socket failed");
			}

			else{
				args.conn_socket = events[i].data.fd;
				args.thread_id = thread_id;

				if(pthread_create (&thread, &thread_attr, talk_to_client, &args) != 0)
					error("Error creating a thread.");
			}
		}
				
	}//end of while

	printf("Exited while(1) ?!?!? \n");
	pthread_exit(NULL);
	return 0; 					//should never get here
}
