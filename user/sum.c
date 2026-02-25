#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUF_SZ 256

int main(int argc, char *argv[]) {
    char buf[BUF_SZ], c;
    int i = 0, a;

    while (BUF_SZ - 1 > i) {
        a = read(0, &c, 1);
        
        if (a < 0) {
            char e_msg[] = "Error: Read failure\n";
            write(2, e_msg, sizeof(e_msg) - 1);
            exit(1);
        }
        if (a == 0) {
            break;
        }
        if (c == '\n') {
            break;
        }
        
        buf[i] = c; i++;
    }
    
    buf[i] = '\0';

    if (i == 0) {
        char e_msg[] = "Error: Empty input\n";
        write(2, e_msg, sizeof(e_msg) - 1);
        exit(1);
    }

    printf("|%s|\n", buf);

    char *sp = 0;
    for (int j = 0; j < i; j++) {
        if (buf[j] == ' ') {
            sp = &buf[j];
            break;
        }
    }

    if (sp == 0){
        char e_msg[] = "Error: Invalid input format (missing space)\n";
        write(2, e_msg, sizeof(e_msg) - 1);
        exit(1);
    }

    *sp = '\0';
    char *ptr_n2 = sp + 1;  

    if (*buf < '0' || *buf > '9') {
        char e_msg[] = "Error: No first number\n";
        write(2, e_msg, sizeof(e_msg) - 1);
        exit(1);
    }

    if (*ptr_n2 < '0' || *ptr_n2 > '9') {
        char e_msg[] = "Error: No second number\n";
        write(2, e_msg, sizeof(e_msg) - 1);
        exit(1);
    }

    int n = atoi(buf);
    int m = atoi(ptr_n2);
    printf("%d\n", n + m);

    exit(0);
}