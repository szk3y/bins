TARGETS = cat head echo ls grep
CC = gcc
CFLAGS = -g -Wall -Wextra -std=gnu11

all: $(TARGETS)

cat: cat.c
	$(CC) $(CFLAGS) -o $@ $<

head: head.c
	$(CC) $(CFLAGS) -o $@ $<

echo: echo.c
	$(CC) $(CFLAGS) -o $@ $<

ls: ls.c
	$(CC) $(CFLAGS) -o $@ $<

grep: grep.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	$(RM) a.out
