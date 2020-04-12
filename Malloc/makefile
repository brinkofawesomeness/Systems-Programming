CC = gcc

INCLUDES = -I/home/jplank/cs360/include

CFLAGS = -g -Wall -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc $(INCLUDES)

LIBDIR = /home/jplank/cs360/objs

LIBS = $(LIBDIR)/libfdr.a 

DIR = /home/spatel84/ta/cs360_fall2019/lab6

EXECUTABLES = malloc.o famtree fakemake gradescript_famtree gradeall_famtree gradescript_fakemake gradeall_fakemake malloc-tester

all: $(EXECUTABLES)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c

malloc.o: malloc.c malloc.h
	$(CC) $(CFLAGS) -c malloc.c

famtree: malloc.o
	rm -f famtree.o
	cp $(DIR)/famtree.o .
	$(CC) $(CFLAGS) -o famtree famtree.o malloc.o $(LIBS)

fakemake: malloc.o
	rm -f fakemake.o
	cp $(DIR)/fakemake.o .
	$(CC) $(CFLAGS) -o fakemake fakemake.o malloc.o $(LIBS)

gradescript_famtree:
	rm -f gradescript_fametree
	cp $(DIR)/gradescript_famtree .

gradeall_famtree:
	rm -f gradeall_famtree
	cp $(DIR)/gradeall_famtree .

gradescript_fakemake:
	rm -f gradescript_fakemake
	cp $(DIR)/gradescript_fakemake .

gradeall_fakemake:
	rm -f gradeall_fakemake
	cp $(DIR)/gradeall_fakemake .

malloc-tester: malloc.o
	rm -f malloc-tester.c
	rm -f malloc-tester.o
	cp $(DIR)/malloc-tester.c .
	$(CC) $(CFLAGS) -c malloc-tester.c
	$(CC) $(CFLAGS) -o malloc-tester malloc-tester.o malloc.o $(LIBS)

#make clean will rid your directory of the executable,
#object files, and any core dumps you've caused
clean:
	rm -f core $(EXECUTABLES) *.o
	rm -f correct-script.sh first_three* fmakefile onefile* s1_header* testexec testlib.a testmain.c testmain.o tf* tmp* twoexec twofile* your-script.sh
