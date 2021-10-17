CC=gcc
CFLAGS=-g -std=gnu11

default:
	$(CC) $(CFLAGS) -o ash ./src/main.c ./src/globals.c ./src/utils/*.c ./src/commands/*.c
