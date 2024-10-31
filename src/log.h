#include <stdarg.h>
#include <stdio.h>

#ifdef LOG_IMPL

void log_init(void) {
    // clear out logfile
    FILE *f = fopen(LOGFILE, "w");
    fclose(f);
}

void log_write(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);

    FILE *f = fopen(LOGFILE, "a");
    vfprintf(f, fmt, va);

    va_end(va);
    fclose(f);
}

#else

extern void log_init (void);
extern void log_write(const char *fmt, ...);

#endif
