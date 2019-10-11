#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ulimit.h>
#include <sys/resource.h>
#include <limits.h>
extern char **environ;

void print_reugid() {
	printf("Real user ID + group ID: %d + %d\n", getuid(), getgid());
	printf("Effective user ID + group ID: %d + %d\n", geteuid(), getegid());
}

void print_ppppgid() {
	printf("Process ID: %d\n", getpid());
	printf("Parent process ID: %d\n", getppid());
	printf("Process group ID: %d\n", getpgid(0));
}

void errprint_usage(char *argv[]) {
	fprintf(stderr, "Usage: %s [-cdipsuv] [-Csize] [-Unew_ulimit] [-Vname=value]\n", argv[0]);
}

int main(int argc, char *argv[]) {
	int opt;

	while( (opt = getopt(argc, argv, "cC:dipsuU:vV:")) != -1) {
		switch(opt) {
		case 'i':
			print_reugid();
			break;
		case 's':
			setpgid(0, 0);
			break;
		case 'p':
			print_ppppgid();
			break;
		case 'u':
			printf("ulimit = %li\n", ulimit(UL_GETFSIZE, 0));
			break;
		case 'U':
			ulimit(UL_SETFSIZE, atol(optarg));
			break;
		case 'c':
		{
			struct rlimit *rlp = malloc(sizeof(struct rlimit));
			if(rlp) {
				getrlimit(RLIMIT_CORE, rlp);
				if(rlp->rlim_cur == RLIM_INFINITY)
					printf("core file size limit = infinity\n");
				else
					printf("core file size limit = %li\n", rlp->rlim_cur);
				free(rlp);
			}
			else {
				fprintf(stderr, "Memory failure\n");
				exit(EXIT_FAILURE);
			}
		}
			break;
		case 'C':
		{
			struct rlimit *rlp = malloc(sizeof(struct rlimit));
			if(rlp) {
				getrlimit(RLIMIT_CORE, rlp);
				
				rlp->rlim_cur = atol(optarg);
				setrlimit(RLIMIT_CORE, rlp);

				free(rlp);
			}
			else {
				fprintf(stderr, "Memory failure\n");
				exit(EXIT_FAILURE);
			}
		}
			break;
		case 'd':
		{
			char *cwd = malloc(sizeof(char) * PATH_MAX);
			if(cwd) {
				getcwd(cwd, PATH_MAX);
				printf("Current working directory: %s\n", cwd);
				free(cwd);
			}
			else
				fprintf(stderr, "Failure when getting current working directory\n");
		}
			break;
		case 'v':
			for(char **variable = environ; *variable; ++variable)
				printf("%s\n", *variable);
			break;
		case 'V':
			putenv(optarg);
			break;
		default:
			errprint_usage(argv);
			exit(EXIT_FAILURE);
		}
	}

	if(argc == 1) 
		errprint_usage(argv);
	exit(EXIT_SUCCESS);
}
