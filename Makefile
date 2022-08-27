CC = gcc

default: afs-extract

afs-extract: main.c
	$(CC) -c $< -o $@
