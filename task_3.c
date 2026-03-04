#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define BUF_SZ 8192 

int main(int argc, char *argv[]){
    pid_t pid;
    
    int pipefd[2];

    if (pipe(pipefd) < 0)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    switch(pid){
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            if (close(pipefd[1]) < 0) {
                perror("child close pipefd[1]");
                exit(EXIT_FAILURE);
            }

            char buffer[BUF_SZ];
            int has_read;

            while ((has_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[has_read] = '\0';
                printf("%s", buffer);
            }

            if (has_read < 0) {
                perror("read error");
                exit(EXIT_FAILURE);
            }

            if (close(pipefd[0])){
                perror("child close pipfd[0]");
                exit(EXIT_FAILURE);
            }

            exit(EXIT_SUCCESS);
        default:
            int status;
            if (close(pipefd[0])){
                perror("par close pipfd[0]");
                exit(EXIT_FAILURE);
            }

            char buf[BUF_SZ];
            int buf_pos = 0;

            for (int i = 1; i < argc; i++) {
                char *arg = argv[i];
                int len = strlen(arg);
                
                for (int j = 0; j < len; j++) {
                    buf[buf_pos++] = arg[j];

                    if (buf_pos == BUF_SZ) {
                        int has_written = 0;
                        while (has_written < BUF_SZ) {
                            int a = write(pipefd[1], buf + has_written, BUF_SZ - has_written);
                            if (a < 0) {
                                perror("write error in parent");
                                exit(EXIT_FAILURE);
                            }
                            has_written += a;
                        }
                        buf_pos = 0;
                    }
                }
                
                buf[buf_pos++] = '\n';
                if (buf_pos == BUF_SZ) {
                    int has_written = 0;
                    while (has_written < BUF_SZ) {
                        int a = write(pipefd[1], buf + has_written, BUF_SZ - has_written);
                        if (a < 0) {
                            perror("write error in parent");
                            exit(EXIT_FAILURE);
                        }
                        has_written += a;
                    }
                    buf_pos = 0;
                }
            }
            
            if (buf_pos > 0) {
                int has_written = 0;
                while (has_written < buf_pos) {
                    int a = write(pipefd[1], buf + has_written, buf_pos - has_written);
                    if (a < 0) {
                        perror("write error in parent");
                        exit(EXIT_FAILURE);
                    }
                    has_written += a;
                }
            }

            if (close(pipefd[1])){
                perror("par close pipfd[1]");
                exit(EXIT_FAILURE);
            }

            wait(&status);

            exit(EXIT_SUCCESS);
    }
}