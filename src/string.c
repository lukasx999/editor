#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

void string_delete(String *s, size_t index) {

    const void *src = s->str + index + 1;
    void *dest      = s->str + index;
    size_t n        = (s->size - index - 1) * sizeof(char);

    memmove(dest, src, n);
    --s->size;
    s->str[s->size] = '\0';

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
    void *dest      = s->str + index + 1;
    size_t n        = (s->size - index - 1) * sizeof(char);

    memmove(dest, src, n);

    s->str[index] = c;

}

void string_insert_char_after(String *s, size_t index, char c) {

    assert(index < s->size);

    ++s->capacity;
    ++s->size;
    s->str = realloc(s->str, s->capacity * sizeof(char));

    const void *src = s->str + index + 1;
    void *dest      = s->str + index + 2;
    size_t n        = (s->size - index - 1) * sizeof(char);

    memmove(dest, src, n);

    s->str[index++] = c;
}
