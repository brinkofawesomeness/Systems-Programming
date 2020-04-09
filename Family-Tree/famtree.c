/* 
 * Conner Brinkley
 * CS360 -- Lab 1
 * 09.08.2019
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fields.h"
#include "jval.h"
#include "dllist.h"
#include "jrb.h"


/* Struct to hold data for each person */

typedef struct {
	char *name;
	char *sex;
	char *father;
	char *mother;
	Dllist children;
	int visited;
	int printed;
} Person;


/* Reads the name into a char* from the inputstruct.
 * Returns the name. */

char* readName(IS input) {

	/* Variables */
	char *name;
	int name_size, i;

	/* Calculate the size of their name */
	name_size = strlen(input->fields[1]);
	for (i = 2; i < input->NF; i++) {
		name_size += (strlen(input->fields[i]) + 1);
	}

	/* Allocate memory for the name */
	name = (char *) malloc(sizeof(char) * (name_size + 1));

	/* Create the string name */
	strcpy(name, input->fields[1]);
	name_size = strlen(input->fields[1]);
	for (i = 2; i < input->NF; i++) {
		name[name_size] = ' ';
		strcpy(name + name_size + 1, input->fields[i]);
		name_size += strlen(name + name_size);
	}

	return name;
}


/* Function to create a new person struct and add them to the Red Black Tree.
 * Checks to see if the person is already in the tree before creating a new person.
 * Returns a pointer to the person struct. */

Person* addPerson(char *name, JRB tree) {

	/* Variables */
	Person *p;
	int str_size;

	/* Search JRB to see if name is already there (if it is, just return it) */
	if (jrb_find_str(tree, name) != NULL) {
		p = (Person *) jrb_find_str(tree, name)->val.v;
		return p;
	}

	/* Allocate space for the new person */
	p = malloc(sizeof(Person));
	p->name = malloc(strlen(name) + 1);	
	str_size = (strlen("Unknown") + 1);
	p->sex = malloc(str_size);
	p->father = malloc(str_size);
	p->mother = malloc(str_size);

	/* Init values */
	strcpy(p->name, name);
	strcpy(p->sex, "Unknown");
	strcpy(p->father, "Unknown");
	strcpy(p->mother, "Unknown");
	p->children = new_dllist();
	p->visited = 0;
	p->printed = 0;

	/* Add the person to the tree */
	jrb_insert_str(tree, name, new_jval_v((void *) p));

	return p;
}


/* Print the tree. */

void printTree(JRB tmp, JRB tree) {

	/* Variables */
	Person *p;
	Dllist to_print, list_tmp;
	to_print = new_dllist();

	/* Add all the people to the queue */
	jrb_traverse(tmp, tree) {
		p = (Person *) tmp->val.v;
		dll_append(to_print, new_jval_v((void *) p));
	}

	/* BFS print */
	while (!dll_empty(to_print)) {

		/* Take p off the head of to_print */
		p = dll_first(to_print)->val.v;
		dll_delete_node(dll_first(to_print));
		if (!p->printed) {

			/* If p doesn't have parents, or if p's parents have been printed */
			if (((strcmp(p->father, "Unknown") == 0) || (((Person *) jrb_find_str(tree, p->father)->val.v)->printed)) && 
				((strcmp(p->mother, "Unknown") == 0) || (((Person *) jrb_find_str(tree, p->mother)->val.v)->printed))) {

				/* Print */
				printf("%s\n", p->name);
				printf("  Sex: %s\n", p->sex);
				printf("  Father: %s\n", p->father);
				printf("  Mother: %s\n", p->mother);
				printf("  Children: ");
				if (dll_empty(p->children)) printf("None\n");
				else {
					printf("\n");

					/* Print and append each child to to_print */
					dll_traverse(list_tmp, p->children) {
						printf("    %s\n", jval_s(list_tmp->val));
						dll_append(to_print, jrb_find_str(tree, jval_s(list_tmp->val))->val);
					}
				}
				printf("\n");
				p->printed = 1;
			}
		}
	}
}


/* Checks the sex of a person for accuracy. Does nothing if it's fine.
 * Else it prints an error and exits. */

void checkSex(Person *p, char *sex, int line_number) {	

	/* Update the sex if it's not already defined */
	if (strcmp(p->sex, "Unknown") == 0) {
		strcpy(p->sex, sex);
	} else {
		/* Check if the sex is a mismatch */
		if (strcmp(p->sex, sex) != 0) {
			fprintf(stderr, "Bad input - sex mismatch on line %d\n", line_number);
			exit(1);
		}
	}
}


/* A simple depth first search that checks for cycles. Taken from Dr. Plank's 
 * pseudocode. */

int checkCycles(Person *p, JRB tree) {

	/* Variables */
	Person *tmp_person;
	Dllist tmp;

	/* Base cases */
	if (p->visited == 1) return 0;
	if (p->visited == 2) return 1;
	p->visited = 2;

	/* Recurse */
	dll_traverse(tmp, p->children) {
		tmp_person = (Person *) jrb_find_str(tree, jval_s(tmp->val))->val.v;
		if (checkCycles(tmp_person, tree)) return 1;
	}

	p->visited = 1;
	return 0;
}


/* Driver code. */

int main() {

	/* Variables */
	JRB tree, tmp;
	Person *p, *curr;
	IS input;
	Dllist tmp_list;
	int flag;

	/* Init the inputstruct and red black tree */
	input = new_inputstruct(NULL);
	tree = make_jrb();

	/* Read in the data */
	while (get_line(input) >= 0) {

		/* Check for a person */
		if (strcmp(input->fields[0], "PERSON") == 0) {

			/* Create person struct */
			curr = addPerson(readName(input), tree);

			/* Keep reading in data until we reach a blank line */
			while(get_line(input) > 0) {

				/* Check if line is SEX and update */
				if (strcmp(input->fields[0], "SEX") == 0) {
					if (strcmp(input->fields[1], "M") == 0) {
						checkSex(curr, "Male", input->line);
					} else {
						checkSex(curr, "Female", input->line);
					}
				}

				/* Else, we know it's going to define a person */
				else {

					p = addPerson(readName(input), tree);

					/* Check if line is FATHER and update */
					if (strcmp(input->fields[0], "FATHER") == 0) {

						/* Create, check that father field is empty, and update */
						if (strcmp(curr->father, "Unknown") != 0 &&
							strcmp(curr->father, readName(input)) != 0) {
							fprintf(stderr, "Bad input -- child with two fathers on line %d\n", input->line);
							exit(1);
						}
						strcpy(curr->father, readName(input));

						/* Error handing for sex */
						checkSex(p, "Male", input->line);

						/* Add curr person to p's children list */
						flag = 0;
						dll_traverse(tmp_list, p->children) {
							if (strcmp(curr->name, jval_s(tmp_list->val)) == 0) flag = 1; 
						}
						if (!flag) dll_append(p->children, new_jval_s(strdup(curr->name)));
					}

					/* Check if line is MOTHER and update */
					if (strcmp(input->fields[0], "MOTHER") == 0) {

						/* Create, check that mother field is empty, and update */
						if (strcmp(curr->mother, "Unknown") != 0 &&
							strcmp(curr->mother, readName(input)) != 0) {
							fprintf(stderr, "Bad input -- child with two mothers on line %d\n", input->line);
							exit(1);
						}
						strcpy(curr->mother, readName(input));

						/* Error handling for sex */
						checkSex(p, "Female", input->line);

						/* Add curr person to p's children list */
						flag = 0;
						dll_traverse(tmp_list, p->children) {
							if (strcmp(curr->name, jval_s(tmp_list->val)) == 0) flag = 1;
						}
						if (!flag) dll_append(p->children, new_jval_s(strdup(curr->name)));
					}

					/* Check if line is FATHER_OF and update */
					if (strcmp(input->fields[0], "FATHER_OF") == 0) {

						/* Create, check that father field is empty, and update */
						if (strcmp(p->father, "Unknown") != 0 &&
							strcmp(p->father, curr->name) != 0) {
							fprintf(stderr, "Bad input -- child with two fathers on line %d\n", input->line);
							exit(1);
						}
						strcpy(p->father, curr->name);	

						/* Error handling for sex */
						checkSex(curr, "Male", input->line);

						/* Add p to curr person's children list */
						flag = 0;
						dll_traverse(tmp_list, p->children) {
							if (strcmp(p->name, jval_s(tmp_list->val)) == 0) flag = 1;
						}
						if (!flag) dll_append(curr->children, new_jval_s(strdup(p->name)));
					}

					/* Check if line is MOTHER_OF and update */
					if (strcmp(input->fields[0], "MOTHER_OF") == 0) {

						/* Create, check that mother field is empty, and update */
						if (strcmp(p->mother, "Unknown") != 0 &&
							strcmp(p->mother, readName(input)) != 0) {
							fprintf(stderr, "Bad input -- child with two mothers on line %d\n", input->line);
							exit(1);
						}
						strcpy(p->mother, curr->name);

						/* Error handling for sex */
						checkSex(curr, "Female", input->line);

						/* Add p to curr person's children list */
						flag = 0;
						dll_traverse(tmp_list, p->children) {
							if (strcmp(p->name, jval_s(tmp_list->val)) == 0) flag = 1;
						}
						if (!flag) dll_append(curr->children, new_jval_s(strdup(p->name)));
					}
				}
			}
		}
	}

	/* Check the graph for cycles with a simple DFS */
	jrb_traverse(tmp, tree) {
		p = (Person *) tmp->val.v;
		if (checkCycles(p, tree)) {
			fprintf(stderr, "Bad input -- cycle in specification\n");
			exit(1);
		}
	}

	/* Print out the tree */
	printTree(tmp, tree);

	/* Do it for the mems (free the memory) */
	jettison_inputstruct(input);
	jrb_free_tree(tree);
	exit(0);
}
