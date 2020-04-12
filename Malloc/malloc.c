/*
 * Conner Brinkley
 * November 10, 2019
 * Systems Programming
 *
 * Malloc Lab
 * Part II -- Implementing the free list
 *
 * */

#include <string.h>
#include <unistd.h>
#include <strings.h>


/* FREE LIST OF MEMORY */

typedef struct Freelist {
	int size;
	struct Freelist *next;
} Freelist;


/* GLOBAL */

Freelist *head = NULL;


/* ALLOCATE MEMORY */

void *jmalloc(unsigned int size) {
	
	// Variables
	int multiple, remaining;
	void *allocated, *remainder;
	Freelist *block, *it, *tmp;

	// Calculate the actual size needed
	if (size == 0) return NULL;
	size += 8;
	if (size % 8 != 0) {
		multiple = size / 8;
		multiple++;
		size = multiple * 8;
	}

	// If the size is greater than 8192, handle this seperately
	if (size > 8192) {
		allocated = sbrk(size);
		if (*((int *) allocated) == -1) return NULL;
		memcpy(allocated, &size, sizeof(int));
		allocated = (char *) allocated + 8;
		return allocated;
	}

	// Traverse through the free list to find a block big enough
	block = head;
	while (block != NULL) {
		if (block->size >= size) break;
		else block = block->next;
	}

	// Check if we've found a block 
	if (block == NULL) {
		allocated = sbrk(8192 + 8);
		if (*((int *) allocated) == -1) return NULL;
		block = (Freelist *) allocated;
		block->size = 8192 + 8;
		block->next = NULL;
	}

	// Carve out the memory requested
	allocated = (void *) block;
	memcpy(allocated, &size, sizeof(int));
	remaining = block->size - size;
	if (remaining >= 16) {
		remainder = allocated + size;
	}
	else remainder = NULL;
	allocated = (char *) allocated + 8;

	// Return the remainder to the free list
	if (remainder != NULL) {
		block = (Freelist *) remainder;
		block->size = remaining;
		it = head;
		while (it->size <= block->size && it->next != NULL) {
			it = it->next;
		}
		tmp = it->next;
		it->next = block;
		block->next = tmp;
	}

	// Give the people what they want
	return allocated;
}


/* FREE MEMORY */

void jfree(void *memory) {

	// Variables
	int size;
	Freelist *block, *it, *tmp;

	// Check to make sure it's a valid memory address
	memory = (void *) ((char *) memory - 8);
	size = *((int *) memory);
	if (size < 0 || size > 999999999) return;

	// Create a new free memory block
	block = (Freelist *) memory;
	block->size = size;
	
	// Edge case
	if (head == NULL) {
		head = block;
		head->next = NULL;
		return;
	}

	// Return it to the free list, ordered by size
	it = head;
	while (it->size <= block->size && it->next != NULL) {
		it = it->next;
	}
	tmp = it->next;
	it->next = block;
	block->next = tmp;
}


/* INITIALIZED ALLOCATE */

void *jcalloc(size_t nmemb, size_t size) {

	// Variables
	void *allocated;
	int totalSize;

	// Check to make sure it's valid
	if (nmemb == 0 || size == 0) return NULL;

	// Allocate enough memory
	totalSize = nmemb * size;
	allocated = jmalloc(totalSize);
	
	// Init the memory to 0
	bzero(allocated, totalSize);
	return allocated;
}


/* RESIZE MEMORY LOCATION */

void *jrealloc(void *ptr, size_t size) {
	
	// Variables
	void *allocated;
	
	// Edge case
	if (ptr == NULL) {
		allocated = jmalloc(size);
		return allocated;
	}

	// Allocate new memory, and copy the old memory over
	allocated = jmalloc(size);
	bcopy(ptr, allocated, size);
	jfree(ptr);
	return allocated;
}

