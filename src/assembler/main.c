#include <stdio.h>
#include <string.h>

#include "ansi.h"
#include "compile.h"
#include "fatal_error.h"
#include "file.h"
#include "lexer.h"
#include "version.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    const char* input = NULL;
    const char* output = NULL;
    const char* version = NULL;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--version") == 0) {
            version = argv[i];
            continue;
        }
        else if (strcmp(argv[i], "-o") == 0) {
            ++i;
            if (i >= argc) {
                fatal_error("required 'output' after '-o");
            }
            output = argv[i];
            continue;
        }

        if (!input) {
            input = argv[i];
        }
        else {
            fatal_error("only one input file supported");
        }
    }

    if (version != NULL) {
        if (input != NULL || output != NULL) {
            fatal_error("'--version' should be the only argument");
            return 1;
        }
        printf(ANSI_BOLD_GREEN "Mallard" ANSI_RESET " "
                ANSI_BOLD MALLARD_VERSION ANSI_RESET "\n");
        return 0;
    }
    else {
        if (input == NULL || output == NULL) {
            fatal_error("require both input and output files");
        }
    }

    struct str str = file_open(input);
    compile(&str);
    file_close(&str);

    return 0;
}
