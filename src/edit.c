#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#include "./edit.h"

String string_new(void) {
    String new = {
        .size     = 0,
        .capacity = 5,
    };
    new.str = calloc(new.capacity, sizeof(char));
    return new;
}

String string_from(const char *str) {
    String new = {
        .size     = strlen(str),
        .capacity = strlen(str) + 1, // +1 for terminating nullbyte
    };
    new.str = calloc(new.capacity, sizeof(char));
    strncpy(new.str, str, new.size);
    return new;
}

void string_append_char(String *s, char c) {
    if (s->size + 2 > s->capacity) { // +1 for keeping nullbyte at the end
        s->capacity *= 2;
        s->str = realloc(s->str, s->capacity * sizeof(char));
    }
    s->str[s->size++] = c;
}

void string_append_string(String *s, const String *other) {
    for (size_t i = 0; i < strlen(other->str); ++i)
        string_append_char(s, other->str[i]);
}


void string_insert_char_before(String *s, size_t index, char c) {

    assert(index < s->size);

    ++s->capacity;
    ++s->size;
    s->str = realloc(s->str, s->capacity * sizeof(char));

    const void *src = s->str + index;
    void *dest = s->str + index + 1;
    size_t n = (s->size - index - 1) * sizeof(char);

    memmove(dest, src, n);

    s->str[index] = c;

}

void string_insert_char_after(String *s, size_t index, char c) {

    assert(index < s->size);

    ++s->capacity;
    ++s->size;
    s->str = realloc(s->str, s->capacity * sizeof(char));

    const void *src = s->str + index + 1;
    void *dest = s->str + index + 2;
    size_t n = (s->size - index - 1) * sizeof(char);

    memmove(dest, src, n);

    s->str[index+1] = c;

}





Lines lines_new(void) {
    Lines new = {
        .capacity = 5,
        .size     = 0,
    };

    new.lines = malloc(new.capacity * sizeof(String));

    return new;
}

void lines_append(Lines *l, const String *s) {

    if (l->size + 1 > l->capacity) {
        l->capacity *= 2;
        l->lines = realloc(l->lines, l->capacity * sizeof(String));
    }

    l->lines[l->size++] = *s;
}

// TODO: this
void lines_remove(Lines *l, size_t index) {
    assert(index < l->size);

    // void *dest;
    // const void *src;
    // size_t n;
    // memmove(dest, src, n);

    --l->size;
}





Editor editor_new(const char *filename) {

    Editor ed = {
        .cursor_line     = 0,
        .cursor_column   = 0,
        .mode            = MODE_NORMAL,
        .wrap_vertical   = true,
        .wrap_horizontal = true,
        .filename        = filename,
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




String* editor_get_current_string(Editor *ed) {
    return &ed->text.lines[ed->cursor_line];
}


void editor_insert(Editor *ed, char c) {

    // foo| <-- appending here
    if ((size_t) ed->cursor_column == editor_get_current_string(ed)->size) {
        string_insert_char_after(
            editor_get_current_string(ed),
            ed->cursor_column - 1, c
        );
    }
    else {
        string_insert_char_before(
            editor_get_current_string(ed),
            ed->cursor_column, c
        );
    }

    ed->cursor_column++;
}



// Helper functions

// TODO: maybe check ahead
// _editor_is_last_char_right()
static bool _editor_is_outofbounds_right(Editor *ed) {
    // checks if the cursor has overstepped the string to the right
    // it only checks if the cursor is CURRENTLY out of bounds, not if its about to be
    String *s = editor_get_current_string(ed);
    return ed->cursor_column + 1 > (int) s->size;
}

static bool _editor_is_last_line(Editor *ed) {
    // checks if the current line (indicated by `cursor_line`) is the last line in the editor

    bool a = &ed->text.lines[ed->cursor_line] == &ed->text.lines[ed->text.size-1];
    bool b = ed->cursor_line + 1 == (int) ed->text.size;
    assert(a == b); // sanity check
    return a;

}


void editor_move_right(Editor *ed) {
    ed->cursor_column++;

    // wraparound
    if (_editor_is_outofbounds_right(ed)) {

        if (ed->wrap_horizontal) {

            if (_editor_is_last_line(ed)) { // go to the beginning of the document
                ed->cursor_line = ed->cursor_column = 0;
            }
            else { // go to the beginning of the next line
                ed->cursor_column = 0;
                ed->cursor_line++;
            }

        }
        else { // stay at the current position
            ed->cursor_column--;
        }

    }

}

void editor_move_left(Editor *ed) {
    ed->cursor_column--;

    String *s = editor_get_current_string(ed);

    if (ed->cursor_column < 0) {
        ed->cursor_line--;
        s = editor_get_current_string(ed);
        ed->cursor_column = s->size - 1;
    }
}

void editor_move_up(Editor *ed) {
    ed->cursor_line--;

    // wraparound
    if (ed->cursor_line < 0)
        ed->cursor_line = ed->wrap_vertical
            ? (int) ed->text.size - 1
            : ed->cursor_line + 1;


    // // move to end of previous line
    // // eg:
    // // short text <--
    // // loooooong text      |
    // //                     ^
    // s = editor_get_current_string(ed);
    // if (ed->cursor_column + 1 > (int) s->size)
    //     ed->cursor_column = s->size - 2;


}

void editor_move_down(Editor *ed) {
    ed->cursor_line++;

    // wraparound
    if (ed->cursor_line + 1 > (int) ed->text.size)
        ed->cursor_line = ed->wrap_vertical
            ? 0
            : ed->cursor_line - 1;


    // // move to end of next line
    // // eg:
    // // loooooong text      |
    // // short text <--      ^
    // s = editor_get_current_string(ed);
    // if (ed->cursor_column + 1 > (int) s->size)
    //     ed->cursor_column = s->size - 2;


}
