CC=gcc
CFLAGS=-Wall -Wextra -ggdb -std=c11 -pedantic
LIBS=-lncurses

all: main.o edit.o edit.h
	@$(CC) $(CFLAGS) $(LIBS) $^ -o edit
	@echo CC *.o

%.o: %.c Makefile
	@$(CC) $(CFLAGS) -c $< -o $@ 
	@echo CC $<

clean:
	rm *.o edit

.PHONY: clean
