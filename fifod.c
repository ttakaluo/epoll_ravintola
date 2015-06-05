#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "print_error.h"

//#define LOGFILE "/tmp/logfile"
#define FIFOFILE "/tmp/fifo_pipe"

int fifod(int logfile){

	int fifo_pipe, n;
	//int logfile;
	char buffer[255];

	pid_t childPid;
	childPid = fork();

	if (childPid > 0) {			//This is parent, returning.
      	return 0;
	}	
	else if (childPid == -1)
      	error("Problem while forking");

	n = mkfifo(FIFOFILE, 0777);		//create world-accessible pipe
	
//	logfile = open(LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777);

	while (1) {

		n = 1;
		fifo_pipe = open(FIFOFILE, O_RDONLY);

		while (n != 0) {

			memset(buffer, 0, 255);
			n = read(fifo_pipe, buffer, 255);

			if (n > 0) {

				write(logfile, buffer, strlen(buffer));

			}
		}
		close(fifo_pipe);
	}
	return 0;
}
