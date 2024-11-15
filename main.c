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
static char friend_name[MAX_FRIEND_NAME_LEN];   // current process name
FILE* read_fp = NULL;
char program_name[MAX_CMD_LEN]; // program name

char command[MAX_CMD_LEN]; // command buffer

// Is Root of tree
static inline bool is_Not_Tako() {
    return (strcmp(friend_name, root) == 0);
}

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
void Adopt(char parent) {
	if (parent == '$') { //第四次遞迴，把資料送進FIFO
		fork_pid = fork();
		if (fork_pid < 0)
			ERR_EXIT("fork error");
		if (fork_pid == 0) {
			#ifdef CLOSE
			if (is_Not_Tako())
				fclose(stdin);
			#endif
			int fd = open("Adopt.fifo", O_WRONLY);
			if (fd < 0)
				ERR_EXIT("open fifo error");
			char str[] = "s_5\nend";
			if (write(fd, str, strlen(str)+1) < 0)
				ERR_EXIT("adopt write fifo error");
			if (close(fd) < 0)
				ERR_EXIT("close fifo error");
			exit(0);
		}
    }
    else {
        char check_parent[MAX_CMD_LEN];
		int parent_value = 100;
        if (mkfifo("Adopt.fifo", 0666) < 0 && errno != EEXIST)
            ERR_EXIT("mkfifo error");
		Adopt('$');
        FILE *fp = fopen("Adopt.fifo", "r");
        char buf[MAX_CMD_LEN];
        fgets(buf, MAX_CMD_LEN, fp);
		Meet();
        while (true) {
            if (fgets(buf, MAX_CMD_LEN, fp) == NULL)
                continue;
            break;
        }
        fclose(fp);
        if (unlink("Adopt.fifo") < 0)
            ERR_EXIT("unlink error");
    }
}

int main(int argc, char *argv[]) {
	strcpy(program_name, argv[0]);
    
    if(strcmp(argv[1], root) == 0){
        strcpy(friend_name, root);
        read_fp = stdin;        // takes commands from stdin
    }
    else{
		long long i = 0;
		for (i = 0; i > -1; i++);
    }

    while (fgets(command, MAX_CMD_LEN, read_fp) != NULL && !feof(read_fp)) {
        if (strlen(command) == 0)
            continue;
        if (command[strlen(command) - 1] == '\n')
            command[strlen(command) - 1] = '\0';
        if (command[strlen(command) - 1] == '\r')
            command[strlen(command) - 1] = '\0';
        if (strlen(command) == 0)
            continue;
        char command_copy[MAX_CMD_LEN];
        strcpy(command_copy, command);
		if (is_Not_Tako())
			fprintf(stderr, "%lld command: %s\n", getpid(), command);
			
        char *main_cmd = strtok(command, " ");
        if (strcmp(main_cmd, "Meet") == 0) {
            Meet();
        }
        else if (strcmp(main_cmd, "Adopt") == 0) {
            Adopt(' ');
		}
    }
	return 0;
}