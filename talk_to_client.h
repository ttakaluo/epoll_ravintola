#ifndef TALK_TO_CLIENT_H
#define TALK_TO_CLIENT_H

void * talk_to_client(void * arg);

struct arg_struct {

	int newsockfd;
	int logfile;
	pthread_key_t thr_id_key;
	int * thread_id;
};

#endif
