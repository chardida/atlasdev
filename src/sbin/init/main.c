#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

char* default_env[] = {
    "PATH=/bin:/usr/bin",
    "HOME=/",
    "TERM=linux",
    "CWD=/",
    NULL
};

int main() {
    // Mount essential virtual filesystems
    mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);
    mknod("/dev/console", S_IFCHR | 0600, makedev(5, 1));
    mount("proc", "/proc", "proc", 0, NULL);
    mount("sysfs", "/sys", "sysfs", 0, NULL);

    // Bind stdio to /dev/console
    int fd = open("/dev/console", O_RDWR);
    if (fd >= 0) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }

    puts("Launching shell...\n");

    // Launch shell
    execle("/bin/sh", "sh", NULL, default_env);
    perror("execle");
    return 1;
}
