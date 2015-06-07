#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "print_error.h"
#include "talk_to_client.h"

void * talk_to_client(void * arguments){

	struct arg_struct *args;
	args = (struct arg_struct *) arguments;

	//this is a thread handling incoming communication
	char buffer[256];
	memset(buffer,0,256);

	int n;
	int thread_id = *(args -> thread_id);

	pthread_setspecific(args -> thr_id_key, args -> thread_id);
	
	int done = 0;
      do{
      	n = read(args -> conn_socket, buffer, sizeof(buffer));

 
		if (n == 0){
			done = 1;
      		break;      //return to main loop now
		}
		else if (n == -1){
      		//if errno is EAGAIN, we have read all data, return to main loop
      		if (errno != EAGAIN){
				perror("Read in process data failed");
			}
			//printf("I have read everything!\n");
			done = 1;
			break; //return to main loop, all data read!
		}	
      }
      while (!done);
	
	printf("Client[%d] said this: %s\n", thread_id, buffer);

	//closing the descriptor makes epoll remove it from the monitored set
	printf("Closed connection to connecting socket: %d\n", args -> conn_socket);

      close(args -> conn_socket);
      //write(args -> logfile, buffer, strlen(buffer));

	return 0;
}
