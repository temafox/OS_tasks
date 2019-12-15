#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define CHILDCOUNT_MAX ( 5 )

struct child_p2open_record {
	int pid;
	FILE *fp[ 2 ];
};

struct child_p2open_record children_p2opened[ CHILDCOUNT_MAX ];
int childcount = 0;

int close_pair( int fd[ 2 ] );

int add_child_to_array();
int remove_child_from_array( int index );
int locate_child( FILE *fp[ 2 ] );

int p2open( const char *cmd, FILE *fp[ 2 ] );
int p2close( FILE *fp[2] );

int main() {
	FILE *fp[ 2 ];
	if( p2open( "sort -n", fp ) == -1 ) {
		fprintf( stderr, "p2open() failed\n" );
		exit( EXIT_FAILURE );
	}
	FILE *tosort = fp[ 0 ];
	FILE *fromsort = fp[ 1 ];

	srand( time( NULL ) );
	for( int i = 0; i < 100; ++i ) {
		int randint = rand() % 100; // 0..99
		fprintf( tosort, "%d\n", randint );
		fflush( tosort );
	}

	int sortedint[ 100 ];
	for( int i = 0; i < 100; ++i )
		fscanf( fromsort, "%d", sortedint + i );

	if( p2close( fp ) == -1 ) {
		fprintf( stderr, "p2close() failed\n" );
		exit( EXIT_FAILURE );
	}

	for( int i = 0; i < 100; ++i ) {
		printf( "%2d ", sortedint[ i ] );
		printf( (i+1) % 10 == 0 ? "\n" : " " );
	}
	exit( EXIT_SUCCESS );
}

int p2open( const char *cmd, FILE *fp[2] ) {
	if( childcount == CHILDCOUNT_MAX ) {
		fprintf( stderr, "Too many children\n" );
		return -1;
	}

	int parent_child_fd[ 2 ], child_parent_fd[ 2 ];
	if( pipe( parent_child_fd ) == -1 ) {
		perror( "pipe() failed for parent-to-child IPC" );
		return -1;
	}
	if( pipe( child_parent_fd ) == -1 ) {
		perror( "pipe() failed for child-to-parent IPC" );
		close_pair( parent_child_fd );
		return -1;
	}

	int childpid = fork();
	if( childpid == -1 ) {
		perror( "fork() failure" );
		close_pair( parent_child_fd );
		close_pair( child_parent_fd );
		return -1;
	} else if( childpid == 0 ) { // child
		close( STDIN_FILENO );
		dup2( parent_child_fd[ 0 ], STDIN_FILENO );

		close( STDOUT_FILENO );
		dup2( child_parent_fd[ 1 ], STDOUT_FILENO );

		close_pair( parent_child_fd );
		close_pair( child_parent_fd );

		execl( "/bin/sh", "sh", "-c", cmd, ( char * )0 );
		perror( "execl() failure" );
		return -1;
	} else { // parent
		FILE *result_fp[ 2 ];
		result_fp[ 0 ] = fdopen( parent_child_fd[ 1 ], "w" );
		if( result_fp[ 0 ] == NULL ) {
			perror( "fdopen() failure for parent-to-child IPC" );
			close_pair( parent_child_fd );
			close_pair( child_parent_fd );
			return -1;
		}

		result_fp[ 1 ] = fdopen( child_parent_fd[ 0 ], "r" );
		if( result_fp[ 1 ] == NULL ) {
			perror( "fdopen() failure for parent-to-child IPC" );
			fclose( result_fp[ 0 ] );
			close_pair( parent_child_fd );
			close_pair( child_parent_fd );
			return -1;
		}

		close_pair( parent_child_fd );
		close_pair( child_parent_fd );

		int childindex = add_child_to_array();
		children_p2opened[ childindex ].pid = childpid;
		children_p2opened[ childindex ].fp[ 0 ] = result_fp[ 0 ];
		children_p2opened[ childindex ].fp[ 1 ] = result_fp[ 1 ];

		fp[ 0 ] = result_fp[ 0 ];
		fp[ 1 ] = result_fp[ 1 ];
		return 0;
	}
}

int p2close( FILE *fp[ 2 ] ) {
	int childindex = locate_child( fp );
	if( childindex == -1 ) {
		fprintf( stderr, "No child works with these streams\n" );
		return -1;
	}

	fclose( children_p2opened[ childindex ].fp[ 0 ] );
	fclose( children_p2opened[ childindex ].fp[ 1 ] );

	int childstatus;
	waitpid( children_p2opened[ childindex ].pid, &childstatus, 0 );

	remove_child_from_array( childindex );

	return childstatus;
}

int add_child_to_array() {
	if( childcount == CHILDCOUNT_MAX )
		return -1;

	++childcount;
	return childcount - 1; // index of the new child
}

int remove_child_from_array( int index ) {
	if( childcount == 0 )
		return -1;
	if( index < 0 || index > childcount - 1 )
		return -1;

	if( index < childcount - 1 ) {
		children_p2opened[ index ].pid = children_p2opened[ childcount - 1 ].pid;
		children_p2opened[ index ].fp[ 0 ] = children_p2opened[ childcount - 1 ].fp[ 0 ];
		children_p2opened[ index ].fp[ 1 ] = children_p2opened[ childcount - 1 ].fp[ 1 ];
	}

	--childcount;
	return childcount;
}

int locate_child( FILE *fp[ 2 ] ) {
	for( int i = 0; i < childcount; ++i )
		if( children_p2opened[ i ].fp[ 0 ] == fp[ 0 ]
		    && children_p2opened[ i ].fp[ 1 ] == fp[ 1 ] )
				return i;
	
	return -1;
}
 int close_pair( int fd[ 2 ] ) {
	int zeroth_status = close( fd[ 0 ] );
	int first_status  = close( fd[ 1 ] );

	if( zeroth_status == -1 || first_status == -1 )
		return -1;
	else
		return 0;
 }
