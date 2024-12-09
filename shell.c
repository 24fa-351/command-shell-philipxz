#include "shell.h"

void parse_input(char *input, char **args) {
    for (int ix = 0; ix < MAX_ARGS; ix++) {
        args[ix] = strsep(&input, " ");
        if (args[ix] == NULL) {
            break;
        }
        if (strlen(args[ix]) == 0) {
            ix--;
        }
    }
}

void change_directory(char *path) {
    if (path == NULL) {
        fprintf(stderr, "cd: missing argument\n");
    } else if (chdir(path) != 0) {
        perror("cd");
    }
}

void set_env_variable(char *name, char *value) {
    if (setenv(name, value, 1) != 0) {
        perror("setenv");
    }
}

void unset_env_variable(char *name) {
    if (unsetenv(name) != 0) {
        perror("unsetenv");
    }
}

void handle_echo(char **args) {
    for (int ix = 1; args[ix] != NULL; ix++) {
        if (args[ix][0] == '$') {
            char *env_var = getenv(args[ix] + 1);
            if (env_var != NULL) {
                printf("%s ", env_var);
            } else {
                printf("No value for %s ", args[ix]);
            }
        } else {
            printf("%s", args[ix]);
        }
    }
    printf("\n");
}

void handle_redirection(char **args, int *input_fd, int *output_fd) {
    for (int ix = 0; args[ix] != NULL; ix++) {
        if (strcmp(args[ix], "<") == 0) {
            *input_fd = open(args[ix + 1], O_RDONLY);
            if (*input_fd < 0) {
                perror("Input redirection failed");
            }
            args[ix] = NULL;
        } else if (strcmp(args[ix], ">") == 0) {
            *output_fd = open(args[ix + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (*output_fd < 0) {
                perror("Output redirection failed");
            }
            args[ix] = NULL;
        }
    }
}

// Handle piping between two commands
void handle_piping(char **cmd1, char **cmd2) {
    int pipe_fd[2];
    pipe(pipe_fd);

    if (fork() == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        execvp(cmd1[0], cmd1);
        perror("execvp");
        exit(1);
    }

    if (fork() == 0) {
        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        execvp(cmd2[0], cmd2);
        perror("execvp");
        exit(1);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);
    wait(NULL);
    wait(NULL);
}

void execute_single_command(char **args, int input_fd, int output_fd) {
    if (fork() == 0) {
        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }
    wait(NULL);
}

int parse_background(char **args) {
    for (int ix = 0; args[ix] != NULL; ix++) {
        if (strcmp(args[ix], "&") == 0) {
            args[ix] = NULL;
            return 1;
        }
    }
    return 0;
}

void execute_command(char **args) {
    if (strcmp(args[0], "echo") == 0) {
        handle_echo(args);
        return;
    }
    
    int input_fd = -1, output_fd = -1;
    int background = parse_background(args);

    char *cmd1[MAX_ARGS], *cmd2[MAX_ARGS];
    int piping = 0;

    for (int ix = 0; args[ix] != NULL; ix++) {
        if (strcmp(args[ix], "|") == 0) {
            piping = 1;
            args[ix] = NULL;

            for (int jx = 0; jx < ix; jx++) {
                cmd1[jx] = args[jx];
            }
            cmd1[ix] = NULL;

            for (int jx = ix + 1, kx = 0; args[jx] != NULL; jx++, kx++) {
                cmd2[kx] = args[jx];
            }
            break;
        }
    }

    if (piping) {
        handle_piping(cmd1, cmd2);
    } else {
        handle_redirection(args, &input_fd, &output_fd);
        execute_single_command(args, input_fd, output_fd);

        if (input_fd != -1) close(input_fd);
        if (output_fd != -1) close(output_fd);
    }

    if (!background) {
        wait(NULL);
    } else {
        printf("Command running in background\n");
    }
}
