#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>

#include "./ui.h"
#include "./log.h"



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

static void _ui_draw_border(WINDOW *w) {
    wborder(w, '|', '|', '-', '-', '+', '+', '+', '+');
}

static void _ui_draw_text(Ui *ui) {

    // TODO: this
    size_t len = editor_get_document_size(ui->editor);
    if (len + 1 > (size_t) ui->text_area_height)
        len = ui->text_area_height;

    // TODO: refactor this! (using helper functions)
    // keep offset in bounds
    size_t offset = ui->scroll_offset;
    if (ui->text_area_height + offset > editor_get_document_size(ui->editor)) {
        // less lines than canvas size
        if (editor_get_document_size(ui->editor) < (size_t) ui->text_area_height)
            ui->scroll_offset = offset = 0;
        // at the end
        else
            ui->scroll_offset = offset = ui->editor->text.size - ui->text_area_height;
    }

    for (size_t i = 0; i < len; ++i) {
        mvwprintw(ui->window_text_area,
                  i + ui->text_border, // interpreting bool as decimal!
                  ui->text_border,
                  "%s", editor_get_string_by_index(ui->editor, i+offset));
    }

}

static void _ui_scroll_down(Ui *ui) {
    ui->scroll_offset++;
}

static void _ui_scroll_up(Ui *ui) {
    if (ui->scroll_offset == 0)
        return;
    ui->scroll_offset--;
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
    keypad(window, true);
    // intrflush(window, false);
    set_escdelay(0);

    // TODO: clean up this mess
    bool border           = true;
    int status_space      = 4;
    int status_top_height = 2;
    int text_area_height  = getmaxy(window) - status_space;
    WINDOW *text_area     = newwin(text_area_height,
                                   getmaxx(window),
                                   status_top_height,
                                   0);

    wbkgd(text_area, COLOR_PAIR(PAIR_TEXT));

    Ui ui = {
        .window           = window,
        .editor           = ed,
        .window_text_area = text_area,
        .text_border      = border,
        .text_area_height = text_area_height - 2 * border,
        .scroll_offset    = 0,
    };

    return ui;
}

void ui_destroy(Ui *ui) {
    (void) ui;
    // delwin(ui->window);
}


// TODO: scrollable text + wrapping

void ui_loop(Ui *ui) {

    // nodelay(ui->window_text_area, true);
    // nodelay(ui->window, true);

    bool quit = false;
    while (!quit) {

        curs_set(0); // dont show cursor while moving it to draw stuff

        /* Root Window */
        _ui_draw_statusline(ui);
        wrefresh(ui->window);
        /* ----------- */

        /* Text area */
        _ui_draw_text(ui);
        if (ui->text_border)
            _ui_draw_border(ui->window_text_area);

        wmove(ui->window_text_area,
              ui->editor->cursor_line   + ui->text_border,
              ui->editor->cursor_column + ui->text_border);

        wrefresh(ui->window_text_area);
        /* --------- */


        // sanity check
        assert(ui->editor->cursor_line   >= 0);
        assert(ui->editor->cursor_column >= 0);



        curs_set(1);
        char c = getch();

        werase(ui->window);
        werase(ui->window_text_area);

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
                    case 45: // Minus
                        _ui_scroll_up(ui);
                        break;
                    case 43: // Plus
                        _ui_scroll_down(ui);
                        break;
                    case 'x':
                        editor_delete_char(ui->editor);
                        break;
                    case 'i':
                        ui->editor->mode = MODE_INSERT;
                        break;
                    case 'o':
                        editor_insert_line_after(ui->editor);
                        editor_move_down(ui->editor);
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
                        editor_move_end_line_append_mode(ui->editor);
                        ui->editor->mode = MODE_APPEND;
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

            case MODE_APPEND:
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

        // wclear(ui->window);
        // wclear(ui->window_text_area);
        // werase(ui->window);
        // werase(ui->window_text_area);

    }

}
