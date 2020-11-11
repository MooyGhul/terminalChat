CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	gcc $(CFLAGS) main.c process.c list.o -lpthread -o s-talk

run: build
	./main

valgrind: build
	valgrind --leak-check=full ./main

clean:
	rm -f s-talk 