FLAGS = -g
DEPS = sieve.h
LIBS = -lm

all: sieve

sieve: sieve.o
	gcc ${FLAGS} -o $@ $^ ${LIBS}

%.o: %.c ${DEPS}
	gcc ${FLAGS} -c $<

clean:
	rm -f *.o sieve
