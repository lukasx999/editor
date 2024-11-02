#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <ncurses.h>
#include "./edit.h"


#define KEY_ESCAPE 27

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
    bool    text_border;
    // int     visual_cursor_line;
    // int     visual_cursor_column;
} Ui;

extern Ui   ui_init   (Editor *ed);
extern void ui_loop   (Ui *ui);
extern void ui_destroy(Ui *ui);
