#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void user_failure() {
	printf("Real user ID: %d\n", getuid());
	printf("Effective user ID: %d\n", geteuid());

	char name[] = "123.abc";
	FILE *file = fopen(name, "r");
	if(file)
		fclose(file);
	else
		perror(name);
}

int main() {
	printf("*** 03. User identificators ***\n");
	
	printf("\nBefore setuid():\n");
	user_failure();

	printf("\nAfter setuid():\n");
	setuid(geteuid());
	user_failure();

	return 0;
}
