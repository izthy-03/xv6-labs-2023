#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int p[2];
    char buf[8];
    pipe(p);
    if (fork() == 0) {
        read(p[0], buf, 1); 
        fprintf(1, "%d: received ping\n", getpid());
        write(p[1], "c", 1);
        exit(0);
    }
    else {
        write(p[1], "p", 1);
        read(p[0], buf, 1);
        fprintf(1, "%d: received pong\n", getpid());
    }
    exit(0);
}
