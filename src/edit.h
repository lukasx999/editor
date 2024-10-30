#pragma once

#include <stddef.h>

typedef struct {
    size_t size;
    size_t capacity;
    char *str;
} String;

typedef struct {
    size_t size;
    size_t capacity;
    char *lines;
} Lines;

typedef struct {
    int cursor_column;
    int cursor_line;
    Lines text;
} Editor;
