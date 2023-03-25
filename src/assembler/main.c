#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi.h"
#include "lexer.h"
#include "version.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--version") == 0) {
            printf(ANSI_BOLD_GREEN "Mallard" ANSI_RESET " "
                   ANSI_BOLD MALLARD_VERSION ANSI_RESET "\n");
            exit(0);
        }
    }

    return 0;
}
