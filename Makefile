
CFLAGS = -g -Wall -std=gnu99

all: guess

guess: guess.c
	gcc ${CFLAGS} $^ -o $@

clean:
	rm guess
