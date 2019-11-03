#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

/*
 * Write a program that forks one child for each command line argument.
 * Children will compute the length of the command line argument and 
 *  write it to a pipe
 * Parent will sum data from the pipe and prints the total length
 *  of all the command line arguments
 */

int main(int argc, char** argv)
{
    int sum = 0;

    for(int i = 1; i < argc; ++i)
    {
        int fd[2];
        pipe(fd);

        int f = fork();

        if(f < 0)
        {
            perror("Fork");
            exit(EXIT_FAILURE);
        }
        else if(f == 0)
        {
            close(fd[0]);

            int length = (int)strlen(argv[i]);

            write(fd[1], &length, sizeof(int));

            close(fd[1]);

            exit(EXIT_SUCCESS);
        }
        else
        {
            close(fd[1]);

            int buffer;
            read(fd[0], &buffer, sizeof(int));

            sum += buffer;

            close(fd[0]);
        }
    }

    printf("The length of all arguments is %d.\n", sum);

    return 0;
}