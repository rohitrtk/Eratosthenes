FLAGS = -g
DEPS = sieve.h sievedebug.h
LIBS = -lm

all: sieve

sieve: sieve.o
	gcc ${FLAGS} -o $@ $^ ${LIBS}

%.o: %.c ${DEPS}
	gcc ${FLAGS} -c $<

clean:
	rm -f *.o sieve
