#pragma once

#include <stddef.h>

typedef struct {
    size_t size;
    size_t capacity;
    char *str;
} String;

extern String string_new (void);
extern String string_from(const char *str);
extern void string_append_char       (String *s, char c);
extern void string_append_string     (String *s, const String *other);
extern void string_insert_char_before(String *s, size_t index, char c);

typedef struct {
    size_t size;
    size_t capacity;
    String *lines;
} Lines;

typedef enum {
    MODE_NORMAL,
    MODE_INSERT,
    MODE_COMMAND,
} Modes;

#pragma GCC diagnostic ignored "-Wunused-variable"
static const char *modes_repr[] = { "normal", "insert", "command" };

typedef struct {
    int cursor_column;
    int cursor_line;
    Modes mode;
    Lines text;
} Editor;

extern Editor editor_new    (const char *filename);
extern void   editor_destroy(Editor *ed);
extern String* editor_get_current_string(Editor *ed);
extern void editor_insert    (Editor *ed, char c);
extern void editor_move_right(Editor *ed);
extern void editor_move_left (Editor *ed);
extern void editor_move_up   (Editor *ed);
extern void editor_move_down (Editor *ed);
