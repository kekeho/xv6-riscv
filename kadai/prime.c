#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

int shutdown = -1;

void handler(int fd, int my_number, int until) {
    int got_num;
    bool next = false;
    pid_t pid = 0;
    int fds[2];
    int status;
    printf("%d\n", my_number);

    while (1) {
        if (read(fd, &got_num, sizeof(int)) < 0) {
            perror("read");
            exit(1);
        }

        if (got_num == shutdown) {
            if (next) {
                write(fds[1], &shutdown, sizeof(int));
                wait(&status);
            }
            exit(0);
        }

        if (got_num % my_number == 0) {
            continue;  // 割り切れるのでおしまい
        }

        if (!next) {
            if (pipe(fds) < 0) {
                perror("pipe");
                exit(1);
            }
            pid = fork();
            if (pid == 0) {
                // child
                handler(fds[0], got_num, until);
            } else {
                next = true;
                write(fds[1], &got_num, sizeof(int));
            }
        } else {
            // 子プロセスがいる
            write(fds[1], &got_num, sizeof(int));
        }
    }
}


int main(int argc, const char* argv[]) {
    pid_t pid;
    int fds[2];
    int until;
    int status;

    if (argc < 2) {
        fprintf(stderr, "Error: missing number!\n");
        exit(-1);
    }
    until = atoi(argv[1]);

    if (pipe(fds) < 0) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid == 0) {
        // child
        handler(fds[0], 2, until);
    } else {
        // Number generator
        for (int i = 2; i <= until; i++) {
            write(fds[1], &i, sizeof(int));
        }

        write(fds[1], &shutdown, sizeof(int));
        wait(&status);
    }

    return 0;
}