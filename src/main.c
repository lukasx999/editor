#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include <ncurses.h>

#include "./edit.h"

#define LOGFILE "log.txt"
#define LOG_IMPL
#include "./log.h"

#define KEY_ESCAPE 27





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
                    case 'x':
                        editor_delete(&editor);
                        break;
                    case 'i':
                        editor.mode = MODE_INSERT;
                        break;
                    case 'I':
                        editor_move_start_line(&editor);
                        editor.mode = MODE_INSERT;
                        break;
                    case 'A': {
                        // TODO: this!
                        editor_move_end_line(&editor);
                        editor.cursor_column++;
                        editor.mode = MODE_INSERT;
                    } break;
                    case '$': {
                        editor_move_end_line(&editor);
                    } break;
                    case '_':
                        editor_move_start_line_skip_whitespace(&editor);
                        break;
                    case '0':
                        editor_move_start_line(&editor);
                        break;
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
