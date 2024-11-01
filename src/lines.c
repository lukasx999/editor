#include <stdlib.h>
#include <assert.h>

#include "./edit.h"

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
// TODO: insert line when pressing ENTER in insert mode
void lines_remove(Lines *l, size_t index) {
    assert(index < l->size);

    // void *dest;
    // const void *src;
    // size_t n;
    // memmove(dest, src, n);

    --l->size;
}
