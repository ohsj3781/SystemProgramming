#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#define MAXSTRINGLENGTH 200

char *cmd_type1[6] = {"ls", "man", "grep", "sort", "awk", "bc"};
const int cmd_type1_num = 6;

char *cmd_type2[3] = {"head", "tail", "cat"};
const int cmd_type2_num = 3;

char *cmd_type3[4] = {"mv", "rm", "cp", "cd"};
const int cmd_type3_num = 4;

char *cmd_type4[2] = {"pwd", "exit"};
const int cmd_type4_num = 2;

const int identify_cmd_type(char *command)
{
    for (int i = 0; i < cmd_type1_num; ++i)
    {
        if (strcmp(command, cmd_type1[i]) == 0)
        {
            return 1;
        }
    }
    for (int i = 0; i < cmd_type2_num; ++i)
    {
        if (strcmp(command, cmd_type2[i]) == 0)
        {
            return 1;
        }
    }
    for (int i = 0; i < cmd_type3_num; ++i)
    {
        if (strcmp(command, cmd_type3[i]) == 0)
        {
            return 1;
        }
    }
    for (int i = 0; i < cmd_type4_num; ++i)
    {
        if (strcmp(command, cmd_type4[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

const int execute_commands(char *commands, int *fd)
{
    char *is_input_redirection_exist = strstr(commands, "<");
    char *is_output_redirection_exist = strstr(commands, ">");
    char *is_output_redirection_appending_exist = strstr(commands, ">>");

    //redirection 
    if (is_output_redirection_appending_exist != NULL)
    {
    }
    else
    {
        if (is_input_redirection_exist != NULL)
        {
        }
        if (is_output_redirection_exist != NULL)
        {
        }
    }

    //execute commands

    return 0;
}

const int execute_input(char *input, int *fd)
{
    if (fd != NULL)
    {
        close(fd[0]);
    }
    const int input_length = strlen(input);

    char *is_pipeline_exist = strchr(input, '|');

    // pipeline exist
    if (is_pipeline_exist != NULL)
    {
        *is_pipeline_exist = '\0';
        int fd[2];
        if (pipe(fd) < 0)
        {
            printf("make pipeline failed\n");
            exit(1);
        }
        char *left_pipeline = input;
        char *right_pipeline = is_pipeline_exist + 1;
        // tokenizet str
        execute_commands(left_pipeline, fd);
        // handel remain input
        // execute_(input)
        execute_input(right_pipeline, fd);
    }
    // pipeline doesn't exist
    else
    {
        execute_commands(input, NULL);
    }

    return 0;
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

        execute_input(input, NULL);
    }
    return 0;
}