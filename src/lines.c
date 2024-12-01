#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "./edit.h"
#include "./util.h"



Lines lines_new(void) {
    Lines new = {
        .capacity = 5,
        .size     = 0,
        .lines    = NULL,
    };

    new.lines = malloc(new.capacity * sizeof(String));

    return new;
}

String* lines_get(Lines *l, size_t index) {

    assert((int) index >= 0);
    assert(index < l->size);

    return &l->lines[index];
}

void lines_append(Lines *l, const String *s) {

    if (l->size == l->capacity) {
        l->capacity *= 2;
        l->lines = realloc(l->lines, l->capacity * sizeof(String));
    }

    *lines_get(l, l->size++) = *s;

}

// TODO: this
// TODO: insert line when pressing ENTER in insert mode
void lines_delete(Lines *l, size_t index) {
    assert(index < l->size);

    void *dest      = l->lines + index;
    const void *src = l->lines + index + 1;
    size_t n        = l->size  - index;

    memmove(dest, src, n * sizeof(String));

    --l->size;
}

void lines_insert_after(Lines *l, size_t index, String *s) {

    assert(index < l->size);

    ++l->capacity;
    ++l->size;
    l->lines = realloc(l->lines, l->capacity * sizeof(String));
    if (l->lines == NULL)
        HANDLE_ERROR("realloc() failed");

    const void *src = l->lines + index + 1;
    void *dest      = l->lines + index + 2;
    size_t n        = l->size  - index - 2;

    memmove(dest, src, n * sizeof(String));

    *lines_get(l, index+1) = *s;

}

void lines_join_next(Lines *l, size_t index) {
    assert(index < l->size);
    string_append_string(lines_get(l, index), lines_get(l, index+1));
    lines_delete(l, index+1);
}
