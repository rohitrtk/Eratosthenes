#ifndef SIEVE_H_
#define SIEVE_H_

#include <unistd.h>

/*
 * Contains function & macro definitions for use by sieve.c.
 */

#define PIPE_READ  0
#define PIPE_WRITE 1

#define ERROR_STAGING     -1
#define ERROR_INVALID_ARGS 1
#define ERROR_PROG_FAILURE 2

#define INITIAL_FILTER 2

#define MAKE_PIPE(fd)                   if (pipe(fd) == -1) { perror("Pipe"); exit(ERROR_PROG_FAILURE); }

#define N_ISPRIME(n)                    printf("%d is prime\n", n)
#define N_ISNPRODOF2PRIMES(n)           printf("%d is not the product of 2 primes\n", n)
#define N_ISPRODOF2PRIMES(n, f1, f2)    printf("%d is the product of primes %d & %d\n", n, f1, f2)

#define PRINT_NUM_FILTERS(filters)      printf("Number of filters: %d\n", filters)

/*
 * Reads an integer from r and writes it to w if
 * it is not a multiple of m. Returns 1 on error.
 */
int     filter(int m, int r, int w);

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
pid_t   makeStage(int m, int r, int** fds);

/*
 * Returns the numner of factors of n found from array of filters. Function
 * also outputs the factors, if any, to the array factors.
 */
int    findFactors(int n, int* filters, int filtersSize, int* factors);

/*
 * Prints to stdout if the number n is prime, a product of 2 primes or 
 * not a product of 2 primes.
 */
void    printPrimeFactor(int n, int numFactors, int* factors, int factorsSize);

/*
 * Prints integers from pipe r to stdout
 */
void    printFromPipe(int r);

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

#endif