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
            return 2;
        }
    }
    for (int i = 0; i < cmd_type3_num; ++i)
    {
        if (strcmp(command, cmd_type3[i]) == 0)
        {
            return 3;
        }
    }
    for (int i = 0; i < cmd_type4_num; ++i)
    {
        if (strcmp(command, cmd_type4[i]) == 0)
        {
            return 4;
        }
    }
    return -1;
}

const int execute_commands(char *commands, int *fd)
{

    char *is_input_redirection_exist = strstr(commands, "<");
    char *is_output_redirection_exist = strstr(commands, ">");
    char *is_output_redirection_appending_exist = strstr(commands, ">>");

    // tokenize command and redirection
    if (is_input_redirection_exist != NULL)
    {
        *is_input_redirection_exist = '\0';
        ++is_input_redirection_exist;
        while(*is_input_redirection_exist==' '){
            ++is_input_redirection_exist;
        }
    }
    if (is_output_redirection_exist != NULL)
    {
        *is_output_redirection_exist = '\0';
        ++is_output_redirection_exist;
        while(*is_output_redirection_exist==' '){
            ++is_output_redirection_exist;
        }
    }
    if (is_output_redirection_appending_exist != NULL)
    {
        while (*is_output_redirection_appending_exist == '>')
        {
            *is_output_redirection_appending_exist = '\0';
            ++is_output_redirection_appending_exist;
            while(*is_output_redirection_appending_exist==' '){
            ++is_output_redirection_appending_exist;
        }
        }
    }

    int input_fd = -1, output_fd = -1;

    // redirection
    if (is_output_redirection_appending_exist != NULL)
    {
        // output file(appending) doesn't exist
        if ((output_fd = open(is_output_redirection_appending_exist, O_WRONLY | O_APPEND)) < 0)
        {
            fprintf(stderr, "No such file\n");
            exit(1);
        }
        dup2(output_fd, STDOUT_FILENO);
    }
    else
    {
        printf("input file is %s\n",is_input_redirection_exist);
        // input file doesn't exist
        if (is_input_redirection_exist != NULL)
        {
            if ((input_fd = open(is_input_redirection_exist, O_RDONLY)) < 0)
            {
                fprintf(stderr, "No such file\n");
                exit(1);
            }
            dup2(input_fd, STDIN_FILENO);
        }
        if (is_output_redirection_exist != NULL)
        {
            output_fd = open(is_output_redirection_exist, O_WRONLY | O_CREAT);
            dup2(output_fd, STDOUT_FILENO);
        }
    }

    // execute commands

    char **args = (char **)calloc(strlen(commands), sizeof(char *));
    int args_len = 0;
    // tokenize commands
    char *ptr = strtok(commands, " ");
    while (ptr != NULL)
    {

        args[args_len] = (char *)calloc(strlen(ptr), sizeof(char));
        strcpy(args[args_len++], ptr);
        ptr = strtok(NULL, " ");
    }
    args[args_len] = NULL;

    char path[100];
    sprintf(path, "/bin/%s", args[0]);

     const int cmd_type = identify_cmd_type(args[0]);

    if (cmd_type < 0)
    {
        fprintf(stderr, "Command not found\n");
        exit(1);
    }

    execv(path, args);
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
    pid_t pid;
    // // pipeline exist
    // if (is_pipeline_exist != NULL)
    // {
    //     *is_pipeline_exist = '\0';
    //     int fd[2];
    //     if (pipe(fd) < 0)
    //     {
    //         printf("make pipeline failed\n");
    //         exit(1);
    //     }
    //     char *left_pipeline = input;
    //     char *right_pipeline = is_pipeline_exist + 1;
    //     // tokenizet str
    //     execute_commands(left_pipeline, fd);
    //     // handel remain input
    //     // execute_(input)
    //     execute_input(right_pipeline, fd);
    // }
    // pipeline doesn't exist
    if((pid=fork())==0)
    {
        execute_commands(input, NULL);

    }
    wait(NULL);
    
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