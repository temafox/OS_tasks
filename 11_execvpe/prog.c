#include <stdio.h>
#include <stdlib.h>

int main() {
	printf( "*** prog output start ***\n$V=%s\n***  prog output end  ***\n", getenv( "V" ) );
	exit( EXIT_SUCCESS );
}
