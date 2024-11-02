#pragma once

#include <errno.h>
#include <stdio.h>

#define HANDLE_ERROR(message) { \
    perror((message));          \
    exit(1);                    \
} assert(true)
