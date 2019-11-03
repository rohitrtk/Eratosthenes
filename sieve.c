#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>

#define PIPE_READ  0
#define PIPE_WRITE 1

#define ERROR_STAGING     -1
#define ERROR_INVALID_ARGS 1
#define ERROR_PROG_FAILURE 2

#define MAKE_PIPE(fd) if (pipe(fd) == -1) { perror("Pipe"); exit(ERROR_PROG_FAILURE); }

int     filter(int m, int r, int w);
pid_t   makeStage(int m, int r, int** fds);
void    printFromPipe(int r);

/*
 * Reads an integer from r and writes it to w if
 * it is not a multiple of m. Returns 1 on error.
 */
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

/*
 * Arguments: m is the filter value, r is a file descriptors from which the
 * integers to be filtered are recieved from, and fds is a pointer to an array
 * of file descriptors to be used by a pipe.
 *
 * The purpose of this function is to: Create a pipe between the current
 * stage in the data pipeline and the next stage, create a fork to set up
 * the next stage or to print the final result, filter the data recieved 
 * for the current stage, and close any file descriptors that are no longer
 * needed.
 * 
 * Returns twice; The child process will return 0, signalling to handle the
 * next stage in the data pipeline or to print. The parent process will return
 * the childs PID, signalling to wait to recieve number of known filters. Function
 * will exit with -1 if there is an error.
 */
pid_t makeStage(int m, int r, int** fds)
{
    int fd[2];
    pipe(fd);

    int f = fork();

    if(f < 0)
    {
        perror("Fork - makeStage");
        exit(ERROR_STAGING);
    }
    else if(f == 0)
    {
        close(fd[PIPE_READ]);
        
        if(filter(m, r, fd[PIPE_WRITE]) == EXIT_FAILURE)
        {
            printf("%s\n", strerror(errno));

            exit(ERROR_PROG_FAILURE);
        }

        close(fd[PIPE_WRITE]);

        return 0;
    }
    else
    {
        close(fd[PIPE_READ]);
        close(fd[PIPE_WRITE]);

        return waitpid(f, NULL, 0);
    }
}

/*
 * Prints integers from pipe r to stdout
 */
void printFromPipe(int r)
{
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
        fprintf(stderr, "Program takes in 1 positive integer as parameter!");
        exit(ERROR_INVALID_ARGS);
    }

    // Turns of sigpipe
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        perror("Signal");
        exit(ERROR_PROG_FAILURE);
    }

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

        int* dataPipe = malloc(2 * sizeof(int));
        MAKE_PIPE(dataPipe);

        int ms = makeStage(filter, fd[PIPE_READ], &dataPipe);

        // Handle child return of makeStage
        if(ms == 0)
        {
            printf("Return of makeStage() child: %d\n", ms);

            exit(115);
        }
        // Handle parent return of makeStage
        else
        {
            int status;
            waitpid(ms, &status, 0);

            printf("Return of makeStage() parent: %d\n", ms);
        }

        close(fd[PIPE_READ]);

        free(dataPipe);

        exit(EXIT_SUCCESS);
    }
    else // Parent - Will write data to pipe
    {
        // Closing read end of pipe
        close(fd[PIPE_READ]);

        // Get maximum number to be in pipes
        int  n          = strtol(argv[1], NULL, 10);
        int  rn         = (int)sqrt(n);
        int* numbers    = malloc((n - 1)* sizeof(int)); // Array of numbers from 2 to n
        
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

// Test code for printing stuff in pipe
/*
        int buffer;
        int bytesRead;
        while((bytesRead = read(dataPipe[PIPE_READ], &buffer, sizeof(int))) > 0)
        {
            int length = snprintf(NULL, 0, "%d", buffer);
            char* str = malloc(length + 1);

            snprintf(str, length + 1, "%d", buffer);
            
            write(STDOUT_FILENO, str, sizeof(str));
            write(STDOUT_FILENO, "\n", 1);

            free(str);
        }
*/   