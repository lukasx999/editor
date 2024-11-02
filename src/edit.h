#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>





typedef struct {
    size_t size;
    size_t capacity;
    char *str;
} String;

extern String string_new (void);
extern String string_from(const char *str);
extern void   string_delete            (String *s, size_t index);
extern void   string_append_char       (String *s, char c);
extern void   string_append_string     (String *s, const String *other);
extern void   string_insert_char_before(String *s, size_t index, char c);
extern void   string_insert_char_after (String *s, size_t index, char c);





typedef struct {
    size_t size;
    size_t capacity;
    String *lines;
} Lines;

extern Lines lines_new         (void);
extern void  lines_append      (Lines *l, const String *s);
extern void  lines_remove      (Lines *l, size_t index); // TODO: this
extern void  lines_insert_after(Lines *l, size_t index, String *s);




typedef enum {
    MODE_NORMAL,
    MODE_COMMAND,
    MODE_INSERT,
    MODE_APPEND,
} Modes;

#pragma GCC diagnostic ignored "-Wunused-variable"
static const char *modes_repr[] = {
    "normal",
    "command",
    "insert",
    "append",
};

typedef struct {
    int cursor_column;
    int cursor_line;
    Modes mode;
    Lines text;

    // bool last_char; // TODO: this

    const char *filename;

    // Options:
    bool wrap_vertical,
         wrap_horizontal;
} Editor;
// TODO: allocate editor on heap and return ptr to user

// Instance Mangagement
extern Editor editor_new    (const char *filename); // filename == NULL will create an empty buffer
extern void   editor_destroy(Editor *ed);
extern void   editor_write  (Editor *ed, const char *filename); // writes the contents of `text` into the opened file - filename == NULL will write into the already opened file, otherwise text will be written to filename

// Char Operations
extern void editor_insert           (Editor *ed, char c);
extern void editor_delete_char      (Editor *ed); // delete char under cursor
extern void editor_insert_line_after(Editor *ed);

// Read Operations
extern char* editor_get_string_by_index(Editor *ed, size_t index);
extern size_t editor_get_document_size (Editor *ed);

// Move Operations
extern void editor_move_right                     (Editor *ed);
extern void editor_move_left                      (Editor *ed);
extern void editor_move_up                        (Editor *ed);
extern void editor_move_down                      (Editor *ed);
extern void editor_move_end_line                  (Editor *ed);
extern void editor_move_start_line                (Editor *ed);
extern void editor_move_start_line_skip_whitespace(Editor *ed);
extern void editor_move_end_document              (Editor *ed);
extern void editor_move_start_document            (Editor *ed);
extern void editor_move_end_line_append_mode      (Editor *ed);
