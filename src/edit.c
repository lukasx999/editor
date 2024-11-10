#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#include "./edit.h"

#include "./log.h"


/* ---------------- */
/* Helper Functions */
/* ---------------- */

static inline String*
_editor_get_current_string(Editor *ed) {
    return &ed->text.lines[ed->cursor_line];
}

static inline bool
_editor_is_at_last_line_document(Editor *ed) {
    // checks if the current line (indicated by `cursor_line`) is the last line in the document
    return (size_t) ed->cursor_line + 1 == editor_get_document_size(ed);
}

static inline bool
_editor_is_over_last_line_document(Editor *ed) {
    // checks if the current line (indicated by `cursor_line`) is over the last line in the document
    return (size_t) ed->cursor_line + 1 > editor_get_document_size(ed);
}

static inline bool
_editor_is_at_first_line_document(Editor *ed) {
    // checks if the current line (indicated by `cursor_line`) is the first line in the document
    return ed->cursor_line == 0;
}

static inline bool
_editor_is_at_end_of_line(Editor *ed) {
    // returns true if the cursor is hovering over the last char in the line
    return (size_t) ed->cursor_column + 1 == _editor_get_current_string(ed)->size;
}

static inline bool
_editor_is_over_end_of_line(Editor *ed) {
    // returns true if the cursor is out of bounds of the current line
    return (size_t) ed->cursor_column + 1 > _editor_get_current_string(ed)->size;
}

static inline bool
_editor_is_at_start_of_line(Editor *ed) {
    // returns true if the cursor is hovering over the first char in the line
    return ed->cursor_column == 0;
}

static inline bool
_editor_is_at_empty_line(Editor *ed) {
    // returns true if the cursor is hovering over an empty line
    return _editor_get_current_string(ed)->size == 0;
}

// TODO: this
// static inline void
// _editor_snap(Editor *ed) {
//     if (_editor_is_over_end_of_line(ed)) {
//         ...
//     }
// }



/* ---------------- */
/* ---------------- */
/* ---------------- */



/* ------------------- */
/* Instance Management */
/* ------------------- */

Editor editor_new(const char *filename) {

    Editor ed = {
        .cursor_line     = 0,
        .cursor_column   = 0,
        .mode            = MODE_NORMAL,
        .filename        = filename,
        .wrap_vertical   = false,
        .wrap_horizontal = false,
    };

    if (ed.filename == NULL) { // create empty buffer
        ed.text = lines_new();
        String s = string_from("");
        lines_append(&ed.text, &s);
        return ed;
    }

    FILE *f = fopen(ed.filename, "r");
    if (f == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    ed.text = lines_new();

    char buf[BUFSIZ] = { 0 };
    while (fgets(buf, BUFSIZ, f) != NULL) {
        buf[strcspn(buf, "\n")] = '\0'; // get rid of newline at the end
        String s = string_from(buf);
        lines_append(&ed.text, &s);
        memset(buf, 0, BUFSIZ);
    }

    // TODO: editor_create_empty_line
    // Create empty line if file is empty
    if (editor_get_document_size(&ed) == 0) {
        String s = string_from("");
        lines_append(&ed.text, &s);
    }

    fclose(f);
    return ed;

}

void editor_destroy(Editor *ed) {
    for (size_t i = 0; i < ed->text.size; ++i)
        free(ed->text.lines[i].str);
    free(ed->text.lines);
}

void editor_write(Editor *ed, const char *filename) {

    const char *file = filename == NULL ? ed->filename : filename;
    assert(!(filename == NULL && file == NULL)); // empty buffer (ed->filename == NULL) and filename == NULL

    FILE *f = fopen(file, "w");

    for (size_t i = 0; i < ed->text.size; ++i)
        fprintf(f, "%s\n", ed->text.lines[i].str);

    fclose(f);

}

/* ------------------- */
/* ------------------- */
/* ------------------- */



/* --------------- */
/* Edit Operations */
/* --------------- */


void editor_delete_till_end_of_line(Editor *ed) {

    string_delete_till_end_of_string(
        _editor_get_current_string(ed), ed->cursor_column
    );
    editor_move_end_line(ed);

}


void editor_split_line(Editor *ed) {

    String new = string_from(
        _editor_get_current_string(ed)->str + ed->cursor_column
    );

    lines_insert_after(&ed->text, ed->cursor_line, &new);
    editor_delete_till_end_of_line(ed);

    editor_move_down(ed);
    editor_move_start_line(ed);

}



void editor_delete_char(Editor *ed) {

    if (_editor_is_at_empty_line(ed)) // cannot delete empty line
        return;

    string_delete_char(_editor_get_current_string(ed), ed->cursor_column);

    // out of bounds check after deleting
    if (_editor_is_over_end_of_line(ed))
        editor_move_end_line(ed);

}

void editor_delete_char_backspace(Editor *ed) {

    if (_editor_is_at_start_of_line(ed))
        return;

    editor_move_left(ed);
    string_delete_char(_editor_get_current_string(ed), ed->cursor_column);

}

void editor_delete_line(Editor *ed) {

    if (editor_get_document_size(ed) == 1) {
        lines_delete(&ed->text, ed->cursor_line);
        String s = string_from("");
        lines_append(&ed->text, &s);
        editor_move_start_line(ed);
        return;
    }

    lines_delete(&ed->text, ed->cursor_line);

    // Bounds Checking

    if (_editor_is_over_end_of_line(ed))
        editor_move_end_line(ed);

    if (_editor_is_over_last_line_document(ed))
        editor_move_up(ed);

}

void editor_insert(Editor *ed, char c) {

    String *current = _editor_get_current_string(ed);

    // corner case: appending to the end of a line
    // foo| <-- appending here
    // TODO: this:
    if ((size_t) ed->cursor_column == current->size) { // append mode
        string_append_char(current, c);
    }
    else {
        string_insert_char_before(current, ed->cursor_column, c);
    }

    ed->cursor_column++;
}

void editor_insert_line_after(Editor *ed) {
    String s = string_from("");
    lines_insert_after(&ed->text, ed->cursor_line, &s);
}

void editor_set_mode(Editor *ed, Mode mode) {
    ed->mode = mode;
}



/* --------------- */
/* --------------- */
/* --------------- */



/* --------------- */
/* Read Operations */
/* --------------- */



char* editor_get_string_by_index(Editor *ed, size_t index) {
    assert(index < ed->text.size);
    return ed->text.lines[index].str;
}

size_t editor_get_document_size(Editor *ed) {
    return ed->text.size;
}

Mode editor_get_current_mode(Editor *ed) {
    return ed->mode;
}

size_t editor_get_current_string_length(Editor *ed) {
    return _editor_get_current_string(ed)->size;
}



/* --------------- */
/* --------------- */
/* --------------- */



/* --------------- */
/* Move Operations */
/* --------------- */



void editor_move_right(Editor *ed) {

    if (!_editor_is_at_end_of_line(ed) && !_editor_is_at_empty_line(ed)) {
        ed->cursor_column++;
        return;
    }

    if (!ed->wrap_horizontal) { // wraparound
        return;
    }

    if (_editor_is_at_last_line_document(ed)) { // go to the beginning of the document
        editor_move_start_document(ed);
        editor_move_start_line(ed);
    }
    else { // go to the beginning of the next line
        ed->cursor_line++; // TODO: use editor move
        editor_move_start_line(ed);
    }

}

void editor_move_left(Editor *ed) {

    if (!_editor_is_at_start_of_line(ed)) {
        ed->cursor_column--;
        return;
    }

    if (!ed->wrap_horizontal) { // wraparound
        return;
    }

    if (_editor_is_at_first_line_document(ed)) { // go to the end of the document
        editor_move_end_document(ed);
        editor_move_end_line(ed);
    }
    else { // go to the end of the previous line
        ed->cursor_line--;
        editor_move_end_line(ed);
    }

}

void editor_move_up(Editor *ed) {

    if (!_editor_is_at_first_line_document(ed)) {
        ed->cursor_line--;

        // check for overshoot on line above
        if (_editor_is_over_end_of_line(ed)) {
            editor_move_end_line(ed);
        }

        return;
    }

    if (!ed->wrap_vertical) { // wraparound
        return;
    }

    editor_move_end_document(ed);

}

void editor_move_down(Editor *ed) {
    if (!_editor_is_at_last_line_document(ed)) {
        ed->cursor_line++;

        // check for overshoot on line below
        if (_editor_is_over_end_of_line(ed)) {
            editor_move_end_line(ed);
        }

        return;
    }

    if (!ed->wrap_vertical) { // wraparound
        return;
    }

    editor_move_start_document(ed);

}

void editor_move_end_line(Editor *ed) {
    bool  empty = _editor_is_at_empty_line(ed);
    size_t size = _editor_get_current_string(ed)->size;
    ed->cursor_column = empty ? 0 : size - 1; // check for empty line (size == 0)
}

void editor_move_start_line(Editor *ed) {
    ed->cursor_column = 0;
}

void editor_move_start_line_skip_whitespace(Editor *ed) {
    char *current = NULL;
    char *start = current = _editor_get_current_string(ed)->str;

    while (*current == ' ')
        current++;

    ed->cursor_column = current - start;
}

void editor_move_end_document(Editor *ed) {
    ed->cursor_line = ed->text.size - 1;
}

void editor_move_start_document(Editor *ed) {
    ed->cursor_line = 0;
}

void editor_move_end_line_append_mode(Editor *ed) {
    if (_editor_is_at_empty_line(ed))
        return;

    editor_move_end_line(ed);
    ed->cursor_column++;
}

/* --------------- */
/* --------------- */
/* --------------- */
