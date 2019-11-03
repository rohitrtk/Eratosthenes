#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>

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

int main(int argc, char** argv)
{
    if(argc != 2 || strtol(argv[1], NULL, 10) < 0)
    {
        fprintf(stderr, "Program takes in 1 positive integer as parameter!");
        exit(1);
    }

    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        perror("Signal");
        exit(1);
    }

    int fd[2];

    pipe(fd);
    
    int f = fork();

    if(f < 0)
    {
        perror("Fork");
    }
    else if(f == 0)
    {
        close(fd[1]);
        
        int fd2[2];
        pipe(fd2);

        if(filter(2, fd[0], fd2[1]) == 1)
        {
            printf("%s\n", strerror(errno));
        }

        printFromPipe(fd2[0]);

        close(fd[0]);
    }
    else
    {
        close(fd[0]);

        int  n          = strtol(argv[1], NULL, 10);
        int  rn         = (int)sqrt(n);
        int* numbers    = malloc((n - 1)* sizeof(int)); 
     
        for(int i = 2; i < rn + 1; ++i)
        {
            numbers[i - 2] = i;
            write(fd[1], &numbers[i - 2], sizeof(int));

            printf("Wrote %d to pipe\n", numbers[i - 2]);
        }

        close(fd[1]);
    } 

    return 0;
}