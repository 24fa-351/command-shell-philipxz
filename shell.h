#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef SHELL_H
#define SHELL_H

#define MAX_INPUT 1024
#define MAX_ARGS 100

void execute_command(char **args);

void change_directory(char *path);

void set_env_variable(char *name, char *value);

void unset_env_variable(char *name);

void parse_input(char *input, char **args);

void handle_echo(char **args);

void handle_redirection(char **args, int *input_fd, int *output_fd);

void handle_piping(char **cmd1, char **cmd2);

void execute_single_command(char **args, int input_fd, int output_fd);

int parse_background(char **args);

#endif