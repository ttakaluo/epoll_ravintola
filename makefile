CC=gcc
CFLAGS= -Wall -I.
EXE_NAME = socket_server
FILES = print_error.o talk_to_client.o fifod.o make_socket_non_blocking.o main.o
LIBS = -lpthread
all: $(FILES)
	$(CC) -o $(EXE_NAME) $(FILES) $(CFLAGS) $(LIBS)

clean:
	rm -f *.o

run:
	./$(EXE_NAME)

rebuild: clean all run

