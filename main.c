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

#define MAX_FRIEND_INFO_LEN 12
#define MAX_FRIEND_NAME_LEN 9
#define MAX_CMD_LEN 256
#include <sys/types.h>



#define ERR_EXIT(s) perror(s), exit(errno)

// somethings I recommend leaving here, but you may delete as you please
static char root[MAX_FRIEND_INFO_LEN] = "Not_Tako";     // root of tree
char program_name[MAX_CMD_LEN]; // program name

char command[MAX_CMD_LEN]; // command buffer

void Meet() {
	pid_t pid = fork();
	if (pid < 0) {
		ERR_EXIT("fork error");
	}
	if (pid == 0) {
		execlp(program_name, program_name, "A", NULL);
	}
}

pid_t fork_pid = 0, old_friend_pid = 0;
void Adopt() {
	// char check_parent[MAX_CMD_LEN];
	// int parent_value = 100;
	if (mkfifo("Adopt.fifo", 0666) < 0 && errno != EEXIST)
		ERR_EXIT("mkfifo error");
	fork_pid = fork();
	if (fork_pid < 0)
		ERR_EXIT("fork error");
	if (fork_pid == 0) {
		#ifdef CLOSE
		fclose(stdin);
		#endif
		int fd = open("Adopt.fifo", O_WRONLY);
		if (fd < 0)
			ERR_EXIT("open fifo error");
		char str[] = "1111";
		if (write(fd, str, strlen(str)+1) < 0)
			ERR_EXIT("adopt write fifo error");
		if (close(fd) < 0)
			ERR_EXIT("close fifo error");
		exit(0);
	}

	FILE *fp = fopen("Adopt.fifo", "r");
	sleep(1);
	fclose(fp);
	if (unlink("Adopt.fifo") < 0)
		ERR_EXIT("unlink error");
}

int main(int argc, char *argv[]) {
	strcpy(program_name, argv[0]);
    
    if (strcmp(argv[1], root) != 0) {
		long long i = 0;
		for (i = 0; i > -1; i++);
		exit(0);
    }

    for (int i = 0; fgets(command, MAX_CMD_LEN, stdin) != NULL; i++) {
		if (command[strlen(command)-1] == '\n')
			command[strlen(command)-1] = '\0';
		fprintf(stderr, "%lld command: %s\n", getpid(), command);
			
        char *main_cmd = strtok(command, " ");
        if (i < 2) {
            Meet();
        }
        else if (i == 2) {
            Adopt();
		}
    }
	return 0;
}