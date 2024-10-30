#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#include "./edit.h"

String string_new(void) {
    String new = {
        .size     = 0,
        .capacity = 5
    };
    new.str = calloc(new.capacity, sizeof(char));
    return new;
}

String string_from(const char *str) {
    String new = {
        .size     = strlen(str),
        .capacity = strlen(str)
    };
    new.str = calloc(new.capacity, sizeof(char));
    strncpy(new.str, str, new.size);
    return new;
}

void string_append_char(String *s, char c) {
    if (s->size + 1 > s->capacity) {
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

    ++s->capacity;
    ++s->size;
    s->str = realloc(s->str, s->capacity * sizeof(char));

    const void *src = s->str + index;
    void *dest = s->str + index + 1;
    size_t n = (s->size - index - 1) * sizeof(char);

    memmove(dest, src, n);

    s->str[index] = c;

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






Editor editor_new(const char *filename) {

    Editor ed = {
        .cursor_line   = 0,
        .cursor_column = 0,
        .mode          = MODE_NORMAL,
    };

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    ed.text = lines_new();

    char buf[BUFSIZ] = { 0 };
    while (fgets(buf, BUFSIZ, f) != NULL) {
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


String* editor_get_current_string(Editor *ed) {
    return &ed->text.lines[ed->cursor_line];
}


void editor_insert(Editor *ed, char c) {
    string_insert_char_before(
        editor_get_current_string(ed),
        ed->cursor_column, c
    );
    ed->cursor_column++;
}



void editor_move_right(Editor *ed) {
    ed->cursor_column++;

    String *s = editor_get_current_string(ed);

    if (ed->cursor_column + 1 > (int) s->size) {
        ed->cursor_column = 0;
        ed->cursor_line++;
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

    String *s = editor_get_current_string(ed);

    // wraparound
    if (ed->cursor_line < 0)
        ed->cursor_line = (int) ed->text.size - 1;


    // move to end of previous line
    // eg:
    // short text <--
    // loooooong text      |
    //                     ^
    s = editor_get_current_string(ed);
    if (ed->cursor_column + 1 > (int) s->size)
        ed->cursor_column = s->size - 1;


}

void editor_move_down(Editor *ed) {
    ed->cursor_line++;

    String *s = editor_get_current_string(ed);

    // wraparound
    if (ed->cursor_line + 1 > (int) ed->text.size)
        ed->cursor_line = 0;


    // move to end of next line
    // eg:
    // loooooong text      |
    // short text <--      ^
    s = editor_get_current_string(ed);
    if (ed->cursor_column + 1 > (int) s->size)
        ed->cursor_column = s->size - 1;


}
