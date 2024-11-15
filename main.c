#define _GNU_SOURCE
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

#define PARENT_READ_FD 3
#define PARENT_WRITE_FD 4
#define MAX_CHILDREN 32
#define MAX_CHILD_DEEP 8
#define MAX_FIFO_NAME_LEN 9
#define MAX_FRIEND_INFO_LEN 12
#define MAX_FRIEND_NAME_LEN 9
#define MAX_CMD_LEN 256
#include <sys/types.h>
typedef struct {
    char name[MAX_FRIEND_NAME_LEN];
} friend;



#define ERR_EXIT(s) {char msg[1024]; sprintf(msg, "friend %s happen: %s", friend_name, s); perror(msg), exit(errno);}

char fail_feedback = 1;
char success_feedback = 0;
pid_t process_pid;

// somethings I recommend leaving here, but you may delete as you please
static char root[MAX_FRIEND_INFO_LEN] = "Not_Tako";     // root of tree
static bool show = true;    // 在root上到底要不要show
static char friend_info[MAX_FRIEND_INFO_LEN];   // current process info
static char friend_name[MAX_FRIEND_NAME_LEN];   // current process name
static int friend_value;    // current process value
FILE* read_fp = NULL;
char program_name[MAX_CMD_LEN]; // program name

friend *children[MAX_CHILDREN] = {0}; // array of children
char command[MAX_CMD_LEN]; // command buffer

// Is Root of tree
static inline bool is_Not_Tako() {
    return (strcmp(friend_name, root) == 0);
}

void Meet(char *parent, char *child) {
	int fds[2][2];
	if (pipe(fds[0]) < 0 || pipe(fds[1]) < 0) {
		ERR_EXIT("pipe create error");
	}
	pid_t pid = fork();
	if (pid < 0) {
		ERR_EXIT("fork error");
	}
	if (pid == 0) {
		close(fds[0][0]);
		close(fds[1][1]);
		if (fds[0][1] != PARENT_WRITE_FD) {
			if (fds[1][0] == PARENT_WRITE_FD) {
				int new_fd = dup(fds[1][0]);
				if (new_fd < 0)
					ERR_EXIT("dup error");
				close(fds[1][0]);
				fds[1][0] = new_fd;
			}
			if (dup2(fds[0][1], PARENT_WRITE_FD) < 0)
				ERR_EXIT("dup2 error");
			close(fds[0][1]);
		}
		if (fds[1][0] != PARENT_READ_FD) {
			if (dup2(fds[1][0], PARENT_READ_FD) < 0)
				ERR_EXIT("dup2 error");
			close(fds[1][0]);
		}
		execlp(program_name, program_name, child, NULL);
	}
	close(fds[0][1]);
	close(fds[1][0]);
	friend *new_friend = (friend *)malloc(sizeof(friend));
	if (new_friend == NULL)
		ERR_EXIT("malloc new_friend error");
	strcpy(new_friend->name, child);
	for (int i = 0; i < MAX_CHILDREN; i++) {
		if (children[i] == NULL) {
			children[i] = new_friend;
			break;
		}
		if (i == MAX_CHILDREN - 1)
			ERR_EXIT("children array full");
	}
}

pid_t fork_pid = 0, old_friend_pid = 0;
char Adopt(char *parent, char *child) {
	if (parent[0] == '$') { //第四次遞迴，把資料送進FIFO
        char ccmd[MAX_CMD_LEN], buf;
		friend *old_friend = children[0];
		children[MAX_CHILDREN - 1] = NULL;
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
        return success_feedback;
    }
    else if (is_Not_Tako()) {
        char check_parent[MAX_CMD_LEN];
		int parent_value = 100;
        if (mkfifo("Adopt.fifo", 0666) < 0 && errno != EEXIST)
            ERR_EXIT("mkfifo error");
        sprintf(check_parent, "$%d", parent_value);
        Adopt(check_parent, child);
        FILE *fp = fopen("Adopt.fifo", "r");
        char buf[MAX_CMD_LEN];
        fgets(buf, MAX_CMD_LEN, fp);
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = '\0';
        char to_meet[MAX_CMD_LEN];
        strcpy(to_meet, buf);
        show = false;
        Meet(parent, to_meet);
        while (true) {
            if (fgets(buf, MAX_CMD_LEN, fp) == NULL)
                continue;
            if (buf[strlen(buf) - 1] == '\n')
                buf[strlen(buf) - 1] = '\0';
            char *main_cmd = strtok(buf, " ");
            if (strcmp(main_cmd, "Meet") == 0) {
                char *mparent = strtok(NULL, " ");
                char *mchild = strtok(NULL, " ");
                Meet(mparent, mchild);
            } else if (strcmp(main_cmd, "end") == 0)
                break;
        }
        // show = true;
        fclose(fp);
        if (unlink("Adopt.fifo") < 0)
            ERR_EXIT("unlink error");
    }
    return success_feedback;
}

int main(int argc, char *argv[]) {
	strcpy(program_name, argv[0]);
    // Hi! Welcome to SP Homework 2, I hope you have fun
    process_pid = getpid(); // you might need this when using fork()
    if (argc != 2) {
        fprintf(stderr, "Usage: ./friend [friend_info]\n");
        return 0;
    }
    setvbuf(stdout, NULL, _IONBF, 0); // prevent buffered I/O, equivalent to fflush() after each stdout, study this as you may need to do it for other friends against their parents
    
    // put argument one into friend_info
    strncpy(friend_info, argv[1], MAX_FRIEND_INFO_LEN);
    
    if(strcmp(argv[1], root) == 0){
        memset(friend_name, 0, MAX_FRIEND_NAME_LEN);
        strcpy(friend_name, root);
        read_fp = stdin;        // takes commands from stdin
        friend_value = 100;     // Not_Tako adopting nodes will not mod their values
    }
    else{
        // char *sep = strchr(friend_info, '_');
        // memset(friend_name, 0, MAX_FRIEND_NAME_LEN);
        // strncpy(friend_name, friend_info, sep - friend_info);
        // friend_value = atoi(sep + 1);
        // // where do you read from?
        // read_fp = fdopen(PARENT_READ_FD, "r");
        // if (read_fp == NULL) {
        //     ERR_EXIT("fdopen error");
        // }
        // fclose(stdin);
        // // fclose(stdout);
		int i = 0;
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
            char *parent = strtok(NULL, " ");
            char *child = strtok(NULL, " ");
            Meet(parent, child);
        }
        else if (strcmp(main_cmd, "Adopt") == 0) {
            char *parent = strtok(NULL, " ");
            char *child = strtok(NULL, " ");
            Adopt(parent, child);
		}
    }
	return 0;
}