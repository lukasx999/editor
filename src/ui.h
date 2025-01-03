#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <ncurses.h>
#include "./edit.h"



// ED_KEY_* because KEY_* prefix is already being used by ncurses
#define ED_KEY_ESCAPE 27
#define ED_KEY_RETURN 10
#define ED_KEY_PLUS 43
#define ED_KEY_MINUS 45



typedef enum {
    PAIR_DEFAULT    = 0,
    PAIR_TEXT       = 1,
    PAIR_STATUSLINE = 2,
} ColorPairs;



typedef struct {
    WINDOW *window; // root window (entire screen)
    Editor *editor;

    WINDOW *window_text_area; // bounds of actual text
    int     scroll_offset;
    int     text_area_height;
    int     statusbar_space;
    bool    text_border;

    int     visual_cursor_line;
    int     visual_cursor_column;

} Ui;

extern Ui   ui_init   (Editor *ed);
extern void ui_loop   (Ui *ui);
extern void ui_destroy(Ui *ui);
