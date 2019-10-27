#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

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

    int pipe;

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

        pipe = fd[0];

        int buffer[10];
        
        for(int i = 0; i < 10; ++i)
        {
            read(pipe, buffer, 1);
            write(STDOUT_FILENO, buffer, 1);
            write(STDOUT_FILENO, "\n", 1);
        }

        close(fd[PIPE_READ]);

        exit(EXIT_SUCCESS);
    }
    else            // Parent
    {
        // Writes data to pipe
        close(fd[PIPE_READ]);

        pipe = fd[1];

        for(int i = 0; i < 10; ++i)
        {
            char c = 48 + i;
            write(pipe, &c, sizeof(char));
        }

        close(fd[PIPE_WRITE]);

        int status;
        waitpid(f, &status, WNOHANG);

        if(WIFEXITED(status) && WIFEXITED(status) == 255)
        {
            exit(255);
        }
    }
    

    return EXIT_SUCCESS;
}