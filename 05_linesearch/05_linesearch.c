#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLEN ( 1024 )

struct globalstruct_type {
	char *filename;
	int fd;
	char *data;
	int data_length;
	int *newlines_lastchar_off; // the last is last char offset
} globalstruct = { NULL, -1, NULL, 0, NULL };

void shutdown( int exitcode );
char *ask_filename( int max_length );
void process_lines( char *data, int data_size, int **newlines_lastchar_off, int *newlines_lastchar_off_size );
int ask_linenumber( int max_linenumber, int *why_returned );
void print_all( char *data, int size );
void select_line( int number, char *data, int *offsets, int offsets_size, char **ptr_position, int *length );
int append_elem( int elem, int **array, int *size, int *buffer_size, int add_size );
int ensure_memory( int **array, int *size, int *buffer_size, int add_size );
char *store_file_by_read( int fd, int length );

int main() {
	printf("*** 05. Line search + file read ***\n");

	char *filename = globalstruct.filename = ask_filename( MAXLEN );
	if( filename == NULL ) {
		perror( "ask_filename() failed memory allocation" );
		shutdown( EXIT_FAILURE );
	}

	int fd = globalstruct.fd = open( filename, O_RDONLY );
	if( fd == -1 ) {
		perror( "File open error" );
		shutdown( EXIT_FAILURE );
	}
	int file_length = globalstruct.data_length = lseek( fd, 0, SEEK_END ) + 1;

	// Tasks 05, 06 peculiarity
	char *data = globalstruct.data = store_file_by_read( fd, file_length );
	if( data == NULL ) {
		perror( "read_file() failed" );
		shutdown( EXIT_FAILURE );
	}

	int *newlines_lastchar_off;
	int newlines_lastchar_off_size;
	process_lines( data, file_length, &newlines_lastchar_off, &newlines_lastchar_off_size );
	globalstruct.newlines_lastchar_off = newlines_lastchar_off;
	if( newlines_lastchar_off == NULL ) {
		perror( "process_lines() failed, most likely because of malloc()" );
		shutdown( EXIT_FAILURE );
	}

	while( 1 ) {
		int why; // 0 - timeout, -1 - error, 1 - input
		int num = ask_linenumber( newlines_lastchar_off_size, &why );

		if( why == 0 ) { // will occur only in 06, 07
			printf( "Timeout exceeded\n\n" );
			print_all( data, file_length );
			break;
		} else if( why == -1 ) {
			perror( "ask_linenumber() failed" );
			shutdown( EXIT_FAILURE );
		} else {
			if( num < 0 || num > newlines_lastchar_off_size )
				printf( "Incorrect line number. Try again\n" );
			else if( num == 0 )
				break;
			else { // a valid line number, never timed-out
				char *ptr_line;
				int line_length;
				select_line( num, data, newlines_lastchar_off,
					       	newlines_lastchar_off_size, &ptr_line, &line_length );
				write( /*stdout*/ 1, ptr_line, line_length );
				write( 1, "\n", 1 );
			}
		}
	}

	shutdown( EXIT_SUCCESS );
}

void shutdown( int exitcode ) {
	if( globalstruct.filename != NULL )
		free( globalstruct.filename );
	if( globalstruct.fd != -1 ) {
		if( close( globalstruct.fd ) == -1 )
			perror( "close() failed" );
	}
	if( globalstruct.data != NULL )
		free( globalstruct.data );
	if( globalstruct.newlines_lastchar_off != NULL )
		free( globalstruct.newlines_lastchar_off );
	exit( exitcode );
}

char *ask_filename( int max_length ) {
	char *filename = ( char * )malloc( sizeof( char ) * max_length + 2 );
	if( filename == NULL )
		return NULL;

	printf( "Type a file name of <=%d characters\n> ", max_length );
	fgets( filename, max_length + 1, stdin );
	if( filename[ strlen( filename ) - 1 ] == '\n' )
		filename[ strlen( filename ) - 1 ] = '\0';
	
	return filename;
}

void process_lines( char *data, int data_size, int **newlines_lastchar_off, int *newlines_lastchar_off_size ) {
	*newlines_lastchar_off = NULL;
	*newlines_lastchar_off_size = 0;
	int buffer_size = 0, add_size = 50;

	for( int offset = 0; offset < data_size; ++offset )
		if( data[ offset ] == '\n' )
			if( append_elem( offset, newlines_lastchar_off, newlines_lastchar_off_size,
						&buffer_size, add_size ) == -1 ) {
				perror("append_elem() failed");
				return;
			}
	if( append_elem( data_size, newlines_lastchar_off, newlines_lastchar_off_size, &buffer_size, add_size ) == -1 )
		perror("append_elem() failed");
}

// implementation specific to 05
int ask_linenumber( int max_linenumber, int *why_returned ) {
	int num;

	printf( "Type a line number 1-%d (or 0 to quit)\n> ", max_linenumber );
	if( scanf( "%d", &num ) == 1 ) {
		*why_returned = 1;
		return num;
	} else {
		*why_returned = -1;
		return 0;
	}
}

void print_all( char *data, int size ) {
	write( /*stdout*/ 1, data, size );
}

void select_line( int number, char *data, int *offsets, int offsets_size, char **ptr_position, int *length ) {
	if( ptr_position == NULL || length == NULL )
		return;
	if( data == NULL || offsets == NULL || offsets_size < 1 ) {
		*ptr_position = 0;
		*length = 0;
	}

	if( number == 1 )
		*ptr_position = data;
	else if( number >= 2 && number <= offsets_size - 1 )
		*ptr_position = data + offsets[ number - 2 ] + 1;
	else {
		*ptr_position = NULL;
		*length = 0;
		return;
	}

	if( number == 1 )
		*length = offsets[ 0 ];
	else // number >= 2
		*length = offsets[ number - 1 ] - offsets[ number - 2 ] - 1;
}

int append_elem( int elem, int **array, int *size, int *buffer_size, int add_size ) {
	if( ensure_memory( array, size, buffer_size, add_size ) == -1 ) {
		perror("ensure_memory() failed");
		return -1;
	}

	++( *size );
	( *array )[ *size - 1 ] = elem;
	return 0;
}

int ensure_memory( int **array, int *size, int *buffer_size, int add_size ) {
	if( *buffer_size == 0 ) { // initial memory allocation
		int *temp;
		temp = (int *)malloc( sizeof(int) * add_size );
		if( temp == NULL )
			return -1;
		*array = temp;
		*buffer_size = add_size;
		return 0;
	}

	if( *size == *buffer_size ) { // we've reached the end of the buffer
		int *expanded = (int *)malloc( sizeof(int) * ( *buffer_size + add_size ) );
		if( expanded == NULL )
			return -1;
		memcpy( expanded, *array, sizeof(int) * *size );
		free( *array );
		*array = expanded;
		*buffer_size += add_size;
	}
	return 0;
}

char *store_file_by_read( int fd, int length ) {
	char *buffer = ( char * )malloc( sizeof( char ) * length );
	if( buffer == NULL ) {
		perror( "malloc() failed" );
		return NULL;
	}

	lseek( fd, 0, SEEK_SET );
	int successfully_read = read( fd, buffer, sizeof( char ) * length );
	if( successfully_read + 1 != sizeof( char ) * length ) {
		perror( "read() failed" );
		free( buffer );
		return NULL;
	}

	return buffer;
}
