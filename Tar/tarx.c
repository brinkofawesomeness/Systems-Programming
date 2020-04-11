/*
 * Conner Brinkley
 * October 17, 2019
 * Systems Programming
 *
 * Lab 4 – jtar
 * Part II – tarx.c: Extracting from a Tar file
 *
 * */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <utime.h>
#include <fcntl.h>
#include <stdio.h>
#include "fields.h"
#include "jval.h"
#include "dllist.h"
#include "jrb.h"


/* FILE DATA */

typedef struct {
	char *filename;
	char *bytes;
	char *link;
	int mode;
	long inode;
	long modTime;
	long size;
} File;


/* EXTRACTING */

void extractTar() {

	// Variables
	int namesize, is_file, fd, read;
	struct utimbuf times;
	File *f, *stored;
	Dllist files, tmp;
	JRB inodes;

	// Open the file for reading and make structures
	inodes = make_jrb();
	files = new_dllist();

	// While we can read the size of a new filename
	while ((read = fread(&namesize, sizeof(int), 1, stdin))) {
		if (read != 1) { fprintf(stderr, "Couldn't read size of new filename.\n"); exit(1); }
		f = malloc(sizeof(File));

		// Read in the file name
		f->filename = (char *) malloc(namesize + 1);
		read = fread(f->filename, namesize, 1, stdin);
		if (read != 1) { fprintf(stderr, "Couldn't read in file name.\n"); exit(1); }
		f->filename[namesize] = '\0';

		// Read in the inode
		read = fread(&f->inode, sizeof(long), 1, stdin);
		if (read != 1) { fprintf(stderr, "Couldn't read in inode.\n"); exit(1); }
		is_file = 0;

		// If the inode is unique, store the data in a tree
		if (jrb_find_int(inodes, f->inode) == NULL) {
			f->link = NULL;

			// Read in the mode
			read = fread(&f->mode, sizeof(int), 1, stdin);
			if (read != 1) { fprintf(stderr, "Couldn't read in mode.\n"); exit(1); }
			if (!S_ISDIR(f->mode)) is_file = 1;

			// Read in the modification time
			read = fread(&f->modTime, sizeof(long), 1, stdin);
			if (read != 1) { fprintf(stderr, "Couldn't read in modification time.\n"); exit(1); }

			// If it's a file, read in more data
			if (is_file) {
				
				// Read in the file's size (in bytes)
				read = fread(&f->size, sizeof(long), 1, stdin);
				if (read != 1 || f->size < 0 || f->size > 1000) { fprintf(stderr, "Couldn't read in the file size.\n"); exit(1); }

				// Read in the file's bytes
				f->bytes = (char *) malloc(f->size);
				read = fread(f->bytes, f->size, 1, stdin);
				if (read != 1) { fprintf(stderr, "Couldn't read in the contents of the file.\n"); exit(1); }
			}

			// Insert it into the tree so we can find the info in case the inode isn't unique
			jrb_insert_int(inodes, f->inode, new_jval_v((void *) f));
		} 

		// If it's not, link it, then grab the mode and mod time from the inode
		else {
			// Link
			stored = (File *) jrb_find_int(inodes, f->inode)->val.v;
			f->link = strdup(stored->filename);

			// Mode and modtime
			f->mode = stored->mode;
			if (!S_ISDIR(f->mode)) is_file = 1;
			f->modTime = stored->modTime;

			// If it's a file, grab more data
			if (is_file) {
				f->size = stored->size;
				f->bytes = strdup(stored->bytes);
			}
		}

		// Store the file in a list to create the contents later
		dll_append(files, new_jval_v((void *) f));
	}

	// Create all of the files and directories
	dll_traverse(tmp, files) {
		f = (File *) tmp->val.v;
		if (S_ISDIR(f->mode)) mkdir(f->filename, 0777);
		else {

			// If inode isn't unique, create a hard link
			if (f->link != NULL) link(f->link, f->filename);

			// Else, create a new file
			else {
				fd = open(f->filename, O_WRONLY | O_CREAT | O_TRUNC);
				write(fd, f->bytes, f->size);
				close(fd);
			}
		}
	}
	
	// Go back through and set permissions and modification times
	dll_rtraverse(tmp, files) {
		f = (File *) tmp->val.v;
		chmod(f->filename, f->mode);
		times.modtime = f->modTime;
		utime(f->filename, &times);

		// Let the memories go
		if (f->link != NULL) free(f->link);
		free(f->filename);
		free(f->bytes);
		free(f);
	}

	// Free it up
	jrb_free_tree(inodes);
	free_dllist(files);
}


/* DRIVER CODE */

int main(int argc, char **argv) {

	// Extract the data
	extractTar();
	return 0;
}

