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
#include "fifod.h"  //contains hardcoded path	//create a fifo-pipe daemon
#include "make_socket_non_blocking.h"


#define LOGFILE "/tmp/logfile"
#define MAXEVENTS 64

int main(int argc, char *argv[]){

	if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
	}
	int logfile;
	logfile = open(LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777); //create log-file

	fifod(logfile);					//create fifo-daemon

	int sockfd;						//create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // ipv4 connection, sequenced two-way
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	struct sockaddr_in serv_addr;			 
	int portno;

	memset((char *) &serv_addr ,0,sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;   //accept all interfaces
	serv_addr.sin_port = htons(portno);		//convert network byte order to host byte order

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	if (make_socket_non_blocking(sockfd) == -1)
		error("Error in setting non_block");

     	if (listen(sockfd,5) == -1)
		error("Listen failed");

	struct sockaddr_in cli_addr;
	socklen_t clilen;

	struct arg_struct {
		int newsockfd;
		int logfile;
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

	int epollfd;
	
	epollfd = epoll_create1 (0);
		if (epollfd == -1)
			error ("epoll_create");
	
	struct epoll_event event;
	struct epoll_event *events;

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

	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET;

	if (epoll_ctl (epollfd, EPOLL_CTL_ADD, sockfd, &event) == -1)
		error ("epoll_ctl fail");
	
	events = calloc (MAXEVENTS, sizeof(event));


	while(1){
		
		printf("Waiting for connections....\n");

		int n, i;

		n = epoll_wait(epollfd, events, MAXEVENTS, -1);    //Returns number of events waiting
		int newsockfd;

		for (i = 0; i < n; i++){
			//Error occured in this fd
			if ((events[i].events & EPOLLERR) || 
		    	(events[i].events & EPOLLHUP) ||
		  	(!(events[i].events & EPOLLIN))) {

				fprintf(stderr, "epoll error\n");
				close (events[i].data.fd);
				continue;
			}

			//We have incoming connections

			else if (sockfd == events[i].data.fd){

				while(1){						//Add new connections to event-list
	
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					if (newsockfd == -1){

						if ((errno == EAGAIN) || (errno == EWOULDBLOCK)){
							printf("Al incoming processed.\n");
							break;  //all incoming connections processed.
						}
						else{
							printf("FAILFAIL");
							error("ERROR on accept");
						}
					}
					//maybe add getnameinfo printing here
	
					//make the new socket non-blocking
					if (make_socket_non_blocking(newsockfd) == -1)
						error("Failed to make newsockfd non-blocking");

					//add newsockfd to event_list
					event.data.fd = newsockfd;
					event.events = EPOLLIN | EPOLLET;
					if (epoll_ctl (epollfd, EPOLL_CTL_ADD, newsockfd, &event) == -1)
						error("epoll_Ctl failed on newsockfd");
				}
				printf("Doing continue now, back to while-loop\n");
				continue;
			}
			else{
				//We have data on the fd to read
				args.newsockfd = events[i].data.fd;
				args.logfile = logfile;
				args.thread_id = thread_id;

				printf("args.newsockfd = %d\n", args.newsockfd);

				int thread_count = 0;//this is wrong
				*thread_id = thread_count;

				printf("Creating thread\n");

				if(pthread_create (&thread, &thread_attr, talk_to_client, &args) )
				{
					printf("Error perror\n");
					error("Error creating a thread.");
 				}
			}
		}//end of for			

				
	}//end of while
	printf("Exited while(1) ?!?!? \n");
	pthread_exit(NULL);
	return 0; 					//should never get here
}
