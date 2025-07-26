#include <unistd.h>
#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>

char* default_env[] = {
    "PATH=/bin:/usr/bin",
    "HOME=/",
    "TERM=linux",
    "CWD=/",
    NULL
};

int main() {
    mount("proc", "/proc", "proc", 0, NULL);
    mount("sysfs", "/sys", "sysfs", 0, NULL);

    execle("/bin/sh", "sh", NULL, default_env);
    perror("execl");
    return 1;
}
