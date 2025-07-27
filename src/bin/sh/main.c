/* help */
int sh_help(char **args) {
    (void)args;
    printf("Geo Shell v1 for Atlas\n");
    // ...
    return SH_SUCCESS;
}

/* exit */
int sh_exit(char **args) {
    (void)args;
    return SH_EXIT;
}

/* launch */
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

/* main */
int main(int argc, char **argv, char **envp) {
    (void)argc; (void)argv;
    // ...
}
