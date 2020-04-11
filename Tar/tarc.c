/*
 * Conner Brinkley
 * October 17, 2019
 * Systems Programming
 *
 * Lab 4 – jtar
 * Part I – tarc.c: Creating a Tar file
 *
 * */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include "fields.h"
#include "jval.h"
#include "dllist.h"
#include "jrb.h"


/* FORMAT THE TAR FILE */

void formatTar(char *path, char *relativePath, struct stat *buf, int unique, int file) {
	
	// Variables
	FILE *f;
	char c;
	unsigned long inode, modTime, fsize;
	unsigned int size, mode;

	// First 4 bytes are the length of the filename
	size = (unsigned int) strlen(relativePath);
	fwrite(&size, sizeof(int), 1, stdout);

	// Print the name
	printf("%s", relativePath);

	// Next 8 bytes are the inode
	inode = (long) buf->st_ino;
	fwrite(&inode, sizeof(long), 1, stdout);

	// For a unique inode
	if (unique) {

		// Next 4 bytes are the mode
		mode = (unsigned int) buf->st_mode;
		fwrite(&mode, sizeof(int), 1, stdout);

		// Next 8 bytes are the mod time
		modTime = (unsigned long) buf->st_mtime;
		fwrite(&modTime, sizeof(long), 1, stdout);
	}

	// For a file, NOT a directory
	if (file) {
		
		// Next 8 bytes are the size of the file
		fsize = (unsigned long) buf->st_size;
		fwrite(&fsize, sizeof(long), 1, stdout);

		// Print out all the bytes of the file
		f = fopen(path, "r");
		c = fgetc(f);
		while (c != EOF) {
			printf("%c", c);
			c = fgetc(f);
		}
		fclose(f);
	}
}


/* INITIALIZE TAR FILE */
void initTar(char *path, char *absolutePath) {
	
	// Make sure the argument is valid
	DIR *directory = opendir(path);
	if (directory == NULL) { fprintf(stderr, "Bad argument %s\n", path); exit(2); }

	// Get the info on the directory
	struct stat buf;
	int exists = lstat(path, &buf);

	// Find the relative path, given the absolute path
	char *relativePath;
	if (absolutePath == NULL) relativePath = path;
	else {
		int index = strlen(absolutePath);
		relativePath = (char *) &path[index];
	}

	// Print out the relevant information
	if (exists >= 0) formatTar(path, relativePath, &buf, 1, 0);	
	closedir(directory);
}


/* CREATES THE REST OF THE TAR FILE */

void createTar(char *filename, char *absolutePath, JRB inodes) {
	
	// Variables
	char *path, *relativePath;
	DIR *directory;
	struct dirent *entry;
	struct stat buf;
	int exists, unique, file;
	Dllist directories, tmp;
	
	// Try to open the directory
	directory = opendir(filename);
	if (directory == NULL) { fprintf(stderr, "Couldn't open directory %s\n", filename); exit(2); }
	
	// Create enough space for the path and the directories
	path = (char *) malloc(sizeof(char) * (strlen(filename) + 258));	
	directories = new_dllist();

	// Traverse the directory
	for (entry = readdir(directory); entry != NULL; entry = readdir(directory)) {
		
		// Look for filename/entry->d_name
		sprintf(path, "%s/%s", filename, entry->d_name);	
		exists = lstat(path, &buf); 
		unique = 0;
		file = 0;
		if (exists < 0) fprintf(stderr, "%s not found.\n", path);

		// Print the filename as long as it's not . or ..
		else if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)) {
			
			// Only print out hard links and directories, not soft links
			if (!S_ISLNK(buf.st_mode)) {

				// Check if the inode has already been added to the tree to avoid printing redundant info
				if (jrb_find_int(inodes, buf.st_ino) == NULL) {
					unique = 1;
					jrb_insert_int(inodes, buf.st_ino, JNULL);
				}
				
				// Check to see if the path is a file or a directory
				if (!S_ISDIR(buf.st_mode) && unique) file = 1;

				// Find the relative path, given the absolute path
				if (absolutePath == NULL) relativePath = path;
				else {
					int index = strlen(absolutePath);
					relativePath = (char *) &path[index];
				}
				
				// Print out the info
				formatTar(path, relativePath, &buf, unique, file);
			}
		}
		
		// Recurse if the file is a directory EXCLUDING . and ..
		if (S_ISDIR(buf.st_mode) 
			&& (strcmp(entry->d_name, ".") != 0) 
			&& (strcmp(entry->d_name, "..") != 0)) 
		{	
			dll_append(directories, new_jval_s(strdup(path)));
		}
	}
	
	// Clean up the directory calls
	closedir(directory);

	// Traverse the other directories
	dll_traverse(tmp, directories) {
		createTar(tmp->val.s, absolutePath, inodes);
		free(tmp->val.s);
	}

	// Let the memories go
	free(path);
	free_dllist(directories);
}


/* DRIVER CODE */

int main(int argc, char **argv) {

	// Check to see there is the correct amout of arguments
	if (argc > 2) { fprintf(stderr, "Too many arguments.\n"); exit(1); }
	else if (argc < 2) { fprintf(stderr, "Not enough arguments.\n"); exit(1); }

	// Find the absolute path
	char *tmp, *relativePath, *absolutePath;
	if (argv[1][0] == '/') {
		tmp = strdup(argv[1]);	
		relativePath = strrchr(tmp, '/');
		relativePath++;
		relativePath[0] = '\0';
		absolutePath = tmp;
	} else {
		absolutePath = NULL;
	}

	// Tar it up and return
	JRB inodes = make_jrb();
	initTar(argv[1], absolutePath);
	createTar(argv[1], absolutePath, inodes);
	
	// Housekeeping
	free(tmp);
	jrb_free_tree(inodes);
	return 0;
}

