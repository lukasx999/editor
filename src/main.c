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


    WINDOW *window = initscr();

    bool quit = false;
    while (!quit) {

        // Draw Text
        for (size_t i = 0; i < editor.text.size; ++i)
            printw("%s", editor.text.lines[i].str);

        // Draw Statusline
        char fmt[10] = { 0 };
        snprintf(fmt, 10, "[%s]", modes_repr[editor.mode]);
        mvprintw(getmaxy(window)-1, 0, "%s", fmt);


        // Move Cursor
        move(editor.cursor_line, editor.cursor_column);



        char c = getch();
        switch (editor.mode) {
            case MODE_NORMAL: {

                switch (c) {

                    case 'q':
                        quit = true;
                        break;

                    case 'i':
                        editor.mode = MODE_INSERT;
                        break;

                    case 'h':
                        editor_move_left(&editor);
                        break;

                    case 'l':
                        editor_move_right(&editor);
                        break;

                    case 'j':
                        editor_move_down(&editor);
                        break;

                    case 'k':
                        editor_move_up(&editor);
                        break;
                }


            } break;
            case MODE_INSERT: {

                if (c == 27) { // ESCAPE
                    editor.mode = MODE_NORMAL;
                    break;
                }

                editor_insert(&editor, c);

            } break;
            case MODE_COMMAND: {
                if (c == 27) { // ESCAPE
                    editor.mode = MODE_NORMAL;
                    break;
                }
            } break;
        }

        clear();

    }

    endwin();
    editor_destroy(&editor);
    return EXIT_SUCCESS;

}
