#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
//
int main(int argc, char *argv[]) {
	char buf[1024];
	if (argc > 1) {
		while (true);
	}
	if (fork() == 0) {
		#ifdef CLOSE
		fclose(stdin);
		#endif
		execlp(argv[0], argv[0], "1", NULL);
	}
	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		printf("%s", buf);
		sleep(1);
	}
	return 0;
}