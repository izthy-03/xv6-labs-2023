#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int sift(int p[]) {
    int buk[40], cnt = 0, endzero = 0;
    while (read(p[0], (void *)&buk[cnt], 4)) {
        if (buk[cnt] == 0)
            break;
        cnt++;
    }
    if (cnt == 0) 
        return 1;
        
    fprintf(1, "prime %d\n", buk[0]);
    for (int i = 1; i < cnt; i++) {
        if (buk[i] % buk[0] != 0) {
            write(p[1], &buk[i], 4);
        }
    }
    write(p[1], &endzero, 4);
    return 0;
}

int main(int argc, char *argv[])
{
    int p[2], endzero = 0;
    pipe(p);
    for (int i = 2; i <= 35; i++)
        write(p[1], &i, 4);
    write(p[1], &endzero, 4);
    while (1)
    {
        if (fork() == 0) {
            int stat = sift(p);
            exit(stat);
        }
        int stat;
        wait(&stat);
        if (stat) 
            break;
    }
    exit(0);
}
