#include "fatal_error.h"

#include "ansi.h"

#include <stdio.h>
#include <stdlib.h>

void fatal_error(const char* message) {
    dprintf(2, ANSI_BOLD_RED "fatal error:" ANSI_RESET " "
               ANSI_RED "%s" ANSI_RESET "\n", message);
    exit(1);
}
