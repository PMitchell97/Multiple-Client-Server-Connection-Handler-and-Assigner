CC = gcc
MAKE = make
RM = rm

all: cliMajor2.c svrMajor2.c
	$(CC) -lpthread -o cliMajor2 cliMajor2.c
	$(CC) -lpthread -o svrMajor2 svrMajor2.c

clean: svrMajor2 cliMajor2
	$(RM) svrMajor2
	$(RM) cliMajor2
