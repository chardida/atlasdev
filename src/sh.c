#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int sh_cd(char** args);
int sh_help(char** args);
int sh_exit(char** args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char**) = {
    &sh_cd,
    &sh_help,
    &sh_exit
};

int num_builtins(void) {
    return sizeof(builtin_str) / sizeof(char*);
}

int sh_cd(char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, "sh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("sh");
        }
    }
    return 1;
}

int sh_help(char** args) {
    printf("A simple shell\n"
           "The following are built in:\n");

    for (int i = 0; i < num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    return 1;
}

int sh_exit(char** args) {
    return 0;
}

int launch(char** args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("sh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("sh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int execute(char** args) {
    if (args[0] == NULL) {
        return 1;
    }

    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return launch(args);
}

#define ALLOC_ERROR(...) if(!__VA_ARGS__) { fprintf(stderr, "sh: allocation error"); exit(EXIT_FAILURE); }

#define TOKEN_BUFFER_SIZE 64
#define TOKEN_DELIM " \t\r\n\a"
char** splitLine(char* line) {
    int bufferSize = TOKEN_BUFFER_SIZE, position = 0;
    char** tokens = malloc(sizeof(char*) * bufferSize);
    char* token;

    ALLOC_ERROR(tokens);

    token = strtok(line, TOKEN_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufferSize) {
            bufferSize += TOKEN_BUFFER_SIZE;
            tokens = realloc(tokens, bufferSize);
            ALLOC_ERROR(tokens);
        }

    token = strtok(NULL, TOKEN_DELIM);
    }

    tokens[position] = NULL;
    return tokens;

}

#define BUFFERSIZE 1024
char* readLine(void) {
    int bufferSize = BUFFERSIZE;
    int position = 0;
    char* buffer = malloc(sizeof(char) * bufferSize);
    int c;

    ALLOC_ERROR(buffer);

    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        if (position >= bufferSize) {
            buffer += BUFFERSIZE;
            buffer = realloc(buffer, bufferSize);
            ALLOC_ERROR(buffer);
        }
    }
}

void loop(void) {
    char* line;
    char** args;
    int status;

    do {
        printf("$ ");
        line = readLine();
        args = splitLine(line);
        status = execute(args);
        
        free(line);
        free(args);
    } while (status);
}

int main(void) {
    loop();

    return EXIT_SUCCESS;
}