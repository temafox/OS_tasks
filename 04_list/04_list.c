#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN (1024)

struct Node {
	char *str;
	struct Node *next;
};

struct Node *init_node(char *str) {
	struct Node *res = (struct Node *)malloc( sizeof(struct Node) );
	if(!res)
		return NULL;

	res->str = (char *)malloc( sizeof(char) * strlen(str) );
	if(!res->str) {
		free(res);
		return NULL;
	}
	strncpy(res->str, str, strlen(str) - 1); /* -1 to dispose of the trailing \n */

	res->next = NULL;	
	return res;
}

void delete_list(struct Node **list) {
	if( !*list )
		return;

	delete_list( &(*list)->next );
	free( (*list)->str );
	free( *list );
}

void print_list(struct Node *list) {
	if( !list ) {
		printf("*** END OF LIST ***\n");
		return;
	}
	
	printf("%s\n", list->str);
	print_list(list->next);
}

/********************************************/

int main() {
	struct Node *list = NULL, **current_node = &list;
	char buffer[MAXLEN];

	do {
		if( !fgets(buffer, MAXLEN - 1, stdin) ) {
			fprintf(stderr, "Input error\n");
			break;
		}

		*current_node = init_node(buffer);
		if(!current_node) {
			fprintf(stderr, "Memory error\n");
			break;
		}
		current_node = &( (*current_node)->next );
	} while(buffer[0] != '.');

	printf("*** BEGINNING OF LIST ***\n");
	print_list(list);

	delete_list(&list);
	exit(EXIT_SUCCESS);
}
