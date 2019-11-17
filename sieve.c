#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>

#include "sieve.h"

#define SIEVEDEBUG

#include "sievedebug.h"

int filter(int m, int r, int w)
{
    int buffer;
    while(read(r, &buffer, sizeof(int)) > 0)
    {
        printf("Filtering %d\n", buffer);
    
        if(buffer % m != 0)
        {
            if(write(w, &buffer, sizeof(int)) == -1)
            {
                return EXIT_FAILURE;
            }
            
            LOG("Filtered %d\n", buffer);
        }
    }

    LOG("Finished filtering\n");
    
    return EXIT_SUCCESS;
}

pid_t makeStage(int m, int r, int** fds)
{
    int f = fork();

    if(f < 0)
    {
        perror("Fork - makeStage");
        exit(ERROR_STAGING);
    }
    else if(f == 0)
    {
        close((*fds)[PIPE_READ]);

        if(filter(m, r, (*fds)[PIPE_WRITE]) == EXIT_FAILURE)
        {
            LOG("%s\n", strerror(errno));

            exit(ERROR_PROG_FAILURE);
        }

        close((*fds)[PIPE_WRITE]);

        return 0;
    }
    else
    {
        return f;
    }
}

void printFromPipe(int r)
{
    printf("Printing from pipe!\n");

    int buffer;
    int bytesRead;
    while((bytesRead = read(r, &buffer, sizeof(int))) > 0)
    {
        int length = snprintf(NULL, 0, "%d", buffer);
        int size = sizeof(char) * (length + 1);

        char* str = malloc(size);

        snprintf(str, size, "%d", buffer);
                
        write(STDOUT_FILENO, str, sizeof(str));
        write(STDOUT_FILENO, "\n", 1);

        free(str);
    }
}

/*
 * Main
 */
int main(int argc, char** argv)
{
    // Determines if program arguments are valid
    if(argc != 2 || strtol(argv[1], NULL, 10) < 0)
    {
        fprintf(stderr, "Program takes in 1 positive integer as parameter!\n");
        exit(ERROR_INVALID_ARGS);
    }

    // Turns of sigpipe
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        perror("Signal");
        exit(ERROR_PROG_FAILURE);
    }

    // Square root of n will be maximum number in pipes
    int n       = strtol(argv[1], NULL, 10);
    double sqrn = sqrt(n);

    // Pipe setup
    int fd[2];
    MAKE_PIPE(fd);

    int f = fork();
    
    if(f < 0)
    {
        perror("Fork");
        exit(ERROR_PROG_FAILURE);
    }
    else if(f == 0) // Child - Will read data from pipe and create stages based on data
    {
        // Closing write end of pipe
        close(fd[PIPE_WRITE]);

        // Filter will always be the first value in the pipe
        int currentFilter;
        read(fd[PIPE_READ], &currentFilter, sizeof(int));
        
        int** dataPipes = malloc(sizeof(int*) * (int)(sqrn - 1));
        dataPipes[0] = malloc(sizeof(int) * 2);
        MAKE_PIPE(dataPipes[0]);

        dup2(fd[PIPE_READ], dataPipes[0][PIPE_READ]);

        int numKnownFilters = 1;

        while(currentFilter < sqrn)
        {
            dataPipes[numKnownFilters] = malloc(sizeof(int) * 2);
            MAKE_PIPE(dataPipes[numKnownFilters]);

            int stage = makeStage(currentFilter, dataPipes[numKnownFilters--][PIPE_READ], &dataPipes[numKnownFilters]);
        
            // Child return of makeStage()
            if(stage == 0)
            {
                printf("Exiting from makeStage() - CHILD\n");
                exit(1);
            }
            
            int status;
            waitpid(f, &status, 0);

            break;
        }

        close(fd[PIPE_READ]);

        

        exit(EXIT_SUCCESS);
    }
    else // Parent - Will write data to pipe
    {
        // Closing read end of pipe
        close(fd[PIPE_READ]);
        
        // Array of storing integers from 2 to n
        int* numbers = malloc((n - 1) * sizeof(int));

        // Write numbers 2...n to pipe
        for(int i = 2; i < n + 1; ++i)
        {
            numbers[i - 2] = i;
            write(fd[PIPE_WRITE], &numbers[i - 2], sizeof(int));

            LOG("Wrote %d to pipe\n", numbers[i - 2]);
        }

        close(fd[PIPE_WRITE]);

        int status;
        f = waitpid(f, &status, 0);

        free(numbers);

        if(WIFEXITED(status))
        {
            int exitStatus = WEXITSTATUS(status);

            LOG("First child pid: %d\n", f);
            LOG("Exit status of parent: %d\n", exitStatus);
            
            if(exitStatus == ERROR_STAGING)
            {
                exit(ERROR_PROG_FAILURE);
            }
        }
    }
    
    return EXIT_SUCCESS;
}
