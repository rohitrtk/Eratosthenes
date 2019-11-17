#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>

#include "sieve.h"

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
            
            printf("Filtered %d\n", buffer); 
        }
    }
    
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
        if(filter(m, r, (*fds)[PIPE_WRITE]) == EXIT_FAILURE)
        {
            printf("%s\n", strerror(errno));

            exit(ERROR_PROG_FAILURE);
        }

        //close((*fds)[PIPE_WRITE]);
        //printFromPipe((*fds)[PIPE_READ]);
        //close((*fds)[PIPE_READ]);

        return 0;
    }
    else
    {

        //close((*fds)[PIPE_READ]);
        //close((*fds)[PIPE_WRITE]);

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
        //close(fd[PIPE_WRITE]);

        // Filter will always be the first value in the pipe
        int currentFilter;
        read(fd[PIPE_READ], &currentFilter, sizeof(int));

        //int numKnownFilters = 0;
        
        while(currentFilter < sqrn)
        {
            int* dataPipe = malloc(2 * sizeof(int));
            MAKE_PIPE(dataPipe);

            dup2(fd[PIPE_WRITE], dataPipe[PIPE_WRITE]);

            int ms = makeStage(currentFilter, fd[PIPE_READ], &dataPipe);

            // Handle child return of makeStage
            if(ms == 0)
            {
                // Add 1 to numKnownFilters since the child of makeStage ran
                // which means that we filtered some value
                
                printf("Return of makeStage() child: %d\n", ms);

                free(dataPipe);

                exit(1); // Test exit code
            }1
            // Handle parent return of makeStage
            else
            {
                close(dataPipe[PIPE_WRITE]);

                int status;
                waitpid(ms, &status, 0);

                if(WIFEXITED(status))
                {
                    //printf("Prog exit status %d\n", WEXITSTATUS(status));
                    //numKnownFilters += WEXITSTATUS(status);
                }

                read(dataPipe[PIPE_READ], &currentFilter, sizeof(int));
                printf("Next filter: %d\n", currentFilter);

                close(dataPipe[PIPE_READ]);
                printf("Return of makeStage() parent: %d\n", ms);
            }

            free(dataPipe);
        }

        close(fd[PIPE_READ]);
        
        //printf("Number of filters: %d\n", numKnownFilters);

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

            printf("Wrote %d to pipe\n", numbers[i - 2]);
        }

        close(fd[PIPE_WRITE]);

        int status;
        f = waitpid(f, &status, 0);

        free(numbers);

        if(WIFEXITED(status))
        {
            int exitStatus = WEXITSTATUS(status);

            printf("First child pid: %d\n", f);
            printf("Exit status of parent: %d\n", exitStatus);
            
            if(exitStatus == ERROR_STAGING)
            {
                exit(ERROR_PROG_FAILURE);
            }
        }
    }
    
    return EXIT_SUCCESS;
}
