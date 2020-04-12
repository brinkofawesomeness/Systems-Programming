/*
 * Conner Brinkley
 * November 8, 2019
 * Systems Programming
 *
 * Malloc Lab
 * jmalloc1 -- Part I
 *
 * */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>

/* GLOBALS */

char *head = NULL;


/* ALLOCATE MEMORY */

void *jmalloc(unsigned int size) {
	
	// Variables
	void *allocated;
	int multiple;

	// Check the size requested
	if (size > 8192) {
		size += 8;
		allocated = sbrk(size);
		if ((int) allocated == -1) return NULL;
		memcpy(allocated, &size, sizeof(unsigned int));
		allocated += 8;
		return allocated;
	}

	// Check if we already have a buffer
	if (head == NULL) {
		head = sbrk(8192);
		if ((int) head == -1) return NULL;
	}

	// Make sure to allocate a multiple of 8
	if (size % 8 != 0) {
		multiple = size / 8;
		multiple++;
		size = multiple * 8;
	}

	// Make sure there's enough space in the buffer
	size += 8;
	if (head + size >= sbrk(0)) {
		allocated = sbrk(8192);
		if ((int) allocated == -1) return NULL;
	}
	
	// Allocate a chunk of size + 8 bytes
	allocated = head;
	memcpy(allocated, &size, sizeof(unsigned int));
	allocated += 8;
	head += size;

	// Give the people what they want
	return allocated;
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
	allocated = malloc(totalSize);
	
	// Init the memory to 0
	bzero(allocated, totalSize);
	return allocated;
}


/* RESIZE MEMORY LOCATION */

void *jrealloc(void *ptr, size_t size) {
	
	// Variables
	void *allocated;
	return allocated;
}


/* FREE MEMORY */

void jfree(void *memory) {
	// Do nothing!
	// We're using ALL the memory
}

