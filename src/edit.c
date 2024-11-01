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

static String* _editor_get_current_string(Editor *ed) {
    return &ed->text.lines[ed->cursor_line];
}

static bool _editor_is_last_line_document(Editor *ed) {
    // checks if the current line (indicated by `cursor_line`) is the last line in the document
    return (size_t) ed->cursor_line + 1 == ed->text.size;
}

static bool _editor_is_first_line_document(Editor *ed) {
    // checks if the current line (indicated by `cursor_line`) is the first line in the document
    return ed->cursor_line == 0;
}

static bool _editor_is_at_end_of_line(Editor *ed) {
    // returns true if the cursor is hovering over the last char in the line
    return (size_t) ed->cursor_column + 1 == _editor_get_current_string(ed)->size;
}

static bool _editor_is_over_end_of_line(Editor *ed) {
    // returns true if the cursor is out of bounds of the current line
    return (size_t) ed->cursor_column + 1 > _editor_get_current_string(ed)->size;
}

static bool _editor_is_at_start_of_line(Editor *ed) {
    // returns true if the cursor is hovering over the first char in the line
    return ed->cursor_column == 0;
}

static bool _editor_is_empty_line(Editor *ed) {
    // returns true if the cursor is hovering over an empty line
    return _editor_get_current_string(ed)->size == 0;
}

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
        .wrap_vertical   = true,
        .wrap_horizontal = true,
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
/* Char Operations */
/* --------------- */

// TODO: this isnt working properly
void editor_delete(Editor *ed) {
    string_delete(_editor_get_current_string(ed), ed->cursor_column);
}

void editor_insert(Editor *ed, char c) {

    String *current = _editor_get_current_string(ed);

    // corner case: appending to the end of a line
    // foo| <-- appending here
    // TODO: this:
    if ((size_t) ed->cursor_column == current->size) {

        if (_editor_is_empty_line(ed)) { // empty line
            string_append_char(current, c);
        }
        else {
            string_insert_char_after(current, ed->cursor_column - 1, c);
        }

    }
    else {
        string_insert_char_before(current, ed->cursor_column, c);
    }

    ed->cursor_column++;
}

/* --------------- */
/* --------------- */
/* --------------- */



/* --------------- */
/* Move Operations */
/* --------------- */



// BUG: not working correctly on empty line
void editor_move_right(Editor *ed) {

    if (!_editor_is_at_end_of_line(ed)) {
        ed->cursor_column++;
        return;
    }

    if (!ed->wrap_horizontal) { // wraparound
        return;
    }

    if (_editor_is_last_line_document(ed)) { // go to the beginning of the document
        editor_move_start_document(ed);
        editor_move_start_line(ed);
    }
    else { // go to the beginning of the next line
        ed->cursor_line++;
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

    if (_editor_is_first_line_document(ed)) { // go to the end of the document
        editor_move_end_document(ed);
        editor_move_end_line(ed);
    }
    else { // go to the end of the previous line
        ed->cursor_line--;
        editor_move_end_line(ed);
    }

}

void editor_move_up(Editor *ed) {

    if (!_editor_is_first_line_document(ed)) {
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
    if (!_editor_is_last_line_document(ed)) {
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
    size_t size = _editor_get_current_string(ed)->size;
    ed->cursor_column = size == 0 ? 0 : size - 1; // check for empty line (size == 0)
}

void editor_move_start_line(Editor *ed) {
    ed->cursor_column = 0;
}

void editor_move_start_line_skip_whitespace(Editor *ed) {

    char *current;
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

/* --------------- */
/* --------------- */
/* --------------- */
