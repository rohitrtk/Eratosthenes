FLAGS = -g
LIBS = -lm

all: filtertest forkonarg forkonarg2

filtertest: filtertest.o
	gcc ${FLAGS} -o $@ $^ ${LIBS}

forkonarg: forkonarg.o
	gcc ${FLAGS} -o $@ $^ ${LIBS}

forkonarg2: forkonarg2.o
	gcc ${FLAGS} -o $@ $^ ${LIBS}

%.o: %.c
	gcc ${FLAGS} -c $<

clean:
	rm -f *.o filtertest forkonarg forkonarg2