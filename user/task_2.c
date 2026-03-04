#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUF_SZ 256 

int main(int argc, char *argv[]) {
    int pid;
    
    int pipefd[2];

    if (pipe(pipefd) < 0)
    {
        fprintf(2, "Error pipe failure\n");
        exit(1);
    }

    pid = fork();
    switch(pid){
        case -1:
            fprintf(2, "Error fork failure\n");
            exit(1);
        case 0:
            if (close(0)){
                fprintf(2, "Error close 0 failure\n");
                exit(1);
            }
            
            if (dup(pipefd[0]) < 0){
                fprintf(2, "Error dup failure\n");
                exit(1);
            }

            if (close(pipefd[0])){
                fprintf(2, "Error close pipefd[0] failure\n");
                exit(1);
            }

            if (close(pipefd[1])){
                fprintf(2, "Error close pipefd[1] failure\n");
                exit(1);
            }

            char *wc_argv[] = {"/wc", 0};
            exec("/wc", wc_argv);
            fprintf(2, "Error exec failure\n");
            exit(1);
        default:
            int status;

            if (close(pipefd[0])){
                fprintf(2, "Error close pipefd[0] failure\n");
                exit(1);
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
                                fprintf(2, "Error write failure\n");
                                exit(1);
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
                            fprintf(2, "Error write failure\n");
                            exit(1);
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
                        fprintf(2, "Error write failure\n");
                        exit(1);
                    }
                    has_written += a;
                }
            }

            if (close(pipefd[1])){
                fprintf(2, "Error close pipefd[1] failure\n");
                exit(1);
            }

            wait(&status);
    }
    exit(0);
}
