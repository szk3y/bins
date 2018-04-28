TARGETS = cat
CC = gcc
CFLAGS = -g -Wall -Wextra -std=gnu11

all: $(TARGETS)

cat: cat.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	$(RM) a.out
