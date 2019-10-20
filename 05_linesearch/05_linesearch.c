#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAXLEN (1024)
#define ever (;;)

int *expand(int *array, int *size);

int main() {
	// Ask for filename
	char filename[MAXLEN];
	printf("*** 05. Line search ***\n\nPlease, type a file name (fewer than %d characters)\n> ", MAXLEN - 1);
	fgets(filename, MAXLEN - 1, stdin);
	filename[strlen(filename) - 1] = '\0'; /* get rid of the trailing \n */

	// Open the file
	int file = open(filename, O_RDONLY);
	if( file == -1 ) {
		fprintf(stderr, "File error\n");
		exit(EXIT_FAILURE);
	}

	// List the lines in the file
	int size = 20;
	int number = 0;
	int *newline_offsets = (int *)malloc( sizeof(int) * size );
	if( !newline_offsets ) {
		fprintf(stderr, "Memory failure\n");
		close(file);
		exit(EXIT_FAILURE);
	}
	
	int offset;
	char symbol;
	while( read(file, &symbol, sizeof(char) != 0 ) ) { /* while not EOF */
		if( symbol == '\n' ) {
			offset = lseek(file, 0, SEEK_CUR);
			if( number == size ) {
				int *temp = expand(newline_offsets, &size);
				if( !temp ) {
					free(newline_offsets);
					close(file);
					exit(EXIT_FAILURE);
				}

				free(newline_offsets);
				newline_offsets = temp;
			}
			newline_offsets[ number++ ] = offset - 1;
		}
	}
	
	// List the line lengths
	// Now offset is at the end of file
	int *line_lengths = (int *)malloc( sizeof(int) * number );
	if( !line_lengths ) {
		fprintf(stderr, "Memory failure\n");
		free(newline_offsets);
		close(file);
		exit(EXIT_FAILURE);
	}
	
	line_lengths[0] = newline_offsets[0];
	for(int i = 1; i < number; ++i)
		line_lengths[i] = newline_offsets[i] - newline_offsets[i-1] - 1;

	// Actual line search
	int num;
	for ever {
		printf("Type a line number 1-%d (or 0 to quit)\n> ", number);
		scanf("%d", &num);
		if(num == 0)
			break;
		if(num < 0 || num > number) {
			fprintf(stderr, "Incorrect line number. Try again\n");
			continue;
		}
		
		// Output the result
		int len = line_lengths[ num - 1 ];
		char *line = (char *)malloc( sizeof(char) * (len+1) );
		if( !line ) {
			fprintf(stderr, "Memory failure\n");
			free(newline_offsets);
			free(line_lengths);
			close(file);
			exit(EXIT_FAILURE);
		}

		lseek(file, ( num == 1 ? 0 : newline_offsets[ num - 2 ] + 1 ), SEEK_SET);
		read(file, line, len);
		line[len] = '\0';
		printf("%s\n", line);

		free(line);
	}

	free(newline_offsets);
	free(line_lengths);
	close(file);
	exit(EXIT_SUCCESS);
}

int *expand(int *array, int *size) {
	int new_size = *size + 10;
	int *res = (int *)malloc( sizeof(int) * new_size );
	if( !res )
		return NULL;

	memcpy(res, array, sizeof(int) * *size);
// TODO: double free()
	free(array);
	*size = new_size;

	return res;
}
