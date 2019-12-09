#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define CHILDCOUNT_MAX ( 5 )

int childpids[ CHILDCOUNT_MAX ];
FILE *childfps[ CHILDCOUNT_MAX ][ 2 ];
int childfds[ CHILDCOUNT_MAX ][ 2 ];
int childcount = 0;

int p2open( const char *cmd, FILE *fp[2] );
int p2close( FILE *fp[2] );

int main() {
	FILE *filep[ 2 ];
	if( p2open( "sort", filep ) == -1 ) {
		fprintf( stderr, "p2open() failed\n" );
		exit( EXIT_FAILURE );
	}
	FILE *tosort = filep[ 0 ];
	FILE *fromsort = filep[ 1 ];

	srand( time( NULL ) );
	for( int i = 1; i <= 100; ++i ) {
		int randint = rand() % 100; // 0..99
		fprintf( tosort, "%d\n", randint );
	}

	for( int i = 1; i <= 100; ++i ) {
		int sortedint;
		fscanf( fromsort, "%d\n", &sortedint );
		printf( "%3d", sortedint );
		if( i % 10 == 0 )
			printf( "\n" );
	}

	if( p2close( filep ) == -1 ) {
		fprintf( stderr, "p2close() failed\n" );
		exit( EXIT_FAILURE );
	}
	exit( EXIT_SUCCESS );
}

int p2open( const char *cmd, FILE *fp[2] ) {
	if( childcount == CHILDCOUNT_MAX ) {
		fprintf( stderr, "Too many children\n" );
		return -1;
	}

	int tochildfd[2], fromchildfd[2];
	int pipeerr_to, pipeerr_from;
	pipeerr_to = pipe( tochildfd );
	if( pipeerr_to == -1 ) {
		perror( "pipe() failure" );
		return -1;
	}

	pipeerr_from = pipe( fromchildfd );
	if( pipeerr_from == -1 ) {
		perror( "pipe() failure" );
		close( tochildfd[ 0 ] );
		close( tochildfd[ 1 ] );
		return -1;
	}

	int childpid = fork();
	if( childpid == -1 ) {
		perror( "fork() failure" );
		return -1;
	} else if( childpid ) { // parent
		close( tochildfd[ 0 ] );
		close( fromchildfd[ 1 ] );

		FILE *fp_res[ 2 ];
		fp_res[ 0 ] = fdopen( tochildfd[ 1 ], "w" );
		fp_res[ 1 ] = fdopen( fromchildfd[ 0 ], "r" );
		if( fp_res[ 0 ] == NULL
		    || fp_res[ 1 ] == NULL ) {
			perror( "fdopen() failure" );
			fclose( fp_res[ 0 ] );
			fclose( fp_res[ 1 ] );
			close( tochildfd[ 1 ] );
			close( fromchildfd[ 0 ] );
			return -1;
		}

		close( tochildfd[ 1 ] );
		close( fromchildfd[ 0 ] );

		childpids[ childcount ] = childpid;

		childfps[ childcount ][ 0 ] = fp_res[ 0 ];
		childfps[ childcount ][ 1 ] = fp_res[ 1 ];

		childfds[ childcount ][ 0 ] = tochildfd[ 1 ];
		childfds[ childcount ][ 1 ] = fromchildfd[ 0 ];

		++childcount;

		fp[ 0 ] = fp_res[ 0 ];
		fp[ 1 ] = fp_res[ 1 ];
		return 0;
	} else { // child
		close( tochildfd[ 1 ] );
		close( fromchildfd[ 0 ] );

		close( STDIN_FILENO );
		dup2( tochildfd[ 0 ], STDIN_FILENO );

		close( STDOUT_FILENO );
		dup2( fromchildfd[ 1 ], STDOUT_FILENO );

		execlp( cmd, cmd, ( char * )NULL );

		perror( "execlp() failure" );
		return -1;
	}
}

int p2close( FILE *fp[2] ) {
	int index = -1;
	for( int i = 0; i < childcount; ++i ) {
		if( childfps[ i ][ 0 ] == fp[ 0 ] ) {
			index = i;
			break;
		}
	}
	if( index == -1 ) {
		fprintf( stderr, "FILE *s not found\n" );
		return -1;
	}

	if( childfps[ index ][ 1 ] != fp[ 1 ] ) {
		fprintf( stderr, "Broken FILE * pair\n" );
		return -1;
	}

	fclose( childfps[ index ][ 0 ] );
	fclose( childfps[ index ][ 1 ] );

	//close( childfds[ index ][ 0 ] );
	//close( childfds[ index ][ 1 ] );

	waitpid( childpids[ index ], NULL, 0 );
	childpids[ index ] = childpids[ childcount - 1 ];

	--childcount;
	return 0;
}
