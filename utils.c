/**
 * @file utils.c
 * @brief Utility functions for executing shell commands.
 */

#include "utils.h"
#include "czram.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Executes a shell command.
 * 
 * @param format The format string for the command.
 * @param ... Arguments for the format string.
 * @return Exit status of the command.
 */
int execute_command(const char *format, ...) {
    char cmd[MAX_CMD_LEN];
    va_list args;
    va_start(args, format);
    vsnprintf(cmd, sizeof(cmd), format, args);
    va_end(args);

    FILE *pipe = popen(cmd, "r");
    if (!pipe) return -1;
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe)) {}
    
    return pclose(pipe);
}

/**
 * @brief Executes a shell command and captures its output.
 * 
 * @param format The format string for the command.
 * @param ... Arguments for the format string.
 * @return Pointer to the command output, or NULL on failure.
 */
char *execute_command_with_output(const char *format, ...) {
    char cmd[MAX_CMD_LEN];
    va_list args;
    va_start(args, format);
    vsnprintf(cmd, sizeof(cmd), format, args);
    va_end(args);

    FILE *pipe = popen(cmd, "r");
    if (!pipe) return NULL;

    char *output = malloc(MAX_OUTPUT_LEN);
    if (!output) {
        pclose(pipe);
        return NULL;
    }
    output[0] = '\0';

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        strncat(output, buffer, MAX_OUTPUT_LEN - strlen(output) - 1);
    }

    pclose(pipe);
    return output;
}
