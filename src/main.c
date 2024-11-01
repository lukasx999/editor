#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include <ncurses.h>

#include "./edit.h"
#include "./ui.h"

#define LOGFILE "log.txt"
#define LOG_IMPL
#include "./log.h"


void check_usage(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        exit(1);
    }
}


int main(int argc, char **argv) {
    log_init();
    check_usage(argc, argv);

    Editor editor = editor_new(argc == 1 ? NULL : argv[1]);
    Ui ui = ui_init(&editor);

    ui_loop(&ui);

    ui_destroy(&ui);

    editor_destroy(&editor);
    return EXIT_SUCCESS;

}
