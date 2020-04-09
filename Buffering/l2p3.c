/*
 * Conner Brinkley
 * October 20, 2019
 * Systems Programming
 *
 * Lab 2 -- Buffering
 * Part III -- l2p2.c: Storing machine information using only buffered system calls.
 *
 * */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "fields.h"
#include "dllist.h"
#include "jval.h"
#include "jrb.h"


/* MACHINE DATA */

typedef struct {
	unsigned char ip[4];  // holds IP address
	JRB names;            // holds all names associated with this IP
} Machine;


/* PRINT DATA */

void printNode(JRB node) {

	// Variables
	int i;
	JRB ptr;
	Machine *m;

	// Print IP address
	m = (Machine *) node->val.v;
	for (i = 0; i < 3; i++) printf("%d.", m->ip[i]);
	printf("%d:  ", m->ip[3]);
	
	// Print list of names
	jrb_traverse(ptr, m->names) {
		printf("%s ", ptr->key.s);
	}
	printf("\n\n");
}


/* READS IN ALL THE HOSTS */

void readHosts(const char *filename, JRB master) {
	
	// Variables
	Machine *m;
	char name[50];
	unsigned char *c, *buffer;
	int count, local, size, ptr, fd, i, n;

	// Open the converted file
	fd = open(filename, O_RDONLY);
	if (fd < 0) { perror(filename); exit(1); }

	// Read the file into the buffer
	c = malloc(sizeof(char));
	buffer = malloc(sizeof(char) * 350000);
	size = read(fd, buffer, 350000);

	// Read from the buffer
	ptr = 0;
	while (ptr != size) {
		
		// Initialize data and read in the first 4 bytes (IP address)
		m = malloc(sizeof(Machine));
		m->names = make_jrb();
		for (i = 0; i < 4; i++) {
			m->ip[i] = buffer[ptr++];
		}

		// Read in the # of names by bit shifting because the number is in big endian
		count = 0;
		for (i = sizeof(int) - 1; i >= 0; i--) {
			count |= (buffer[ptr++] << (8 * i));
		}

		// Read in [count] names and store them into a tree
		for (n = 0; n < count; n++) {
			memset(name, 0, sizeof(name));
			i = 0;
			local = 1;
			while (ptr != size && buffer[ptr] != '\0') {
				if (buffer[ptr] == '.' && local) {
					local = 0;
					jrb_insert_str(m->names, strdup(name), JNULL);
					jrb_insert_str(master, strdup(name), new_jval_v((void *) m));
				}
				name[i++] = buffer[ptr++];
			}
			jrb_insert_str(m->names, strdup(name), JNULL);
			jrb_insert_str(master, strdup(name), new_jval_v((void *) m));
			ptr++;
		}
	}

	// On success
	printf("Hosts all read in\n\n");
	if (close(fd) < 0) { perror(filename); exit(2); }
	free(buffer);
	free(c);
}


/* PERFORMS THE LOOKUP */

void findHost(JRB master) {

	// Variables
	int i;
	char *key, *last;
	char ch, input[50];
	JRB node, sort;

	// User input
	printf("Enter host name: ");
	while ((ch = fgetc(stdin)) != EOF) {
		if (ch != '\n') {

			// Read in the name
			input[0] = ch;
			for (i = 1; i < sizeof(input); i++) {
				ch = fgetc(stdin);
				if (ch == '\n' || ch == ' ') { 
					input[i] = '\0';
					break;
				}
				else input[i] = ch;
			}

			// Look for the name
			if ((node = jrb_find_str(master, input)) == NULL && strcmp(input, "") != 0) printf("no key %s\n\n", input);
			else {

				// Once we find one, put it in another tree so we can sort and match the gradescripts
				sort = make_jrb();
				last = NULL;
				jrb_traverse(node, master) {
					if (strcmp(input, node->key.s) == 0) {
						key = (jrb_first(((Machine *) node->val.v)->names)->flink)->key.s;
						if (last != key) jrb_insert_str(sort, strdup(key), new_jval_v(node->val.v));
						last = key;
					}
				}

				// Traverse the sorted tree and print stuff out
				jrb_traverse(node, sort) {
					printNode(node);
					free(node->key.s);
				}
				jrb_free_tree(sort);
			}
			
			// Reset the string
			memset(input, 0, sizeof(input));
			printf("Enter host name: ");
		}
	}
	printf("\n");
}


/* DRIVER CODE */

int main () {
	
	// Variables
	const char *filename = "converted";
	JRB master = make_jrb();
	JRB node1, node2;
	Machine *m;

	// Read in the hosts, then look them up
	readHosts(filename, master);
	findHost(master);

	// Housekeeping
	jrb_traverse(node1, master) {
		if (node1->key.s != NULL) free(node1->key.s);
		node1->key.s = NULL;
		m = (Machine *) node1->val.v;
		if (m != NULL) {
			jrb_traverse(node2, m->names) {
				if (node2->key.s != NULL) free(node2->key.s);
				node2->key.s = NULL;
			}
			//free(m);
			//node1->val.v = NULL;
		}
	}
	jrb_free_tree(master);
	return 0;
}

