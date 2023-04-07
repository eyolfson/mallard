#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi.h"
#include "compile.h"
#include "file.h"
#include "lexer.h"
#include "version.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    const char* path = NULL;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--version") == 0) {
            printf(ANSI_BOLD_GREEN "Mallard" ANSI_RESET " "
                   ANSI_BOLD MALLARD_VERSION ANSI_RESET "\n");
            exit(0);
        }
        if (!path) {
            path = argv[i];
        }
        else {
            return 1;
        }
    }

    struct str str = file_open(path);
    compile(&str);
    file_close(&str);

    return 0;
}
