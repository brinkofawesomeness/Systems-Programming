# Specify how to compile

CC = gcc
CFLAGS = -Wall -Werror -g
INCLUDES = -I/home/huangj/cs360/include
LIBS = /home/cs360/objs/libfdr.a


# define the C source, object, and executable files

SRCS = l2p1.c l2p2.c l2p3.c
OBJS = $(SRCS:.c=.o)
EXECUTABLES = l2p1 l2p2 l2p3


# generic compilation

all: $(EXECUTABLES)

l2p1: l2p1.o
	$(CC) $(CFLAGS) $(INCLUDES) -o l2p1 l2p1.o $(LIBS)

l2p1.o: l2p1.c
	$(CC) $(CFLAGS) $(INCLUDES) -c l2p1.c

l2p2: l2p2.o
	$(CC) $(CFLAGS) $(INCLUDES) -o l2p2 l2p2.o $(LIBS)

l2p2.o: l2p2.c
	$(CC) $(CFLAGS) $(INCLUDES) -c l2p2.c

l2p3: l2p3.o
	$(CC) $(CFLAGS) $(INCLUDES) -o l2p3 l2p3.o $(LIBS)

l2p3.o: l2p3.c
	$(CC) $(CFLAGS) $(INCLUDES) -c l2p3.c

clean:
	$(RM) *.o $(EXECUTABLES)

