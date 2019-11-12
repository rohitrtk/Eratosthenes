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

        close((*fds)[PIPE_WRITE]);
        printFromPipe((*fds)[PIPE_READ]);
        close((*fds)[PIPE_READ]);

        return 0;
    }
    else
    {
        close((*fds)[PIPE_READ]);
        close((*fds)[PIPE_WRITE]);

        return waitpid(f, NULL, 0);
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
    int  n  = strtol(argv[1], NULL, 10);
    int  rn = (int)sqrt(n);

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

        // Filter will always be the first number in the pipe
        int filter;
        read(fd[PIPE_READ], &filter, sizeof(int));
        printf("Filter: %d\n", filter);

        while(filter < rn)
        {
            int* dataPipe = malloc(2 * sizeof(int));
            MAKE_PIPE(dataPipe);

            int ms = makeStage(filter, fd[PIPE_READ], &dataPipe);

            // Handle child return of makeStage
            if(ms == 0)
            {
                printf("Return of makeStage() child: %d\n", ms);

                exit(115); // Test exit code
            }
            // Handle parent return of makeStage
            else
            {
                close(dataPipe[PIPE_WRITE]);
                dup2(fd[PIPE_READ], dataPipe[PIPE_READ]);

                int status;
                waitpid(ms, &status, 0);

                close(dataPipe[PIPE_READ]);
                printf("Return of makeStage() parent: %d\n", ms);
            }

            close(fd[PIPE_READ]);

            free(dataPipe);
        }

        exit(EXIT_SUCCESS);
    }
    else // Parent - Will write data to pipe
    {
        // Closing read end of pipe
        close(fd[PIPE_READ]);
        
        // Array of storing integers from 2 to n
        int* numbers = malloc((n - 1) * sizeof(int));

        // Write number to pipe
        for(int i = 2; i < rn + 1; ++i)
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
