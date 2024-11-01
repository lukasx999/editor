#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>

#include "./ui.h"


static void _quit(void) {
    endwin();
}

static void _ui_draw_statusline(Ui *ui) {
    // Draw Statusline
    color_set(PAIR_STATUSLINE, NULL);
    char fmt[10] = { 0 };
    snprintf(fmt, 10, "|%s|", modes_repr[ui->editor->mode]);
    mvwprintw(ui->window, getmaxy(ui->window)-1, 0, "%s", fmt);
    color_set(PAIR_DEFAULT, NULL);
    // mvwprintw(ui->window, 0, 0, "[%s]", ui->editor->filename);
}

static void _ui_draw_text(Ui *ui) {
    for (size_t i = 0; i < ui->editor->text.size; ++i) {
        mvwprintw(ui->window_text_area,
                  i + ui->text_border,
                  ui->text_border,
                  "%s", ui->editor->text.lines[i].str);
    }
}


static void _ui_draw_border(WINDOW *w) {
    wborder(w, '|', '|', '-', '-', '+', '+', '+', '+');
}



Ui ui_init(Editor *ed) {
    WINDOW *window = initscr();
    start_color();
    atexit(_quit);
    init_pair(PAIR_STATUSLINE, COLOR_RED, COLOR_BLACK);
    init_pair(PAIR_TEXT, COLOR_WHITE, COLOR_BLACK);
    // init_color(COLOR_RED, 0, 0, 0); // change existing base colors
    curs_set(1);
    noecho();
    cbreak();
    wbkgd(window, COLOR_PAIR(PAIR_DEFAULT)); // set bg color

    int columns = getmaxx(window);
    int lines = getmaxy(window);
    int status_space = 5;
    int status_top_height = 3;
    WINDOW *text_area = newwin(lines - status_space,
                               columns,
                               status_top_height,
                               0);

    wbkgd(text_area, COLOR_PAIR(PAIR_TEXT));


    Ui ui = {
        .window           = window,
        .editor           = ed,
        .window_text_area = text_area,
        .text_border      = false,
    };

    return ui;
}

void ui_destroy(Ui *ui) {
    (void) ui;
    // delwin(ui->window);
}


void ui_loop(Ui *ui) {

    bool quit = false;
    while (!quit) {

        /* Root Window */
        _ui_draw_statusline(ui);
        wrefresh(ui->window);
        /* ----------- */


        /* Text area */
        _ui_draw_text(ui);
        if (ui->text_border) {
            _ui_draw_border(ui->window_text_area);
        }

        wmove(ui->window_text_area,
              ui->editor->cursor_line   + ui->text_border,
              ui->editor->cursor_column + ui->text_border);

        wrefresh(ui->window_text_area);
        /* --------- */




        // sanity check
        assert(ui->editor->cursor_line   >= 0);
        assert(ui->editor->cursor_column >= 0);

        char c = getch();
        switch (ui->editor->mode) {
            case MODE_NORMAL: {
                switch (c) {
                    case 'W':
                        if (ui->editor->filename != NULL) // dont try to write empty buffer
                            editor_write(ui->editor, NULL);
                        break;
                    case 'q':
                        quit = true;
                        break;
                    case 'x':
                        // BUG: doesnt work with empty line
                        editor_delete(ui->editor);
                        break;
                    case 'i':
                        ui->editor->mode = MODE_INSERT;
                        break;
                    case 'I':
                        editor_move_start_line(ui->editor);
                        ui->editor->mode = MODE_INSERT;
                        break;
                    case 'G':
                        editor_move_end_document(ui->editor);
                        break;
                    case 'g':
                        editor_move_start_document(ui->editor);
                        break;
                    case 'A': {
                        // TODO: this!
                        // BUG: dont move cursor if line size is 0 => empty line
                        editor_move_end_line(ui->editor);
                        ui->editor->cursor_column++;
                        ui->editor->mode = MODE_INSERT;
                    } break;
                    case '$': {
                        editor_move_end_line(ui->editor);
                    } break;
                    case '_':
                        editor_move_start_line_skip_whitespace(ui->editor);
                        break;
                    case '0':
                        editor_move_start_line(ui->editor);
                        break;
                    case 'a':
                        ui->editor->mode = MODE_INSERT;
                        ui->editor->cursor_column++;
                        break;
                    case ':':
                        ui->editor->mode = MODE_COMMAND;
                        break;
                    case 'h':
                        editor_move_left(ui->editor);
                        break;
                    case 'l':
                        editor_move_right(ui->editor);
                        break;
                    case 'j':
                        editor_move_down(ui->editor);
                        break;
                    case 'k':
                        editor_move_up(ui->editor);
                        break;
                }


            } break;
            case MODE_INSERT: {

                if (c == KEY_ESCAPE) {
                    ui->editor->mode = MODE_NORMAL;
                    editor_move_left(ui->editor);
                    break;
                }

                editor_insert(ui->editor, c);

            } break;

            case MODE_COMMAND: {
                if (c == KEY_ESCAPE) {
                    ui->editor->mode = MODE_NORMAL;
                    break;
                }
            } break;
        }

        wclear(ui->window);
        wclear(ui->window_text_area);

    }

}
