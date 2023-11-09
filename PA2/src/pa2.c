#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#define MAXSTRINGLENGTH 200

const int execute_input(char *input)
{
    const int input_length = strlen(input);

    char* is_pipeline_exist=strchr(input,'|');

    //pipeline exist
    if(is_pipeline_exist!=NULL){

    }
    //pipeline doesn't exist
    else{

    }

    return 0;
}

int main()
{

    char input[MAXSTRINGLENGTH];

    while (1)
    {
        // get user_input
        fgets(input, MAXSTRINGLENGTH, STDIN_FILENO);
        input[strlen(input) - 1] = '\0';

        // if input == quit finish swsh
        if (strmp(input, "quit") == 0)
        {
            break;
        }

        execute(input);
    }
    return 0;
}