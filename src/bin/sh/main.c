/*
 * Geo Shell beta for Atlas
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <locale.h>
#include <pwd.h>
#include <ctype.h>

#define TOKEN_BUFSIZE 64
#define TOKEN_DELIM    " \t\r\n\a"
#define SESSION_ID_LEN 8

/* exit codes */
#define SH_SUCCESS 1
#define SH_EXIT    0
#define SH_ERROR   1

/* forward declarations for builtins */
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int sh_ver(char **args);
int sh_ls(char **args);
int sh_cp(char **args);
int sh_mv(char **args);
int sh_rm(char **args);
int sh_mkdir(char **args);
int sh_rmdir(char **args);
int sh_echo(char **args);
int sh_export(char **args);

/* built-in commands */
char *builtin_str[] = {
    "cd", "help", "exit", "ver",
    "ls", "cp", "mv", "rm",
    "mkdir", "rmdir", "echo", "export"
};

int (*builtin_func[])(char **) = {
    &sh_cd,   &sh_help, &sh_exit, &sh_ver,
    &sh_ls,   &sh_cp,   &sh_mv,   &sh_rm,
    &sh_mkdir,&sh_rmdir,&sh_echo, &sh_export
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/* Utility: read a line from stdin */
char *read_line(void) {
    int bufsize = 1024, pos = 0;
    char *buffer = malloc(bufsize), c;
    if (!buffer) {
        fprintf(stderr, "geo: allocation error\n");
        exit(EXIT_FAILURE);
    }
    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[pos] = '\0';
            return buffer;
        } else {
            buffer[pos++] = c;
        }
        if (pos >= bufsize) {
            bufsize += 1024;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "geo: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/* Utility: split line into tokens */
char **split_line(char *line) {
    int bufsize = TOKEN_BUFSIZE, pos = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;
    if (!tokens) {
        fprintf(stderr, "geo: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, TOKEN_DELIM);
    while (token) {
        tokens[pos++] = token;
        if (pos >= bufsize) {
            bufsize += TOKEN_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "geo: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOKEN_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}

/* Launch external programs */
int launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("geo");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("geo");
    } else {
        waitpid(pid, &status, WUNTRACED);
    }

    return SH_SUCCESS;
}

/* Execute input: builtin or external */
int execute(char **args) {
    if (args[0] == NULL) {
        return SH_SUCCESS;
    }
    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return launch(args);
}

/* ============================
   Builtin implementations
   ============================ */

/* cd */
int sh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "geo: expected argument to \"cd\"\n");
    } else if (chdir(args[1]) != 0) {
        perror("geo");
    }
    return SH_SUCCESS;
}

/* help */
/* help */
int sh_help(char **args) {
    (void)args;
    printf("Geo Shell v1 for Atlas\n");
    printf("Built-in commands:\n");
    for (int i = 0; i < num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    printf("Use man pages for external commands.\n");
    return SH_SUCCESS;
}

/* exit */
int sh_exit(char **args) {
    (void)args;
    return SH_EXIT;
}

/* ver: show distro info */
int sh_ver(char **args) {
    printf("System:   AtlasOS\n");
    printf("Shell:    Geo Beta1\n");
    printf("Contribs: gingrspacecadet, chardida, Geo Team\n");
    printf("GitHub:   https://github.com/gingrspacecadet/Atlas\n");
    return SH_SUCCESS;
}

/* ls: list directory */
int sh_ls(char **args) {
    DIR *d;
    struct dirent *entry;
    char *path = args[1] ? args[1] : ".";
    d = opendir(path);
    if (!d) {
        perror("geo");
        return SH_ERROR;
    }
    while ((entry = readdir(d)) != NULL) {
        printf("%s  ", entry->d_name);
    }
    printf("\n");
    closedir(d);
    return SH_SUCCESS;
}

/* cp: copy a file */
int sh_cp(char **args) {
    if (!args[1] || !args[2]) {
        fprintf(stderr, "geo: cp src dst\n");
        return SH_ERROR;
    }
    int src = open(args[1], O_RDONLY);
    if (src < 0) { perror("geo"); return SH_ERROR; }
    int dst = open(args[2], O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if (dst < 0) { perror("geo"); close(src); return SH_ERROR; }
    char buf[4096];
    ssize_t n;
    while ((n = read(src, buf, sizeof(buf))) > 0) {
        if (write(dst, buf, n) != n) {
            perror("geo");
            close(src); close(dst);
            return SH_ERROR;
        }
    }
    close(src);
    close(dst);
    return SH_SUCCESS;
}

/* mv: rename a file */
int sh_mv(char **args) {
    if (!args[1] || !args[2]) {
        fprintf(stderr, "geo: mv src dst\n");
        return SH_ERROR;
    }
    if (rename(args[1], args[2]) != 0) {
        perror("geo");
        return SH_ERROR;
    }
    return SH_SUCCESS;
}

/* rm: remove a file */
int sh_rm(char **args) {
    if (!args[1]) {
        fprintf(stderr, "geo: rm file\n");
        return SH_ERROR;
    }
    if (unlink(args[1]) != 0) {
        perror("geo");
        return SH_ERROR;
    }
    return SH_SUCCESS;
}

/* mkdir */
int sh_mkdir(char **args) {
    if (!args[1]) {
        fprintf(stderr, "geo: mkdir dir\n");
        return SH_ERROR;
    }
    if (mkdir(args[1], 0755) != 0) {
        perror("geo");
        return SH_ERROR;
    }
    return SH_SUCCESS;
}

/* rmdir */
int sh_rmdir(char **args) {
    if (!args[1]) {
        fprintf(stderr, "geo: rmdir dir\n");
        return SH_ERROR;
    }
    if (rmdir(args[1]) != 0) {
        perror("geo");
        return SH_ERROR;
    }
    return SH_SUCCESS;
}

/* echo */
int sh_echo(char **args) {
    if (!args[1]) {
        printf("\n");
        return SH_SUCCESS;
    }
    for (int i = 1; args[i]; i++) {
        if (args[i][0] == '$') {
            char *val = getenv(args[i]+1);
            printf("%s ", val ? val : "");
        } else {
            printf("%s ", args[i]);
        }
    }
    printf("\n");
    return SH_SUCCESS;
}

/* export VAR=VAL */
int sh_export(char **args) {
    if (!args[1] || !strchr(args[1], '=')) {
        fprintf(stderr, "geo: export VAR=VALUE\n");
        return SH_ERROR;
    }
    char *eq = strchr(args[1], '=');
    *eq = '\0';
    if (setenv(args[1], eq+1, 1) != 0) {
        perror("geo");
        return SH_ERROR;
    }
    return SH_SUCCESS;
}

/* Generate an 8-char session ID (A–Z,0–9) */
void make_session_id(char *buf) {
    const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < SESSION_ID_LEN; i++) {
        buf[i] = chars[rand() % (strlen(chars))];
    }
    buf[SESSION_ID_LEN] = '\0';
}

int main(int argc, char **argv, char **envp) {
    (void)argc; (void)argv;
    setlocale(LC_ALL, "en_GB.UTF-8");
    extern char **environ;
    environ = envp;

    /* seed random and make session */
    srand(time(NULL) ^ getpid());
    char session[SESSION_ID_LEN+1];
    make_session_id(session);

    /* main loop */
    char *line;
    char **args;
    int status = SH_SUCCESS;
    struct passwd *pw = getpwuid(geteuid());
    const char *user = pw ? pw->pw_name : getenv("USER");

    do {
        printf("%s@session-%s> ", user, session);
        line = read_line();
        args = split_line(line);
        status = execute(args);
        free(line);
        free(args);
    } while (status != SH_EXIT);

    return EXIT_SUCCESS;
}
