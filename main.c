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
    pid_t pid;
    int read_fd;
    int write_fd;
    char info[MAX_FRIEND_INFO_LEN];
    char name[MAX_FRIEND_NAME_LEN];
    int value;
} friend;



#define ERR_EXIT(s) {char msg[1024]; sprintf(msg, "friend %s happen: %s", friend_name, s); perror(msg), exit(errno);}

char fail_feedback = 1;
char success_feedback = 0;
pid_t process_pid;

/*
If you need help from TAs,
please remember :
0. Show your efforts
    0.1 Fully understand course materials
    0.2 Read the spec thoroughly, including frequently updated FAQ section
    0.3 Use online resources
    0.4 Ask your friends while avoiding plagiarism, they might be able to understand you better, since the TAs know the solution, 
        they might not understand what you're trying to do as quickly as someone who is also doing this homework.
1. be respectful
2. the quality of your question will directly impact the value of the response you get.
3. think about what you want from your question, what is the response you expect to get
4. what do you want the TA to help you with. 
    4.0 Unrelated to Homework (wsl, workstation, systems configuration)
    4.1 Debug
    4.2 Logic evaluation (we may answer doable yes or no, but not always correct or incorrect, as it might be giving out the solution)
    4.3 Spec details inquiry
    4.4 Testcase possibility
5. If the solution to answering your question requires the TA to look at your code, you probably shouldn't ask it.
6. We CANNOT tell you the answer, but we can tell you how your current effort may approach it.
7. If you come with nothing, we cannot help you with anything.
*/

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

// a bunch of prints for you
void print_direct_meet(char *new_friend_name) {
    if (show)
        fprintf(stdout, "Not_Tako has met %s by himself\n", new_friend_name);
}

void print_indirect_meet(char *parent_friend_name, char *child_friend_name) {
    if (show)
        fprintf(stdout, "Not_Tako has met %s through %s\n", child_friend_name, parent_friend_name);
}

void print_fail_meet(char *parent_friend_name, char *child_friend_name) {
    if (show)
        fprintf(stdout, "Not_Tako does not know %s to meet %s\n", parent_friend_name, child_friend_name);
}

void print_fail_check(char *parent_friend_name){
    if (show)
        fprintf(stdout, "Not_Tako has checked, he doesn't know %s\n", parent_friend_name);
}

void print_success_adopt(char *parent_friend_name, char *child_friend_name) {
    if (show)
        fprintf(stdout, "%s has adopted %s\n", parent_friend_name, child_friend_name);
}

void print_fail_adopt(char *parent_friend_name, char *child_friend_name) {
    if (show)
        fprintf(stdout, "%s is a descendant of %s\n", parent_friend_name, child_friend_name);
}

void print_compare_gtr(char *new_friend_name){
    if (show)
        fprintf(stdout, "Not_Tako is still friends with %s\n", new_friend_name);
}

void print_compare_leq(char *new_friend_name){
    if (show)
        fprintf(stdout, "%s is dead to Not_Tako\n", new_friend_name);
}

void print_final_graduate(){
    if (show)
        fprintf(stdout, "Congratulations! You've finished Not_Tako's annoying tasks!\n");
}

/* terminate child pseudo code
void clean_child(){
    close(child read_fd);
    close(child write_fd);
    call wait() or waitpid() to reap child; // this is blocking
}

*/

/* remember read and write may not be fully transmitted in HW1?
void fully_write(int write_fd, void *write_buf, int write_len);

void fully_read(int read_fd, void *read_buf, int read_len);

please do above 2 functions to save some time
*/

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
		if (is_Not_Tako()) {
			fclose(stdin);
		}
		for (int i = 0; i < MAX_CHILDREN; i++) {
			if (children[i] != NULL) {
				if (close(children[i]->read_fd) < 0)
					ERR_EXIT("close read_fd error");
				if (close(children[i]->write_fd) < 0)
					ERR_EXIT("close write_fd error");
			}
		}
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
	new_friend->pid = pid;
	new_friend->read_fd = fds[0][0];
	new_friend->write_fd = fds[1][1];
	strcpy(new_friend->info, child);
	strcpy(new_friend->name, strtok(child, "_"));
	new_friend->value = atoi(strtok(NULL, "_"));
	for (int i = 0; i < MAX_CHILDREN; i++) {
		if (children[i] == NULL) {
			children[i] = new_friend;
			break;
		}
		if (i == MAX_CHILDREN - 1)
			ERR_EXIT("children array full");
	}
	if (is_Not_Tako()) {
		print_direct_meet(new_friend->name);
	} else {
		if (write(PARENT_WRITE_FD, &success_feedback, 1) < 0)
			ERR_EXIT("meet parent success write error");
	}
}

pid_t fork_pid = 0, old_friend_pid = 0;
char Adopt(char *parent, char *child) {
    // if (parent[0] == '!') { //第一次遞迴，確認child與parent相對位置
    //     char ccmd[MAX_CMD_LEN], buf;
    //     if (strcmp(child, friend_name) == 0)
    //         parent[0] = '@'; //標記上面有child
    //     sprintf(ccmd, "Adopt %s %s", parent, child);
    //     for (int i = 0; i < MAX_CHILDREN; i++) {
    //         if (children[i] == NULL) {
    //             if (!is_Not_Tako()) {
    //                 if (write(PARENT_WRITE_FD, &success_feedback, 1) < 0)
    //                     ERR_EXIT("adopt parent success write error");
    //             }
    //             return success_feedback;
    //         }
    //         if (write(children[i]->write_fd, ccmd,
    //                     strlen(ccmd)) < 0 ||
    //             write(children[i]->write_fd, "\n", 1) < 0)
    //             ERR_EXIT("adopt child write error");
    //         if (read(children[i]->read_fd, &buf, 1) < 0)
    //             ERR_EXIT("adopt child read error");
    //         if (buf == fail_feedback) {
    //             if (!is_Not_Tako()) {
    //                 if (write(PARENT_WRITE_FD, &fail_feedback, 1) < 0)
    //                     ERR_EXIT("adopt parent fail write error");
    //             }
    //             return fail_feedback;
    //         }
    //     }
    // }
    // else if (parent[0] == '@') { //第二次遞迴，上面有child
    //     if (strcmp(parent + 1, friend_name) == 0) {
    //         if (!is_Not_Tako()) {
    //             if (write(PARENT_WRITE_FD, &fail_feedback, 1) < 0)
    //                 ERR_EXIT("adopt parent fail write error");
    //         }
    //         return fail_feedback;
    //     } else {
    //         char ccmd[MAX_CMD_LEN], buf;
    //         sprintf(ccmd, "Adopt %s %s", parent, child);
    //         for (int i = 0; i < MAX_CHILDREN; i++) {
    //             if (children[i] == NULL) {
    //                 if (!is_Not_Tako()) {
    //                     if (write(PARENT_WRITE_FD, &success_feedback, 1) < 0)
    //                         ERR_EXIT("adopt parent success write error");
    //                 }
    //                 return success_feedback;
    //             }
    //             if (write(children[i]->write_fd, ccmd,
    //                         strlen(ccmd)) < 0 ||
    //                 write(children[i]->write_fd, "\n", 1) < 0)
    //                 ERR_EXIT("adopt child write error");
    //             if (read(children[i]->read_fd, &buf, 1) < 0)
    //                 ERR_EXIT("adopt child read error");
    //             if (buf == fail_feedback) {
    //                 if (!is_Not_Tako()) {
    //                     if (write(PARENT_WRITE_FD, &fail_feedback, 1) < 0)
    //                         ERR_EXIT("adopt parent fail write error");
    //                 }
    //                 return fail_feedback;
    //             }
    //         }
    //     }
    // }
    // else if (parent[0] == '#') { //第三次遞迴，找parent的值
    //     if (strcmp(friend_name, parent + 1) == 0) {
    //         char buf = friend_value + 2;
    //         if (!is_Not_Tako()) {
    //             if (write(PARENT_WRITE_FD, &buf, 1) < 0)
    //                 ERR_EXIT("adopt parent write error");
    //         }
    //         return buf;
    //     }
    //     char ccmd[MAX_CMD_LEN], buf;
    //     sprintf(ccmd, "Adopt %s %s", parent, child);
    //     for (int i = 0; i < MAX_CHILDREN; i++) {
    //         if (children[i] == NULL) {
    //             if (!is_Not_Tako()) {
    //                 if (write(PARENT_WRITE_FD, &fail_feedback, 1) < 0)
    //                     ERR_EXIT("adopt parent fail write error");
    //             }
    //             return fail_feedback;
    //         }
    //         if (write(children[i]->write_fd, ccmd,
    //                     strlen(ccmd)) < 0 ||
    //             write(children[i]->write_fd, "\n", 1) < 0)
    //             ERR_EXIT("adopt child write error");
    //         if (read(children[i]->read_fd, &buf, 1) < 0)
    //             ERR_EXIT("adopt child read error");
    //         if (buf != fail_feedback) {
    //             if (!is_Not_Tako()) {
    //                 if (write(PARENT_WRITE_FD, &buf, 1) < 0)
    //                     ERR_EXIT("adopt parent write error");
    //             }
    //             return buf;
    //         }
    //     }
    // }
    // else
	 if (parent[0] == '$') { //第四次遞迴，把資料送進FIFO
        if (strcmp(friend_name, child) == 0) {
            int fd = open("Adopt.fifo", O_WRONLY);
            if (fd < 0)
                ERR_EXIT("open fifo error");
            char ccmd[MAX_CMD_LEN];
            int be_mod = atoi(parent + 1);
            for (int i = 0; i < MAX_CHILDREN; i++) {
                if (children[i] == NULL)
                    break;
                sprintf(ccmd, "Meet %s %s_%02d", friend_name,
                    children[i]->name, children[i]->value % be_mod);
                if (write(fd, ccmd, strlen(ccmd)) < 0 ||
                    write(fd, "\n", 1) < 0)
                    ERR_EXIT("adopt write fifo error");
                sprintf(ccmd, "Adopt %s %s", parent, children[i]->name);
                if (write(children[i]->write_fd, ccmd,
                            strlen(ccmd)) < 0 ||
                    write(children[i]->write_fd, "\n", 1) < 0)
                    ERR_EXIT("adopt child write error");
                char buf;
                if (read(children[i]->read_fd, &buf, 1) < 0)
                    ERR_EXIT("adopt child read error");
                if (close(children[i]->read_fd) < 0)
                    ERR_EXIT("close read_fd error");
                if (close(children[i]->write_fd) < 0)
                    ERR_EXIT("close write_fd error");
                waitpid(children[i]->pid, NULL, 0);
                free(children[i]);
            }
            if (close(fd) < 0)
                ERR_EXIT("close fifo error");
            if (write(PARENT_WRITE_FD, &success_feedback, 1) < 0)
                ERR_EXIT("adopt parent write error");
            exit(0);
        }
        char ccmd[MAX_CMD_LEN], buf;
        for (int i = 0; i < MAX_CHILDREN; i++) {
            if (children[i] == NULL)
                break;
            if (strcmp(children[i]->name, child) == 0) {
                friend *old_friend = children[i];
                for (int j = i; j < MAX_CHILDREN - 1; j++) {
                    children[j] = children[j + 1];
                }
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
                    int be_mod = atoi(parent + 1);
                    sprintf(ccmd, "%s_%02d",
                        old_friend->name, old_friend->value % be_mod);
                    if (write(fd, ccmd, strlen(ccmd)) < 0 ||
                        write(fd, "\n", 1) < 0)
                        ERR_EXIT("adopt write fifo error");
                    sprintf(ccmd, "Adopt %s %s", parent, child);
                    if (write(old_friend->write_fd, ccmd,
                                strlen(ccmd)) < 0 ||
                        write(old_friend->write_fd, "\n", 1) < 0)
                        ERR_EXIT("adopt child write error");
                    if (read(old_friend->read_fd, &buf, 1) < 0)
                        ERR_EXIT("adopt child read error");
                    sprintf(ccmd, "end");
                    if (write(fd, ccmd,
                                strlen(ccmd)) < 0 ||
                        write(fd, "\n", 1) < 0)
                        ERR_EXIT("adopt child write error");
                    if (close(fd) < 0)
                        ERR_EXIT("close fifo error");
                    exit(0);
                }
                if (close(old_friend->read_fd) < 0)
                    ERR_EXIT("close read_fd error");
                if (close(old_friend->write_fd) < 0)
                    ERR_EXIT("close write_fd error");
                old_friend_pid = old_friend->pid;
                free(old_friend);
                break;
            }
            sprintf(ccmd, "Adopt %s %s", parent, child);
            if (write(children[i]->write_fd, ccmd,
                        strlen(ccmd)) < 0 ||
                write(children[i]->write_fd, "\n", 1) < 0)
                ERR_EXIT("adopt child write error");
            if (read(children[i]->read_fd, &buf, 1) < 0)
                ERR_EXIT("adopt child read error");
        }
        if (!is_Not_Tako()) {
            if (write(PARENT_WRITE_FD, &success_feedback, 1) < 0)
                ERR_EXIT("adopt parent write error");
        }
        return success_feedback;
    }
    else if (parent[0] == '%') { //第五次遞迴，清光所有fork
        if (fork_pid > 0) {
            if (waitpid(fork_pid, NULL, 0) < 0)
                ERR_EXIT("waitpid error");
            fork_pid = 0;
        }
        if (old_friend_pid > 0) {
            if (waitpid(old_friend_pid, NULL, 0) < 0)
                ERR_EXIT("waitpid error");
            old_friend_pid = 0;
        }
        char ccmd[MAX_CMD_LEN], buf;
        sprintf(ccmd, "Adopt %s %s", parent, child);
        for (int i = 0; i < MAX_CHILDREN; i++) {
            if (children[i] == NULL)
                break;
            if (write(children[i]->write_fd, ccmd,
                        strlen(ccmd)) < 0 ||
                write(children[i]->write_fd, "\n", 1) < 0)
                ERR_EXIT("adopt child write error");
            if (read(children[i]->read_fd, &buf, 1) < 0)
                ERR_EXIT("adopt child read error");
        }
        if (!is_Not_Tako()) {
            if (write(PARENT_WRITE_FD, &success_feedback, 1) < 0)
                ERR_EXIT("adopt parent write error");
        }
        return success_feedback;
    }
    else if (is_Not_Tako()) {
        char check_parent[MAX_CMD_LEN];
        // sprintf(check_parent, "!%s", parent);
        // if (Adopt(check_parent, child) == fail_feedback) {
        //     print_fail_adopt(parent, child);
        //     return fail_feedback;
        // }
        // sprintf(check_parent, "#%s", parent);
        // int parent_value = Adopt(check_parent, child) - 2;
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
        show = true;
        fclose(fp);
        if (unlink("Adopt.fifo") < 0)
            ERR_EXIT("unlink error");
        Adopt("%", child);
        print_success_adopt(parent, child);
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
        // is Not_Tako
        // strncpy(friend_name, friend_info,MAX_FRIEND_NAME_LEN);      // put name into friend_nae
        // friend_name[MAX_FRIEND_NAME_LEN - 1] = '\0';        // in case strcmp messes with you
        memset(friend_name, 0, MAX_FRIEND_NAME_LEN);
        strcpy(friend_name, root);
        read_fp = stdin;        // takes commands from stdin
        friend_value = 100;     // Not_Tako adopting nodes will not mod their values
    }
    else{
        // is other friends
        // extract name and value from info
        char *sep = strchr(friend_info, '_');
        memset(friend_name, 0, MAX_FRIEND_NAME_LEN);
        strncpy(friend_name, friend_info, sep - friend_info);
        friend_value = atoi(sep + 1);
        // where do you read from?
        read_fp = fdopen(PARENT_READ_FD, "r");
        if (read_fp == NULL) {
            ERR_EXIT("fdopen error");
        }
        // anything else you have to do before you start taking commands?
        fclose(stdin);
        // fclose(stdout);
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