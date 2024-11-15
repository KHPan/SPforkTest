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

int main(int argc, char *argv[]) {
	char buf[1024];
	if (fork() == 0) {
		#ifdef CLOSE
		fclose(stdin);
		#endif
		while (true);
	}
	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		printf("%s", buf);
	}
	return 0;
}