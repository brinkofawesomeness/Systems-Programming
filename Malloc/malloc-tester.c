#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include "malloc.h"

#define SIZE 100

long long mallocs = 0;
long long frees = 0;

long long total_allocated = 0;
long long total_freed = 0;

int seed;

void seg_fault_handler( int dummy );

int main( int argc, char *argv[] )
{
	if ( ( argc != 2 ) && ( argc != 3 ) )
	{
		printf( "usage: %s malloc-calls [seed]\n", argv[0] );
		return -1;
	}

	int malloc_calls = atoi( argv[1] );

	if ( malloc_calls < 0 )
	{
		printf( "malloc-calls cannot be negative\n" );
		return -1;
	}

	seed = time( NULL );

	if ( argc == 3 )
	{
		seed = atoi( argv[2] );

		if ( seed < 0 )
		{
			printf( "seed cannot be negative\n" );
			return -1;
		}
	}

	signal( SIGSEGV, seg_fault_handler );

	srand( seed );

	printf( "Running until jmalloc, jcalloc, or jrealloc returns NULL\n\n" );

	void *p;

	printf( "trying to malloc %d bytes\n", 8184 );

	p = jmalloc( 8184 );
			
	printf( "bookkeeping bytes: %d\n", *( ( int * ) ( p - 8 ) ) );

	mallocs++;

	total_allocated += 8184;
				
	printf( "trying to free %d bytes\n", 8184 );

	jfree( p );

	frees++;

	total_freed += 8184;

	printf( "\n" );

	printf( "trying to malloc %d bytes\n", ( 8192 * 2 ) + 1 );

	p = jmalloc( ( 8192 * 2 ) + 1 );

	printf( "bookkeeping bytes: %d\n", *( ( int * ) ( p - 8 ) ) );

	mallocs++;

	total_allocated += ( ( 8192 * 2 ) + 1 );

	printf( "trying to free %d bytes\n", ( 8192 * 2 ) + 1 );

	jfree( p );

	frees++;

	total_freed += ( ( 8192 * 2 ) + 1 );

	printf( "\n" );

	printf( "trying to malloc %d bytes\n", ( 8192 * 3 ) - 1 );

	p = jmalloc( ( 8192 * 3 ) - 1 );

	printf( "bookkeeping bytes: %d\n", *( ( int * ) ( p - 8 ) ) );

	mallocs++;

	total_allocated += ( ( 8192 * 3 ) - 1 );

	printf( "trying to free %d bytes\n", ( 8192 * 3 ) - 1 );

	jfree( p );

	frees++;

	total_freed += ( ( 8192 * 3 ) - 1 );

	printf( "\n" );

	while ( p )
	{
		if ( mallocs >= malloc_calls )
		{
			break;
		}

		int size = 1 + rand() % SIZE;

		printf( "trying to malloc %d bytes\n", size );

		p = jmalloc( size );

		printf( "bookkeeping bytes: %d\n", *( ( int * ) ( p - 8 ) ) );

		if ( p )
		{
			mallocs++;
			total_allocated += size;
		}

		int Free = rand() % 2;

		if ( Free && p )
		{
			printf( "trying to free %d bytes\n", size );
			jfree( p );
			frees++;
			total_freed += size;
		}

		printf( "\n" );

		fflush( stdout );
	}

	printf( "\033[1;31m" );
	printf( "Summary:\n" );
	printf( "\033[0;32m" );
	printf( "calls to jmalloc: %lld\n", mallocs );
	printf( "calls to jfree: %lld\n", frees );
	printf( "\n" );
	printf( "\033[0;34m" );
	printf( "total bytes allocated: %lld\n", total_allocated );
	printf( "total bytes freed: %lld\n", total_freed );
	printf( "\n" );
	printf( "\033[0;35m" );
	printf( "seed used: %d\n", seed );
	printf( "\033[0;32m" );
	printf( "\nPassed %d malloc call test\n", malloc_calls );

	return 0;
}

void seg_fault_handler( int dummy )
{
	printf( "\n" );
	printf( "\033[1;31m" );
	printf( "Summary:\n" );
	printf( "\033[0;32m" );
	printf( "calls to jmalloc: %lld\n", mallocs );
	printf( "calls to jfree: %lld\n", frees );
	printf( "\n" );
	printf( "\033[0;34m" );
	printf( "total bytes allocated: %lld\n", total_allocated );
	printf( "total bytes freed: %lld\n", total_freed );
	printf( "\n" );
	printf( "\033[0;35m" );
	printf( "seed used: %d\n", seed );

	printf( "\033[1;31m" );
	printf( "\nFailed malloc call test\n\n" );

	exit( -1 );
}
