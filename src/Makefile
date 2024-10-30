

all: main.o edit.o edit.h
	cc -lncurses $^ -o edit

%.o: %.c Makefile
	cc -c $< -o $@
