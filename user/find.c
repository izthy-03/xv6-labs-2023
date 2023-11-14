#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char *
fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    buf[strlen(p)] = 0;
    return buf;
}

void find(char *path, const char *filename)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    // printf("Now looking up %s in %s, fmtname=%s\n", filename, path, fmtname(path));
    // File or directory found
    if (!strcmp(fmtname(path), filename)) {
        // fprintf(1, "%s\n", path);
        write(1, path, strlen(path));
        write(1, "\n", 1);
    }

    switch (st.type) {
        case T_DEVICE:
        case T_FILE:
            break;
        // Recurse into subdirectory
        case T_DIR:
            // printf("%s is a subdirectory, recursing...\n", path);
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                fprintf(2, "find: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0) 
                    continue;
                // Skip . and ..
                if (!strcmp(".", de.name) || !strcmp("..", de.name))
                    continue;
                // Concat de.name to the path
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = '\0';
                find(buf, filename);
            }
            break;
    }

    close(fd);
}
int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(2, "Usage: find [path] [filename]\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
