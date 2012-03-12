#include <services.h>

#include <stdio.h>
#include <stdarg.h>

void errprintf(int flags, ...){
    char *fmt;
    va_list args;
    va_start(args, flags);
    fmt = va_arg(args, char*);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void log_init(){
    aclog = &errprintf;
}
