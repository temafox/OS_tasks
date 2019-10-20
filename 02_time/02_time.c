#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
extern char *tzname[];

int main() {
	time_t now;
	struct tm *sp;
	
	now = time(0);

	printf("********\n");
	printf("Our time\n");
	printf("********\n");

	printf("%s", ctime( &now ));

	sp = localtime( &now );
	printf("%d/%d/%02d %d:%02d:%02d %s\n\n",
			sp->tm_mon + 1, sp->tm_mday, sp->tm_year + 1900,
			sp->tm_hour, sp->tm_min, sp->tm_sec, tzname[ sp->tm_isdst ]);

	/* Read configuration for the timezone */
	FILE *conf = fopen("02_time.conf", "r");
	char my_tz_name[256], my_tz_data[256];
	if(!conf) {
		perror("Error when opening the \"02_time.conf\" file");
		printf("-> set timezone to UTC\n");
		strcpy(my_tz_name, "UTC");
		strcpy(my_tz_data, "");
	} else {
		fscanf(conf, "%s", my_tz_name);
		fscanf(conf, "%s", my_tz_data);
		fclose(conf);
	}

	printf("************************\n");
	printf("%s time\n", my_tz_name);
	printf("************************\n");

	if(setenv("TZ", my_tz_data, /*overwrite*/ 1))
		perror("setenv() TZ to PST/PDT has thrown an error");
	tzset();

	printf("%s", ctime( &now ));

	sp = localtime( &now );
	printf("%d/%d/%02d %d:%02d:%02d %s\n\n",
			sp->tm_mon + 1, sp->tm_mday, sp->tm_year + 1900,
			sp->tm_hour, sp->tm_min, sp->tm_sec, tzname[ sp->tm_isdst ]);


	return 0;
}

