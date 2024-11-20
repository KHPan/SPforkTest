#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#define ERR_EXIT(s) perror(s), exit(errno)

void Adopt() {
	if (mkfifo("Adopt.fifo", 0666) < 0 && errno != EEXIST)
		ERR_EXIT("mkfifo error");
	pid_t fork_pid = fork();
	if (fork_pid < 0)
		ERR_EXIT("fork error");
	if (fork_pid == 0) {
		#ifdef CLOSE
		fclose(stdin);
		#endif
		exit(0);
	}
	if (unlink("Adopt.fifo") < 0)
		ERR_EXIT("unlink error");
}

int main(int argc, char *argv[]) {
	int adopt = 2;
	if (argc > 1) {
		adopt = atoi(argv[1]);
	}
	char command[1024];
	if (adopt < 0)
		Adopt();
    for (int i = 0; fgets(command, 1024, stdin) != NULL; i++) {
		if (command[strlen(command)-1] == '\n')
			command[strlen(command)-1] = '\0';
		fprintf(stderr, "%lld command: %s\n", (long long)getpid(), command);
		if (i == adopt)
			Adopt();
    }
	return 0;
}