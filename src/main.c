#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include <ncurses.h>

#include "./edit.h"




#define LOGFILE "log.txt"

void log_init(void) {
    FILE *f = fopen(LOGFILE, "w");
    fclose(f);
}

void log_write(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);

    FILE *f = fopen(LOGFILE, "w");
    vfprintf(f, fmt, va);

    va_end(va);
    fclose(f);
}




int main(int argc, char **argv) {
    log_init();

    if (argc < 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        exit(1);
    }

    Editor editor = editor_new(argv[1]);






}
