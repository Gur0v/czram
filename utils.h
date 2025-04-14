/**
 * @file utils.h
 * @brief Header file for utility functions.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>

/**
 * @brief Executes a shell command.
 * 
 * @param format The format string for the command.
 * @param ... Arguments for the format string.
 * @return Exit status of the command.
 */
int execute_command(const char *format, ...);

/**
 * @brief Executes a shell command and captures its output.
 * 
 * @param format The format string for the command.
 * @param ... Arguments for the format string.
 * @return Pointer to the command output, or NULL on failure.
 */
char *execute_command_with_output(const char *format, ...);

#endif
