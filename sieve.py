import sys
import math

INITIAL_FILTER = 2

def msg_n_is_prime(n):
    print('%d is prime.' % n)

def msg_n_is_not_prod_primes(n):
    print('%d is not the product of 2 primes.' % n)

def msg_n_is_prod_primes(n, x, y):
    print('%d is the product of primes %d & %d' % (n, x, y))

if __name__ == '__main__':

    if len(sys.argv) != 2:
        print('Program takes in 1 positive integer as parameter!')
        exit()

    # Get n and root n from argument
    n = int(sys.argv[1])
    rn = math.sqrt(n)

    # Set up list to keep track of filters
    known_filters = []

    # Set up list for values from 2...n
    values = []
    for i in range(INITIAL_FILTER, n):
        values.append(i)

    # Set starting value of current filter to 2
    current_filter = INITIAL_FILTER
    
    # While the filter is less than the root of n
    while current_filter < rn:

        # Add the current filter to the list of known filters
        known_filters.append(current_filter)

        # For each value in values, if the value is a factor of
        # the current filter, remove it from the list
        for val in values:
            if val % current_filter == 0:
                values.remove(val)

        # Set the current filter to the first value in the list
        # since the smallest value in values is now the smallest 
        # prime in the list
        current_filter = values[0]

    # Once the current filter is greater than or equal to the
    # sqaure root of n, set the new list of values to be the
    # list of known filters and the previous list of values
    values = known_filters + values

    # List of potential factors of n found in values
    pot_factors = []

    # For each value in values, if value is a factor add it to
    # the list of potential factors
    for val in values:
        #print('Checking %d %% %d == 0' % (n, val))
        if n % val == 0:
            pot_factors.append(val)
            #print('Apennd %d' % val)

    num_factors = len(pot_factors)
    #print('Potential factors: %d' % num_factors)
    
    # If there are no prime factors, the number is prime
    if num_factors == 0:
        msg_n_is_prime(n)
    # If there is one prime factor...
    elif num_factors == 1:
        # If the potential factor is the sqaure root of n,
        # n is the product of 2 primes
        if pot_factors[0] ** 2 == n:
                msg_n_is_prod_primes(n, pot_factors[0], pot_factors[0])
        else:
            msg_n_is_not_prod_primes(n)
    # If there are 2 factors, then n not is the product of the
    # potential primes
    else:
        msg_n_is_not_prod_primes(n)
