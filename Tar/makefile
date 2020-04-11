# Specify how to compile

CC = gcc
CFLAGS = -Wall -Werror -g
INCLUDES = -I/home/huangj/cs360/include
LIBS = /home/cs360/objs/libfdr.a


# define the C source, object, and executable files

SRCS = tarc.c tarx.c
OBJS = $(SRCS:.c=.o)
EXECUTABLES = tarc tarx


# generic compilation

all: $(EXECUTABLES)

tarc: tarc.o
	$(CC) $(CFLAGS) $(INCLUDES) -o tarc tarc.o $(LIBS)

tarc.o: tarc.c
	$(CC) $(CFLAGS) $(INCLUDES) -c tarc.c

tarx: tarx.o
	$(CC) $(CFLAGS) $(INCLUDES) -o tarx tarx.o $(LIBS)

tarx.o: tarx.c
	$(CC) $(CFLAGS) $(INCLUDES) -c tarx.c

clean:
	$(RM) *.o $(EXECUTABLES)

