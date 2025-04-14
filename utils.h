#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>

int execute_command(const char *format, ...);
char *execute_command_with_output(const char *format, ...);

#endif
