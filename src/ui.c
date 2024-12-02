#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

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

    // limit rendering document to text area height
    size_t len = editor_get_document_size(ui->editor);
    if (len + 1 > (size_t) ui->text_area_height) {
        len = ui->text_area_height;
    }



    // TODO: this

#if 0

    if (ui->text_area_height + ui->scroll_offset >
        (int) editor_get_document_size(ui->editor)) {

        // all lines fit on screen
        if ((size_t) ui->text_area_height > editor_get_document_size(ui->editor)) {
            ui->scroll_offset = 0;
        }

        // at the end
        else {
            ui->scroll_offset = editor_get_document_size(ui->editor) - ui->text_area_height;
        }

    }

#else
    size_t offset = 0;
#endif

    for (size_t i = 0; i < len; ++i) {
        mvwprintw(ui->window_text_area,
                  i + ui->text_border, // interpreting bool as decimal!
                  ui->text_border,
                  "%s", editor_get_string_by_index(ui->editor, i + ui->scroll_offset));
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


static void _ui_ncurses_init(WINDOW *window) {
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
}


Ui ui_init(Editor *ed) {
    WINDOW *window = initscr();

    _ui_ncurses_init(window);

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
// TODO: tabs for indenting
// TODO: joining lines

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
        assert((size_t) ui->editor->cursor_line < editor_get_document_size(ui->editor));





        curs_set(1);
        int c = getch();

        werase(ui->window);
        werase(ui->window_text_area);

        switch (editor_get_current_mode(ui->editor)) {
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

                    case 'd':
                        editor_delete_line(ui->editor);
                        break;

                    case 'D':
                        editor_delete_till_end_of_line(ui->editor);
                        break;

                    case 'y':
                        editor_copy_text_line(ui->editor);
                        break;

                    case 'p':
                        editor_paste_clipboard(ui->editor);
                        break;

                    case 'J':
                        editor_join_line_next(ui->editor);
                        break;

                    case 'i':
                        editor_set_mode(ui->editor, MODE_INSERT);
                        break;

                    case 'o':
                        editor_insert_line_after(ui->editor);
                        editor_move_down(ui->editor);
                        editor_set_mode(ui->editor, MODE_INSERT);
                        break;

                    case 'I':
                        editor_move_start_line(ui->editor);
                        editor_set_mode(ui->editor, MODE_INSERT);
                        break;

                    case 'G':
                        editor_move_end_document(ui->editor);
                        break;

                    case 'g':
                        editor_move_start_document(ui->editor);
                        break;

                    case 'A': {
                        editor_move_end_line_append_mode(ui->editor);
                        editor_set_mode(ui->editor, MODE_APPEND);
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
                        editor_set_mode(ui->editor, MODE_INSERT);
                        ui->editor->cursor_column++;
                        break;

                    case ':':
                        editor_set_mode(ui->editor, MODE_COMMAND);
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

            // TODO: snapping behaviour: do bounds check, and move cursor back into canvas

            case MODE_APPEND:
            case MODE_INSERT: {

                if (c == KEY_ESCAPE) {
                    editor_set_mode(ui->editor, MODE_NORMAL);
                    editor_move_left(ui->editor);
                    break;
                }

                if (c == KEY_RETURN) {
                    editor_split_line(ui->editor);
                    break;
                }

                if (c == KEY_BACKSPACE) {
                    editor_delete_char_backspace(ui->editor);
                    break;
                }

                if (!isprint(c))
                    break;

                editor_insert_char(ui->editor, c);

            } break;

            case MODE_COMMAND: {
                if (c == KEY_ESCAPE) {
                    editor_set_mode(ui->editor, MODE_NORMAL);
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
