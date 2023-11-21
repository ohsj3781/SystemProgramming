#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#define MAXSTRINGLENGTH 200
#define PATHSIZE 128

pid_t swsh_pid;
pid_t child_pid = -1;

const int cmd_size = 15;
char *valid_cmd[] = {"ls", "man", "grep", "sort", "awk", "bc", "head", "tail", "cat", "mv", "rm", "cp", "cd", "pwd", "exit"};

void tstp_handler(int sig)
{
    // if (child_pid == -1)
    // {
    //     return;
    // }
    // if (swsh_pid != getpid())
    // {
    //     exit(SIGKILL);
    // }
    if (child_pid != -1 && child_pid != 0)
    {
        kill(child_pid, sig);
        if (swsh_pid != getpid())
        {
            exit(SIGKILL);
        }
    }
    if (child_pid > 0)
    {
        kill(child_pid, SIGKILL);
    }
}

void error_check(const int status)
{
    if (status == EACCES)
    {
        fprintf(stderr, "Permission denied\n");
    }
    else if (status == EISDIR)
    {
        fprintf(stderr, "IS a directory\n");
    }
    else if (status == ENOENT)
    {
        fprintf(stderr, "No such file or directory\n");
    }
    else if (status == ENOTDIR)
    {
        fprintf(stderr, "Not a directory\n");
    }
    else if (status == EPERM)
    {
        fprintf(stderr, "Permission denied\n");
    }
    else if (status != 0)
    {
        fprintf(stderr, "Error occurred: <%d>\n", status);
    }
    return;
}

void head(int K, char *file)
{
    int line_cnt = 0;
    int file_fd = open(file, O_RDONLY);

    char buf[1];
    while (read(file_fd, buf, 1) > 0 && line_cnt < K)
    {
        fprintf(stdout, "%c", buf[0]);
        if (buf[0] == '\n')
        {
            ++line_cnt;
        }
    }
    close(file_fd);
    return;
}

void tail(int K, char *file)
{
    int line_cnt = 1;
    int file_fd = open(file, O_RDONLY);
    char buf[1];
    lseek(file_fd, 0, SEEK_END);
    while (1)
    {
        read(file_fd, buf, 1);
        if (buf[0] == '\n')
        {
            if (++line_cnt > K)
            {
                break;
            }
        }
        lseek(file_fd, -2 * sizeof(char), SEEK_CUR);
    }

    while (read(file_fd, buf, 1) > 0)
    {
        fprintf(stdout, "%c", buf[0]);
        if (buf[0] == '\n')
        {
            ++line_cnt;
        }
    }
    close(file_fd);
    return;
}

void cat(char *file)
{
    int file_fd = open(file, O_RDONLY);
    char buf[MAXSTRINGLENGTH];
    int nbytes;
    while ((nbytes = read(file_fd, buf, MAXSTRINGLENGTH)) > 0)
    {
        buf[nbytes] = '\0';
        fprintf(stdout, "%s", buf);
    }
    return;
}

void cp(char *file1, char *file2)
{
    int src_fd = open(file1, O_RDONLY);
    int dest_fd = open(file2, O_WRONLY | O_CREAT, 0666);
    char buf[MAXSTRINGLENGTH];
    int nbytes;
    while ((nbytes = read(src_fd, buf, MAXSTRINGLENGTH)) > 0)
    {
        write(dest_fd, buf, nbytes);
    }
    close(src_fd);
    close(dest_fd);
    return;
}

void rm(char *file)
{
    const int status = unlink(file);
    error_check(status);
    return;
}

void cd(char *path)
{
    char now_dir[MAXSTRINGLENGTH];
    getcwd(now_dir, MAXSTRINGLENGTH);
    char mv_dir[MAXSTRINGLENGTH];
    if (strcmp(path, "../") == 0)
    {
        char *s_ptr = strrchr(now_dir, '/');
        while (*s_ptr != '\0')
        {
            *s_ptr++ = 0;
        }

        strcpy(mv_dir, now_dir);
    }
    else
    {
        sprintf(mv_dir, "%s/%s", now_dir, path);
    }
    const int status = chdir(path);
    error_check(status);
    return;
}

void mv(char *file1, char *file2)
{
    const int status = rename(file1, file2);
    error_check(status);
    return;
}

void pwd()
{
    char dir[MAXSTRINGLENGTH];
    getcwd(dir, MAXSTRINGLENGTH);
    fprintf(stdout, "%s\n", dir);
    return;
}

void my_exit(const int i)
{
    fprintf(stderr, "exit\n");
    exit(i);
}

void makepath(char *path, char *command)
{
    if (strncmp(command, "./", 2) == 0)
    {
        char dir[MAXSTRINGLENGTH];
        getcwd(dir, MAXSTRINGLENGTH);
        sprintf(path, "%s/%s", dir, command);
    }
    else
    {
        sprintf(path, "/bin/%s", command);
    }
    return;
}

const int identify_cmd(char *command)
{
    for (int i = 0; i < cmd_size; ++i)
    {
        if (strcmp(command, valid_cmd[i]) == 0)
        {
            if (i == 12 || i == 14)
            {
                return 3;
            }
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

    // tokenize commands
    char *commands_arr[MAXSTRINGLENGTH];
    const int commands_arr_len = commands_tok(commands, commands_arr);

    // identify command
    char *command = commands_arr[0];
    int cmd_type;

    if ((cmd_type = identify_cmd(command)) < 0)
    {
        fprintf(stderr, "Command not found\n");
        return -1;
    }
    char path[PATHSIZE];
    makepath(path, command);

    // no need to make fork
    if (cmd_type == 3)
    {
        // execute command

        if (strcmp(command, "exit") == 0)
        {
            const int idx = commands_arr[1] == NULL ? 0 : atoi(commands_arr[1]);
            my_exit(idx);
        }
        if (strcmp(command, "cd") == 0)
        {
            cd(commands_arr[1]);
        }
        return 0;
    }

    int status;

    // child process
    if ((child_pid = fork()) == 0)
    {
        setpgid(getpid(), getpid());
        // redirection
        int input_redirection = -1;
        int ouput_redirection = -1;
        for (int i = 0; i < commands_arr_len; ++i)
        {
            // input redirection
            if (strcmp(commands_arr[i], "<") == 0)
            {
                commands_arr[i] = NULL;
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
                commands_arr[i] = NULL;
                ouput_redirection = open(commands_arr[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                dup2(ouput_redirection, STDOUT_FILENO);
            }
            // output redirection appending
            else if (strcmp(commands_arr[i], ">>") == 0)
            {
                commands_arr[i] = NULL;
                ouput_redirection = open(commands_arr[i + 1], O_WRONLY | O_APPEND, 0666);
                dup2(ouput_redirection, STDOUT_FILENO);
                break;
            }
        }

        // execute command

        if (strcmp(command, "head") == 0)
        {
            int K = 10;
            char *file = commands_arr[1];
            if (strcmp(commands_arr[1], "-n") == 0)
            {
                K = atoi(commands_arr[2]);
                file = commands_arr[3];
            }
            head(K, file);
        }
        else if (strcmp(command, "tail") == 0)
        {
            int K = 10;
            char *file = commands_arr[1];
            if (strcmp(commands_arr[1], "-n") == 0)
            {
                K = atoi(commands_arr[2]);
                file = commands_arr[3];
            }
            tail(K, file);
        }
        else if (strcmp(command, "cat") == 0)
        {
            cat(commands_arr[1]);
        }
        else if (strcmp(command, "cp") == 0)
        {
            cp(commands_arr[1], commands_arr[2]);
        }
        else if (strcmp(command, "rm") == 0)
        {
            rm(commands_arr[1]);
        }
        else if (strcmp(command, "mv") == 0)
        {
            mv(commands_arr[1], commands_arr[2]);
        }
        else if (strcmp(command, "pwd") == 0)
        {
            pwd();
        }
        else
        {
            execv(path, commands_arr);
        }
        exit(0);
    }
    // parent process
    else
    {
        waitpid(child_pid, &status, WNOHANG | WUNTRACED);
        if (WIFSTOPPED(status))
        {
            child_pid = -1;
            fprintf(stderr, "child process stop!\n");
            return 0;
        }
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
        if ((child_pid = fork()) == 0)
        {
            setpgid(getpid(), getpid());
            // close fd read and connect fd write to stdout
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);

            execute_commands(left_pipeline);
            exit(0);
        }
        else
        {
            waitpid(pid, &status, WNOHANG | WUNTRACED);
            if (WIFSTOPPED(status))
            {
                fprintf(stderr, "child process stop!\n");
                return 0;
            }

            // close fd write and connect fd read to stdin
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            execute_input(right_pipeline);
        }
    }
    // pipeline doesn't exist
    else
    {
        execute_commands(input);
    }

    return 0;
}

int main()
{
    int status;
    char input[MAXSTRINGLENGTH];

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, tstp_handler);
    swsh_pid = getpid();
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
        execute_input(input);
    }
    return 0;
}