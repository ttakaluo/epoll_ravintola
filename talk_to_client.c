#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "print_error.h"
#include "fifod.h"
#include "talk_to_client.h"

void * talk_to_client(void * arguments){

	printf("talk_to_client running\n");
	//this is a thread handling incoming communication
	char buffer[256];
	memset(buffer,0,256);

	int n;
	struct arg_struct *args;
	args = (struct arg_struct *) arguments;
	int thread_id = *(args -> thread_id);

	pthread_setspecific(args -> thr_id_key, args -> thread_id);
	
	int done = 0;
      do{
      	n = read(args -> newsockfd, buffer, sizeof(buffer));

      	if (n == -1){
      		//if errno is EAGAIN, we have read all data, return to main loop
      		if (errno != EAGAIN){
				perror("Read in process data failed");
			}
			printf("I have read everything!\n");
			done = 1;
			break; //return to main loop, all data read!
		}
		else if (n == 0){   //EOF, remote has closed the connection
			done = 1;
      		break;      //return to main loop now
		}
      }
      while (!done);

	//closing the descriptor makes epoll remove it from the monitored set
	printf("Closed connection to descriptor%d\n", args -> newsockfd);

      close(args -> newsockfd);
      write(args -> logfile, buffer, strlen(buffer));

	printf("Client thread [%d] is done, exiting.\n", thread_id );
	return 0;
}
