CC = gcc
CFLAGS = -O2 -g -ggdb
main:
	${CC} -c main.c -o main.o ${CFLAGS}

clean:
	rm -v main.o
