#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[]) {
    int pid = fork();
    switch(pid){
        case -1:
            fprintf(2, "Error fork failure\n");
            exit(1);
        case 0:
            pause(10 * 10);
            exit(1);    
        default:
            printf("Parrent Pid = %d, Child Pid = %d\n", getpid(), pid);

            if (kill(pid) < 0){
                fprintf(2, "Error kill failure\n");
                exit(1);
            }

            int status;
            int cpid = wait(&status);

            if (cpid < 0){
                fprintf(2, "Error wait failure\n");
                exit(1);
            }
            
            printf("Child Pid = %d, exit status = %d\n", cpid, status);
    }
    exit(0);
}