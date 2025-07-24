#include <unistd.h>
#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    mount("proc", "/proc", "proc", 0, NULL);
    mount("sysfs", "/sys", "sysfs", 0, NULL);
    execl("/bin/sh", "sh", NULL);
    perror("execl");
    return 1;
}
