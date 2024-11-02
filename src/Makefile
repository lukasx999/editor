CC=gcc
CFLAGS=-Wall -Wextra -ggdb -std=c11 -pedantic
LIBS=-lncurses
DEPS=edit.h ui.h log.h

all: main.o ui.o edit.o string.o lines.o
	@$(CC) $(CFLAGS) $(LIBS) $^ -o edit
	@echo CC *.o

%.o: %.c Makefile $(DEPS)
	@$(CC) $(CFLAGS) -c $< -o $@ 
	@echo CC $<

clean:
	rm *.o edit

.PHONY: clean
