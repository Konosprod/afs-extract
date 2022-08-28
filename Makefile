CC = gcc

default: afs-extract

afs-extract: main.c
	$(CC) $< -o $@

.PHONY: clean

clean:
	rm afs-extract
