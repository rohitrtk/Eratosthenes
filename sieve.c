#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <math.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

#define STAGE_ERROR -1

/*
 * Reads an integer from r and writes it to w if
 * it is not a multiple of m. Returns 1 on error.
 */
int filter(int m, int r, int w)
{
    int buffer;
    if(read(r, &buffer, sizeof(int)) == -1)
    {
        perror("Filter");
        return EXIT_FAILURE;    
    }

    if(buffer % m != 0)
    {
        if(write(w, &buffer, sizeof(int)) == -1)
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/*
 * 
 * 
 * 
 * 
 */
pid_t makeStage(int m, int r, int** fds)
{
    int fd[2];
    if(pipe(fd) == -1)
    {
        perror("Stage Pipe");
        return STAGE_ERROR;
    }

}

int main(int argc, char** argv)
{
    if(argc != 2 || strtol(argv[1], NULL, 10) < 0)
    {
        fprintf(stderr, "Program takes in 1 positive integer as parameter!");
        exit(EXIT_FAILURE);
    }

    int fd[2];
    if(pipe(fd) == -1)
    {
        perror("Pipe");
        exit(EXIT_FAILURE);
    }

    int p;

    pid_t f = fork();

    if(f < 0)       // Error
    {
        perror("Fork");
        exit(EXIT_FAILURE);
    }
    else if(f == 0) // Child
    {
        // Reads data from pipe
        close(fd[PIPE_WRITE]);

        p = fd[PIPE_READ];

        int* dataPipe = malloc(sizeof(int*) * 2);
        if(pipe(dataPipe) == -1)
        {
            perror("Data Pipe");
            exit(EXIT_FAILURE);
        }

        int filter;
        read(p, &filter, sizeof(int));
        makeStage(filter, p, &dataPipe);

        free(dataPipe);

        /*
        int buffer;
        int bytesRead;
        while((bytesRead = read(p, &buffer, sizeof(int))) > 0)
        {
            int length = snprintf(NULL, 0, "%d", buffer);
            char* str = malloc(length + 1);

            snprintf(str, length + 1, "%d", buffer);
            
            write(STDOUT_FILENO, str, sizeof(str));
            write(STDOUT_FILENO, "\n", 1);

            free(str);
        }
        */

        close(fd[PIPE_READ]);

        exit(EXIT_SUCCESS);
    }
    else            // Parent
    {
        // Writes data to pipe
        close(fd[PIPE_READ]);

        p = fd[1];

        int  n          = strtol(argv[1], NULL, 10);
        int  rn         = (int)sqrt(n);
        int* numbers    = malloc((n - 1)* sizeof(int)); // Array of numbers from 2 to n
        
        for(int i = 2; i < rn + 1; ++i)
        {
            numbers[i - 2] = i;
            write(p, &numbers[i - 2], sizeof(int));
        }

        close(fd[PIPE_WRITE]);

        int status;
        waitpid(f, &status, WNOHANG);

        if(WIFEXITED(status) && WIFEXITED(status) == 255)
        {
            free(numbers);
            exit(255);
        }
    }
    

    return EXIT_SUCCESS;
}