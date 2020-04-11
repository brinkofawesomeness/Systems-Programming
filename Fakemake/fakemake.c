/*
 * Conner Brinkley
 * 09.29.2019
 * CS 360 -- Lab 3 // Fakemake
 *
 * */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fields.h"
#include "jval.h"
#include "dllist.h"

/* Read files into lists */

void storeFiles(IS input, Dllist files) {	
	int i;
	char *tmp;
	for (i = 1; i < input->NF; i++) {
		dll_append(files, new_jval_s(strdup(input->fields[i])));
	}
}

/* Process header files */

time_t maxTimeStamp(Dllist list) {

	/* Variables */
	int i, exists;
	time_t max;
	Dllist tmp;
	struct stat info;

	/* Set the max to the first file, then see if any are greater */
	exists = stat(jval_s(dll_first(list)->val), &info);
	max = info.st_mtime;
	dll_traverse(tmp, list) {
		exists = stat(jval_s(tmp->val), &info);
		if (exists < 0) {
			fprintf(stderr, "Header file '%s' not found\n", jval_s(tmp->val));
			exit(4);
		}
		if (info.st_mtime > max) max = info.st_mtime;
	}

	/* Return the most recent modification */
	return max;
}

/* Process C files: returns 1 if files were recompiled, 0 otherwise */

int processC(Dllist list, Dllist flags, time_t headerTimeStamp) {
	
	/* Variables */
	int i, c_exists, o_exists, update = 0;
	struct stat c_info, o_info;
	char command[100];
	Dllist tmp, tmp2;

	/* Make sure each .c file actually exists */
	dll_traverse(tmp, list) {
		c_exists = stat(jval_s(tmp->val), &c_info);
		if (c_exists < 0) {
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", jval_s(tmp->val));
			exit(5);
		}
		
		/* Check to see if there is a .o file */
		char *o_file = strdup(jval_s(tmp->val));
		i = 0;
		while (o_file[i] != 'c') {
			i++;
		} o_file[i] = 'o';
		o_exists = stat(o_file, &o_info);
		if (o_exists < 0) {
			strcpy(command, "gcc -c ");
			dll_traverse(tmp2, flags) {
				strcat(command, jval_s(tmp2->val));
				strcat(command, " ");
			}
			strcat(command, jval_s(tmp->val));
			printf("%s\n", command);
			if (system(command) != 0) {
				fprintf(stderr, "Command failed.  Exiting\n");
				exit(7);
			}
			update = 1;
		} 
		
		/* If it does exist, see if the .c file is more recent */
		else {
			
			/* If it is more recent, compile again */
			if (c_info.st_mtime >= o_info.st_mtime) {
				strcpy(command, "gcc -c ");
				dll_traverse(tmp2, flags) {
					strcat(command, jval_s(tmp2->val));
					strcat(command, " ");
				}
				strcat(command, jval_s(tmp->val));
				printf("%s\n", command);
				if (system(command) != 0) {
					fprintf(stderr, "Command failed.  Exiting\n");
					exit(7);
				}
				update = 1;
			}

			/* Now check to see if the header files are more recent. If they are, compile again */
			if (!update) {
				if (headerTimeStamp >= o_info.st_mtime) {
					strcpy(command, "gcc -c ");
					dll_traverse(tmp2, flags) {
						strcat(command, jval_s(tmp2->val));
						strcat(command, " ");
					}
					strcat(command, jval_s(tmp->val));
					printf("%s\n", command);
					if (system(command) != 0) {
						fprintf(stderr, "Command failed.  Exiting\n");
						exit(7);
					}
					update = 1;
				}
			}
		}
		free(o_file);
	}
	return update;
}

/* Driver code */

int main (int argc, char **argv) {

	/* Variables */
	IS input;
	int i, exists = 0, changed = 0, E_flag = 0, old = 0, line = 1;
	struct stat info;
	time_t headerTimeStamp, objTimeStamp;
	char command[100];
	char *desc_file, *executable, *o_file;
	Dllist tmp, c_files, h_files, l_files, f_flags;

	/* Define the description file */
	if (argc == 1) desc_file = "fmakefile";
	else desc_file = argv[1];
	
	/* Initialize the data structures */
	input = new_inputstruct(desc_file);
	c_files = new_dllist();
	h_files = new_dllist();
	l_files = new_dllist();
	f_flags = new_dllist();
	
	/* Make sure the file exists */
	if (input == NULL) {
		fprintf(stderr, "%s No such file or directory\n", desc_file);
		exit(1);
	}

	/* Read the description file */
	while (get_line(input) >= 0) {

		/* Do nothing if it's a blank line */
		if (input->NF == 0);
		else {
			
			/* Read in the specification lines */
			if (strcmp(input->fields[0], "C") == 0) storeFiles(input, c_files);
			if (strcmp(input->fields[0], "H") == 0) storeFiles(input, h_files);
			if (strcmp(input->fields[0], "L") == 0) storeFiles(input, l_files);
			if (strcmp(input->fields[0], "F") == 0) storeFiles(input, f_flags);
			if (strcmp(input->fields[0], "E") == 0) {
				if (E_flag) {
					fprintf(stderr, "fmakefile (%d) cannot have more than one E line\n", line);
					exit(2);
				} else {
					executable = strdup(input->fields[1]);
					E_flag = 1;
				}
			}
		}
		line++;
	}

	/* Check to make sure we read an executable */
	if (!E_flag) {
		fprintf(stderr, "No executable specified\n");
		exit(3);
	}

	/* Process header files */
	if (!dll_empty(h_files)) {
		headerTimeStamp = maxTimeStamp(h_files);
	}

	/* Process the executable */
	if (!dll_empty(c_files)) {

		/* Start by processesing the .c files and seeing if the executable exists */
		changed = processC(c_files, f_flags, headerTimeStamp);
		exists = stat(executable, &info);

		/* Change the .c list into a .o list, then go through and find the max time stamp */
		dll_traverse(tmp, c_files) {
			(jval_s(tmp->val)[strlen(jval_s(tmp->val)) - 1]) = 'o';
		}
		objTimeStamp = maxTimeStamp(c_files);
		if (objTimeStamp > info.st_mtime) old = 1;
		
		/* If any of the above conditions are true, remake the executable */
		if (changed || (exists < 0) || old) {
			strcpy(command, "gcc -o ");
			strcat(command, executable);
			dll_traverse(tmp, f_flags) {
				strcat(command, " ");
				strcat(command, jval_s(tmp->val));
			}
			dll_traverse(tmp, c_files) {
				strcat(command, " ");
				strcat(command, jval_s(tmp->val));
			}
			dll_traverse(tmp, l_files) {
				strcat(command, " ");
				strcat(command, jval_s(tmp->val));
			}
			printf("%s\n", command);
			if (system(command) != 0) {
				fprintf(stderr, "Command failed.  Fakemake exiting\n");
				exit(7);
			}
		} else {
			printf("%s up to date\n", executable);
			exit(6);
		}
	}

	/* Housekeeping */
	jettison_inputstruct(input);
	free_dllist(c_files);
	free_dllist(h_files);
	free_dllist(l_files);
	free_dllist(f_flags);
	free(executable);
	exit(0);
}
