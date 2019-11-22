#ifndef SIEVEDEBUG_H_
#define SIEVEDEBUG_H_

/*
 * Helper macro to print debugging messages to command line.
 * 
 * SIEVEDEBUG must be defined before include.
 */
#ifdef SIEVEDEBUG
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...) ((void)0)
#endif // SIEVEDEBUG

#endif // SIEVEDEBUG_H_