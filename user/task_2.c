#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

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

            for(int i = 0; i < argc; i++) {
                char *arg = argv[i];
                int len = strlen(arg);
                int has_written = 0;

                while (has_written < len) {
                    int n = write(pipefd[1], arg + has_written, len - has_written);
                    if (n < 0) {
                        fprintf(2, "Error write failure\n");
                        exit(1);
                    }
                    has_written += n;
                }

                if (write(pipefd[1], "\n", 1) < 0) {
                    fprintf(2, "Error write newline failure\n");
                    exit(1);
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
