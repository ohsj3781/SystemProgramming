#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#define MAXSTRINGLENGTH 200
#define PATHSIZE 128

const int cmd_size = 15;
char *valid_cmd[] = {"ls", "man", "grep", "sort", "awk", "bc", "head", "tail", "cat", "mv", "rm", "cp", "cd", "pwd", "exit"};

void head()
{
}

void tail() {}
void cat() {}
void cp() {}
void rm() {}
void cd() {}
void mv() {}
void pwd()
{
    char dir[MAXSTRINGLENGTH];
    getcwd(dir, MAXSTRINGLENGTH);
    fprintf(stdout, "%s\n", dir);
    return;
}
// void exit(){}

void makepath(char *path, char *command)
{
}

const int identify_cmd(char *command)
{
    for (int i = 0; i < cmd_size; ++i)
    {
        if (strcmp(command, valid_cmd[i]) == 0)
        {
            return 1;
        }
    }
    if (strstr(command, "./") != NULL)
    {
        return 2;
    }

    return -1;
}

const int commands_tok(char *commands, char *commands_arr[MAXSTRINGLENGTH])
{
    int len = 0;
    char *ptr = strtok(commands, " ");
    while (ptr != NULL)
    {
        commands_arr[len++] = ptr;
        ptr = strtok(NULL, " ");
    }
    commands_arr[len] = NULL;
    return len;
}

const int execute_commands(char *commands)
{
    pid_t pid;
    int status;
    // child process
    if ((pid = fork()) == 0)
    {
        // tokenize commands
        char *commands_arr[MAXSTRINGLENGTH];
        const int commands_arr_len = commands_tok(commands, commands_arr);

        // redirection
        int input_redirection = -1;
        int ouput_redirection = -1;
        for (int i = 0; i < commands_arr_len; ++i)
        {
            // input redirection
            if (strcmp(commands_arr[i], "<") == 0)
            {
                // input file doesn't exist
                if (access(commands_arr[i + 1], F_OK) != 0)
                {
                    fprintf(stderr, "No such file\n");
                    return -1;
                }
                if (access(commands_arr[i + 1], R_OK) != 0)
                {
                    fprintf(stderr, "%s doesn't have read permission\n", commands_arr[i + 1]);
                    return -1;
                }
                input_redirection = open(commands_arr[i + 1], O_RDONLY);
                dup2(input_redirection, STDIN_FILENO);
            }
            // output redirection
            else if (strcmp(commands_arr[i], ">") == 0)
            {
                ouput_redirection = open(commands_arr[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                dup2(ouput_redirection, STDOUT_FILENO);
            }
            // output redirection appending
            else if (strcmp(commands_arr[i], ">>") == 0)
            {
                ouput_redirection = open(commands_arr[i + 1], O_WRONLY | O_APPEND, 0666);
                dup2(ouput_redirection, STDOUT_FILENO);
                break;
            }
        }

        // identify command
        char *command = commands_arr[0];
        int cmd_type;

        if ((cmd_type = identify_cmd(command)) < 0)
        {
            fprintf(stderr, "Command not found\n");
            return -1;
        }

        // execute command
        char path[PATHSIZE];
        makepath(path, command);

        if (strcmp(command, "head") == 0)
        {
        }
        else if (strcmp(command, "tail") == 0)
        {
        }
        else if (strcmp(command, "cat") == 0)
        {
        }
        else if (strcmp(command, "cp") == 0)
        {
        }
        else if (strcmp(command, "rm") == 0)
        {
        }
        else if (strcmp(command, "cd") == 0)
        {
        }
        else if (strcmp(command, "mv") == 0)
        {
        }
        else if (strcmp(command, "pwd") == 0)
        {
            pwd();
        }
        else if (strcmp(command, "exit") == 0)
        {
        }
        else
        {
            execv(path, commands_arr);
        }
    }
    // parent process
    else
    {
        waitpid(pid, &status, 0);
    }
    return 0;
}

const int execute_input(char *input)
{
    const int input_length = strlen(input);

    char *pipeline = strchr(input, '|');
    // pipeline exist
    if (pipeline != NULL)
    {
        *pipeline++ = '\0';
        while (*pipeline == ' ')
        {
            ++pipeline;
        }

        int fd[2];
        pipe(fd);

        char *left_pipeline = input;
        char *right_pipeline = pipeline;

        pid_t pid;
        int status;
        // child process
        if ((pid = fork()) == 0)
        {
            // close fd read and connect fd write to stdout
            close(fd[0]);
            dup2(STDOUT_FILENO, fd[1]);

            execute_commands(left_pipeline);
        }
        else
        {
            waitpid(pid, &status, 0);
            // close fd write and connect fd read to stdin
            close(fd[1]);
            dup2(STDIN_FILENO, fd[0]);
            execute_input(right_pipeline);
        }
    }
    // pipeline doesn't exist
    else
    {
        execute_commands(input);
    }

    exit(0);
}

int main()
{
    pid_t pid;
    int status;
    char input[MAXSTRINGLENGTH];

    while (1)
    {
        // get user_input
        fgets(input, MAXSTRINGLENGTH, stdin);
        input[strlen(input) - 1] = '\0';

        // if input == quit finish swsh
        if (strcmp(input, "quit") == 0)
        {
            break;
        }
        // child process
        if ((pid = fork()) == 0)
        {
            execute_input(input);
        }
        // parent process
        else
        {
            waitpid(pid, &status, 0);
        }
    }
    return 0;
}