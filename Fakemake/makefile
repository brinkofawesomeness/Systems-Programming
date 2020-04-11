CC = gcc
INCLUDES = -I/home/huangj/cs360/include
CFLAGS = -g $(INCLUDES)
LIBDIR = /home/cs360/objs
LIBS = $(LIBDIR)/libfdr.a
EXECUTABLES = fakemake

all: $(EXECUTABLES)

clean:
	rm $(EXECUTABLES)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c

fakemake: fakemake.o
	$(CC) $(CFLAGS) -o fakemake fakemake.o $(LIBS)
	rm *.o
