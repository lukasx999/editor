#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include <ncurses.h>

#include "./edit.h"


#define KEY_ESCAPE 27



#define LOGFILE "log.txt"

void log_init(void) {
    FILE *f = fopen(LOGFILE, "w");
    fclose(f);
}

void log_write(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);

    FILE *f = fopen(LOGFILE, "a");
    vfprintf(f, fmt, va);

    va_end(va);
    fclose(f);
}


int main(int argc, char **argv) {
    log_init();

    if (argc > 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        exit(1);
    }


    Editor editor = editor_new(argc == 1 ? NULL : argv[1]);


    WINDOW *window = initscr();

    bool quit = false;
    while (!quit) {

        // Draw Text
        for (size_t i = 0; i < editor.text.size; ++i)
            printw("%s\n", editor.text.lines[i].str);

        // Draw Statusline
        char fmt[10] = { 0 };
        snprintf(fmt, 10, "<%s>", modes_repr[editor.mode]);
        mvprintw(getmaxy(window)-1, 0, "%s", fmt);

        // Move Cursor
        move(editor.cursor_line, editor.cursor_column);

        log_write("cursor_line: %d\n", editor.cursor_line);

        char c = getch();
        switch (editor.mode) {
            case MODE_NORMAL: {
                switch (c) {
                    case 'W':
                        if (editor.filename != NULL) // dont try to write empty buffer
                            editor_write(&editor, NULL);
                        break;
                    case 'q':
                        quit = true;
                        break;
                    case 'i':
                        editor.mode = MODE_INSERT;
                        break;
                    case 'I':
                        editor.cursor_column = 0;
                        editor.mode = MODE_INSERT;
                        break;
                    case 'A': {
                        String *current = editor_get_current_string(&editor);
                        editor.cursor_column = current->size;
                        editor.mode = MODE_INSERT;
                    } break;
                    case '$': {
                        String *current = editor_get_current_string(&editor);
                        editor.cursor_column = current->size-1;
                    } break;
                    case '_':
                    case '0': {
                        editor.cursor_column = 0;
                    } break;
                    case 'a':
                        editor.mode = MODE_INSERT;
                        editor.cursor_column++;
                        break;
                    case ':':
                        editor.mode = MODE_COMMAND;
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

                if (c == KEY_ESCAPE) {
                    editor.mode = MODE_NORMAL;
                    editor_move_left(&editor);
                    break;
                }

                editor_insert(&editor, c);

            } break;

            case MODE_COMMAND: {
                if (c == KEY_ESCAPE) {
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
