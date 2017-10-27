SRCS = main.c
CC = gcc
CFLAGS = -g -W -Wall -std=c11

# Instead of writing about complex dependencies, I always compile all source files.
all: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

.PHONY: clean
clean:
	$(RM) a.out
