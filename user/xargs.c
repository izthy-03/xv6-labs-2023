#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

static char *buffer(int fd) {
    int n, size = 512, used = 0;
    char *buf = (char *)malloc(512);
    char *p = buf;
    while ((n = read(fd, p, size - used)) > 0) {
        // printf("read=%s\n", p);
        used += n;
        p += n;
        if (used == size) {
            char *newbuf = (char *)malloc(size + 512);
            memcpy(newbuf, buf, size);
            free(buf);
            buf = newbuf;
            p = buf + size;
            size += 512;
        }
    }
    buf[used] = '\0';
    return buf;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(2, "Usage: ([commands] | xargs [command] [arg1] [arg2] ...)\n");
    }
    char *buf = buffer(0);
    int len = strlen(buf);
    // printf("buf=%s\n", buf);
    int nargc = argc - 1;
    char *nargv[MAXARG];
    for(int i = 0; i < nargc; i++) {
        nargv[i] = (char *)malloc(strlen(argv[i + 1]));
        memcpy(nargv[i], argv[i + 1], strlen(argv[i + 1]));
    }

    int k = 0;
    char *p = buf;
    while (p < buf + len) {
        // Skip nonspace
        while(p[k] && p[k] != ' ' && p[k] != '\n') {
            k++;
        }
            
        // printf("\nleft=%s\n", p);
        nargv[nargc] = (char *)malloc(k + 2);
        memcpy(nargv[nargc], p, k);
        nargv[nargc][k] = '\0';
        // printf("nargv[%d]=%s\n", nargc, nargv[nargc]);
        nargc++;
        p += k;
        k = 0;
        // Fork a child to runcmd
        if (!(*p) || (*p) == '\n') {
            // printf("creating newline, nargc=%d, cmd:\n", nargc);
            // for(int i = 0; i < nargc; i++)
            //     printf("%s ", nargv[i]);
            // printf("\n");
            if (fork() == 0) {
                exec(nargv[0], nargv);
                fprintf(2, "xargs: exec %s error\n", nargv[0]);
                exit(1);
            }
            else {
                wait(0);
                // Clear nargv[] for next line
                for (int i = argc - 1; i < nargc; i++)
                    free(nargv[i]);
                nargc = argc - 1;
            }
        }
        // Skip blanks
        while(*p == ' ' || *p == '\n') 
            p++;
    }

    exit(0);
}
