#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

/*
 * Write a program that forks one child for each command line argument.
 * Children will compute the length of the command line argument and 
 *  exits with that integer as the return value.
 * Parent will sum exit codes and prints the total length of all
 *  command line arguments
 */

int main(int argc, char** argv)
{
    for(int i = 1; i < argc; ++i)
    {
        int f = fork();

        if (f < 0)
        {
            perror("Fork");
            exit(EXIT_FAILURE);
        }
        else if(f == 0)
        {
            int length = (int)strlen(argv[i]);
            exit(length);
        }
    }

    int sum = 0;
    int status;

    for(int i = 1; i < argc; ++i)
    {
        wait(&status);
        
        if(WIFEXITED(status))
        {
            sum += WEXITSTATUS(status);
        }
    }

    printf("The length of all arguments is %d.\n", sum);
    return 0;
}