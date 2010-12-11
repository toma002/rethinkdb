#include "errors.hpp"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef NDEBUG

#include <execinfo.h>

static bool parse_backtrace_line(char *line, char **filename, char **function, char **offset, char **address) {
    /*
    backtrace() gives us lines in one of the following two forms:
       ./path/to/the/binary(function+offset) [address]
       ./path/to/the/binary [address]
    */

    *filename = line;

    // Check if there is a function present
    if (char *paren1 = strchr(line, '(')) {
        char *paren2 = strchr(line, ')');
        if (!paren2) return false;
        *paren1 = *paren2 = '\0';   // Null-terminate the offset and the filename
        *function = paren1 + 1;
        char *plus = strchr(*function, '+');
        if (!plus) return false;
        *plus = '\0';   // Null-terminate the function name
        *offset = plus + 1;
        line = paren2 + 1;
        if (*line != ' ') return false;
        line += 1;
    } else {
        *function = NULL;
        *offset = NULL;
        char *bracket = strchr(line, '[');
        if (!bracket) return false;
        line = bracket - 1;
        if (*line != ' ') return false;
        *line = '\0';   // Null-terminate the file name
        line += 1;
    }

    // We are now at the opening bracket of the address
    if (*line != '[') return false;
    line += 1;
    *address = line;
    line = strchr(line, ']');
    if (!line || line[1] != '\0') return false;
    *line = '\0';   // Null-terminate the address

    return true;
}

static bool run_addr2line(char *executable, char *address, char *line, int line_size) {
    // Generate and run addr2line command
    char cmd_buf[255] = {0};
    snprintf(cmd_buf, sizeof(cmd_buf), "addr2line -s -e %s %s", executable, address);
    FILE *fline = popen(cmd_buf, "r");
    if (!fline) return false;

    int count = fread(line, sizeof(char), line_size - 1, fline);
    pclose(fline);
    if (count == 0) return false;

    if (line[count-1] == '\n') line[count-1] = '\0';
    else line[count] = '\0';

    if (strcmp(line, "??:0") == 0) return false;

    return true;
}

void print_backtrace(FILE *out, bool use_addr2line) {
    // Get a backtrace
    static const int max_frames = 100;
    void *stack_frames[max_frames];
    int size = backtrace(stack_frames, max_frames);
    char **symbols = backtrace_symbols(stack_frames, size);

    if (symbols) {
        for (int i = 0; i < size; i ++) {
            // Parse each line of the backtrace
            char *line = strdup(symbols[i]);
            char *executable, *function, *offset, *address;

            fprintf(out, "%d: ", i+1);

            if (!parse_backtrace_line(line, &executable, &function, &offset, &address)) {
                fprintf(out, "%s\n", symbols[i]);
            } else {
                if (function) {
                    if (char *demangled = demangle_cpp_name(function)) {
                        fprintf(out, "%s", demangled);
                        free(demangled);
                    } else {
                        fprintf(out, "%s+%s", function, offset);
                    }
                } else {
                    fprintf(out, "?");
                }

                fprintf(out, " at ");

                char line[255] = {0};
                if (use_addr2line && run_addr2line(executable, address, line, sizeof(line))) {
                    fprintf(out, "%s", line);
                } else {
                    fprintf(out, "%s (%s)", address, executable);
                }

                fprintf(out, "\n");
            }

            free(line);
        }

        free(symbols);
    } else {
        fprintf(out, "(too little memory for backtrace)\n");
    }
}

#endif

void report_user_error(const char *msg, ...) {
    flockfile(stderr);

    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\nError: ");
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);

    funlockfile(stderr);
}

void report_fatal_error(const char *file, int line, const char *msg, ...) {
    flockfile(stderr);

    va_list args;
    va_start(args, msg);
    fprintf(stderr, "\nError in %s at line %d:\n", file, line);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);

#ifndef NDEBUG
    fprintf(stderr, "\nBacktrace:\n");
    print_backtrace();
#endif

    fprintf(stderr, "\nExiting.\n\n");

    funlockfile(stderr);
}

#ifndef NDEBUG

#include <cxxabi.h>
char *demangle_cpp_name(const char *mangled_name) {
    int res;
    char *name = abi::__cxa_demangle(mangled_name, NULL, NULL, &res);
    if (res == 0) {
        return name;
    } else {
        return NULL;
    }
}

#endif
