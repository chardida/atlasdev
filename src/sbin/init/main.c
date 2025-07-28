#include <unistd.h>
#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int fd = open("/dev/console", O_RDWR);
if (fd >= 0) {
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
}

char* default_env[] = {
    "PATH=/bin:/usr/bin",
    "HOME=/",
    "TERM=linux",
    "CWD=/",
    NULL
};

int main() {
    mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);
    mknod("/dev/console", S_IFCHR | 0600, makedev(5, 1));
    mount("proc", "/proc", "proc", 0, NULL);
    mount("sysfs", "/sys", "sysfs", 0, NULL);
    puts("Launching shell...\n");
    execle("/bin/sh", "sh", NULL, default_env);
    perror("execl");
    return 1;
}
